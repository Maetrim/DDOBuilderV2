// BreakdownItemAC.h
//
// A standard base class that all items that can have breakdowns calculated
// for them inherit from to allow a common interface to the CBreakdownsView
#pragma once

#include "BreakdownItem.h"

class CBreakdownsPane;

class BreakdownItemAC :
        public BreakdownItem
{
    public:
        BreakdownItemAC(
                CBreakdownsPane* pPane,
                BreakdownType type,
                MfcControls::CTreeListCtrl * treeList,
                HTREEITEM hItem);
        virtual ~BreakdownItemAC();

        // required overrides
        virtual CString Title() const override;
        virtual CString Value() const override;
        virtual void CreateOtherEffects() override;
        virtual bool AffectsUs(const Effect & effect) const override;

        virtual void LinkUp() override;

        // BreakdownObserver overrides
        virtual void UpdateTotalChanged(BreakdownItem * item, BreakdownType type) override;
        virtual void StanceActivated(Build* pBuild, const std::string& stanceName) override;
        virtual void StanceDeactivated(Build* pBuild, const std::string& stanceName) override;
};
