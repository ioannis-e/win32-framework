/////////////////////////////
// main.cpp
//

#include "stdafx.h"
#include "FrameApp.h"


int WINAPI WinMain (_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int)
{
    try
    {
        // Start Win32++.
        CFrameApp theApp;

        // Run the application and the message loop.
        return theApp.Run();
    }

    // Catch all unhandled CException types.
    catch (const CException &e)
    {
        // Display the exception and quit.
        CString str;
        str << e.GetText() << _T("\n") << e.GetErrorString();
        ::MessageBox(NULL, str, _T("An exception occurred"), MB_ICONERROR);

        return -1;
    }
}

