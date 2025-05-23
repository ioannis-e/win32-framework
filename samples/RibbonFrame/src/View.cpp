/////////////////////////////
// View.cpp
//

#include "stdafx.h"
#include "View.h"
#include "Resource.h"

//////////////////////////////
// CView function definitions.
//
CView::CView() : m_penColor(RGB(0,0,0))
{
    m_brush.CreateSolidBrush(RGB(255,255,230));
}

void CView::DrawLine(int x, int y)
{
    CClientDC clientDC(*this);
    clientDC.CreatePen(PS_SOLID, 1, GetAllPoints().back().color);
    clientDC.MoveTo(GetAllPoints().back().x, GetAllPoints().back().y);
    clientDC.LineTo(x, y);
}

CDoc& CView::GetDoc()
{
    return m_doc;
}

std::vector<PlotPoint>& CView::GetAllPoints()
{
    return GetDoc().GetAllPoints();
}

int CView::OnCreate(CREATESTRUCT&)
{
    // Support Drag and Drop on this window.
    DragAcceptFiles(TRUE);
    return 0;
}

void CView::OnDraw(CDC& dc)
{
    // Here we use double buffering (drawing to a memory DC) for smoother rendering
    // Set up our Memory DC and bitmap.
    CMemDC memDC(dc);
    int width = GetClientRect().Width();
    int height = GetClientRect().Height();
    memDC.CreateCompatibleBitmap(dc, width, height);
    memDC.FillRect(GetClientRect(), m_brush);

    // Start with the pen up.
    bool isDrawing = false;

    // Draw the lines.
    for (const PlotPoint& p : GetAllPoints())
    {
        memDC.CreatePen(PS_SOLID, 1, p.color);

        if (isDrawing)
            memDC.LineTo(p.x, p.y);
        else
            memDC.MoveTo(p.x, p.y);

        isDrawing = p.isPenDown;
    }

    // Copy from the memory DC to our painting dc.
    dc.BitBlt(0, 0, width, height, memDC, 0, 0, SRCCOPY);
}

LRESULT CView::OnDropFiles(UINT, WPARAM wparam, LPARAM)
{
    HDROP hDrop = (HDROP)wparam;
    UINT length = DragQueryFile(hDrop, 0, 0, 0);

    if (length > 0)
    {
        CString FileName;
        DragQueryFile(hDrop, 0, FileName.GetBuffer(length), length +1);
        FileName.ReleaseBuffer();

        // Send a user defined message to the frame window.
        GetParent().SendMessage(UWM_DROPFILE, (WPARAM)FileName.c_str(), 0);
    }

    DragFinish(hDrop);
    return 0;
}

LRESULT CView::OnLButtonDown(UINT msg, WPARAM wparam, LPARAM lparam)
{
    // Capture mouse input.
    SetCapture();
    GetDoc().StorePoint(GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam), true, m_penColor);
    return FinalWindowProc(msg, wparam, lparam);
}

LRESULT CView::OnLButtonUp(UINT msg, WPARAM wparam, LPARAM lparam)
{
    // Release the capture on the mouse.
    ReleaseCapture();
    GetDoc().StorePoint(GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam), false, m_penColor);
    return FinalWindowProc(msg, wparam, lparam);
}

LRESULT CView::OnMouseMove(UINT msg, WPARAM wparam, LPARAM lparam)
{
    // Hold down the left mouse button and move mouse to draw lines.
    if ( (wparam & MK_LBUTTON) && (GetCapture() == *this) )
    {
        CString str;
        str.Format( L"Draw Point:  %hd, %hd\n", GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam) );
        TRACE(str);

        DrawLine(GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam));
        GetDoc().StorePoint(GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam), true, m_penColor);
    }

    return FinalWindowProc(msg, wparam, lparam);
}

void CView::PreCreate(CREATESTRUCT& cs)
{
    // Set the extra style to provide a sunken effect.
    cs.dwExStyle = WS_EX_CLIENTEDGE;
}

