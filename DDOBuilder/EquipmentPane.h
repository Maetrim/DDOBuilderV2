// EquipmentPane.h
//
#pragma once
#include "Resource.h"
#include "Character.h"
#include "InventoryDialog.h"
#include "SetBonusButton.h"

class CEquipmentPane :
    public CFormView,
    public InventoryObserver,
    public CharacterObserver,
    public LifeObserver,
    public BuildObserver
{
    public:
        enum { IDD = IDD_EQUIPMENT_PANE };
        DECLARE_DYNCREATE(CEquipmentPane)

#ifdef _DEBUG
        virtual void AssertValid() const;
    #ifndef _WIN32_WCE
    virtual void Dump(CDumpContext& dc) const;
#endif
#endif
        virtual void OnInitialUpdate();
    protected:
        CEquipmentPane();           // protected constructor used by dynamic creation
        virtual ~CEquipmentPane();

        virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
        afx_msg void OnSize(UINT nType, int cx, int cy);
        afx_msg BOOL OnEraseBkgnd(CDC* pDC);
        afx_msg LRESULT OnNewDocument(WPARAM wParam, LPARAM lParam);
        afx_msg void OnGearNew();
        afx_msg void OnGearCopy();
        afx_msg void OnGearPaste();
        afx_msg void OnGearDelete();
        afx_msg void OnUpdateGearNew(CCmdUI * pCmdUi);
        afx_msg void OnUpdateGearCopy(CCmdUI * pCmdUi);
        afx_msg void OnUpdateGearPaste(CCmdUI * pCmdUi);
        afx_msg void OnUpdateGearDelete(CCmdUI * pCmdUi);
        afx_msg void OnGearSelectionSelEndOk();
        afx_msg void OnGearNumFiligreesSelEndOk();
        afx_msg void OnMouseMove(UINT nFlags, CPoint point);
        afx_msg LRESULT OnMouseLeave(WPARAM wParam, LPARAM lParam);
        afx_msg BOOL OnTtnNeedText(UINT id, NMHDR* pNMHDR, LRESULT* pResult);
        DECLARE_MESSAGE_MAP()

        // InventoryObserver overrides
        virtual void UpdateSlotLeftClicked(CInventoryDialog * dialog, InventorySlotType slot) override;
        virtual void UpdateSlotRightClicked(CInventoryDialog * dialog, InventorySlotType slot) override;

        // CharacterObserver overrides
        virtual void UpdateActiveLifeChanged(Character*) override;
        virtual void UpdateActiveBuildChanged(Character*) override;

        // LifeObserver overrides
        virtual void UpdateRaceChanged(Life*, const std::string&) override;

        // BuildObserver overrides
        virtual void UpdateBuildLevelChanged(Build*) override;
        virtual void UpdateGearChanged(Build*, InventorySlotType) override;
        virtual void UpdateNewSetBonusStack(Build*, const SetBonus&) override;
        virtual void UpdateRevokeSetBonusStack(Build*, const SetBonus&) override;

    private:
        void PopulateCombobox();
        void EnableControls();
        void PopulateGear();
        std::string SelectedGearSet() const;
        void ShowTip(const CSetBonusButton& item, CRect itemRect);
        void HideTip();
        void SetTooltipText(const CSetBonusButton& item, CPoint tipTopLeft, CPoint tipAlternate);
        void DestroyAllSets();
        void AddSetBonusStack(const SetBonus& setBonus);
        void RevokeSetBonusStack(const SetBonus& setBonus);

        CComboBox m_comboGearSelections;
        CMFCButton m_buttonNew;
        CMFCButton m_buttonCopy;
        CMFCButton m_buttonPaste;
        CMFCButton m_buttonDelete;
        CStatic m_staticNumFiligrees;
        CComboBox m_comboNumFiligrees;
        CInventoryDialog * m_inventoryView;
        std::vector<CSetBonusButton*> m_setbuttons;
        const CSetBonusButton* m_pTooltipItem;
        CInfoTip m_tooltip;
        bool m_showingTip;
        bool m_tipCreated;
        int m_nextSetId;
        CString m_tipText;

        CDDOBuilderDoc* m_pDocument;
        Character* m_pCharacter;
};
