// FavorProgressBar.h
//
#pragma once
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
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()

    CString m_patronName;
    std::vector<int> m_favorTiers;
    int m_maxFavor;
    int m_currentFavor;
    int m_maxValue;
};
