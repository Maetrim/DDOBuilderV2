// BreakdownItemMRRCap.h
//
#pragma once
#include "BreakdownItem.h"

class CBreakdownsPane;

class BreakdownItemMRRCap :
        public BreakdownItem
{
    public:
        BreakdownItemMRRCap(
                CBreakdownsPane* pPane,
                BreakdownType type,
                const CString & Title,
                MfcControls::CTreeListCtrl * treeList,
                HTREEITEM hItem);
        virtual ~BreakdownItemMRRCap();

        // required overrides
        virtual CString Title() const override;
        virtual CString Value() const override;
        virtual void CreateOtherEffects() override;
        virtual bool AffectsUs(const Effect & effect) const override;
    protected:
        virtual void StanceActivated(Build* pBuild, const std::string& stanceName) override;
        virtual void StanceDeactivated(Build* pBuild, const std::string& stanceName) override;
        virtual void GearChanged(Build* pBuild, InventorySlotType slot) override;
    private:
        CString m_title;
};
