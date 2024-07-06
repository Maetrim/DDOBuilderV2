// BreakdownItemWeaponAttackSpeed.h
//
#pragma once
#include "BreakdownItemSimple.h"

class BreakdownItemWeaponAttackSpeed :
        public BreakdownItemSimple
{
    public:
        BreakdownItemWeaponAttackSpeed(
                CBreakdownsPane* pPane,
                BreakdownType type,
                EffectType effect,
                const CString & Title,
                MfcControls::CTreeListCtrl * treeList,
                HTREEITEM hItem);
        virtual ~BreakdownItemWeaponAttackSpeed();

        // required overrides
        virtual CString Title() const override;
        virtual CString Value() const override;
        virtual void CreateOtherEffects() override;
        virtual bool AffectsUs(const Effect & effect) const override;

    private:
        CString m_title;
        EffectType m_effect;
};
