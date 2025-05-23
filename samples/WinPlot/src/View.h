/////////////////////////////
// View.h
//

#ifndef VIEW_H
#define VIEW_H

#include "Calc.h"
#include "InputDlg.h"


//////////////////////////////////////////
// CView manages CMainFrame's view window.
class CView : public CWnd
{
public:
    struct PointData
    {
        PointData(double x, double y, int status) : x(x), y(y), status(status)
        {}
        PointData() : x(0), y(0), status(0) {}

        double x;
        double y;
        int status;
    };

    CView();
    virtual ~CView() override = default;

    Calc::Calculator& GetCalc() { return m_calc; }
    CInputDlg& GetInput() { return m_inputDlg; }

protected:
    virtual void OnDraw(CDC& dc) override;
    virtual void OnInitialUpdate() override;
    virtual void PreCreate(CREATESTRUCT& cs) override;
    virtual void PreRegisterClass(WNDCLASS& wc) override;
    virtual LRESULT WndProc(UINT msg, WPARAM wparam, LPARAM lparam) override;

private:
    CView(const CView&) = delete;
    CView& operator=(const CView&) = delete;

    void CalcPoints(double xmin, double xmax);
    void DoPlot(CDC& dc);
    void DrawLabel(CDC& dc);
    double GetXMin() const { return m_inputDlg.GetMin(); }
    double GetXMax() const { return m_inputDlg.GetMax(); }
    void DrawXAxis(CDC& dc, double xnorm, double ynorm, double xoffset, double yoffset);
    void DrawYAxis(CDC& dc, double xnorm, double ynorm, double xoffset, double yoffset);
    void PlotFunction(CDC& dc, double xnorm, double ynorm, double xoffset, double yoffset);
    void PrepareDC(CDC& dc);

    // Member variables
    Calc::Calculator m_calc;
    std::vector<PointData> m_points;  // vector of Data, stores x, y, & status
    CInputDlg m_inputDlg;

    double m_ymin;
    double m_ymax;
};


#endif // VIEW_H
