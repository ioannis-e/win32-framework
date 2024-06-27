/////////////////////////////
// Splash.h
//

#ifndef SPLASH_H
#define SPLASH_H

// Declare min and max for older versions of Visual Studio
#if defined (_MSC_VER) && (_MSC_VER < 1920) // < VS2019
using std::min;
using std::max;
#endif

#include <gdiplus.h>
#include <memory>

typedef std::unique_ptr<Gdiplus::Bitmap> BitmapPtr;

///////////////////////////////////////////////////////////////////////
// CSplash creates a splash screen with optional text and progress bar.
// The splash screen is shown to indicate time consuming tasks, and
// hidden when they're complete.
class CSplash : public CWnd
{
public:
    CSplash();
    virtual ~CSplash();

    const CProgressBar& GetBar() const { return m_progress; }

    void AddBar();
    void Hide();
    void ShowText(LPCTSTR text, CWnd* parent);

protected:
    // Virtual functions that override base class functions
    virtual int  OnCreate(CREATESTRUCT& cs);
    virtual void OnDraw(CDC& dc);
    virtual void PreCreate(CREATESTRUCT& cs);
    virtual void PreRegisterClass(WNDCLASS& wc);
    virtual LRESULT WndProc(UINT msg, WPARAM wparam, LPARAM lparam);

private:
    CSplash(const CSplash&);               // Disable copy construction
    CSplash& operator=(const CSplash&);    // Disable assignment operator

    void LoadFont();
    BitmapPtr LoadPngResource(UINT id);
    void RemoveBar();

    CProgressBar m_progress;
    CString      m_text;
    HICON        m_hIcon;
    HANDLE       m_fontHandle;
    ULONG_PTR   m_gdiplusToken;
};


#endif // SPLASH_H
