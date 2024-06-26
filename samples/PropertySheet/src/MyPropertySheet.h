/////////////////////////////
// MyPropertySheet.h
//

#ifndef MYPROPERTYSHEET_H
#define MYPROPERTYSHEET_H


///////////////////////////////////////////////
// CMyPropertySheet manages the property sheet.
// A property sheet has several property pages.
class CMyPropertySheet : public CPropertySheet
{
public:
    CMyPropertySheet(LPCTSTR caption = NULL, HWND hParent = NULL);
    virtual ~CMyPropertySheet() {}

protected:
    // Virtual functions that override base class functions
    virtual void OnAttach();
    virtual void OnInitialUpdate();
    virtual LRESULT OnSetDefID(WPARAM wparam);
    virtual LRESULT WndProc(UINT msg, WPARAM wparam, LPARAM lparam);

private:
    CMyPropertySheet(const CMyPropertySheet&);               // Disable copy construction
    CMyPropertySheet& operator=(const CMyPropertySheet&);    // Disable assignment operator
};


/////////////////////////////////////////////////////////
// CButtonPage manages a property page that displays a
// number of buttons. A property page behaves much like a
// dialog.
class CButtonPage : public CPropertyPage
{
public:
    CButtonPage(UINT templateID, LPCTSTR title = NULL);
    virtual ~CButtonPage() {}

protected:
    // Virtual functions that override base class functions
    virtual INT_PTR DialogProc(UINT msg, WPARAM wparam, LPARAM lparam);
    virtual BOOL OnApply();
    virtual void OnCancel();
    virtual BOOL OnInitDialog();
    virtual void OnOK();
    virtual BOOL OnQueryCancel();
    virtual BOOL OnSetActive();
    virtual BOOL Validate();

private:
    CButtonPage(const CButtonPage&);                // Disable copy construction
    CButtonPage& operator=(const CButtonPage&);   // Disable assignment operator
};


//////////////////////////////////////////////////////
// CComboPage manages a property page that displays a
// number of combo boxes. A property page behaves much
// like a dialog.
class CComboPage : public CPropertyPage
{
public:
    CComboPage(UINT templateID, LPCTSTR title  = NULL);
    virtual ~CComboPage() {}

protected:
    // Virtual functions that override base class functions.
    virtual INT_PTR DialogProc(UINT msg, WPARAM wparam, LPARAM lparam);
    virtual BOOL OnApply();
    virtual BOOL OnInitDialog();
    virtual BOOL OnSetActive();

private:
    CComboPage(const CComboPage&);                // Disable copy construction
    CComboPage& operator=(const CComboPage&);   // Disable assignment operator
};


#endif // MYPROPERTYSHEET_H
