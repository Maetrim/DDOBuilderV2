#pragma once
#include "Character.h"
#include "InfoTip.h"

class CAutomaticFeatListControl :
    public CWnd
{
    DECLARE_DYNAMIC(CAutomaticFeatListControl)

public:
    CAutomaticFeatListControl();
    virtual ~CAutomaticFeatListControl();

    void SetCharacter(Character* pCharacter);
    CSize RequiredSize();
    void SetAutomaticFeats(
            const std::string& label,
            const std::list<TrainedFeat>& feats,
            int level);

protected:
    afx_msg void OnPaint();
    afx_msg LRESULT OnMouseLeave(WPARAM wParam, LPARAM lParam);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    DECLARE_MESSAGE_MAP()

private:
    void SetupControl();
    void ShowTip(CRect rctItem);
    void HideTip();

    Character* m_pCharacter;
    CSize m_headerItemSize;
    CSize m_featItemSize;
    std::string m_label;
    std::list<TrainedFeat> m_feats;
    int m_level;
    int m_selectedItem;
    // feat tooltip
    CInfoTip m_tooltip;
    bool m_showingTip;
    bool m_tipCreated;
};


