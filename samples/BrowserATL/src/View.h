/////////////////////////////
// View.h


#ifndef VIEW_H
#define VIEW_H

#include <atlbase.h>            // ATL support
#include <atlcom.h>             // ATL COM support
#include <atlhost.h>            // ATL ActiveX support
#include <exdispid.h>           // ATL IDispatchID helper
#include <exdisp.h>             // ATL WebBrowser2 helper


////////////////////////////////////////////////////////////
// CView uses ATL to create a web browser. ATL is then used
// to connect a dispatch sink to the browser.
// The dispatch sink uses IDispatch to forward browser events
// via its invoke function.
//
class CView : public CWnd
{
    // A private class that uses CAtlExeModuleT to initialize ATL.
    // This implementation requires ATL version 10.0 (VS2010) or later.
    // It eliminates the need for a global CComModule _Module variable.
    // CAtlExeModuleT replaces CComModule which is deprecated.
    class MyModule : public CAtlExeModuleT <MyModule> {};

public:
    CView();
    virtual ~CView() override;
    virtual IWebBrowser2* GetIWebBrowser2() {return m_pInetExplorer;}
    virtual void Navigate(LPCWSTR str);

protected:
    virtual int  OnCreate(CREATESTRUCT& cs) override;
    virtual void OnDestroy() override;
    virtual void OnInitialUpdate() override;
    virtual void PreCreate(CREATESTRUCT& cs) override;

private:
    CView(const CView&) = delete;
    CView& operator=(const CView&) = delete;

    MyModule m_module;                  // Instanciates (initializes) ATL
    IWebBrowser2* m_pInetExplorer;      // pointer to IWebBrowser2
    LPUNKNOWN m_pSourceUnk;             // pointer to IUnknown for the ActiveX control
    LPUNKNOWN m_pSinkUnk;               // pointer to IUnknown for CDispatchSink
    DWORD m_adviseCookie;               // The cookie returned by AtlAdvise.
};

#endif
