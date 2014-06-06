/////////////////////////////////////////////////////////////////////////////
// Name:        TestClient.cpp
// Author:      XX
// Created:     XX/XX/XX
// Copyright:   XX
/////////////////////////////////////////////////////////////////////////////

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
    #pragma implementation "TestClient.cpp"
#endif

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#include "TestClient.h"
#include "OSThread.h"
#include "MainApp.h"
#include "Uri.h"
#include "Log.h"

using namespace GSP;

static INT  OnClientEvent(CIClientSection *pcsClient,CIClientChannel *pChannel, 
						  EnumGSPClientEventType eEvtType, 
						  void *pEventData,  INT iEvtDataLen, void *pUserData  )
{
CTestClientDialog *p = (CTestClientDialog*)pUserData;
   GS_ASSERT_RET_VAL(p, 0);
    return p->OnSectionEvent(pcsClient, pChannel, eEvtType, pEventData, iEvtDataLen);
}


class CThreadPoolCallback : public CGSPObject
{
public :
	CTestClientDialog  *m_pDialog;
	CThreadPoolCallback(CTestClientDialog* pDialog )		
	{
		m_pDialog = pDialog;
	}
	~CThreadPoolCallback(void)
	{

	}
	void OnTaskEvent( CGSThreadPool *pPool,void *pTaskData)
	{
		m_pDialog->OnOCTaskEvent(pTaskData);
	}
};

// static void OnThreadPoolCallback(CGSThreadPool *pPool,void *pTaskData, void *pDebugInfo )
// {
// 	CTestClientDialog *p = (CTestClientDialog*)pPool->GetUserData();
// 	GS_ASSERT_RET(p);
// 	 p->OnOCTaskEvent( pTaskData );
// }

// WDR: class implementations

//----------------------------------------------------------------------------
// CTestClientDialog
//----------------------------------------------------------------------------

// WDR: event table for CTestClientDialog

BEGIN_EVENT_TABLE(CTestClientDialog,wxDialog)

    EVT_BUTTON( ID_BUTTON_SELCFGFILE, CTestClientDialog::OnBtnSelCfgFileClick )
    EVT_TOGGLEBUTTON( ID_TOGGLEBUTTON_START_SECTION, CTestClientDialog::OnTglbtnStartSectionClick )
    EVT_BUTTON( ID_BUTTON_ADD_SINGLE, CTestClientDialog::OnBtnAddSingleClick )
    EVT_BUTTON( ID_BUTTON_ADD_LOTS, CTestClientDialog::OnBtnAddLotsClick )
    EVT_BUTTON( ID_TOGGLEBUTTON_AD_RAND, CTestClientDialog::OnBtnADRandClick )
    EVT_TOGGLEBUTTON(ID_TGLBTN_PLAY, CTestClientDialog::OnTglbtnPlayClick )
    EVT_TOGGLEBUTTON( ID_TGLBTN_PAUSE, CTestClientDialog::OnTglbtnPauseClick )
    EVT_BUTTON( ID_BTN_FAST, CTestClientDialog::OnBtnFastClick )
    EVT_BUTTON( ID_BTN_SLOW, CTestClientDialog::OnBtnSlowClickEvt )
    EVT_BUTTON( ID_BTN_STEP, CTestClientDialog::OnBtnStepClickEvt )
    EVT_SLIDER( ID_SLIDER_SETPOINT, CTestClientDialog::OnSliderSetPointSlidEvt )
    EVT_TIMER(REFRESH_INFO_TIMER, CTestClientDialog::OnRefreshInfoTimer)
    EVT_BUTTON( ID_BTN_CLOSE, CTestClientDialog::OnBtnCloseClick )
    EVT_BUTTON( ID_BTN_DEL, CTestClientDialog::OnBtnDelClick )
    EVT_TOGGLEBUTTON(ID_TBLBTN_OCRAND, CTestClientDialog::OnTglBtnOCRandClick )
    EVT_TIMER(REFRESH_OCRAND_TIMER, CTestClientDialog::OnOCRandTimer)
    EVT_BUTTON( ID_BTN_OPEN, CTestClientDialog::OnBtnOpenClick )
	EVT_CLOSE( CTestClientDialog::OnDialogClose)
END_EVENT_TABLE()

