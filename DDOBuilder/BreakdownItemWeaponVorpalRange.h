// BreakdownItemWeaponVorpalRange.h
//
#pragma once
#include "BreakdownItem.h"

class BreakdownItemWeaponVorpalRange :
        public BreakdownItem
{
    public:
        BreakdownItemWeaponVorpalRange(
                BreakdownType type,
                MfcControls::CTreeListCtrl * treeList,
                HTREEITEM hItem);
        virtual ~BreakdownItemWeaponVorpalRange();

        // required overrides
        virtual CString Title() const override;
        virtual CString Value() const override;
        virtual void CreateOtherEffects() override;
        virtual bool AffectsUs(const Effect & effect) const override;
};
