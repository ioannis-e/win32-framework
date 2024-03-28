/////////////////////////////
// MainContainer.cpp
//

#include "stdafx.h"
#include "MainContainer.h"


///////////////////////////////////////
// CMainContainer function definitions
//


void CMainContainer::AddContainer(CDockContainer* pContainer, BOOL insert, BOOL selectPage)
{
    // Call the base class function first.
    CDockContainer::AddContainer(pContainer, insert, selectPage);

    // Use different dock styles when we have a single tab vs multiple tabs.
    DWORD dockStyle = GetDocker()->GetDockStyle();
    if (GetItemCount() > 1)
        dockStyle = dockStyle & ~(DS_NO_CLOSE | DS_NO_UNDOCK);
    else
        dockStyle = dockStyle | DS_NO_CLOSE | DS_NO_UNDOCK;

    GetDocker()->SetDockStyle(dockStyle);
}

void CMainContainer::RemoveContainer(CDockContainer* pContainer, BOOL updateParent)
{
    // Call the base class function first.
    CDockContainer::RemoveContainer(pContainer, updateParent);

    // Use different dock styles when we have a single tab vs multiple tabs.
    DWORD dockStyle = GetDocker()->GetDockStyle();
    if (GetItemCount() > 1)
        dockStyle = dockStyle & ~(DS_NO_CLOSE | DS_NO_UNDOCK);
    else
        dockStyle = dockStyle | DS_NO_CLOSE | DS_NO_UNDOCK;

    GetDocker()->SetDockStyle(dockStyle);
}