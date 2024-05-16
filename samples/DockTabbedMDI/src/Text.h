///////////////////////////////////////////////////////
// Text.h - Declaration of the CViewText, CContainText,
//          and CDockText classes
//

#ifndef TEXT_H
#define TEXT_H

#include <richedit.h>


///////////////////////////////////////////////////////////
// CViewText manages a rich edit control. It displays text.
// This is the view window used byCContainText.
class CViewText : public CRichEdit
{
public:
    CViewText();
    virtual ~CViewText();

protected:
    // Virtual functions that override base class functions
    virtual void OnAttach();
    virtual BOOL OnCommand(WPARAM wparam, LPARAM lparam);
    void PreCreate(CREATESTRUCT& cs);

private:
    CViewText(const CViewText&);               // Disable copy construction
    CViewText& operator=(const CViewText&);    // Disable assignment operator

    // Command handlers
    BOOL OnEditCopy();
    BOOL OnEditPaste();
    BOOL OnEditCut();
    BOOL OnEditDelete();
    BOOL OnEditRedo();
    BOOL OnEditUndo();
};


/////////////////////////////////////////////////
// CContainText manages a dock container. It uses
// CViewText as its view.
class CContainText : public CDockContainer
{
public:
    CContainText();
    virtual ~CContainText() {}

private:
    CContainText(const CContainText&);                // Disable copy construction
    CContainText& operator=(const CContainText&);   // Disable assignment operator

    CViewText m_viewText;
};


////////////////////////////////////////////////////
// CDockText manages a docker that uses CContainText
// as its view.
class CDockText : public CDocker
{
public:
    CDockText();
    virtual ~CDockText() {}

    // Sets the CREATESTRUCT parameters before the window is created.
    virtual void PreCreate(CREATESTRUCT& cs)
    {
        // Call base clase to set defaults.
        CDocker::PreCreate(cs);

        // Add the WS_EX_COMPOSITED to reduce flicker.
        if (GetWinVersion() >= 3000)  // Windows 10 or later.
            cs.dwExStyle |= WS_EX_COMPOSITED;
    }

private:
    CDockText(const CDockText&);                // Disable copy construction
    CDockText& operator=(const CDockText&);   // Disable assignment operator

    CContainText m_view;
};


#endif // TEXT_H

