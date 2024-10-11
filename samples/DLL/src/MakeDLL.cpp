/////////////////////////////
// MakeDLL.cpp
//

#include "StdAfx.h"
#include "MyDialog.h"
#include "MakeDLL.h"
#include "MyWinThread.h"
#include "resource.h"


// Start Win32++ for this DLL.
CWinApp dllApp;

using ThreadPtr = std::unique_ptr<CMyWinThread>;
std::vector<ThreadPtr> allThreads;

// DllMain defines the entry point for this DLL.
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

// Define the ShowDialog function exported by this DLL.
__declspec(dllexport) void ShowDialog()
{
    TRACE("ShowDialog called by the DLL.\n");

    allThreads.push_back(std::make_unique<CMyWinThread>());
    allThreads.back()->CreateThread();
}
