/////////////////////////////////////////////////////////////////////////////
// Name:        TestClient.h
// Author:      XX
// Created:     XX/XX/XX
// Copyright:   XX
/////////////////////////////////////////////////////////////////////////////

#ifndef __TestClient_H__
#define __TestClient_H__

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
    #pragma interface "TestClient.cpp"
#endif

#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include "test_wdr.h"
#include "IGSPClient.h"
#include "ThreadPool.h"

using namespace GSP;

// WDR: class declarations

//----------------------------------------------------------------------------
// CTestClientDialog
//----------------------------------------------------------------------------
#define REFRESH_INFO_TIMER     (ID_TEXTCTRL_OUPUT+1012)
#define REFRESH_OCRAND_TIMER     (ID_TEXTCTRL_OUPUT+1013)

class CThreadPoolCallback;

class CTestClientDialog: public wxDialog
{
public:
    // constructors and destructors
    CTestClientDialog( wxWindow *parent, wxWindowID id, const wxString &title,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxDEFAULT_DIALOG_STYLE );
    virtual ~CTestClientDialog();
    
    // WDR: method declarations for CTestClientDialog
    wxToggleButton* GetTblbtnOcrand()  { return (wxToggleButton*) FindWindow( ID_TBLBTN_OCRAND ); }
    wxButton* GetBtnDel()  { return (wxButton*) FindWindow( ID_BTN_DEL ); }
    wxButton* GetBtnClose()  { return (wxButton*) FindWindow( ID_BTN_CLOSE ); }
    wxButton* GetBtnStep()  { return (wxButton*) FindWindow( ID_BTN_STEP ); }
    wxButton* GetBtnSlow()  { return (wxButton*) FindWindow( ID_BTN_SLOW ); }
    wxButton* GetBtnFast()  { return (wxButton*) FindWindow( ID_BTN_FAST ); }
    wxButton* GetButtonAddLots()  { return (wxButton*) FindWindow( ID_BUTTON_ADD_LOTS ); }
    wxButton* GetButtonAddSingle()  { return (wxButton*) FindWindow( ID_BUTTON_ADD_SINGLE ); }
    wxChoice* GetChoiceTransmodule()  { return (wxChoice*) FindWindow( ID_CHOICE_TRANSMODULE ); }
    wxSlider* GetSliderSetpoint()  { return (wxSlider*) FindWindow( ID_SLIDER_SETPOINT ); }
    wxToggleButton* GetTglbtnPause()  { return (wxToggleButton*) FindWindow( ID_TGLBTN_PAUSE ); }
    wxToggleButton* GetTglbtnPlay()  { return (wxToggleButton*) FindWindow( ID_TGLBTN_PLAY ); }
    wxSpinCtrl* GetSpinctrlAdInterval()  { return (wxSpinCtrl*) FindWindow( ID_SPINCTRL_AD_INTERVAL ); }
    wxToggleButton* GetTogglebuttonAdRand()  { return (wxToggleButton*) FindWindow( ID_TOGGLEBUTTON_AD_RAND ); }
    wxSpinCtrl* GetSpinctrlKeyNumber()  { return (wxSpinCtrl*) FindWindow( ID_SPINCTRL_KEY_NUMBER ); }
    wxSpinCtrl* GetSpinctrlKeyidxStart()  { return (wxSpinCtrl*) FindWindow( ID_SPINCTRL_KEYIDX_START ); }
    wxTextCtrl* GetTextctrlPort()  { return (wxTextCtrl*) FindWindow( ID_TEXTCTRL_PORT ); }
    wxTextCtrl* GetTextctrlIp()  { return (wxTextCtrl*) FindWindow( ID_TEXTCTRL_IP ); }
    wxChoice* GetChoicePro()  { return (wxChoice*) FindWindow( ID_CHOICE_PRO ); }
    wxCheckBox* GetCheckboxChnRefresh()  { return (wxCheckBox*) FindWindow( ID_CHECKBOX_CHN_REFRESH ); }
    wxSpinCtrl* GetSpinctrlChnRefinterval()  { return (wxSpinCtrl*) FindWindow( ID_SPINCTRL_CHN_REFINTERVAL ); }
    wxGrid* GetGridChannel()  { return (wxGrid*) FindWindow( ID_GRID_CHANNEL ); }
    wxGrid* GetGridProModule()  { return (wxGrid*) FindWindow( ID_GRID_PRO_MODULE ); }
    wxToggleButton* GetTogglebuttonStartSection()  { return (wxToggleButton*) FindWindow( ID_TOGGLEBUTTON_START_SECTION ); }
    wxTextCtrl* GetTextctrlCfgfile()  { return (wxTextCtrl*) FindWindow( ID_TEXTCTRL_CFGFILE ); }
	INT OnSectionEvent( CIClientSection *pcsClient,
		CIClientChannel *pChannel, 
		EnumGSPClientEventType eEvtType, 
		void *pEventData,  
		INT iEvtDataLen );
	void OnOCTaskEvent(void *pParam );
private:
    // WDR: member variable declarations for CTestClientDialog
    CIClientSection *m_pSection;
    wxTimer   m_csRefInfoTimer; 
    CGSWRMutex m_csMutex;
    std::map<int , CIClientChannel*> m_csChnMap;
    wxTimer m_csOCTimer;
    bool m_bPlay;
    INT m_iRandCnts;
	CGSPThreadPool m_csOCThread;
	CThreadPoolCallback *m_pTaskCallback;
private:
    // WDR: handler declarations for CTestClientDialog
	void OnDialogClose( wxCloseEvent &event);
    void OnBtnOpenClick(  wxCommandEvent &event );
    void OnOCRandTimer(wxTimerEvent &event );
    void OnTglBtnOCRandClick( wxCommandEvent &event );
    void OnBtnDelClick( wxCommandEvent &event );
    void OnBtnCloseClick( wxCommandEvent &event );
    void OnSliderSetPointSlidEvt( wxCommandEvent &event );
    void OnBtnStepClickEvt( wxCommandEvent &event );
    void OnBtnSlowClickEvt( wxCommandEvent &event );
    void OnBtnFastClick( wxCommandEvent &event );
    void OnTglbtnPauseClick( wxCommandEvent &event );
    void OnTglbtnPlayClick( wxCommandEvent &event );
    void OnBtnADRandClick( wxCommandEvent &event );
    void OnBtnAddLotsClick( wxCommandEvent &event );
    void OnBtnAddSingleClick( wxCommandEvent &event );
    void OnTglbtnStartSectionClick( wxCommandEvent &event );
    void OnBtnSelCfgFileClick( wxCommandEvent &event );
    virtual void InitDialog();
    void EnableOtherFunction( bool bEnable );
    void OnRefreshInfoTimer( wxTimerEvent& event);
    BOOL AddChannel( const char *czKey, const char *czServerHost, int iPort,const char *czPro  );
    void   UpdateChannelInfo(void);

	

private:
    DECLARE_EVENT_TABLE()
};




#endif
