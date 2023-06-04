// BreakdownItemWeaponCriticalThreatRange.h
//
#pragma once
#include "BreakdownItem.h"

class BreakdownItemWeaponCriticalThreatRange :
        public BreakdownItem
{
    public:
        BreakdownItemWeaponCriticalThreatRange(
                BreakdownType type,
                MfcControls::CTreeListCtrl * treeList,
                HTREEITEM hItem);
        virtual ~BreakdownItemWeaponCriticalThreatRange();

        // required overrides
        virtual CString Title() const override;
        virtual CString Value() const override;
        virtual void CreateOtherEffects() override;
        virtual bool AffectsUs(const Effect & effect) const override;

        // EffectCallbackItem
        virtual void FeatEffectApplied(Build*, const Effect& effect) override;
        virtual void FeatEffectRevoked(Build*, const Effect& effect) override;
        virtual void ItemEffectApplied(Build*, const Effect& effect) override;
        virtual void ItemEffectRevoked(Build*, const Effect& effect) override;
        virtual void EnhancementEffectApplied(Build*, const Effect& effect) override;
        virtual void EnhancementEffectRevoked(Build*, const Effect& effect) override;
    private:
        void HandleAddSpecialEffects(const Effect& effect);
        void HandleRevokeSpecialEffects(const Effect& effect);
        size_t WeaponBaseCriticalRange() const;
        size_t m_keenCount;
        size_t m_improvedCriticalCount;
};
