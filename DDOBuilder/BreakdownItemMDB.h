// BreakdownItemMDB.h
//
// A standard base class that all items that can have breakdowns calculated
// for them inherit from to allow a common interface to the CBreakdownsView
#pragma once

#include "BreakdownItem.h"

class CBreakdownsPane;

class BreakdownItemMDB :
        public BreakdownItem
{
    public:
        BreakdownItemMDB(
            CBreakdownsPane* pPane,
            BreakdownType type,
                MfcControls::CTreeListCtrl * treeList,
                HTREEITEM hItem);
        virtual ~BreakdownItemMDB();

        // required overrides
        virtual CString Title() const override;
        virtual CString Value() const override;
        virtual void CreateOtherEffects() override;
        virtual bool AffectsUs(const Effect & effect) const override;
    private:
        // EffectCallbackItem
        virtual void StanceActivated(Build* pBuild, const std::string& stanceName) override;
        virtual void StanceDeactivated(Build* pBuild, const std::string& stanceName) override;

        bool m_bNoLimit;
};
