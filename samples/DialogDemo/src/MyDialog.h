/////////////////////////////
// MyDialog.h
//

#ifndef MYDIALOG_H
#define MYDIALOG_H


#include "MyButton.h"
#include "Hyperlink.h"


///////////////////////////////////////////////////
// CMyDialog manages the application's main dialog.
class CMyDialog : public CDialog
{
public:
    CMyDialog(UINT resID);
    CMyDialog(LPCTSTR resName);
    virtual ~CMyDialog();

    void SetStatic(LPCTSTR text);
    void SetURLFont();

protected:
    // Virtual functions that override base class functions
    virtual void OnDestroy();
    virtual BOOL OnInitDialog();
    virtual INT_PTR DialogProc(UINT msg, WPARAM wparam, LPARAM lparam);
    virtual BOOL OnCommand(WPARAM wparam, LPARAM lparam);
    virtual void OnOK();

private:
    CMyDialog(const CMyDialog&);               // Disable copy construction
    CMyDialog& operator=(const CMyDialog&);    // Disable assignment operator

    // Command handlers
    BOOL OnButton();
    BOOL OnCheck1();
    BOOL OnCheck2();
    BOOL OnCheck3();
    BOOL OnRangeOfRadioIDs(UINT idFirst, UINT idLast, UINT idClicked);

    // Message handlers
    INT_PTR OnDpiChanged();
    INT_PTR OnSetStatic(WPARAM wparam);

    // Member variables
    CRichEdit   m_richEdit;
    CMyButton   m_button;
    CHyperlink  m_hyperlink;
    CListBox    m_listBox;
};

#endif //MYDIALOG_H
