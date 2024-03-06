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

    // Update the dock style for multiple tabs.
    int tabCount = GetItemCount();
    DWORD dockStyle = GetDocker()->GetDockStyle();
    if (tabCount > 1)
        dockStyle = dockStyle & ~DS_NO_CLOSE;
    else
        dockStyle = dockStyle | DS_NO_CLOSE;

    GetDocker()->SetDockStyle(dockStyle);
}

void CMainContainer::RemoveContainer(CDockContainer* pContainer, BOOL updateParent)
{
    // Call the base class function first.
    CDockContainer::RemoveContainer(pContainer, updateParent);

    // Update the dock style for multiple tabs.
    int tabCount = GetItemCount();
    DWORD dockStyle = GetDocker()->GetDockStyle();
    if (tabCount > 1)
        dockStyle = dockStyle & ~DS_NO_CLOSE;
    else
        dockStyle = dockStyle | DS_NO_CLOSE;

    GetDocker()->SetDockStyle(dockStyle);
}