void CView::PreRegisterClass(WNDCLASS& wc)
{
    // Set the background brush, class name and cursor.
    wc.hbrBackground = m_brush;
    wc.lpszClassName = L"Scribble Window";
    wc.hCursor = GetApp()->LoadCursor(IDC_CURSOR1);
}

// Sends the bitmap extracted from the View window to a printer of your choice.
// This function provides a useful reference for printing bitmaps in general.
// This function throws an exception if unable to print.
void CView::Print()
{
    // Get the dimensions of the View window.
    CRect viewRect = GetClientRect();
    int width = viewRect.Width();
    int height = viewRect.Height();

    // Copy the bitmap from the View window.
    CClientDC viewDC(*this);
    CMemDC memDC(viewDC);
    memDC.CreateCompatibleBitmap(viewDC, width, height);
    BitBlt(memDC, 0, 0, width, height, viewDC, 0, 0, SRCCOPY);
    CBitmap bmView = memDC.DetachBitmap();
    CPrintDialog printDlg;

    // Bring up a dialog to choose the printer.
    if (printDlg.DoModal(*this) == IDOK)    // Throws exception if there is no default printer.
    {
        // Zero and then initialize the members of a DOCINFO structure.
        DOCINFO di;
        memset(&di, 0, sizeof(DOCINFO));
        di.cbSize = sizeof(DOCINFO);
        di.lpszDocName = L"Scribble Printout";

        // Begin a print job by calling the StartDoc function.
        CDC printDC = printDlg.GetPrinterDC();

        printDC.StartDoc(&di);

        // Inform the driver that the application is about to begin sending data.
        printDC.StartPage();

        BITMAPINFOHEADER bih{};
        bih.biSize = sizeof(bih);
        bih.biHeight = height;
        bih.biWidth = width;
        bih.biPlanes = 1;
        bih.biBitCount = 24;
        bih.biCompression = BI_RGB;

        // Note: BITMAPINFO and BITMAPINFOHEADER are the same for 24 bit bitmaps.
        // Get the size of the image data
        BITMAPINFO* pBI = reinterpret_cast<BITMAPINFO*>(&bih);
        VERIFY(memDC.GetDIBits(bmView, 0, height, nullptr, pBI, DIB_RGB_COLORS));

        // Retrieve the image data
        std::vector<byte> vBits(bih.biSizeImage, 0); // A vector to hold the byte array.
        byte* pByteArray = vBits.data();
        VERIFY(memDC.GetDIBits(bmView, 0, height, pByteArray, pBI, DIB_RGB_COLORS));

        // Determine the scaling factors required to print the bitmap and retain its original proportions.
        double viewPixelsX = double(viewDC.GetDeviceCaps(LOGPIXELSX));
        double viewPixelsY = double(viewDC.GetDeviceCaps(LOGPIXELSY));
        double printPixelsX = double(printDC.GetDeviceCaps(LOGPIXELSX));
        double printPixelsY = double(printDC.GetDeviceCaps(LOGPIXELSY));
        double scaleX = printPixelsX / viewPixelsX;
        double scaleY = printPixelsY / viewPixelsY;

        int scaledWidth = int(width * scaleX);
        int scaledHeight = int(height * scaleY);

        // Use StretchDIBits to scale the bitmap and maintain its original proportions.
        VERIFY(printDC.StretchDIBits(0, 0, scaledWidth, scaledHeight, 0, 0,
               width, height, pByteArray, pBI, DIB_RGB_COLORS, SRCCOPY));

        // Inform the driver that the page is finished.
        printDC.EndPage();

        // Inform the driver that document has ended.
        printDC.EndDoc();
    }
}

LRESULT CView::WndProc(UINT msg, WPARAM wparam, LPARAM lparam)
{
    try
    {
        switch (msg)
        {
        case WM_DROPFILES:      return OnDropFiles(msg, wparam, lparam);
        case WM_LBUTTONDOWN:    return OnLButtonDown(msg, wparam, lparam);
        case WM_MOUSEMOVE:      return OnMouseMove(msg, wparam, lparam);
        case WM_LBUTTONUP:      return OnLButtonUp(msg, wparam, lparam);
        }

        // Use the default message handling for remaining messages.
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
