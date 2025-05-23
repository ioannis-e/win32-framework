/////////////////////////////
// View.cpp
//

#include "stdafx.h"
#include "View.h"
#include "Resource.h"
#include "UserMessages.h"

////////////////////////////
// CView function definition
//
CView::CView() : m_penColor(RGB(0,0,0))
{
    m_brush.CreateSolidBrush(RGB(255,255,230));
}

void CView::DrawLine(int x, int y)
{
    CClientDC clientDC(*this);
    std::vector<PlotPoint>& pp = *GetAllPoints();

    clientDC.CreatePen(PS_SOLID, 1, pp.back().penColor);
    clientDC.MoveTo(pp.back().x, pp.back().y);
    clientDC.LineTo(x, y);
}

std::vector<PlotPoint>* CView::GetAllPoints()
{
    LRESULT plotPoints = GetAncestor().SendMessage(UWM_GETALLPOINTS, 0, 0);
    assert(plotPoints);
    return reinterpret_cast<std::vector<PlotPoint>*>(plotPoints);
}

int CView::OnCreate(CREATESTRUCT&)
{
    // Support Drag and Drop on this window
    DragAcceptFiles(TRUE);
    return 0;
}

void CView::OnDraw(CDC& dc)
{
    // Here we use double buffering (drawing to a memory DC) for smoother rendering
    // Set up our Memory DC and bitmap
    CMemDC memDC(dc);
    int width = GetClientRect().Width();
    int height = GetClientRect().Height();
    memDC.CreateCompatibleBitmap(dc, width, height);
    memDC.FillRect(GetClientRect(), m_brush);

    // Start with the pen up.
    bool isPenDown = false;

    // Draw the lines.
    for (const PlotPoint& p : *GetAllPoints())
    {
        if (isPenDown)
            dc.LineTo(p.x, p.y);
        else
            dc.MoveTo(p.x, p.y);

        isPenDown = p.isPenDown;
    }

    // Copy from the memory DC to our painting dc
    dc.BitBlt(0, 0, width, height, memDC, 0, 0, SRCCOPY);
}

LRESULT CView::OnDropFiles(UINT, WPARAM wparam, LPARAM)
{
    HDROP hDrop = (HDROP)wparam;
    UINT length = DragQueryFile(hDrop, 0, 0, 0);

    if (length > 0)
    {
        CString fileName;
        DragQueryFile(hDrop, 0, fileName.GetBuffer(length), length+1);
        fileName.ReleaseBuffer();

        // Send a user defined message to the frame window
        GetAncestor().SendMessage(UWM_DROPFILE, (WPARAM)fileName.c_str(), 0);

    }

    DragFinish(hDrop);
    return 0;
}

LRESULT CView::OnLButtonDown(UINT msg, WPARAM wparam, LPARAM lparam)
{
    // Capture mouse input.
    SetCapture();
    SendPoint(GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam), true);
    return FinalWindowProc(msg, wparam, lparam);
}

LRESULT CView::OnLButtonUp(UINT msg, WPARAM wparam, LPARAM lparam)
{
    //Release the capture on the mouse
    ReleaseCapture();
    SendPoint(GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam), false);
    return FinalWindowProc(msg, wparam, lparam);
}

LRESULT CView::OnMouseActivate(UINT msg, WPARAM wparam, LPARAM lparam)
// Respond to a mouse click on the window
{
    // Set window focus. The docker will now report this as active.
    SetFocus();
    return FinalWindowProc(msg, wparam, lparam);
}

LRESULT CView::OnMouseMove(UINT msg, WPARAM wparam, LPARAM lparam)
{
    // hold down the left mouse button and move mouse to draw lines.
    if ( (wparam & MK_LBUTTON) && (GetCapture() == *this) )
    {
        CString str;
        str.Format( L"Draw Point:  %hd, %hd\n", GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam) );
        TRACE(str);

        DrawLine(GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam));
        SendPoint(GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam), true);
    }

    return FinalWindowProc(msg, wparam, lparam);
}

void CView::PreCreate(CREATESTRUCT& cs)
{
    // Set the extra style to provide a sunken effect
    cs.dwExStyle = WS_EX_CLIENTEDGE;
}

void CView::PreRegisterClass(WNDCLASS& wc)
{
    // Set the background brush, class name and cursor
    wc.hbrBackground = m_brush;
    wc.lpszClassName = L"Scribble Window";
    wc.hCursor = GetApp()->LoadCursor(IDC_CURSOR1);
}

void CView::SendPoint(int x, int y, bool PenDown)
{
    PlotPoint pp;
    pp.x = x;
    pp.y = y;
    pp.isPenDown = PenDown;
    pp.penColor = m_penColor;
    GetAncestor().SendMessage(UWM_SENDPOINT, (WPARAM)&pp, 0);
}

LRESULT CView::WndProc(UINT msg, WPARAM wparam, LPARAM lparam)
{
    try
    {
        switch (msg)
        {
        case WM_DROPFILES:      return OnDropFiles(msg, wparam, lparam);
        case WM_LBUTTONDOWN:    return OnLButtonDown(msg, wparam, lparam);
        case WM_MOUSEACTIVATE:  return OnMouseActivate(msg, wparam, lparam);
        case WM_MOUSEMOVE:      return OnMouseMove(msg, wparam, lparam);
        case WM_LBUTTONUP:      return OnLButtonUp(msg, wparam, lparam);
        }

        //Use the default message handling for remaining messages
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
