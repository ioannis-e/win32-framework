/* (26-Mar-2025)                                             (CRichEditView.h) *
********************************************************************************
|                                                                              |
|                    Authors: Robert Tausworthe, David Nash                    |
|                                                                              |
===============================================================================*

    Contents Description: Declaration of the CRichEditView class for the
    CommonDialogs sample application using the Win32++ Windows interface
    classes. This class serves as both the view object as well as the
    document repository and manager. In similarity to Microsoft Foundation
    Classes (MFC) architecture it also contains the print loop.

*******************************************************************************/

#ifndef CRICHEDITVIEW_H
#define CRICHEDITVIEW_H

#include "PrintInfo.h"

class CRichEditView : public CRichEdit
{
public:
    CRichEditView();
    virtual ~CRichEditView() override = default;

    void    Clean();
    void    DoPrintRichView(LPCWSTR);
    void    DoPrintView();
    BOOL    IsSelected();
    void    SetColors(COLORREF txfg, COLORREF txbg, COLORREF bg);
    void    SetFont(HFONT hFont, BOOL bRedraw) const;
    void    SetPrintPath(LPCWSTR s) { m_printPath = s;}
    void    SetWrapping(int wrap);
    BOOL    StreamInFile(const CFile& file);
    BOOL    StreamOutFile(const CFile& file);

protected:
    virtual void OnAttach() override;

private:
    CRichEditView(const CRichEditView&) = delete;
    CRichEditView& operator=(const CRichEditView&) = delete;

    BOOL    DoPreparePrinting(CPrintInfo& info);
    void    GetPageBreaks(CPrintInfo&);
    void    OnBeginPrinting(CDC& DC, CPrintInfo& info);
    void    OnEndPrinting(CDC& DC, CPrintInfo& info);
    void    OnPrepareDC(CDC& DC, CPrintInfo& info);
    BOOL    OnPreparePrinting(CPrintInfo& info);
    void    OnPrint(CDC& DC, CPrintInfo& info);
    void    SetAppBanding(BOOL b) { m_isAppBanded = b;}

    static  DWORD CALLBACK StreamInCallback(DWORD dwCookie, LPBYTE pbBuff,
        LONG cb, LONG *pcb);
    static  DWORD CALLBACK StreamOutCallback(DWORD dwCookie, LPBYTE pbBuff,
        LONG cb, LONG *pcb);

    long    m_textLength;
    CString m_docPath;                // document path name
    CString m_printPath;              // print-to-file path (not supported)
    CString m_dataType;               // DOCINFO data type for the print job
    BOOL    m_isAppBanded;            // TRUE if app banding supported
    FORMATRANGE m_fr;                 // rich edit format range struct
    std::vector<int> m_firstPageChar; // first character of each page
};

#endif  // CRICHEDITVIEW_H