CTestClientDialog::CTestClientDialog( wxWindow *parent, wxWindowID id, const wxString &title,
    const wxPoint &position, const wxSize& size, long style ) :
    wxDialog( parent, id, title, position, size, style )
    ,m_csRefInfoTimer(this, REFRESH_INFO_TIMER)
    ,m_csMutex()
    ,m_csChnMap()
    ,m_csOCTimer(this, REFRESH_OCRAND_TIMER)
{
    // WDR: dialog function CreateTestClientDialog for CTestClientDialog
	
    CreateTestClientDialog( this, TRUE ); 
     m_pSection = NULL;
     m_bPlay = false;
     m_iRandCnts = 0;
}

CTestClientDialog::~CTestClientDialog()
{
	m_csOCThread.Uninit();
	if( m_pSection )
	{

		m_pSection->Release();
		m_pSection = NULL;
	
	}
	delete m_pTaskCallback;
}

// WDR: handler implementations for CTestClientDialog


void CTestClientDialog::OnDialogClose( wxCloseEvent &event)
{
	delete this;
}

void CTestClientDialog::OnTglBtnOCRandClick( wxCommandEvent &event )
{
     if( GetTblbtnOcrand()->GetValue() )
     {
		  wxGrid* p = GetGridChannel();
		 m_csOCThread.SetMaxWaitTask(p->GetRows() );
         m_csOCTimer.Start(GetSpinctrlAdInterval()->GetValue()+1);
     }
     else
     {
		 m_csOCThread.Clear();
         m_csOCTimer.Stop();
     }
}

void CTestClientDialog::OnBtnDelClick( wxCommandEvent &event )
{
    
}

void CTestClientDialog::OnBtnOpenClick( wxCommandEvent &event )
{
    //关闭
    wxGrid* p = GetGridChannel();
    wxArrayInt vRows;
    CMainApp::GetGridSelectRows(p, vRows);

   CGSAutoWriterMutex wlocker( &m_csMutex); 
    CIClientChannel *pChn;
    int iValue;
    wxString strValue;

    wxString strKey, strServerHost, strPro, strPort;
    int iPort, iBeginIndex, iCnts;

    strServerHost =  GetTextctrlIp()->GetValue();
    iBeginIndex = GetSpinctrlKeyidxStart()->GetValue();
    iCnts = GetSpinctrlKeyNumber()->GetValue();
    strPro = GetChoicePro()->GetStringSelection();
    strPort = GetTextctrlPort()->GetValue();


    iPort = atoi( strPort.c_str());
     int iIndex = iBeginIndex;

    for( UINT i =0; i<vRows.size(); i++ )
    {
        strValue = p->GetCellValue(vRows[i], 6);
        iValue = atoi( strValue.c_str() );
        if( m_csChnMap.find( iValue) == m_csChnMap.end() )
        {
            continue;
        }
        pChn = (CIClientChannel *)iValue;

		if( (int) pChn->GetStatus() > (int) CIClientChannel::ST_READY )
        {
            continue;
        }
        iIndex = iBeginIndex;
        
        strKey = wxString::Format( "test_%d", iIndex);

        CUri csURI;


        csURI.SetHost((const char *)strServerHost.c_str());  //46
        csURI.SetKey((const char *)strKey.c_str());
        csURI.SetPortArgs(iPort);
        csURI.SetScheme( (const char *)strPro.c_str() );
        csURI.AddAttr("pro", "tcp");
        csURI.AddAttr("S", "35d3ed223");
        CGSString strUri = csURI.GetURI();

        if( pChn->SetURI( strUri.c_str() ) )
        {   

            wxMessageBox( strUri.c_str(), wxT("SetURI error"));
            continue;        
        }

        if(  pChn->Open( GSP_TRAN_RTPLAY, -500 )  )
        {
            wxMessageBox( strUri.c_str(), wxT("Open error"));                
        }        
    }

}

void CTestClientDialog::OnBtnCloseClick( wxCommandEvent &event )
{
    //关闭
    wxGrid* p = GetGridChannel();
    wxArrayInt vRows;
    CMainApp::GetGridSelectRows(p, vRows);

    CGSAutoReaderMutex rlocker( &m_csMutex); 
    CIClientChannel *pChn;
    int iValue;
    wxString strValue;

    for( UINT i =0; i<vRows.size(); i++ )
    {
        strValue = p->GetCellValue(vRows[i], 6);
        iValue = atoi( strValue.c_str() );
        if( m_csChnMap.find( iValue) == m_csChnMap.end() )
        {
            continue;
        }
        pChn = (CIClientChannel *)iValue;
        pChn->Close();
    }    
}



