/////////////////////////////
// TestApp.cpp
//

#include "StdAfx.h"
#include "TestApp.h"


// Constructor
CTestApp::CTestApp() : m_dialog(nullptr)
{
    m_module = ::LoadLibrary(L"MyDLL.dll");
    if (m_module == nullptr)
    {
        ::MessageBox(nullptr, L"Failed to load MyDLL.dll", L"Error", MB_OK);
    }
}

// Destructor
CTestApp::~CTestApp()
{
    ::FreeLibrary(m_module);
}

// InitInstance is called when the thread or application starts.
// Return TRUE to indicate success and run the message loop.
BOOL CTestApp::InitInstance()
{
    // Create the test window.
    m_testWindow.Create();

    // Show the dialog from the dll.
    ShowDialog();

    return TRUE;
}

// Call the ShowDialog function from the dll.
void CTestApp::ShowDialog()
{
    // Create a pointer to MyDLL's ShowDialog function.
    using SHOWDIALOG = HWND (WINAPI*)();
    SHOWDIALOG pShowDialog = reinterpret_cast<SHOWDIALOG>
        (::GetProcAddress(m_module, "ShowDialog"));

    if (pShowDialog != nullptr)
    {
        // Call the DLL's ShowDialog function.
        m_dialog = pShowDialog();
    }
    else
        ::MessageBox(nullptr, L"Failed to run ShowDialog", L"Error", MB_OK);
}