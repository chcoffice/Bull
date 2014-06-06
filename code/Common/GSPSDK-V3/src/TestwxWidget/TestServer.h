/////////////////////////////////////////////////////////////////////////////
// Name:        TestServer.h
// Author:      XX
// Created:     XX/XX/XX
// Copyright:   XX
/////////////////////////////////////////////////////////////////////////////

#ifndef __TestServer_H__
#define __TestServer_H__


#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
    #pragma interface "TestServer.cpp"
#endif

#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include "test_wdr.h"
#include "MainLoop.h"
#include "md5.h"
#include "IServer.h"
#include "ThreadPool.h"

using namespace GSP;
// WDR: class declarations


//----------------------------------------------------------------------------
// StruMySource
//----------------------------------------------------------------------------
typedef struct  _StruMySource
{
CISource *pSource;
UINT64 iRcvs;
UINT64 iSends;
bool   bSendData;
INT    iRefs;
GSAtomicInter bWrite;
INT32  iWIndex;
INT32  iRandData;
unsigned char md5res[MD5_LEN]; 
INT    iFReadPos; //文件方式下
     _StruMySource(void)
     {
         pSource = NULL;
         iSends = 0;
         iRcvs = 0;
         bSendData = false;
         iRefs = 0;
         bWrite = 0;
         iWIndex = 0;
         iRandData = rand();
         iFReadPos = 0;
         
     }

     ~_StruMySource(void)
     {
        
     }
}StruMySource;

#define REFRESH_SRC_INFO_TIMER      (ID_TEXTCTRL_OUPUT+1000)
#define REFRESH_CLIENT_INFO_TIMER   (ID_TEXTCTRL_OUPUT+1001)
#define REFRESH_SEND_DATA_TIMER     (ID_TEXTCTRL_OUPUT+1002)

//----------------------------------------------------------------------------
// CSenderObj
//----------------------------------------------------------------------------
class CTestSrvDialog;

class CSenderObj : public CGSPObject
{
  
public :
    CTestSrvDialog *m_pDlg;
    CGSPThreadPool m_csTasker;
    INT m_iStep;
    unsigned char *m_pBuffer;
    INT m_iBufSize;
    INT m_iVideoFileSize;
    UINT64 m_iTicks;
    UINT m_iInterval;
    CSenderObj( void );
    virtual ~CSenderObj(void);
    void OnTaskEvent(CObjThreadPool *pTkPool,void *pTaskData);
    void SendStep(void);
};



//----------------------------------------------------------------------------
// CTestSrvDialog
//----------------------------------------------------------------------------
class CTestSrvDialog: public wxDialog
{
public:
    friend class CSenderObj;
    // constructors and destructors
    CTestSrvDialog( wxWindow *parent, wxWindowID id, const wxString &title,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxDEFAULT_DIALOG_STYLE );
    virtual ~CTestSrvDialog();
    
