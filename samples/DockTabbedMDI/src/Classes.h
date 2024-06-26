//////////////////////////////////////////////////////////
// Classes.h - Declaration of CViewClasses, CContainClasses,
//              and CDockClasses classes
//


#ifndef CLASSES_H
#define CLASSES_H


////////////////////////////////////////////
// CViewClasses manages a tree view control.
// This is the view window CContainClasses,
// uses to display classes.
class CViewClasses : public CTreeView
{
public:
    CViewClasses();
    virtual ~CViewClasses();
    void SetDPIImages();

protected:
    // Virtual functions that override base class functions.
    virtual void OnAttach();
    virtual LRESULT WndProc(UINT msg, WPARAM wparam, LPARAM lparam);

private:
    CViewClasses(const CViewClasses&);               // Disable copy construction
    CViewClasses& operator=(const CViewClasses&);    // Disable assignment operator

    // Command handlers
    LRESULT OnDpiChangedBeforeParent(UINT msg, WPARAM wparam, LPARAM lparam);
    LRESULT OnMouseActivate(UINT msg, WPARAM wparam, LPARAM lparam);
};


/////////////////////////////////////////////////////
// CContainClasses manages a dock container. It has a
// CViewClasses view.
class CContainClasses : public CDockContainer
{
public:
    CContainClasses();
    virtual ~CContainClasses() {}

protected:
    // Virtual functions that override base class functions.
    virtual BOOL OnCommand(WPARAM wparam, LPARAM lparam);
    virtual void PreCreate(CREATESTRUCT& cs);
    virtual void SetupToolBar();

private:
    CContainClasses(const CContainClasses&);              // Disable copy construction
    CContainClasses& operator=(const CContainClasses&);   // Disable assignment operator

    // Command handlers
    BOOL OnFileNew();
    BOOL OnHelpAbout();

    CViewClasses m_viewClasses;
};


///////////////////////////////////////////
// CDockClasses manages a docker that has a
// CContainClasses view.
class CDockClasses : public CDocker
{
public:
    CDockClasses();
    virtual ~CDockClasses() {}

private:
    CDockClasses(const CDockClasses&);              // Disable copy construction
    CDockClasses& operator=(const CDockClasses&);   // Disable assignment operator

    CContainClasses m_classes;
};


#endif  //CLASSES_H
