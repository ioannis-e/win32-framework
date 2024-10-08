/////////////////////////////
// TestApp.h
//

#ifndef TESTAPP_H
#define TESTAPP_H

#include "TestWindow.h"


class CTestApp : public CWinApp
{
public:
    CTestApp();
    virtual ~CTestApp();

    virtual BOOL InitInstance() override;
    void ShowDialog();
    HWND GetDialog() const  { return m_dialog; }

private:
    HMODULE m_module;
    CTestWindow m_testWindow;
    HWND m_dialog;
};


#endif // TESTAPP_H