    // WDR: method declarations for CTestSrvDialog
    wxSpinCtrl* GetSpinctrlSinterval()  { return (wxSpinCtrl*) FindWindow( ID_SPINCTRL_SINTERVAL ); }
    wxCheckBox* GetCheckboxCliRefresh()  { return (wxCheckBox*) FindWindow( ID_CHECKBOX_CLI_REFRESH ); }
    wxCheckBox* GetCheckboxSrcRefresh()  { return (wxCheckBox*) FindWindow( ID_CHECKBOX_SRC_REFRESH ); }
    wxSpinCtrl* GetSpinctrlSrcRefinterval()  { return (wxSpinCtrl*) FindWindow( ID_SPINCTRL_SRC_REFINTERVAL ); }
    wxSpinCtrl* GetSpinctrlCliRefinterval()  { return (wxSpinCtrl*) FindWindow( ID_SPINCTRL_CLI_REFINTERVAL ); }
    wxSpinCtrl* GetSpinctrlAdInterval()  { return (wxSpinCtrl*) FindWindow( ID_SPINCTRL_AD_INTERVAL ); }
    wxSpinCtrl* GetSpinctrlSrcidxStart()  { return (wxSpinCtrl*) FindWindow( ID_SPINCTRL_SRCIDX_START ); }
    wxSpinCtrl* GetSpinctrlSrcidEnd()  { return (wxSpinCtrl*) FindWindow( ID_SPINCTRL_SRCID_END ); }
    wxButton* GetButtonAddSingle()  { return (wxButton*) FindWindow( ID_BUTTON_ADD_SINGLE ); }
    wxButton* GetButtonAddLots()  { return (wxButton*) FindWindow( ID_BUTTON_ADD_LOTS ); }
    wxToggleButton* GetTogglebuttonAdRand()  { return (wxToggleButton*) FindWindow( ID_TOGGLEBUTTON_AD_RAND ); }
    wxToggleButton* GetTogglebuttonSrcSenddata()  { return (wxToggleButton*) FindWindow( ID_TOGGLEBUTTON_SRC_SENDDATA ); }
    wxButton* GetButtonDelSrc()  { return (wxButton*) FindWindow( ID_BUTTON_DEL_SRC ); }
    wxGrid* GetGridSrvSt()  { return (wxGrid*) FindWindow( ID_GRID_SRV_ST ); }
    wxToggleButton* GetTogglebuttonSsserver()  { return (wxToggleButton*) FindWindow( ID_TOGGLEBUTTON_SSSERVER ); }
    wxGrid* GetGridClient()  { return (wxGrid*) FindWindow( ID_GRID_CLIENT ); }
    wxGrid* GetGridSrc()  { return (wxGrid*) FindWindow( ID_GRID_SRC ); }
    wxTextCtrl* GetTextctrlCfgfile()  { return (wxTextCtrl*) FindWindow( ID_TEXTCTRL_CFGFILE ); }
    

    EnumErrno OnServerSourceEvent(CIServer *pServer, 
                                            CISource *pSource, 
                                            EnumGSPServerEvent eEvent, 
                                            void *pEventPararms );
private:
    // WDR: member variable declarations for CTestSrvDialog
     CIServer *m_pSever;
     std::map<UINT16, StruMySource> m_csSourceMap;
     typedef std::map<UINT16, StruMySource>::iterator CSourceMapIterator;
     CGSWRMutex m_csSrcWRMutex;
     wxTimer   m_csRefSrcTimer; //
     wxTimer   m_csRefCliTimer; //
     CSenderObj m_csSender;
     wxTimer   m_csSendDataTimer; //

private:
    // WDR: handler declarations for CTestSrvDialog
	void OnDialogClose( wxCloseEvent &event);
    void OnSendIntervalChanged( wxSpinEvent &event );
    void OnSpinCliRefreshIntervalChanged( wxSpinEvent &event );
    void OnSpinADIntervalChanged( wxSpinEvent &event );
    void OnBtnADRandSrcClick( wxCommandEvent &event );
    void OnBtnAddSrcLotsClick( wxCommandEvent &event );
    void OnBtnAddSingleClick( wxCommandEvent &event );
    void OnSpinSrcRefreshIntervalChanged( wxSpinEvent &event );
    void OnBtnSrcSendDataClick( wxCommandEvent &event );
    void OnBtnDelSrcClick( wxCommandEvent &event );
    void OnStarSrvBtnClick( wxCommandEvent &event );
    void OnSelCfgFileBtnClick( wxCommandEvent &event );
    virtual void InitDialog();
    void UpdateModuleInfo(const StruProtocolService *pList);
    void EnableOtherFunction( bool bEnable );
    BOOL AddSource( UINT16 iIndex);
    void UpdateSourceInfo(void);
    void ClearSource( INT iIndex = -1 );
    void OnRefreshSrcInfoTimer( wxTimerEvent& event);
     void OnRefreshClientInfoTimer( wxTimerEvent& event);
    void UpdateClientInfo( bool bUpdate = true );
    void OnSendDataTimerEvent( wxTimerEvent& event);
private:
    DECLARE_EVENT_TABLE()
};




#endif
