/////////////////////////////
// View.h
//

#ifndef VIEW_H
#define VIEW_H


//////////////////////////////////////////
// CView manages CMainFrame's view window.
class CView : public CWnd
{
public:
    CView();
    virtual ~CView();

    void CreateImageList(int imageWidth, int imageHeight, int colorBits);
    CBitmap GetBitmapFromImageList() const;
    int  GetColorBits() const { return m_colorBits; }
    COLORREF GetMask() const { return m_mask; }
    int  GetPressedButtons() const;
    bool IsMasked() const { return m_isMasked; }
    bool ImageAdd(const CString& pathName);
    bool ImageDelete();
    bool ImageSwap();
    bool LoadToolbar(const CString& fileName);
    bool NewToolbar(int imageSize, int colorBits);
    bool SaveToolbarImage(const CString& fileName);
    void SetBackground(COLORREF background) { m_background = background; RecalcLayout(); }
    void SetMask(COLORREF mask) { m_mask = mask; UpdateToolbar(); }
    void SetToolbarButtons();
    void SetToolbarImageList();
    bool UseMask(bool enable);

protected:
    virtual int     OnCreate(CREATESTRUCT& cs) override;
    virtual void    OnDraw(CDC& dc) override;
    virtual BOOL    OnEraseBkgnd(CDC&) override;
    virtual void    OnInitialUpdate() override;
    virtual LRESULT OnNotify(WPARAM wparam, LPARAM lparam) override;
    virtual void    PreCreate(CREATESTRUCT& cs) override;
    virtual LRESULT WndProc(UINT msg, WPARAM wparam, LPARAM lparam) override;

private:
    CView(const CView&) = delete;
    CView& operator=(const CView&) = delete;
    void RecalcLayout();
    CBitmap ResizeBitmap(const CBitmap& bitmap, double xScale, double yScale);
    void UpdateToolbar();
    bool WriteBitmapToFile(const CBitmap& bitmap, const CString& fileName);

    // Message handlers.
    LRESULT OnDpiChanged(UINT, WPARAM, LPARAM);
    LRESULT OnSize(UINT, WPARAM, LPARAM);

    CPageScroller m_pageScroller;
    CToolBar m_toolbar;
    CImageList m_toolbarImages;
    ULONG_PTR m_gdiplusToken;
    COLORREF m_background;
    COLORREF m_mask;
    int m_colorBits;
    bool m_isMasked;
};


#endif // VIEW_H
