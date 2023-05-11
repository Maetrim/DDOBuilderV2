// BreakdownItemMaximumKi.h
//
#pragma once
#include "BreakdownItem.h"

class CBreakdownsPane;

class BreakdownItemMaximumKi :
        public BreakdownItem
{
    public:
        BreakdownItemMaximumKi(
                CBreakdownsPane * pPane,
                MfcControls::CTreeListCtrl * treeList,
                HTREEITEM hItem);
        virtual ~BreakdownItemMaximumKi();

        // required overrides
        virtual CString Title() const override;
        virtual CString Value() const override;
        virtual void CreateOtherEffects() override;
        virtual bool AffectsUs(const Effect & effect) const override;
    protected:
        // BuildOvserver overrides
        virtual void ClassChanged(Build*, const std::string& classFrom, const std::string& classTo, size_t level) override;

        // BreakdownObserver overrides
        virtual void UpdateTotalChanged(BreakdownItem * item, BreakdownType type) override;
};
