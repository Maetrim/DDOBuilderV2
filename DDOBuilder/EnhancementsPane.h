// EnhancementsPane.h
//
#pragma once
#include "Resource.h"
#include <list>
#include <vector>
#include "EnhancementTree.h"
#include "Character.h"
#include "IconButton.h"
#include "InfoTip.h"

class CDDOBuilderDoc;
class Character;
class CEnhancementTreeDialog;

class CEnhancementsPane :
    public CFormView,
    public CharacterObserver,
    public LifeObserver,
    public BuildObserver
{
public:
    CEnhancementsPane();           // protected constructor used by dynamic creation
    virtual ~CEnhancementsPane();
    enum { IDD = IDD_ENHANCEMENTS_PANE };

    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual void OnInitialUpdate();
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg LRESULT OnNewDocument(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnLoadComplete(WPARAM, LPARAM);
    afx_msg void OnTreeSelect(UINT nID);
    afx_msg LRESULT OnUpdateTrees(WPARAM wParam, LPARAM lParam);
    afx_msg void OnUniversalTree(UINT nID);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg LRESULT OnMouseLeave(WPARAM wParam, LPARAM lParam);

    DECLARE_DYNCREATE(CEnhancementsPane)
    DECLARE_MESSAGE_MAP()

private:
    // Character Observer overrides
    virtual void UpdateActiveLifeChanged(Character*) override;
    virtual void UpdateActiveBuildChanged(Character*) override;

    // Life Observer overrides
    virtual void UpdateAlignmentChanged(Life*, AlignmentType alignmen) override;
    virtual void UpdateRaceChanged(Life*, const std::string& race) override;
    virtual void UpdateActionPointsChanged(Life*) override;

    // Build Observer overrides
    virtual void UpdateBuildLevelChanged(Build*) override;
    virtual void UpdateClassChanged(Build * pBuild, const std::string& classFrom, const std::string& classTo, size_t level) override;
    virtual void UpdateEnhancementTrained(Build*, const EnhancementItemParams& item) override;
    virtual void UpdateEnhancementRevoked(Build*, const EnhancementItemParams& item) override;
    virtual void UpdateFeatEffectApplied(Build*, const Effect & effect) override;
    virtual void UpdateFeatEffectRevoked(Build*, const Effect & effect) override;
    virtual void UpdateEnhancementEffectApplied(Build*, const Effect & effect) override;
    virtual void UpdateEnhancementEffectRevoked(Build*, const Effect & effect) override;
    virtual void UpdateEnhancementTreeOrderChanged(Build*, enum TreeType tt) override;
    virtual void UpdateActionPointsChanged(Build*) override;

    std::list<EnhancementTree> DetermineTrees();
    void CreateEnhancementWindows();
    void DestroyEnhancementWindows();
    void UpdateEnhancementWindows();
    void UpdateWindowTitle();
    void PopulateTreeCombo(CComboBox * combo, const std::string& selectedTree);
    void EnableDisableComboboxes();
    void UpdateTrees();
    void AddCustomButtons();
    void ShowTip(const CIconButton & item, CRect itemRect);
    void HideTip();
    void SetTooltipText(const CIconButton & item, CPoint tipTopLeft, CPoint tipAlternate);
    enum MaxSupportTrees
    {
        MST_Number = MAX_ENHANCEMENT_TREES,
        MST_UniversalTrees = 16
    };
    std::list<EnhancementTree> m_availableTrees;
    CComboBox m_comboTreeSelect[MST_Number-1];  // 1st tree type is always fixed
    CIconButton m_universalTrees[MST_UniversalTrees];
    std::vector<CEnhancementTreeDialog *> m_treeViews;
    std::vector<size_t> m_visibleTrees; // only some trees will be visible (indexes into m_treeViews)
    size_t m_numUniversalButtons;
    CInfoTip m_tooltip;
    bool m_showingTip;
    bool m_tipCreated;
    const CIconButton * m_pTooltipItem;

    CDDOBuilderDoc* m_pDoc;
    Character* m_pCharacter;
    bool m_bHadinitialUpdate;
};