void CTestClientDialog::InitDialog()
{
    wxString strFilename = GSGetApplicationPath().c_str();
    if ( !strFilename.empty() )
    {
        // work with the file
        strFilename += "GSPClientConf.ini";
        GetTextctrlCfgfile()->SetValue(strFilename);
    }
    wxGrid* p = GetGridChannel();
    p->SetLabelFont( g_pMainApp->m_pMainWindow->GetFont());
    p->SetRowLabelSize(30);
    p->SetLabelTextColour(wxColour(0,0,255));
    p->AppendCols(7);
    p->SetColLabelValue(0,wxT("Network"));
    p->SetColSize(0,150);
    p->SetColLabelValue(1,wxT("URI"));
    p->SetColSize(1,300);
    p->SetColLabelValue(2,wxT("Rcvs(Bytes)"));
    p->SetColSize(2,70);
    p->SetColLabelValue(3,wxT("Sends(Bytes)"));
    p->SetColSize(3,70);
    p->SetColLabelValue(4,wxT("Losts"));
    p->SetColLabelValue(5,wxT("Status"));
    p->SetColLabelValue(6,wxT("ChnHdl"));


    p->EnableEditing(false);

    p = GetGridProModule();
    p->SetLabelFont( g_pMainApp->m_pMainWindow->GetFont());
    p->SetRowLabelSize(30);
    p->SetLabelTextColour(wxColour(0,0,255));
    p->AppendCols(3);

    p->SetColLabelValue(0,wxT("Pro"));
    p->SetColSize(0,40); 
    p->SetColLabelValue(1,wxT("Network"));
    p->SetColSize(1,140); 
    p->SetColLabelValue(2,wxT("Status"));
    p->SetColSize(2,70); 
    p->EnableEditing(false);

    EnableOtherFunction(false);

    GetChoicePro()->SetSelection(0);
    GetChoiceTransmodule()->SetSelection(0);

	m_csOCThread.SetUserData(this);
	//m_csOCThread.Init(OnThreadPoolCallback, 3 );
	m_pTaskCallback = new CThreadPoolCallback(this);
	m_csOCThread.Init(m_pTaskCallback, 
						(FuncPtrObjThreadPoolEvent)&CThreadPoolCallback::OnTaskEvent, 3, 
						TRUE);



}

// void CTestClientDialog::UpdateModuleInfo( CProModuleInfoList &csList)
// {
//     //更新模块信息
//     int iRows;
//     wxGrid* p = GetGridProModule();
//     iRows = p->GetRows();
//     if(iRows>0 )
//     {
//         p->DeleteRows(0, iRows );
//     }
//     iRows = 0;
//     CProModuleInfoListIterator csIt;
//     wxString strValue;
//     for( csIt = csList.begin(); csIt!=csList.end(); csIt++, iRows++  )
//     {
//         p->AppendRows();
//         strValue = GetProtocolName((*csIt).ePro);
//         p->SetCellValue(iRows,0,strValue);
// 
//         strValue = (*csIt).strNetInfo.c_str();
//         p->SetCellValue(iRows,1,strValue);
// 
//         strValue = GetModuleStatusName((*csIt).eStatus);
//         p->SetCellValue(iRows,2,strValue);
//     }
// 
// 
// 
// }


void CTestClientDialog::OnSliderSetPointSlidEvt( wxCommandEvent &event )
{
    
}

void CTestClientDialog::OnBtnStepClickEvt( wxCommandEvent &event )
{
    
}

void CTestClientDialog::OnBtnSlowClickEvt( wxCommandEvent &event )
{
    
}

void CTestClientDialog::OnBtnFastClick( wxCommandEvent &event )
{
    
}

void CTestClientDialog::OnTglbtnPauseClick( wxCommandEvent &event )
{
    
}

