/////////////////////////////
// Hyperlink.cpp
//

#include "stdafx.h"
#include "Hyperlink.h"

///////////////////////////////////
// CHyperlink function definitions.
//
CHyperlink::CHyperlink() : m_isUrlVisited(false), m_isClicked(false),
                           m_visitedColor(RGB(128, 0, 128)),
                           m_notVisitedColor(RGB(0,0,255))
{
    m_urlName = L"Win32++";

    // Create the hand cursor.
    m_hCursor = ::LoadCursor(nullptr, IDC_HAND);
}

void CHyperlink::OnAttach()
{
    NONCLIENTMETRICS ncm = GetNonClientMetrics();
    LOGFONT lf = ncm.lfStatusFont;
    lf.lfUnderline = TRUE;
    m_urlFont.CreateFontIndirect(lf);
}

LRESULT CHyperlink::OnLButtonDown(UINT msg, WPARAM wparam, LPARAM lparam)
{
    SetCapture();
    m_isClicked = TRUE;

    return FinalWindowProc(msg, wparam, lparam);
}

LRESULT CHyperlink::OnLButtonUp(UINT msg, WPARAM wparam, LPARAM lparam)
{
    ReleaseCapture();
    if(m_isClicked)
    {
        m_isClicked = FALSE;
        CPoint pt;
        pt.x = LOWORD(lparam);
        pt.y = HIWORD(lparam);
        ClientToScreen(pt);
        CRect rc = GetWindowRect();

        if (rc.PtInRect(pt))
            OpenUrl();
    }

    return FinalWindowProc(msg, wparam, lparam);
}

void CHyperlink::OpenUrl()
{
    CString url("http://sourceforge.net/projects/win32-framework/");

    HINSTANCE result = ShellExecute(nullptr, L"open", url, nullptr, nullptr, SW_SHOWNORMAL);

    if (reinterpret_cast<INT_PTR>(result) > 32)
    {
        m_isUrlVisited = TRUE;

        // Redraw the StatusBar to update the color.
        GetParent().RedrawWindow();
    }
}

LRESULT CHyperlink::OnMessageReflect(UINT msg, WPARAM wparam, LPARAM)
{
    // Messages such as WM_CTLCOLORSTATIC are reflected back to the CWnd object that created them.
    if (msg ==  WM_CTLCOLORSTATIC)
    {
        CDC dc(reinterpret_cast<HDC>(wparam));
        dc.SetTextColor( m_isUrlVisited? m_visitedColor : m_notVisitedColor);
        dc.SetBkMode(TRANSPARENT);
        m_urlFont = DpiScaleFont(m_urlFont, 9);
        dc.SelectObject(m_urlFont);
        dc.DrawText(m_urlName, -1, GetClientRect(), DT_VCENTER | DT_SINGLELINE);

        // Keep our changes.
        dc.Detach();

        // Return a nullptr brush for a transparent background
        return (LRESULT)GetStockObject(NULL_BRUSH);
    }
    return 0;
}

LRESULT CHyperlink::OnSetCursor(UINT, WPARAM, LPARAM)
{
    // Must use ::SetCursor here. CStatic::SetCursor does not do the same thing.
    ::SetCursor(m_hCursor);

    return 1;  // Non-zero return prevents default processing
}

LRESULT CHyperlink::WndProc(UINT msg, WPARAM wparam, LPARAM lparam)
{
    try
    {
        switch (msg)
        {
        case WM_LBUTTONDOWN:  return OnLButtonDown(msg, wparam, lparam);
        case WM_LBUTTONUP:    return OnLButtonUp(msg, wparam, lparam);
        case WM_SETCURSOR:    return OnSetCursor(msg, wparam, lparam);
        case WM_NCHITTEST:    return HTCLIENT;      // Claim that the mouse is in a client area
        }

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
