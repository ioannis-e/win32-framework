/////////////////////////////////////////////
// Mainfrm.h

#ifndef MAINFRM_H
#define MAINFRM_H

#include "FormView.h"


///////////////////////////////////////////////////////////
// CMainFrame manages the application's main window.
// The main window is a frame that has a menubar, toolbar,
// statusbar and view window.
class CMainFrame : public CFrame
{
public:
    CMainFrame();
    CFormDoc& GetDoc()      { return m_formView.GetDoc(); }

    // Virtual functions that override public base class functions
    virtual ~CMainFrame();
    virtual HWND Create(HWND parent = NULL);

protected:
    // Virtual functions that override base class functions
    virtual BOOL LoadRegistrySettings(LPCTSTR keyName);
    virtual BOOL OnCommand(WPARAM wparam, LPARAM lparam);
    virtual int  OnCreate(CREATESTRUCT& cs);
    virtual BOOL OnFileExit();
    virtual void OnInitialUpdate();
    virtual void OnMenuUpdate(UINT nID);
    virtual BOOL SaveRegistrySettings();
    virtual void SetupMenuIcons();
    virtual void SetupToolBar();
    virtual LRESULT WndProc(UINT msg, WPARAM wparam, LPARAM lparam);

private:
    CMainFrame(const CMainFrame&);               // Disable copy construction
    CMainFrame& operator=(const CMainFrame&);    // Disable assignment operator

    // Menu handlers
    void OnUpdateCheckA(UINT id);
    void OnUpdateCheckB(UINT id);
    void OnUpdateCheckC(UINT id);
    void OnUpdateRangeOfIDs(UINT idFirst, UINT idLast, UINT id);

    // Member variables
    CFormView m_formView;
};


#endif //MAINFRM_H
