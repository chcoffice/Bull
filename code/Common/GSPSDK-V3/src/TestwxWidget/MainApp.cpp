/////////////////////////////////////////////////////////////////////////////
// Name:        MainApp.cpp
// Author:      XX
// Created:     XX/XX/XX
// Copyright:   XX
/////////////////////////////////////////////////////////////////////////////

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
    #pragma implementation "MainApp.cpp"
#endif


// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wx.h"
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif


#include "TestServer.h"
#include "TestClient.h"
#include "MainApp.h"


namespace GSP
{


	extern void GSPModuleInit(void);
	extern void GSPModuleUnint(void);

} //end namespace GSP


// WDR: class implementations

//----------------------------------------------------------------------------
// CMainApp
//----------------------------------------------------------------------------
IMPLEMENT_APP(CMainApp)

CMainApp *g_pMainApp = NULL; 

// WDR: event table for MainApp 

CMainApp::CMainApp()
{
   
    m_pMainWindow = NULL;
     g_pMainApp = this;
}


// WDR: handler implementations for CMainApp
bool CMainApp::OnInit(void)
{
    GSPModuleInit();
	 GSP::GSPModuleInit();
    srand((int)time(NULL));

    wxApp::OnInit();
    m_csLocal.Init();
    m_csLocal.AddCatalog("wxDemo");


    m_pMainWindow = new CMainFrame( NULL, -1, "GSS Stack Test", wxPoint(0,0), wxSize(800,600) );
    m_pMainWindow->CentreOnScreen();
    m_pMainWindow->Show( TRUE );
 //   m_pMainWindow->ShowFullScreen(TRUE,wxFULLSCREEN_NOBORDER ); // 键盘输入不了   

    return true;
}

int CMainApp::OnExit(void)
{
	GSP::GSPModuleUnint();
    GSPModuleUnint();
    return 0;
}

void CMainApp::GetGridSelectRows( wxGrid* pGrid, wxArrayInt &vResult )
{
    vResult.Clear();
    int iRows = pGrid->GetRows();
    for( int i = 0 ; i<iRows;  i++)
    {
        if( pGrid->IsInSelection(i, 0) )
        {
            vResult.Add(i);
        }
    }   
}


//----------------------------------------------------------------------------
// CMainFrame
//----------------------------------------------------------------------------

// WDR: event table for CMainFrame

BEGIN_EVENT_TABLE(CMainFrame,wxFrame)
    EVT_MENU( ID_MENU_SERVER, CMainFrame::OnMenuServerClick )
    EVT_MENU( ID_MENU_TEST, CMainFrame::OnMenuClientClient )
    EVT_MENU( ID_MENU_ASSERT, CMainFrame::OnMenuAssertClick )
END_EVENT_TABLE()

CMainFrame::CMainFrame( wxWindow *parent, wxWindowID id, const wxString &title,
    const wxPoint &position, const wxSize& size, long style ) :
    wxFrame( parent, id, title, position, size, style )
{
    // WDR: dialog function MainFrameDialog for CMainFrame
    MainFrameDialog( this, TRUE ); 
    wxMenuBar *pMenuBar = MyMenuBarFunc();
    SetMenuBar(pMenuBar);
}

// WDR: handler implementations for CMainFrame

void CMainFrame::OnMenuClientClient( wxCommandEvent &event )
{
    //启动客户端测试
    CTestClientDialog *pDialog = new  CTestClientDialog(this, -1, "Test Client"); //, wxPoint(0,0), wxSize(800,600))
    pDialog->CentreOnScreen();
    pDialog->Show(TRUE);


}

void CMainFrame::OnMenuServerClick( wxCommandEvent &event )
{
    //启动服务器端测试
    CTestSrvDialog *pDialog = new  CTestSrvDialog(this, -1, "Test Server"); //, wxPoint(0,0), wxSize(800,600))
    pDialog->CentreOnScreen();
    pDialog->Show(TRUE);

}

 void CMainFrame::OnMenuAssertClick( wxCommandEvent &event )
 {
     GS_ASSERT(0);
 }




