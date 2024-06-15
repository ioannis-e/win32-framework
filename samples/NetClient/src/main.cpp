/////////////////////////////
// main.cpp
//

#include "stdafx.h"
#include "DialogApp.h"


#if defined (_MSC_VER) && (_MSC_VER >= 1920) // >= VS2019
  int WINAPI WinMain (__in HINSTANCE, __in_opt HINSTANCE, __in LPSTR, __in int)
#else
  int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
#endif
{
    try
    {
        // Start Win32++.
        CDialogApp theApp;

        // Run the application.
        return theApp.Run();
    }

    // Catch all unhandled CException types.
    catch (const CException &e)
    {
        // Display the exception and quit
        CString str;
        str << e.GetText() << _T("\n") << e.GetErrorString();
        ::MessageBox(NULL, str, _T("An exception occurred"), MB_ICONERROR);

        return -1;
    }
}
