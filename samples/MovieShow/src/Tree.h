//////////////////////////////////////////////////////////
// Tree.h - Declaration of CViewTree and CDockTree classes
//

#ifndef _TREE_H
#define _TREE_H


///////////////////////////////////////////////////////////////////
// CViewTree manages a tree view control. The tree view control
// organises videos into groups such as genres, release dates,
// boxsets, video types (movie / live performance) and favourites.
class CViewTree : public CTreeView
{
public:
    CViewTree();
    virtual ~CViewTree();
    void SetDPIImages();
    void Swap(HTREEITEM item1, HTREEITEM item2);

protected:
    // Virtual functions that override base class functions
    virtual void    OnInitialUpdate();
    virtual LRESULT OnNotifyReflect(WPARAM wparam, LPARAM lparam);
    virtual void    PreCreate(CREATESTRUCT& cs);
    virtual LRESULT WndProc(UINT msg, WPARAM wparam, LPARAM lparam);

private:
    CViewTree(const CViewTree&);               // Disable copy construction
    CViewTree& operator=(const CViewTree&);    // Disable assignment operator

    bool      IsBoxSetUnique(LPCTSTR text, HTREEITEM item);

    // Message handlers
    LRESULT OnDpiChangedBeforeParent(UINT msg, WPARAM wparam, LPARAM lparam);
    LRESULT OnRButtonDown(UINT msg, WPARAM wparam, LPARAM lparam);
    LRESULT OnRButtonUp(UINT msg, WPARAM wparam, LPARAM lparam);

    // Notification message handlers
    BOOL OnBeginLabelEdit(LPARAM lparam);
    BOOL OnEndLabelEdit(LPARAM lparam);
    BOOL OnSelChanged();

    // Member variables
    CString m_itemText;
};


////////////////////////////////////////////////////////
// CDockTree manages the resizing of the treeview window
// when the splitter bar is dragged.
class CDockTree : public CDocker
{
public:
    CDockTree();
    virtual ~CDockTree() {}
    CViewTree& GetViewTree() { return m_treeView; }

private:
    CDockTree(const CDockTree&);                // Disable copy construction
    CDockTree& operator=(const CDockTree&);   // Disable assignment operator

    CViewTree m_treeView;
};


#endif  //_TREE_H
