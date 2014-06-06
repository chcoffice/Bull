/////////////////////////////////////////////////////////////////////////////
// Name:        MainApp.h
// Author:      XX
// Created:     XX/XX/XX
// Copyright:   XX
/////////////////////////////////////////////////////////////////////////////

#ifndef __MainApp_H__
#define __MainApp_H__

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
    #pragma interface "MainApp.cpp"
#endif

#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include "test_wdr.h"

// WDR: class declarations



//----------------------------------------------------------------------------
// CMainFrame
//----------------------------------------------------------------------------

class CMainFrame: public wxFrame
{
public:
    // constructors and destructors
    CMainFrame( wxWindow *parent, wxWindowID id, const wxString &title,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxDEFAULT_FRAME_STYLE );
    
    // WDR: method declarations for CMainFrame
    
private:
    // WDR: member variable declarations for CMainFrame
    
private:
    // WDR: handler declarations for CMainFrame
    void OnMenuClientClient( wxCommandEvent &event );
    void OnMenuServerClick( wxCommandEvent &event );
    void OnMenuAssertClick( wxCommandEvent &event );

private:
    DECLARE_EVENT_TABLE()
};


//----------------------------------------------------------------------------
// CMainApp
//----------------------------------------------------------------------------
class CMainApp: public wxApp
{
public:
    // constructors and destructors
    CMainApp();

    // WDR: method declarations for CMainApp
public:    
    virtual bool OnInit(void);
    virtual int OnExit(void); 
    static void GetGridSelectRows( wxGrid* pGrid, wxArrayInt &vResult);
public :
     wxLocale m_csLocal;
     CMainFrame *m_pMainWindow;
private:
    // WDR: member variable declarations for CMainApp

private:
    // WDR: handler declarations for CMainApp

private:    
};

DECLARE_APP(CMainApp)

extern CMainApp *g_pMainApp;

#endif
