// BreakdownItemImmunities.h
//
#pragma once
#include "BreakdownItem.h"

class CBreakdownsPane;

class BreakdownItemImmunities :
        public BreakdownItem
{
    public:
        BreakdownItemImmunities(
                CBreakdownsPane* pPane,
                BreakdownType type,
                MfcControls::CTreeListCtrl * treeList,
                HTREEITEM hItem);
        virtual ~BreakdownItemImmunities();

        // required overrides
        virtual CString Title() const override;
        virtual CString Value() const override;
        virtual void CreateOtherEffects() override;
        virtual bool AffectsUs(const Effect & effect) const override;
    private:
        void AddEffectToString(
                CString * value,
                const Effect & effect) const;
};
