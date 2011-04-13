////////////////////////////////////////////////////
// Mainfrm.cpp  - definitions for the CMainFrame class

#include "mainfrm.h"
#include "resource.h"


CMainFrame::CMainFrame()
{
	// Set m_View as the view window of the frame
	SetView(m_View);
}

CMainFrame::~CMainFrame()
{
}

BOOL CMainFrame::OnCommand(WPARAM wParam, LPARAM lParam)
{
	// Process the messages from the Menu and Tool Bar
	switch (LOWORD(wParam))
	{
	case IDM_FILE_NEW:
		m_View.ClearPoints();
		m_PathName = _T("");
		return TRUE;
	case IDM_FILE_OPEN:
		OnFileOpen();
		return TRUE;
	case IDM_FILE_SAVE:
		OnFileSave();
		return TRUE;
	case IDM_FILE_SAVEAS:
		OnFileSaveAs();
		return TRUE;
	case IDM_FILE_PRINT:
		OnFilePrint();
		return TRUE;
	case IDM_PEN_RED:
		m_View.SetPen(RGB(255,0,0));
		return TRUE;
	case IDM_PEN_BLUE:
		m_View.SetPen(RGB(0,0,255));
		return TRUE;
	case IDM_PEN_GREEN:
		m_View.SetPen(RGB(0,196,0));
		return TRUE;
	case IDM_PEN_BLACK:
		m_View.SetPen(RGB(0,0,0));
		return TRUE;
	case IDM_HELP_ABOUT:
		OnHelp();
		return TRUE;
	case IDM_FILE_EXIT:
		::PostMessage(m_hWnd, WM_CLOSE, 0, 0);
		return TRUE;
	}

	return FALSE;
}

void CMainFrame::OnFileOpen()
{
	// Fill the OPENFILENAME structure
	TCHAR szFilters[] = _T("Scribble Files (*.dat)\0*.dat\0\0");
	TCHAR szFilePathName[_MAX_PATH] = _T("");
	OPENFILENAME ofn = {0};
	if (GetWinVersion() >= 2500)
	{
		// For Win2000 and above
		ofn.lStructSize = sizeof(OPENFILENAME);
	}
	else
	{
		// For older operating systems
		ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
	}

	ofn.hwndOwner = m_hWnd;
	ofn.lpstrFilter = szFilters;
	ofn.lpstrFile = szFilePathName;
	ofn.nMaxFile = _MAX_PATH;
	ofn.lpstrTitle = _T("Open File");
	ofn.Flags = OFN_FILEMUSTEXIST;

	// Bring up the dialog, and open the file
	if (!::GetOpenFileName(&ofn))
		return;

	// Retrieve the PlotPoint data
	m_View.FileOpen(szFilePathName);

	// Save the filename
	m_PathName = szFilePathName;
}

void CMainFrame::OnFileSave()
{
	if (m_PathName == _T(""))
		OnFileSaveAs();
	else
		m_View.FileSave(m_PathName.c_str());
}

void CMainFrame::OnFileSaveAs()
{
	// Fill the OPENFILENAME structure
	TCHAR szFilters[] = _T("Scribble Files (*.dat)\0*.dat\0\0");
	TCHAR szFilePathName[_MAX_PATH] = _T("");
	OPENFILENAME ofn = {0};
	if (GetWinVersion() >= 2500)
	{
		// For Win2000 and above
		ofn.lStructSize = sizeof(OPENFILENAME);
	}
	else
	{
		// For older operating systems
		ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
	}

	ofn.hwndOwner = m_hWnd;
	ofn.lpstrFilter = szFilters;
	ofn.lpstrFile = szFilePathName;
	ofn.lpstrDefExt = _T("dat");
	ofn.nMaxFile = _MAX_PATH;
	ofn.lpstrTitle = _T("Save File");
	ofn.Flags = OFN_OVERWRITEPROMPT;

	// Open the file save dialog, and open the file
	if (!::GetSaveFileName(&ofn))
		return;

	// Store the PLotPoint data in the file
	m_PathName = szFilePathName;

	// Save the file name
	m_View.FileSave(szFilePathName);
}

