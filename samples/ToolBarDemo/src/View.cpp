/////////////////////////////
// View.cpp
//

#include "stdafx.h"
#include "View.h"
#include "resource.h"

//////////////////////////////
// CView function definitions.
//

void CView::DpiScaleToolBar()
{
    // Create the ToolBar's image list from 4 icons
    int scale = 48 * (GetWindowDpi(*this) / USER_DEFAULT_SCREEN_DPI);
    m_toolBarImages.DeleteImageList();
    m_toolBarImages.Create(scale, scale, ILC_COLOR32 | ILC_MASK, 0, 0);
    m_toolBarImages.AddIcon(IDI_TOP);
    m_toolBarImages.AddIcon(IDI_LEFT);
    m_toolBarImages.AddIcon(IDI_RIGHT);
    m_toolBarImages.AddIcon(IDI_BOTTOM);

    m_toolBar.SetImageList(m_toolBarImages);
    RecalcLayout();
}

// Position the toolbar on the bottom.
BOOL CView::OnBottom()
{
    DWORD style = m_toolBar.GetStyle();

    style &= ~(CCS_VERT);
    style |= CCS_BOTTOM;
    m_toolBar.SetStyle(style);
    RecalcLayout();
    return TRUE;
}

// Process menu or toolbar input
BOOL CView::OnCommand(WPARAM wparam, LPARAM)
{
    // Process the messages from the child ToolBar

    switch(LOWORD(wparam))
    {
    case IDM_TOP:       return OnTop();
    case IDM_LEFT:      return OnLeft();
    case IDM_RIGHT:     return OnRight();
    case IDM_BOTTOM:    return OnBottom();
    }

    return FALSE;
}

// Called during frame creation.
int CView::OnCreate(CREATESTRUCT&)
{
    // Create the ToolBar
    m_toolBar.Create(*this);
    DpiScaleToolBar();

    // Declare a style that supports vertical toolbars.
    DWORD style = WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | TBSTYLE_TOOLTIPS
                    | TBSTYLE_FLAT | CCS_NORESIZE | CCS_NOPARENTALIGN | CCS_NODIVIDER ;

    m_toolBar.SetStyle(style);

    // Add the ToolBar buttons
    TBBUTTON buttonInfo[] =
    {
        { 0, IDM_TOP,       TBSTATE_ENABLED, TBSTYLE_BUTTON|TBSTYLE_CHECK|TBSTYLE_GROUP, {0}, 0, 0 },
        { 1, IDM_LEFT,      TBSTATE_ENABLED, TBSTYLE_BUTTON|TBSTYLE_CHECK|TBSTYLE_GROUP, {0}, 0, 0 },
        { 2, IDM_RIGHT,     TBSTATE_ENABLED, TBSTYLE_BUTTON|TBSTYLE_CHECK|TBSTYLE_GROUP, {0}, 0, 0 },
        { 3, IDM_BOTTOM,    TBSTATE_ENABLED, TBSTYLE_BUTTON|TBSTYLE_CHECK|TBSTYLE_GROUP, {0}, 0, 0 }
    };

    m_toolBar.AddButtons(4, buttonInfo);

    return 0;
}

// Called in response to a WM_DPICHANGED_BEFOREPARENT message that is sent to child
// windows after a DPI change. A WM_DPICHANGED_BEFOREPARENT is only received when the
// application is DPI_AWARENESS_PER_MONITOR_AWARE.
LRESULT CView::OnDpiChangedBeforeParent(UINT msg, WPARAM wparam, LPARAM lparam)
{
    DpiScaleToolBar();

    return FinalWindowProc(msg, wparam, lparam);
}

// OnDraw is called when part or all of the window needs to be redrawn.
void CView::OnDraw(CDC& dc)
{
    CRect clientRect = GetClientRect();
    CRect tbRect = m_toolBar.GetWindowRect();

    // Calculate the view rect excluding the ToolBar rect
    ClientToScreen(clientRect);
    if (clientRect.Width() == tbRect.Width())
    {
        if (clientRect.top == tbRect.top)
            clientRect.top += tbRect.Height();
        else
            clientRect.bottom -= tbRect.Height();
    }
    else
    {
        if (clientRect.left == tbRect.left)
            clientRect.left += tbRect.Width();
        else
            clientRect.right -= tbRect.Width();
    }

    ScreenToClient(clientRect);

    // Use the message font for Windows 7 and higher.
    if (GetWinVersion() >= 2601)
    {
        NONCLIENTMETRICS info = GetNonClientMetrics();
        LOGFONT lf = DpiScaleLogfont(info.lfMessageFont, 10);
        dc.CreateFontIndirect(lf);
    }

    // Display some text in our view window
    CString text = "\nPress the arrows to change the inner toolbar's orientation,";
    text += "\n or choose customize toolbar from the ToolBar menu item to modify the toolbar in the frame.";
    dc.DrawText(text, -1, clientRect, DT_CENTER | DT_WORDBREAK);
}

// OnInitialUpdate is called immediately after the window is created.
void CView::OnInitialUpdate()
{
    TRACE("View window created\n");
}

// Position the toolbar on the left.
BOOL CView::OnLeft()
{
    DWORD style = m_toolBar.GetStyle();

    style &= ~(CCS_BOTTOM);
    style |= CCS_LEFT;
    m_toolBar.SetStyle(style);
    RecalcLayout();
    return TRUE;
}


