/////////////////////////////
// IFileDialogApp.cpp
//

#include "stdafx.h"
#include "IFileDialogApp.h"
#include "IFileDialog.h"
#include "resource.h"


///////////////////////////////////////
// CIFileDialogApp function definitions
//

// Called when the application starts.
BOOL CIFileDialogApp::InitInstance()
{
    myTaskDialog.DoModal();

    ::PostQuitMessage(0);
    return TRUE;
}


