/////////////////////////////
// MainWnd.h
//

#ifndef MAINWND_H
#define MAINWND_H


#include "MyEdit.h"
#include "TestWnd.h"


#define WM_WINDOWCREATED WM_USER + 1   // the message sent when window is created
#define WM_TESTMESSAGE   WM_USER + 2   // the test message


// Note: Modern C++ compilers can use this typedef instead.
// typedef std::shared_ptr<CTestWindow> TestWindowPtr;
typedef Shared_Ptr<CTestWindow> TestWindowPtr;

///////////////////////////////////////////////////////////
// CMainWindow manages the main window for the application.
class CMainWindow : public CWnd
{
public:
    CMainWindow();
    virtual ~CMainWindow();
    virtual HWND Create(HWND hParent = NULL);
    void CreateTestWindows(int windows);
    void SetTestMessages(int testMessages) {m_testMessages = testMessages;}

protected:
    // Virtual functions that override base class functions
    virtual int  OnCreate(CREATESTRUCT& cs);
    virtual void OnDestroy();
    virtual void OnInitialUpdate();
    virtual LRESULT WndProc(UINT msg, WPARAM wparam, LPARAM lparam);

private:
    CMainWindow(const CMainWindow&);               // Disable copy construction
    CMainWindow& operator=(const CMainWindow&);    // Disable assignment operator

    // Message handlers
    LRESULT OnDpiChanged(UINT, WPARAM, LPARAM lparam);
    LRESULT OnSize();
    LRESULT OnWindowCreated();

    LONGLONG GetCounter() const;
    void OnAllWindowsCreated();
    void PerformanceTest() const;
    void SendText(LPCTSTR str) const;

    // Member variables
    std::vector<TestWindowPtr> m_pTestWindows; // A vector CTestWindow smart pointers
    CMyEdit m_edit;        // Handle to the edit window
    int m_testMessages;    // Number of test messages to be sent
    int m_testWindows;     // Number of test windows to create
    int m_windowsCreated;  // Number of windows created
    LONGLONG m_frequency;
};


#endif  //MAINWND_H
