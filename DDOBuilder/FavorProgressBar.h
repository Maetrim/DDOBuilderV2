// FavorProgressBar.h
//
#pragma once
#include "InfoTip.h"
#include <vector>

class CFavorProgressBar : public CWnd
{
    DECLARE_DYNAMIC(CFavorProgressBar)
// Construction
public:
    CFavorProgressBar();
    virtual ~CFavorProgressBar();

    void SetPatronName(const CString& patronName);
    void SetFavorTiers(const std::vector<int>& favorTiers, int maxFavor);
    void SetCurrentFavor(int currentFavor);

protected:
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CFavorProgressBar)
    //}}AFX_VIRTUAL
    //{{AFX_MSG(CFavorProgressBar)
    afx_msg void OnPaint();
    afx_msg LRESULT OnMouseLeave(WPARAM wParam, LPARAM lParam);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()

    void ShowTip(CRect rctItem);
    void HideTip();

    CString m_patronName;
    std::vector<int> m_favorTiers;
    int m_maxFavor;
    int m_currentFavor;
    int m_maxValue;
    std::vector<int> m_favorTiersPoints;
    int m_favorHeight;
    int m_selectedItem;
    CInfoTip m_tooltip;
    bool m_showingTip;
    bool m_tipCreated;
};
