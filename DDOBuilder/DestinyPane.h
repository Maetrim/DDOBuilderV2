// DestinyPane.h
//
#pragma once
#include "Resource.h"
#include <list>
#include "Character.h"
#include "IconButton.h"
#include "InfoTip.h"
#include "BreakdownItem.h"

class EnhancementTree;
class CEnhancementTreeDialog;

class CDestinyPane :
    public CFormView,
    public CharacterObserver,
    public LifeObserver,
    public BuildObserver,
    public BreakdownObserver
{
    public:
        CDestinyPane();           // protected constructor used by dynamic creation
        virtual ~CDestinyPane();
        enum { IDD = IDD_DESTINY_PANE };

        virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
        virtual void OnInitialUpdate();

        afx_msg void OnSize(UINT nType, int cx, int cy);
        afx_msg BOOL OnEraseBkgnd(CDC* pDC);
        afx_msg LRESULT OnNewDocument(WPARAM wParam, LPARAM lParam);
        afx_msg LRESULT OnLoadComplete(WPARAM wParam, LPARAM lParam);
        afx_msg void OnTreeSelect(UINT nID);
        afx_msg LRESULT OnUpdateTrees(WPARAM wParam, LPARAM lParam);
        afx_msg void OnClaimTree(UINT nID);
        afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
        afx_msg void OnMouseMove(UINT nFlags, CPoint point);
        afx_msg LRESULT OnMouseLeave(WPARAM wParam, LPARAM lParam);
        DECLARE_DYNCREATE(CDestinyPane)
        DECLARE_MESSAGE_MAP()

    private:
        // CharacterObserver overrides
        virtual void UpdateActiveLifeChanged(Character*) override;
        virtual void UpdateActiveBuildChanged(Character*) override;

        // Life Observer overrides
        virtual void UpdateActionPointsChanged(Life*) override;

        // Build Observer overrides
        virtual void UpdateEnhancementTrained(Build*, const EnhancementItemParams& item) override;
        virtual void UpdateEnhancementRevoked(Build*, const EnhancementItemParams& item) override;
        virtual void UpdateFeatEffectApplied(Build*, const Effect & effect) override;
        virtual void UpdateFeatEffectRevoked(Build*, const Effect & effect) override;
        virtual void UpdateEnhancementEffectApplied(Build*, const Effect & effect) override;
        virtual void UpdateEnhancementEffectRevoked(Build*, const Effect & effect) override;
        virtual void UpdateEnhancementTreeOrderChanged(Build*) override;

        // BreakdownObserver overrides
        virtual void UpdateTotalChanged(BreakdownItem * item, BreakdownType type) override;

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
            // +1 as we have the preview tree slot
            MST_Number = MAX_DESTINY_TREES + 1
        };
        std::list<EnhancementTree> m_availableTrees;
        CComboBox m_comboTreeSelect[MST_Number];
        CIconButton m_destinyTrees[16];
        CStatic m_staticPreview;
        std::vector<CEnhancementTreeDialog *> m_treeViews;
        std::vector<size_t> m_visibleTrees; // only some trees will be visible (indexes into m_treeViews)
        size_t m_numDestinyButtons;
        CInfoTip m_tooltip;
        bool m_showingTip;
        bool m_tipCreated;
        const CIconButton * m_pTooltipItem;

        CDDOBuilderDoc*  m_pDoc;
        Character * m_pCharacter;
        bool m_bHadinitialUpdate;
};
