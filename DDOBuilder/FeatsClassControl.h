#pragma once
#include "Character.h"
#include "InfoTip.h"
#include "ComboBoxTooltip.h"

enum HitType
{
    HT_None = 0,
    HT_Class1,
    HT_Class2,
    HT_Class3,
    HT_LevelClass1,
    HT_LevelClass2,
    HT_LevelClass3,
    HT_Feat,
};

class HitCheckItem
{
public:
    HitCheckItem(HitType type, const CRect & rect, size_t level, size_t data) :
        m_type(type), m_rect(rect), m_level(level), m_data(data) {};
     ~HitCheckItem() {};

     bool PointInRect(const CPoint & pt) const { return (m_rect.PtInRect(pt) != 0); }
     CRect Rect() const { return m_rect; }
     HitType Type() const { return m_type; }
     size_t Level() const { return m_level; }
     size_t Data() const { return m_data; }
private:
    HitType m_type;
    CRect m_rect;
    size_t m_level;
    size_t m_data;
};

class CFeatsClassControl :
    public CWnd,
    public CharacterObserver,
    public LifeObserver,
    public BuildObserver
{
    DECLARE_DYNAMIC(CFeatsClassControl)

public:
    CFeatsClassControl();
    virtual ~CFeatsClassControl();

    void SetCharacter(Character * pCharacter);
    CSize RequiredSize();

protected:
    afx_msg void OnPaint();
    afx_msg LRESULT OnMouseLeave(WPARAM wParam, LPARAM lParam);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
    afx_msg LRESULT UpdateControl(WPARAM, LPARAM);
    afx_msg void OnFeatSelectOk();
    afx_msg void OnFeatSelectCancel();
    afx_msg LRESULT OnHoverComboBox(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnToggleFeatIgnore(WPARAM wParam, LPARAM lParam);
    DECLARE_MESSAGE_MAP()

private:
    void SetupControl();
    size_t DrawTopLine(CDC * pDC);
    size_t DrawLevelLine(CDC * pDC, size_t top, size_t level);
    void DrawFeat(CDC * pDC, CRect rctItem, size_t featIndex, size_t level);
    HitCheckItem HitCheck(CPoint mouse) const;
    void DoClass1();
    void DoClass2();
    void DoClass3();
    void SetClassLevel(const std::string& ct, size_t level);
    void HideTip();
    int DoClassContextMenu(
            CPoint menuPos,
            const std::string& original,
            const std::string& exclude1,
            const std::string& exclude2);

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
    virtual void UpdateFeatTrained(Build *, const std::string& featName) override;
    virtual void UpdateFeatRevoked(Build *, const std::string& featName) override;
    virtual void UpdateAbilityValueChanged(Build*, AbilityType ability) override;
    virtual void UpdateBuildAutomaticFeatsChanged(Build*, size_t level) override;

    //void UpdateClassChoiceChanged(Character * charData) override;
    //void UpdateClassChanged(Character * charData, ClassType classFrom, ClassType classTo, size_t level) override;

    Character * m_pCharacter;
    bool m_bMenuDisplayed;
    std::vector<std::vector<FeatSlot> > m_availableFeats;
    size_t m_maxRequiredFeats;
    size_t m_numClassColumns;
    std::vector<HitCheckItem> m_hitChecks;
    CRect m_levelRect;
    CRect m_classRects[3];
    CRect m_featRects[3];
    CRect m_statRects[7];
    bool m_bUpdatePending;
    size_t m_highlightedLevelLine;
    size_t m_lastNotifiedLevelLine;
    CRect m_levelRects[MAX_GAME_LEVEL];
    // feat tooltip
    CInfoTip m_tooltip;
    bool m_showingTip;
    bool m_tipCreated;
    HitCheckItem m_tooltipItem;
    HitCheckItem m_featSelectItem;
    CComboBoxTooltip m_featSelector;
};


