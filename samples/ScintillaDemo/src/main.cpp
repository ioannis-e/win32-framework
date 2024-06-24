/////////////////////////////
// main.cpp
//

#include "stdafx.h"
#include "ScintillaApp.h"


#if defined (_MSC_VER) && (_MSC_VER >= 1920) // >= VS2019
  int WINAPI wWinMain (__in HINSTANCE, __in_opt HINSTANCE, __in LPWSTR, __in int)
#else
  int WINAPI wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int)
#endif
{
    try
    {
        // Start Win32++.
        ScintillaApp theApp;

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

