/////////////////////////////
// MyTabbedMDI.h
//

#ifndef MYTABBEDMDI_H
#define MYTABBEDMDI_H

// The docker identifiers (dock IDs)
const int ID_MDI_CLASSES = 1;
const int ID_MDI_FILES = 2;
const int ID_MDI_OUTPUT = 3;
const int ID_MDI_TEXT = 4;
const int ID_MDI_WEB = 5;
const int ID_MDI_RECT = 6;


////////////////////////////////////////
// Declaration of the CMyTabbedMDI class
//
class CMyTabbedMDI : public CTabbedMDI
{
public:
    CMyTabbedMDI();
    virtual ~CMyTabbedMDI() {}

    virtual void OnAttach()
    {
        CTabbedMDI::OnAttach();
        if (GetWinVersion() >= 3000)  // Windows 10 or later.
            SetExStyle(WS_EX_COMPOSITED);
    }

protected:
    // Virtual functions that override base class functions
    virtual CWnd* NewMDIChildFromID(int mdiChild);

    virtual LRESULT OnWindowPosChanged(UINT msg, WPARAM wparam, LPARAM lparam)
    {
        LockWindowUpdate();
        CTabbedMDI::OnWindowPosChanged(msg, wparam, lparam);
        UnlockWindowUpdate();
        UpdateWindow();
        return 0;
    }

private:
    CMyTabbedMDI(const CMyTabbedMDI&);               // Disable copy construction
    CMyTabbedMDI& operator=(const CMyTabbedMDI&);    // Disable assignment operator
};

#endif

