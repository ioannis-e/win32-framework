/* (26-Mar-2025)                                                 (PrintInfo.h) *
********************************************************************************
|                                                                              |
|                    Authors: Robert Tausworthe, David Nash                    |
|                                                                              |
===============================================================================*

    Contents Description: Declaration of the CPrintInfo class for
    applications using the Win32++ Windows interface classes. This class is
    similar to that bearing the same name in the Microsoft Foundation Classes
    (MFC), with noted exceptions.

*******************************************************************************/

#ifndef CPRINTINFO_H
#define CPRINTINFO_H

// This class encapsulates the services provided by the Windows common dialog
// box for printing. It emulates and largely uses the same notation as its
// MFC counterpart.
class CPrintInfo
{
public:
        CPrintInfo();
        ~CPrintInfo() = default;

    UINT    GetFromPage() const         { return m_nFromPage;}
    UINT    GetMaxPage() const          { return m_nMaxPage; }
    UINT    GetMinPage() const          { return m_nMinPage; }
    UINT    GetNCopies() const          { return m_nCopies; }
    UINT    GetToPage() const           { return m_nToPage; }
    void    InitInfo(CPrintDialog*, UINT, UINT, UINT, UINT, UINT);
    void    SetFromPage(UINT nFromPage) { m_nFromPage = nFromPage; }
    void    SetMaxPage(UINT nMaxPage)   { m_nMaxPage = nMaxPage; }
    void    SetMinPage(UINT nMinPage)   { m_nMinPage = nMinPage; }
    void    SetNCopies(UINT nCopies)    { m_nCopies = nCopies; }
    void    SetToPage(UINT nToPage)     { m_nToPage = nToPage; }

    // public data
    CPrintDialog* m_pPD;          // pointer to print dialog
    BOOL    m_bDocObject;         // TRUE if a DocObject (not supported)
    BOOL    m_bDirect;            // TRUE if bypassing Print Dialog
    BOOL    m_bPreview;           // TRUE if in preview mode
    BOOL    m_bContinuePrinting;  // set FALSE to end printing
    UINT    m_nCopies;            // number of copies (not supported)
    UINT    m_nFromPage;          // first printed page, 1 based
    UINT    m_nMaxPage;           // maximum page limit
    UINT    m_nMinPage;           // minimum page limit
    UINT    m_nToPage;            // last printed page
    UINT    m_nCurPage;           // current page
    UINT    m_nNumPreviewPages;   // (not supported)
    LPVOID  m_lpUserData;         // pointer to user created struct
    CRect   m_rectDraw;           // rect of current usable page area
    CString m_strPageDesc;        // format string for page number display
    UINT    m_nMargin;            // page margin, in twips

    // these only valid if m_bDocObject: currently not supported
    DWORD   m_dwFlags;            // flags for DocObject print operations
    UINT    m_nOffsetPage;        // first page offset in combined Doc job
};

#endif // CPRINTINFO_H