void CTestClientDialog::OnTglbtnPlayClick( wxCommandEvent &event )
{
    //播放控制 play
    wxGrid* p = GetGridChannel();
    wxArrayInt vRows;
    CMainApp::GetGridSelectRows(p, vRows);

    CGSAutoReaderMutex rlocker( &m_csMutex); 
    CIClientChannel *pChn;
    int iKey;
    wxString strKey;
    StruGSPCmdCtrl stCtrl;
    bzero( &stCtrl, sizeof(stCtrl));
    //stCtrl.iSubChn = 0;
    m_bPlay = GetTglbtnPlay()->GetValue();
    if(  m_bPlay )
    {
       //
        stCtrl.iCtrlID = GSP_CTRL_PLAY;
    }
    else
    {
        stCtrl.iCtrlID = GSP_CTRL_STOP;
    }

    for( UINT i =0; i<vRows.size(); i++ )
    {
        strKey = p->GetCellValue(vRows[i], 6);
        iKey = atoi( strKey.c_str() );
        if( m_csChnMap.find( iKey) == m_csChnMap.end() )
        {
            continue;
        }
        pChn = (CIClientChannel*)iKey;
        if( !pChn->Ctrl(stCtrl ), 0 )
		{
			wxMessageBox("控制失败","错误");
		}
    }

}

void CTestClientDialog::OnBtnADRandClick( wxCommandEvent &event )
{
    
}

void CTestClientDialog::OnBtnAddLotsClick( wxCommandEvent &event )
{
    //增加多个
    wxString strKey, strServerHost, strPro, strPort;
    int iPort, iBeginIndex, iCnts;

    strServerHost =  GetTextctrlIp()->GetValue();
    iBeginIndex = GetSpinctrlKeyidxStart()->GetValue();
    iCnts = GetSpinctrlKeyNumber()->GetValue();
    strPro = GetChoicePro()->GetStringSelection();
    strPort = GetTextctrlPort()->GetValue();

    
    iPort = atoi( strPort.c_str());
    int i = iBeginIndex;
    iBeginIndex += iCnts;
    for(  ; i<iBeginIndex; i++ )
    {
        strKey = wxString::Format( "test_%d", i);
        if( !AddChannel(strKey.c_str(), strServerHost.c_str(), iPort, strPro.c_str()) )
        {
            wxMessageBox(wxT("add datasource failure"), wxT("error") );
            break;

        }
    }
    GetSpinctrlKeyidxStart()->SetValue(i+1);
    UpdateChannelInfo();
}

void CTestClientDialog::OnBtnAddSingleClick( wxCommandEvent &event )
{
    //增加单个通道
wxString strKey, strServerHost, strPro, strPort;
int iPort, iBeginIndex, iCnts;

    strServerHost =  GetTextctrlIp()->GetValue();
    iBeginIndex = GetSpinctrlKeyidxStart()->GetValue();
    iCnts = GetSpinctrlKeyNumber()->GetValue();
    strPro = GetChoicePro()->GetStringSelection();
    strPort = GetTextctrlPort()->GetValue();

    strKey = wxString::Format( "test_%d", iBeginIndex);
    iPort = atoi( strPort.c_str());

    if( AddChannel(strKey.c_str(), strServerHost.c_str(), iPort, strPro.c_str()) )
    {
        GetSpinctrlKeyidxStart()->SetValue(iBeginIndex+1);
        UpdateChannelInfo();
    }

    

}

BOOL CTestClientDialog::AddChannel( const char *czKey, const char *czServerHost, int iPort, const char *czPro )
{
    //增加通道
    CIClientChannel *pCliChn;

    pCliChn = m_pSection->CreateChannel();
    if( !pCliChn )
    {
       wxMessageBox( wxT("Create Channel Failure"), wxT("Error"));
       return FALSE;

    }
    else
    {
       CGSAutoWriterMutex wlocker( &m_csMutex); 
        m_csChnMap.insert(std::make_pair((int)pCliChn, pCliChn));

        CUri csURI;


        csURI.SetHost(czServerHost);  //46
        csURI.SetKey(czKey);
        csURI.SetPortArgs(iPort);
        csURI.SetScheme( czPro );
        csURI.AddAttr("pro", "tcp");
        csURI.AddAttr("S", "35d3ed223");
        CGSPString strUri = csURI.GetURI();
        if( !pCliChn->SetURI( strUri.c_str() ) )
        {    
           //  pCliChn->Release();
          //  wxMessageBox( strUri.c_str(), wxT("SetURI 错误"));
            return FALSE;           
        }

        if( !pCliChn->Open(GSP_TRAN_RTPLAY,  -500  )  )
        {
          //  wxMessageBox( strUri.c_str(), wxT("Open 错误"));
          //  pCliChn->Release();
            return FALSE;    
        }        
    }
    return TRUE;
}


