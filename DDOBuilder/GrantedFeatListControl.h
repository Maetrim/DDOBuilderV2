#pragma once
#include "Character.h"
#include "InfoTip.h"

struct Section
{
    CString name;
    std::list<Feat> feats;
};

class CGrantedFeatListControl :
    public CWnd
{
    DECLARE_DYNAMIC(CGrantedFeatListControl)

public:
    CGrantedFeatListControl();
    virtual ~CGrantedFeatListControl();

    void SetupControl();
    void SetCharacter(Character* pCharacter);
    CSize RequiredSize();

    void Clear();
    void AddSection(const CString& name, const std::list<Feat>& feats);

protected:
    afx_msg void OnPaint();
    afx_msg LRESULT OnMouseLeave(WPARAM wParam, LPARAM lParam);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    DECLARE_MESSAGE_MAP()

private:
    size_t DrawSection(CDC* pDC, size_t iSection, size_t top);
    void ShowTip(CRect rctItem);
    void HideTip();

    Character* m_pCharacter;
    CSize m_headerItemSize;
    CSize m_featItemSize;
    std::vector<Section> m_sections;
    int m_selectedSection;
    int m_selectedSectionItem;
    // feat tooltip
    CInfoTip m_tooltip;
    bool m_showingTip;
    bool m_tipCreated;
};


