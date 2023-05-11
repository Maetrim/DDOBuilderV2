#pragma once
#include "Character.h"
#include "InfoTip.h"

class CSkillSpendControl :
    public CWnd,
    public CharacterObserver,
    public LifeObserver,
    public BuildObserver
{
    DECLARE_DYNAMIC(CSkillSpendControl)

public:
    CSkillSpendControl();
    virtual ~CSkillSpendControl();

    void SetCharacter(Character * pCharacter);
    CSize RequiredSize();

protected:
    afx_msg void OnPaint();
    afx_msg LRESULT OnMouseLeave(WPARAM wParam, LPARAM lParam);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
    DECLARE_MESSAGE_MAP()

private:
    void SetupControl();
    void DrawButtons(CDC* pDC);
    void DrawButton(CDC* pDC, CRect& rctItem, const CString& text, bool bSelected);
    size_t DrawAvailableSkillPoints(CDC* pDC, size_t top);
    size_t DrawClassImages(CDC* pDC, size_t top);
    size_t DrawLevelLine(CDC* pDC, size_t top);
    size_t DrawSkillLine(CDC* pDC, SkillType skill, size_t top);
    size_t DrawRemainingSkillPoints(CDC* pDC, size_t top);
    void DrawSelection(CDC* pDC);
    void MaxThisSkill(SkillType skill);
    void ClearThisSkill(SkillType skill);
    void AutoBuySkills();
    void ClearAllSkills();

    // Character observer
    virtual void UpdateActiveBuildChanged(Character *) override;

    // Life observer
    virtual void UpdateAbilityTomeChanged(Life*, AbilityType) override;
    virtual void UpdateRaceChanged(Life*, const std::string&) override;

    // Build Observer
    virtual void UpdateBuildLevelChanged(Build *) override;
    virtual void UpdateClassChanged(
            Build *,
            const std::string& classFrom,
            const std::string& classTo,
            size_t level) override;

    Character * m_pCharacter;
    CSize m_skillNameSize;
    CSize m_skillColumnSize;
    CSize m_skillTotalSize;
    CSize m_skillTomeSize;
    int m_selectedRow;
    int m_selectedColumn;
    int m_highlightedColumn;
    bool m_bSkillSelected;
    bool m_bAutoBuy;
    bool m_bClearSkills;
    SkillType m_highlightSkill;
    CRect m_rctSkillNames;
    CRect m_rctSkills;
    CRect m_rctLevels;
    CRect m_rctTomeTitle;
    CRect m_rctTomes;
    CRect m_rctAutoBuy;
    CRect m_rctClearSkills;
    size_t m_maxSkillTome;
};