void CTestClientDialog::OnTglbtnStartSectionClick( wxCommandEvent &event )
{
    //开启停止服务
   /* CProModuleInfoList csList;*/
    if( m_pSection )
    {
		m_csMutex.LockWrite();
		m_csChnMap.clear();
		m_csMutex.UnlockWrite();

        m_pSection->Release();
        m_pSection = NULL;
        EnableOtherFunction(false);
        GetTogglebuttonStartSection()->SetLabel(wxT("Start Section"));

    }
    else
    {
        m_pSection =  CreateGSPClientSectionInterface();
        m_pSection->SetEventListener( (GSPClientEventFunctionPtr) OnClientEvent, this);
        wxString strCfgFilename = GetTextctrlCfgfile()->GetValue();

        if( m_pSection->Init(strCfgFilename.c_str() ) )
        {
            GetTogglebuttonStartSection()->SetLabel(wxT("Stop Section"));
          //  m_pSection->GetCliModules(csList);
            EnableOtherFunction(true);

        }
        else
        {
            m_pSection->Release();
            m_pSection = NULL;
            wxMessageBox( "Init GSP Client Section fail.", "Error");
        }


    }
   
}

void CTestClientDialog::EnableOtherFunction( bool bEnable )
{
    if( bEnable )
    {
        m_csRefInfoTimer.Start(GetSpinctrlChnRefinterval()->GetValue()*1000+1);
    //    m_csRefSrcTimer.Start(GetSpinctrlSrcRefinterval()->GetValue()*1000+1);
    }
    else
    {

        m_csRefInfoTimer.Stop();
     //   m_csRefCliTimer.Stop();
    }

    GetChoiceTransmodule()->Enable(bEnable);
    GetSliderSetpoint()->Enable(bEnable);
    GetTglbtnPause()->Enable(bEnable);  
    GetTglbtnPlay()->Enable(bEnable);

    GetSpinctrlAdInterval()->Enable(bEnable);
    GetTogglebuttonAdRand()->Enable(bEnable);
    GetSpinctrlKeyNumber()->Enable(bEnable);  
    GetSpinctrlKeyidxStart()->Enable(bEnable);

    GetTextctrlPort()->Enable(bEnable);
    GetTextctrlIp()->Enable(bEnable);
    GetChoicePro()->Enable(bEnable);  
    GetCheckboxChnRefresh()->Enable(bEnable);

    GetSpinctrlChnRefinterval()->Enable(bEnable);
    GetGridChannel()->Enable(bEnable);
    GetGridProModule()->Enable(bEnable);  

    GetBtnStep()->Enable(bEnable);  
    GetBtnSlow()->Enable(bEnable);

    GetBtnFast()->Enable(bEnable);
    GetButtonAddLots()->Enable(bEnable);
    GetButtonAddSingle()->Enable(bEnable);

    GetBtnDel()->Enable(bEnable);
    GetBtnClose()->Enable(bEnable);

    UpdateChannelInfo();
//    UpdateSourceInfo();
//    UpdateClientInfo(false);

} 


void CTestClientDialog::OnBtnSelCfgFileClick( wxCommandEvent &event )
{
    //选择配置文件
    wxString strDefPath = GSGetApplicationPath();
    wxString strFilename = wxFileSelector("Choose server config file", strDefPath);
    if ( !strFilename.empty() )
    {
        // work with the file
        GetTextctrlCfgfile()->SetValue(strFilename);
    }
}