// Sends the bitmap extracted from the View window to a printer of your choice
// This function provides a useful reference for printing bitmaps in general
void CMainFrame::OnFilePrint()
{
	// Get the dimensions of the View window
	CRect rcView = m_View.GetClientRect();
	int Width = rcView.Width();
	int Height = rcView.Height();

	// Extract the bitmap from the View window
	CDC ViewDC = m_View.GetDC();
	CDC MemDC = CreateCompatibleDC(ViewDC);
	MemDC.CreateCompatibleBitmap(ViewDC, Width, Height);
	BitBlt(MemDC, 0, 0, Width, Height, ViewDC, 0, 0, SRCCOPY);
	HBITMAP hbmView = MemDC.DetachBitmap();

	// Bring up a dialog to choose the printer
	PRINTDLG pd = {0};
	pd.lStructSize = sizeof( pd );
	pd.Flags = PD_RETURNDC;

	// Retrieve the printer DC
	if( !PrintDlg( &pd ) )
	{
		TRACE(_T("PrintDlg canceled"));
		return;
	}

	// Zero and then initialize the members of a DOCINFO structure.
	DOCINFO di = {0};
	di.cbSize = sizeof(DOCINFO);
	di.lpszDocName = _T("Scribble Printout");
	di.lpszOutput = (LPTSTR) NULL;
	di.lpszDatatype = (LPTSTR) NULL;
	di.fwType = 0;

	// Begin a print job by calling the StartDoc function.
	if (SP_ERROR == StartDoc(pd.hDC, &di))
		throw CWinException(_T("Failed to start print job"));

	// Inform the driver that the application is about to begin sending data.
	if (0 > StartPage(pd.hDC))
		throw CWinException(_T("StartPage failed"));

	BITMAPINFOHEADER bi = {0};
	bi.biSize = sizeof(BITMAPINFOHEADER);
	bi.biHeight = Height;
	bi.biWidth = Width;
	bi.biPlanes = 1;
	bi.biBitCount =  24;
	bi.biCompression = BI_RGB;

	// Note: BITMAPINFO and BITMAPINFOHEADER are the same for 24 bit bitmaps
	// Get the size of the image data
	GetDIBits(MemDC, hbmView, 0, Height, NULL, (BITMAPINFO*)&bi, DIB_RGB_COLORS);

	// Retrieve the image data
	std::vector<byte> vBits(bi.biSizeImage, 0);;	// a vector to hold the byte array
	byte* pByteArray = &vBits.front();

	GetDIBits(MemDC, hbmView, 0, Height, pByteArray, (BITMAPINFO*)&bi, DIB_RGB_COLORS);

	// Determine the scaling factors required to print the bitmap and retain its original proportions.
    float fLogPelsX1 = (float) GetDeviceCaps(ViewDC, LOGPIXELSX);
    float fLogPelsY1 = (float) GetDeviceCaps(ViewDC, LOGPIXELSY);
	float fLogPelsX2 = (float) GetDeviceCaps(pd.hDC, LOGPIXELSX);
    float fLogPelsY2 = (float) GetDeviceCaps(pd.hDC, LOGPIXELSY);
	float fScaleX = MAX(fLogPelsX1, fLogPelsX2) / MIN(fLogPelsX1, fLogPelsX2);
	float fScaleY = MAX(fLogPelsY1, fLogPelsY2) / MIN(fLogPelsY1, fLogPelsY2);

    // Compute the coordinates of the upper left corner of the centered bitmap.
	int cWidthPels = GetDeviceCaps(pd.hDC, HORZRES);
    int xLeft = ((cWidthPels / 2) - ((int) (((float) Width) * fScaleX)) / 2);
    int cHeightPels = GetDeviceCaps(pd.hDC, VERTRES);
    int yTop = ((cHeightPels / 2) - ((int) (((float) Height) * fScaleY)) / 2);

    // Use StretchDIBits to scale the bitmap and maintain its original proportions
    if (GDI_ERROR == StretchDIBits(pd.hDC, xLeft, yTop, (int) ((float) Width * fScaleX),
		(int) ((float) Height * fScaleY), 0, 0, Width, Height, pByteArray, (BITMAPINFO*)&bi, DIB_RGB_COLORS, SRCCOPY))
	{
		throw CWinException(_T("Failed to resize image for printing"));
	}

	// Inform the driver that the page is finished.
	if (0 > EndPage(pd.hDC))
		throw CWinException(_T("EndPage failed"));

	// Inform the driver that document has ended.
	if(0 > EndDoc(pd.hDC))
		throw CWinException(_T("EndDoc failed"));

	// Cleanup
	::DeleteObject(hbmView);
}

void CMainFrame::SetupToolBar()
{
	// Set the Resource IDs for the toolbar buttons
	AddToolBarButton( IDM_FILE_NEW   );
	AddToolBarButton( IDM_FILE_OPEN  );
	AddToolBarButton( IDM_FILE_SAVE  );
	
	AddToolBarButton( 0 );				// Separator
	AddToolBarButton( IDM_EDIT_CUT );
	AddToolBarButton( IDM_EDIT_COPY );
	AddToolBarButton( IDM_EDIT_PASTE );
	
	AddToolBarButton( 0 );				// Separator
	AddToolBarButton( IDM_FILE_PRINT );
	
	AddToolBarButton( 0 );				// Separator
	AddToolBarButton ( IDM_PEN_RED    );	
	AddToolBarButton ( IDM_PEN_BLUE   );
	AddToolBarButton ( IDM_PEN_GREEN  );
	AddToolBarButton ( IDM_PEN_BLACK  );
	AddToolBarButton ( IDM_HELP_ABOUT );
}

LRESULT CMainFrame::WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
//	switch (uMsg)
//	{

//	} // switch (uMsg)

	return WndProcDefault(uMsg, wParam, lParam);
} // LRESULT CMainFrame::WndProc(...)