// Called when a notification is received from a child window.
LRESULT CView::OnNotify(WPARAM wparam, LPARAM lparam)
{
    LPNMHDR pHeader = reinterpret_cast<LPNMHDR>(lparam);
    switch (pHeader->code)
    {
    // Pass the ToolBar's ToolTip info up to the frame
    case TTN_GETDISPINFO: return GetParent().SendMessage(WM_NOTIFY, wparam, lparam);
    }

    return 0;
}

// Position the toolbar on the right.
BOOL CView::OnRight()
{
    DWORD style = m_toolBar.GetStyle();

    style |= CCS_RIGHT;
    m_toolBar.SetStyle(style);
    RecalcLayout();
    return TRUE;
}

// Called after the window's size has changed.
LRESULT CView::OnSize(UINT, WPARAM, LPARAM)
{
    RecalcLayout();
    Invalidate();
    return 0;
}

// Position the toolbar at the top.
BOOL CView::OnTop()
{
    DWORD style = m_toolBar.GetStyle();

    style &= ~(CCS_VERT | CCS_BOTTOM);
    m_toolBar.SetStyle(style);
    RecalcLayout();
    return TRUE;
}

// Here we set the defaults used by the create function for the view window
// Preforming this is optional, but doing so allows us to
// take more precise control over the window we create.
void CView::PreCreate(CREATESTRUCT& cs)
{
    // Set the extended style
    cs.dwExStyle = WS_EX_CLIENTEDGE;
}

// Here we set the Window class parameters.
// Preforming this is optional, but doing so allows us to
// take more precise control over the type of window we create.
void CView::PreRegisterClass(WNDCLASS& wc)
{
    // Set the Window Class name
    wc.lpszClassName = L"Win32++ View";

    // Set a background brush to white
    wc.hbrBackground = static_cast<HBRUSH>(::GetStockObject(WHITE_BRUSH));

    // Set the default cursor
    wc.hCursor = ::LoadCursor(nullptr, IDC_ARROW);

    // Set the class style (not to be confused with the window styles set in PreCreate)
    wc.style = CS_DBLCLKS;  // Generate left button double click messages
}

// Reposition the child windows of the frame.
void CView::RecalcLayout()
{
    // Position the toolbar at the top, left, right or bottom of the view.
    int cxTB = m_toolBar.GetMaxSize().cx;
    int cyTB = m_toolBar.GetMaxSize().cy;
    int cxClient = GetClientRect().Width();
    int cyClient = GetClientRect().Height();

    DWORD style = m_toolBar.GetStyle();
    style &= CCS_VERT | CCS_BOTTOM; // Filter unwanted styles

    switch(style)
    {
    case CCS_LEFT:
        m_toolBar.SetWindowPos(HWND_TOP, 0, 0, cxTB, cyClient, SWP_SHOWWINDOW);
        SetWrapState(TRUE);
        m_toolBar.PressButton(IDM_LEFT, TRUE);
        break;
    case CCS_RIGHT:
        m_toolBar.SetWindowPos(HWND_TOP, cxClient - cxTB, 0, cxTB, cyClient, SWP_SHOWWINDOW);
        SetWrapState(TRUE);
        m_toolBar.PressButton(IDM_RIGHT, TRUE);
        break;
    case CCS_BOTTOM:
        m_toolBar.SetWindowPos(HWND_TOP, 0, cyClient - cyTB, cxClient, cyTB, SWP_SHOWWINDOW);
        SetWrapState(FALSE);
        m_toolBar.PressButton(IDM_BOTTOM, TRUE);
        break;
    default:
        m_toolBar.SetWindowPos(HWND_TOP, 0, 0, cxClient, cyTB, SWP_SHOWWINDOW);
        SetWrapState(FALSE);
        m_toolBar.PressButton(IDM_TOP, TRUE);
        break;
    }

    Invalidate();
}

// Set the wrap state for each toolbar button.
// Note: Vertical toolbars require each button to have TBSTATE_WRAP set.
//       Horizontal toolbars require the TBSTATE_WRAP removed.
void CView::SetWrapState(bool isWrapped)
{
    for (int i = 0; i < m_toolBar.GetButtonCount(); ++i)
    {
        int id = m_toolBar.GetCommandID(i);
        m_toolBar.SetButtonState(id, TBSTATE_ENABLED | (isWrapped ? TBSTATE_WRAP : 0));
    }
}

// All window messages for this window pass through WndProc.
LRESULT CView::WndProc(UINT msg, WPARAM wparam, LPARAM lparam)
{
    try
    {
        switch (msg)
        {
        case WM_SIZE:                     return OnSize(msg, wparam, lparam);
        case WM_DPICHANGED_BEFOREPARENT:  return OnDpiChangedBeforeParent(msg, wparam, lparam);
        }

        // Pass unhandled messages on for default processing.
        return WndProcDefault(msg, wparam, lparam);
    }

    // Catch all unhandled CException types.
    catch (const CException& e)
    {
        // Display the exception and continue.
        CString str1;
        str1 << e.GetText() << L'\n' << e.GetErrorString();
        CString str2;
        str2 << "Error: " << e.what();
        ::MessageBox(nullptr, str1, str2, MB_ICONERROR);
    }

    // Catch all unhandled std::exception types.
    catch (const std::exception& e)
    {
        // Display the exception and continue.
        CString str1 = e.what();
        ::MessageBox(nullptr, str1, L"Error: std::exception", MB_ICONERROR);
    }

    return 0;
}