INT CTestClientDialog::OnSectionEvent( CIClientSection *pcsClient,
									  CIClientChannel *pChannel, 
									  EnumGSPClientEventType eEvtType, 
									  void *pEventData,  
									  INT iEvtDataLen)
{
    switch( eEvtType )
    {
    case GSP_EVT_CLI_FRAME :
        {
			int i = (int) (0+80.0*(rand()/(RAND_MAX+1.0)));
			if( i )
			{
				MSLEEP(i);
			}
        }
    break;
    case GSP_EVT_CLI_STATUS :
        {
        }
        break;
    case GSP_EVT_CLI_TRYFAIL :
        {
        }
        break;
    case GSP_EVT_CLI_ASSERT_CLOSE :
        {
          //  pChannel->Close();
        }
        break;
    case GSP_EVT_CLI_CTRL_OK  :
        {
        }
        break;
    case GSP_EVT_CLI_CTRL_FAIL  :
        {
        }
        break;


    case GSP_EVT_CLI_DESTROY  :
        {
            //
//             int iKey = (int)pChannel;
//             CGSAutoWriterMutex rlocker( &m_csMutex); 
//             m_csChnMap.erase( iKey);
        }
        break;
    case GSP_EVT_CLI_COMPLETE :
        {

        }
        break;
    case GSP_EVT_CLI_RETREQUEST  :
        {
            BOOL bAB  = (BOOL )pEventData;
            if( bAB )
            {
                if( m_bPlay )
                {
                    StruGSPCmdCtrl stCtrl;
                    bzero( &stCtrl, sizeof(stCtrl));                        
                    stCtrl.iCtrlID = GSP_CTRL_PLAY;
                    pChannel->Ctrl(stCtrl, 0);
                }
            }
			
        }
        break;

    case GSP_EVT_CLI_LOSTDATA :
        {
        }
        break;
    default :
        {

        }
    break;
    }
   return 0;
}

void   CTestClientDialog::UpdateChannelInfo(void)
{
    CGSAutoReaderMutex rlocker( &m_csMutex); 


    int iRows;
    wxGrid* p = GetGridChannel();
    iRows = p->GetRows();
    if(iRows>0 )
    {
        p->DeleteRows(0, iRows );
    }
    if( !m_csChnMap.size() )
    {
        return;
    }


    std::map<int, CIClientChannel*>::iterator csIt;
    CIClientChannel *pCli;
    wxString strValue;
	const CIClientChannel::StruChannelInfo *pInfo;

    p->Freeze();
    for( iRows = 0, csIt = m_csChnMap.begin(); csIt!= m_csChnMap.end(); csIt++,iRows++ )
    {
        p->AppendRows();
        pCli = (*csIt).second;

        pInfo = pCli->GetInfo();


		strValue = "0.0.0.0:0";
        p->SetCellValue(iRows,0,strValue); 

        strValue = pCli->GetURI();
        p->SetCellValue(iRows,1,strValue);

        strValue = wxString::Format("%lld", pInfo->iRcvFromNet );
        p->SetCellValue(iRows,2,strValue);

        strValue = wxString::Format("%lld",  pInfo->iSendToNet );
        p->SetCellValue(iRows,3,strValue);

        strValue = wxString::Format("%lld", pInfo->iLostFrames+pInfo->iLostNetFrames );
        p->SetCellValue(iRows,4,strValue);

        strValue =  wxString::Format("%d", (int) pCli->GetStatus() );
        p->SetCellValue(iRows,5,strValue);

        strValue = wxString::Format("%d", (int) pCli );
        p->SetCellValue(iRows,6,strValue);


    }
    p->Thaw();
}


void CTestClientDialog::OnRefreshInfoTimer( wxTimerEvent& event)
{
int bUpdate = FALSE;
     if( GetCheckboxChnRefresh()->IsChecked() )
     {
         
        CGSAutoReaderMutex rlocker( &m_csMutex); 

         int iRows;
         wxGrid* p = GetGridChannel();
         iRows = p->GetRows();
         if(iRows<1 || !m_csChnMap.size() )
         {             
             return;
         }


         CIClientChannel *pCli;
         wxString strValue;
       	const CIClientChannel::StruChannelInfo *pInfo;

         int iKey;
         p->Freeze();
         for( iRows = iRows-1; iRows>-1; iRows--)
         {
             strValue = p->GetCellValue(iRows, 6);
             iKey = atoi( strValue.c_str() );
             if( m_csChnMap.find( iKey) == m_csChnMap.end() )
             {
                bUpdate = TRUE;
                break;
             }
             pCli = (CIClientChannel*)iKey;
			 pInfo = pCli->GetInfo();


			 strValue = "0.0.0.0:0";
             p->SetCellValue(iRows,0,strValue); 

             strValue = pCli->GetURI();
             p->SetCellValue(iRows,1,strValue);

             strValue = wxString::Format("%lld", pInfo->iRcvFromNet );
             p->SetCellValue(iRows,2,strValue);

             strValue = wxString::Format("%lld", pInfo->iSendToNet );
             p->SetCellValue(iRows,3,strValue);

             strValue = wxString::Format("%lld", pInfo->iLostFrames+pInfo->iLostNetFrames);
             p->SetCellValue(iRows,4,strValue);

			 strValue = pCli->StatusString(pCli->GetStatus() );
             p->SetCellValue(iRows,5,strValue);

        }
         p->Thaw();

     }
     if(bUpdate)
     {
         UpdateChannelInfo();
     }
}

