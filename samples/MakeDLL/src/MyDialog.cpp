/////////////////////////////
// MyDialog.cpp
//

#include "StdAfx.h"
#include "MyDialog.h"
#include "resource.h"


/////////////////////////////////
// CMyDialog function definitions
//

// Constructor.
CMyDialog::CMyDialog(UINT resID) : CDialog(resID)
{
}

// Called when the dialog window is destroyed.
void CMyDialog::OnDestroy()
{
    TRACE("Destroyed the dialog created by the DLL.\n");
    PostQuitMessage(0);
}

// Process the dialog's window messages.
INT_PTR CMyDialog::DialogProc(UINT msg, WPARAM wparam, LPARAM lparam)
{
    try
    {

    //  switch (msg)
    //  {
    //  }

        // Pass unhandled messages on to parent DialogProc
        return DialogProcDefault(msg, wparam, lparam);
    }

    // Catch all CException types.
    catch (const CException& e)
    {
        // Display the exception and continue.
        CString str;
        str << e.GetText() << L'\n' << e.GetErrorString();
        ::MessageBox(nullptr, str, L"An exception occurred", MB_ICONERROR);

        return 0;
    }
}

// Process the dialog's command messages (WM_COMMAND).
BOOL CMyDialog::OnCommand(WPARAM wparam, LPARAM)
{
    UINT id = LOWORD(wparam);
    switch (id)
    {
    case IDC_BUTTON1:   return OnButton();
    case IDC_CHECK1:    return OnCheck1();
    case IDC_CHECK2:    return OnCheck2();
    case IDC_CHECK3:    return OnCheck3();

    case IDC_RADIO1:    // intentionally blank
    case IDC_RADIO2:
    case IDC_RADIO3:    return OnRangeOfRadioIDs(IDC_RADIO1, IDC_RADIO3, id);
    }

    return FALSE;
}

// Called before the dialog is displayed.
BOOL CMyDialog::OnInitDialog()
{
    // Set the Icon
    SetIconLarge(IDI_DIALOG);
    SetIconSmall(IDI_DIALOG);

    AttachItem(IDC_EDIT1, m_edit);
    AttachItem(IDC_RICHEDIT1, m_richEdit);

    // Put some text in the edit boxes
    m_edit.SetWindowText(L"Edit Control");
    m_richEdit.SetWindowText(L"Rich Edit Window");

    // Put some text in the list box
    for (int i = 0 ; i < 8 ; i++)
        SendDlgItemMessage(IDC_LIST1, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"List Box"));

    // Select the first radio button
    CheckRadioButton(IDC_RADIO1, IDC_RADIO3, IDC_RADIO1);

    // Ajust the pattern image size.
    m_patternImage.LoadBitmap(IDB_BITMAP1);
    m_patternImage = DpiScaleUpBitmap(m_patternImage);
    LPARAM lparam = reinterpret_cast<LPARAM>(m_patternImage.GetHandle());
    SendDlgItemMessage(IDC_STATIC1, STM_SETIMAGE, IMAGE_BITMAP, lparam);

    return TRUE;
}

// Called when the OK button or Enter key is pressed.
void CMyDialog::OnOK()
{
    ::MessageBox(nullptr, L"OK Button Pressed.  Dialog will now end.", L"Button", MB_OK);
    CDialog::OnOK();
}

// Called when the button is pressed.
BOOL CMyDialog::OnButton()
{
    SetDlgItemText(IDC_STATIC3, L"Button Pressed");
    TRACE("Button Pressed\n");
    return TRUE;
}

// Called when check box 1 is clicked.
BOOL CMyDialog::OnCheck1()
{
    SetDlgItemText(IDC_STATIC3, L"Check Box 1");
    TRACE("Check Box 1\n");
    return TRUE;
}

// Called when check box 2 is clicked.
BOOL CMyDialog::OnCheck2()
{
    SetDlgItemText(IDC_STATIC3, L"Check Box 2");
    TRACE("Check Box 2\n");
    return TRUE;
}

// Called when check box 3 is clicked.
BOOL CMyDialog::OnCheck3()
{
    SetDlgItemText(IDC_STATIC3, L"Check Box 3");
    TRACE("Check Box 3\n");
    return TRUE;
}

// Called when a radio button is selected.
BOOL CMyDialog::OnRangeOfRadioIDs(UINT idFirst, UINT idLast, UINT idClicked)
{
    CheckRadioButton(idFirst, idLast, idClicked);

    CString str;
    int nButton = idClicked - idFirst + 1;
    str.Format(L"Radio%d", nButton);
    SetDlgItemText(IDC_STATIC3, str);
    TRACE(str); TRACE("\n");

    return TRUE;
}
