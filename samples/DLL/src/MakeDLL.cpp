/////////////////////////////
// MakeDLL.cpp
//

// Defines the entry point for the DLL application.

#include "StdAfx.h"
#include "MyDialog.h"
#include "MakeDLL.h"
#include "resource.h"


// Start Win32++ for the DLL
CWinApp myApp;

// MyDialog is global for the DLL
CMyDialog myDialog(IDD_DIALOG1);

// The entry point for the dll.
BOOL WINAPI DllMain( HANDLE, DWORD  ul_reason_for_call, LPVOID )
{
    switch( ul_reason_for_call )
    {
    case DLL_PROCESS_ATTACH:
        TRACE("DLL_PROCESS_ATTACH\n");
        break;
    case DLL_THREAD_ATTACH:
        TRACE("DLL_THREAD_ATTACH\n");
        break;
    case DLL_THREAD_DETACH:
        TRACE("DLL_THREAD_DETACH\n");
        break;
    case DLL_PROCESS_DETACH:
        TRACE("DLL_PROCESS_DETACH\n");
        break;
    }

    return TRUE;
}

HWND __declspec(dllexport) ShowDialog()
{
    TRACE("ShowDialog called by the DLL.\n");

    // Create the dialog.
    if (!myDialog.IsWindow())
    {
        myDialog.Create();
        TRACE("Dialog inside DLL created.\n");
    }
    else
        TRACE("Dialog is already shown!\n");

    return myDialog.GetHwnd();
}
