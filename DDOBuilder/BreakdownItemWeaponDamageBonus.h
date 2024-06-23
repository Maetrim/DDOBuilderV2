// BreakdownItemWeaponDamageBonus.h
//
#pragma once
#include "BreakdownItem.h"

class BreakdownItemWeaponDamageBonus :
        public BreakdownItem
{
    public:
        BreakdownItemWeaponDamageBonus(
                BreakdownType type,
                EffectType effect,
                const CString & Title,
                MfcControls::CTreeListCtrl * treeList,
                HTREEITEM hItem,
                bool bOffhandWeapon,
                bool bCriticalEffects);
        virtual ~BreakdownItemWeaponDamageBonus();

        // required overrides
        virtual CString Title() const override;
        virtual CString Value() const override;
        virtual void CreateOtherEffects() override;
        virtual bool AffectsUs(const Effect & effect) const override;

        virtual void LinkUp() override;

        // EffectCallbackItem
        virtual void ClassChanged(Build*, const std::string& classFrom, const std::string& classTo, size_t level) override;
        virtual void FeatEffectApplied(Build*, const Effect& effect) override;
        virtual void FeatEffectRevoked(Build*, const Effect& effect) override;
        virtual void ItemEffectApplied(Build*, const Effect& effect) override;
        virtual void ItemEffectRevoked(Build*, const Effect& effect) override;
        virtual void EnhancementEffectApplied(Build*, const Effect& effect) override;
        virtual void EnhancementEffectRevoked(Build*, const Effect& effect) override;
        virtual void UpdateTotalChanged(BreakdownItem* item, BreakdownType type) override;
    private:
        CString m_title;
        EffectType m_effect;
        bool m_bOffhandWeapon;
        bool m_bCriticalEffects;
};
