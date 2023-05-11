// InventoryDialog.h
//

#pragma once

#include "Resource.h"
#include "inventorySlotTypes.h"
#include "InfoTip.h"
#include "Character.h"
#include "ComboBoxTooltip.h"

class InventoryHitBox
{
    public:
        InventoryHitBox(InventorySlotType slot, const CRect & rect) :
                m_slot(slot), m_rect(rect)
        {
        };
        ~InventoryHitBox() {};

        bool IsInRect(CPoint point) const
        {
            return (m_rect.PtInRect(point) != 0);
        };
        InventorySlotType Slot() const
        {
            return m_slot;
        };
        CRect Rect() const
        {
            return m_rect;
        };
    private:
        InventorySlotType m_slot;
        CRect m_rect;
};
class FiligreeHitBox
{
    public:
        FiligreeHitBox(int slot, const CRect & rect) :
                m_slot(slot), m_rect(rect)
        {
        };
        ~FiligreeHitBox() {};

        bool IsInRect(CPoint point) const
        {
            return (m_rect.PtInRect(point) != 0);
        };
        int Slot() const
        {
            return m_slot;
        };
        CRect Rect() const
        {
            return m_rect;
        };
    private:
        int m_slot;      // -1 for jewel
        CRect m_rect;
};
class CInventoryDialog;

// this dialog handles selection of gear
class InventoryObserver :
    public Observer<CInventoryDialog>
{
    public:
        virtual void UpdateSlotLeftClicked(CInventoryDialog*, InventorySlotType) {};
        virtual void UpdateSlotRightClicked(CInventoryDialog*, InventorySlotType) {};
};

class CInventoryDialog :
    public CDialog,
    public Subject<InventoryObserver>
{
    public:
        CInventoryDialog(CWnd* pParent);

        void SetGearSet(Build* pBuild, const EquippedGear & gear);

    ///////////////////////////////////////////////////////////////////
    // MFC

    public:
        //{{AFX_DATA(CInventoryDialog)
        enum { IDD = IDD_INVENTORY_SELECTION };
        //}}AFX_DATA

    protected:
        //{{AFX_VIRTUAL(CInventoryDialog)
        virtual void DoDataExchange(CDataExchange* pDX);
        virtual BOOL OnInitDialog();
        virtual void OnCancel();
        //}}AFX_VIRTUAL

        //{{AFX_MSG(CInventoryDialog)
        afx_msg BOOL OnEraseBkgnd(CDC* pDC);
        afx_msg void OnPaint();
        afx_msg void OnMouseMove(UINT nFlags, CPoint point);
        afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
        afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
        afx_msg LRESULT OnMouseLeave(WPARAM wParam, LPARAM lParam);
        //}}AFX_MSG
        DECLARE_MESSAGE_MAP()

    private:
        InventorySlotType FindItemByPoint(CRect * pRect = NULL) const;
        bool FindFiligreeByPoint(
                int * filigree,
                bool * isRareSection,
                bool * bArtifactFiligree,
                CRect * pRect = NULL) const;
        void ShowTip(const Item & item, CRect itemRect);
        void ShowTip(const Filigree& filigree, CRect itemRect);
        void ShowTip(const Gem& gem, CRect itemRect);
        void HideTip();
        void SetTooltipText(const Item& item, CPoint tipTopLeft, CPoint tipAlternate);
        void SetTooltipText(const Filigree& filigree, CPoint tipTopLeft, CPoint tipAlternate, bool rightAlign);
        void SetTooltipText(const Gem& gem, CPoint tipTopLeft, CPoint tipAlternate, bool rightAlign);
        CRect GetItemRect(InventorySlotType slot) const;
        void NotifySlotLeftClicked(InventorySlotType slot);
        void NotifySlotRightClicked(InventorySlotType slot);
        void EditSentientGem(int filigreeIndex, CRect itemRect);
        void ToggleRareState(int filigree, bool isArtifactFiligree);
        void CreatePersonalityMenu();
        void CreateFiligreeMenu(int filigree, bool bArtifact, bool bEmptyFiligreeSlot);

        Build* m_pBuild;
        CSize m_bitmapSize;
        CBitmap m_cachedDisplay;
        CImage m_imageBackground;
        CImage m_imageBackgroundDisabled;
        CImage m_imagesCannotEquip;
        CImage m_imagesJewel;
        CImage m_imagesFiligree;
        CImage m_imagesFiligreeRare;
        std::vector<InventoryHitBox> m_hitBoxesInventory;
        std::vector<FiligreeHitBox> m_hitBoxesFiligrees;
        std::vector<FiligreeHitBox> m_hitBoxesArtifactFiligrees;
        CInfoTip m_tooltip;
        bool m_showingItemTip;
        bool m_showingFiligreeTip;
        bool m_showingAugmentTip;
        bool m_tipCreated;
        InventorySlotType m_tooltipItem;
        EquippedGear m_gearSet;
        int m_filigreeIndex;        // index of filigree being edited
        bool m_bIsArtifactFiligree;
        CMenu m_filigreeMenu;
        CMenu m_personalityMenu;
        int m_tooltipFiligree;
};

//{{AFX_INSERT_LOCATION}}
