// ItemSelectDialog.h
#pragma once
#include "Resource.h"
#include "InventorySlotTypes.h"
#include "Item.h"
#include "SortHeaderCtrl.h"
#include "InfoTip.h"
#include "ComboBoxTooltip.h"

class Build;

class CItemSelectDialog : public CDialog
{
    public:
        CItemSelectDialog(
                CWnd* pParent,
                InventorySlotType slot,
                const Item & item,
                Build* pBuild);   // standard constructor
        virtual ~CItemSelectDialog();

        Item SelectedItem();

        // Dialog Data
        enum { IDD = IDD_ITEM_SELECT };

        DECLARE_DYNAMIC(CItemSelectDialog)

    protected:
        virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
        virtual BOOL OnInitDialog();
        virtual BOOL PreTranslateMessage(MSG* pMsg);
        afx_msg void OnItemSelected(NMHDR* pNMHDR, LRESULT* pResult);
        afx_msg void OnAugmentSelect(UINT nID);
        afx_msg void OnAugmentCancel(UINT nID);
        afx_msg void OnUpgradeSelect(UINT nID);
        afx_msg void OnUpgradeCancel(UINT nID);
        afx_msg void OnKillFocusAugmentEdit(UINT nID);
        afx_msg void OnAugmentLevelSelect(UINT nID);
        afx_msg void OnAugmentLevelCancel(UINT nID);
        afx_msg void OnSize(UINT nType, int cx, int cy);
        afx_msg void OnEndtrackListItems(NMHDR* pNMHDR, LRESULT* pResult);
        afx_msg void OnColumnclickListItems(NMHDR* pNMHDR, LRESULT* pResult);
        afx_msg void OnHoverListItems(NMHDR* pNMHDR, LRESULT* pResult);
        afx_msg void OnSelEndOkFilter();
        afx_msg LRESULT OnHoverComboBox(WPARAM wParam, LPARAM lParam);
        afx_msg LRESULT OnMouseEnter(WPARAM wParam, LPARAM lParam);
        afx_msg LRESULT OnMouseLeave(WPARAM wParam, LPARAM lParam);
        afx_msg void OnWindowPosChanging(WINDOWPOS * pos);
        afx_msg void OnSearchTextKillFocus();
        afx_msg void OnItemLevelSelect();
        afx_msg void OnSelEndLevelRange();
    public:
        DECLARE_MESSAGE_MAP()

    private:
        static int CALLBACK SortCompareFunction(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
        void PopulateAvailableItemList();
        void EnableControls();
        void PopulateAugmentList(
                CComboBox * combo,
                CEdit * edit1,
                CEdit * edit2,
                CComboBox* comboLevel,
                const ItemAugment & augment);
        void PopulateSlotUpgradeList(size_t controlIndex, const SlotUpgrade & upgrade);
        void PopulateDropList(size_t controlIndex, const std::list<std::string> & types);

        void ShowTip(const Item & item, CRect itemRect);
        void HideTip();
        void SetTooltipText(const Item & item, CPoint tipTopLeft, CPoint tipAlternate);
        void SetTooltipText(const Augment & augment, CPoint tipTopLeft, CPoint tipAlternate, bool rightAlign);
        void SetupFilterCombobox();
        void RemoveAugment(std::vector<ItemAugment> * augments, const std::string & name);
        void SetupItemLevel(size_t maxLevel, size_t currentLevel, bool bSelectable);

        InventorySlotType m_slot;
        Item m_item;
        Build* m_pBuild;

        enum
        {
            MAX_Augments = 15,
            MAX_Upgrades = 4,
        };
        CStatic m_staticType;
        CComboBox m_comboFilter;
        CEdit m_editSearchText;
        CButton m_buttonFilter;
        CListCtrl m_availableItemsCtrl;
        CSortHeaderCtrl m_sortHeader;
        CComboBox m_comboLevelRange;
        CComboBox m_comboItemLevel;
        CStatic m_staticAugments;
        CStatic m_augmentType[MAX_Augments];
        CComboBoxTooltip m_comboAugmentDropList[MAX_Augments];
        CEdit m_augmentValues[MAX_Augments];
        CEdit m_augmentValues2[MAX_Augments];
        CComboBox m_comboAugmentLevel[MAX_Augments];
        CStatic m_staticUpgrades;
        CStatic m_upgradeType[MAX_Upgrades];
        CComboBox m_comboUpgradeDropList[MAX_Upgrades];

        std::list<const Item*> m_availableItems;
        bool m_bInitialising;
        ArmorType m_armorType;
        WeaponType m_weaponType;
        // item tooltips
        CInfoTip m_tooltip;
        bool m_showingTip;
        bool m_tipCreated;
        int m_hoverItem;
        UINT m_hoverHandle;
        int m_augmentHookHandles[MAX_Augments];
        bool m_bIgnoreNextMessage;
        CString m_previousSearchText;
        int m_levelRange;
};