void CTestClientDialog::OnOCTaskEvent(void *pParam )
{
	INT bClose =   rand(); // (int) (1.0 * (rand() / (RAND_MAX + 1.0)));

	bClose = bClose<(RAND_MAX/2);
	CIClientChannel *pChn;

	pChn = (CIClientChannel*)pParam;

	CGSAutoReaderMutex rlocker( &m_csMutex); 
	if( m_csChnMap.find((int)pChn) == m_csChnMap.end() )
	{
		return;
	}


	 wxString strKey, strServerHost, strPro, strPort;
	 int iPort, iBeginIndex, iCnts;

	 strServerHost =  GetTextctrlIp()->GetValue();
	 iBeginIndex = GetSpinctrlKeyidxStart()->GetValue();
	 iCnts = GetSpinctrlKeyNumber()->GetValue();
	 strPro = GetChoicePro()->GetStringSelection();
	 strPort = GetTextctrlPort()->GetValue();


	 iPort = atoi( strPort.c_str());

// 	if( bClose )
// 	{
// 		pChn->Close();
// 	}
// 	else
// 	{
		INT iCur = DoGetTickCount();
		pChn->Close();
		INT iCur2 = DoGetTickCount();
		 MY_LOG_FATAL(g_pLog, "Close: *%d*\n", iCur2-iCur);
		int iIndex = iBeginIndex+(int)( (rand()/(RAND_MAX+1.0))*(iBeginIndex+iCnts) );
		strKey = wxString::Format( "test_%d", iIndex);

		CUri csURI;


		csURI.SetHost((const char *)strServerHost.c_str());  //46
		csURI.SetKey((const char *)strKey.c_str());
		csURI.SetPortArgs(iPort);
		csURI.SetScheme( (const char *)strPro.c_str() );
		csURI.AddAttr("pro", "tcp");
		csURI.AddAttr("S", "35d3ed223");
		CGSString strUri = (const char *)csURI.GetURI();

		if( !pChn->SetURI( (const char *) strUri.c_str() ) )
		{   

			/* wxMessageBox( strUri.c_str(), wxT("SetURI 错误"));*/
			return;  
		}

		if( !pChn->Open(GSP_TRAN_RTPLAY,  -500  )  )
		{
			// wxMessageBox( strUri.c_str(), wxT("Open 错误"));                
		} 

/*	}*/

}

void CTestClientDialog::OnOCRandTimer(wxTimerEvent &event )
{
    m_iRandCnts ++;
    

    wxGrid* p = GetGridChannel();
    wxArrayInt vRows;
    CMainApp::GetGridSelectRows(p, vRows);

    CGSAutoReaderMutex rlocker(&m_csMutex);

    int iVal;
    wxString strVal; 
    INT iRows = p->GetRows();

//     wxString strKey, strServerHost, strPro, strPort;
//     int iPort, iBeginIndex, iCnts;
// 
//     strServerHost =  GetTextctrlIp()->GetValue();
//     iBeginIndex = GetSpinctrlKeyidxStart()->GetValue();
//     iCnts = GetSpinctrlKeyNumber()->GetValue();
//     strPro = GetChoicePro()->GetStringSelection();
//     strPort = GetTextctrlPort()->GetValue();    
//     iPort = atoi( strPort.c_str());


	if( m_csOCThread.GetWaitTask() > (UINT) iRows )
	{
		return;
	}

    for( INT i =0; i<iRows; i++ )
    {
        strVal = p->GetCellValue(i, 6);
        iVal = atoi( strVal.c_str() );
        if( m_csChnMap.find( iVal) == m_csChnMap.end() )
        {
            continue;
        }
		m_csOCThread.Task( (CIClientChannel*)iVal);
       
    }

}





