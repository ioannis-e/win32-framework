//////////////////////////////////////////////////////////////
// MainContainer.h - Declaration of the CMainContainer classes
//

#ifndef _MAIN_CONTAINER_H_
#define _MAIN_CONTAINER_H_

class CMainContainer : public CDockContainer
{
public:
    CMainContainer() {}
    virtual ~CMainContainer() {}

protected:
    // Virtual functions that override base class functions.
    virtual void AddContainer(CDockContainer* pContainer, BOOL insert, BOOL selectPage);
    virtual void RemoveContainer(CDockContainer* pContainer, BOOL updateParent);

private:
    CMainContainer(const CMainContainer&);               // Disable copy construction
    CMainContainer& operator=(const CMainContainer&);    // Disable assignment operator
};

#endif  // _MAIN_CONTAINER_H_