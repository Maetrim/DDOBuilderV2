// BreakdownItemWeaponEffects.h
//
// This class is a holder class for all effects for each given
// weapon type. This allows weapon items to be moved in and out of
// gear slots and being able to re-generate all the effects for the
// given weapon type without having to notify all effects from all
// sources.
#pragma once
#include "BreakdownItem.h"
#include "WeaponTypes.h"
#include "EquippedGear.h"
#include "WeaponDamageTypes.h"
// this item is never visible in the UI

class CBreakdownsPane;
class BreakdownItemWeapon;

class BreakdownItemWeaponEffects :
        public BreakdownItem
{
    public:
        BreakdownItemWeaponEffects(
                CBreakdownsPane* pPane,
                MfcControls::CTreeListCtrl * treeList,
                HTREEITEM hItem);
        virtual ~BreakdownItemWeaponEffects();

        // required overrides
        virtual CString Title() const override;
        virtual CString Value() const override;
        virtual void CreateOtherEffects() override;
        virtual bool AffectsUs(const Effect & effect) const override;
        virtual void SetCharacter(Character * charData);

        void WeaponsChanged(const EquippedGear & gear);
        bool AreWeaponsCentering() const;
        void AddForumExportData(std::stringstream & forumExport);

        BreakdownItem * GetWeaponBreakdown(bool bMainhand, BreakdownType bt);

    protected:
        // EffectCallbackItem
        virtual void FeatEffectApplied(Build*, const Effect& effect) override;
        virtual void FeatEffectRevoked(Build*, const Effect& effect) override;
        virtual void ItemEffectApplied(Build*, const Effect& effect) override;
        virtual void ItemEffectRevoked(Build*, const Effect& effect) override;
        virtual void ItemEffectApplied(Build*, const Effect& effect, WeaponType wt) override;
        virtual void ItemEffectRevoked(Build*, const Effect& effect, WeaponType wt) override;
        virtual void EnhancementEffectApplied(Build*, const Effect& effect) override;
        virtual void EnhancementEffectRevoked(Build*, const Effect& effect) override;
    private:
        enum NotificationType
        {
            NT_Feat,
            NT_Item,
            NT_ItemWeapon,
            NT_Enhancement
        };
        void AddToAffectedWeapons(std::vector<std::list<Effect> > * list, const Effect & effect, NotificationType nt);
        void RemoveFromAffectedWeapons(std::vector<std::list<Effect> > * list, const Effect & effect, NotificationType nt);
        void AddToAffectedWeapon(std::vector<std::list<Effect> >* list, const Effect& effect, WeaponType wt, NotificationType nt);
        void RemoveFromAffectedWeapon(std::vector<std::list<Effect> >* list, const Effect& effect, WeaponType wt, NotificationType nt);
        bool AffectsThisWeapon(WeaponType wt, const Effect & effect);
        BreakdownItemWeapon* CreateWeaponBreakdown(
                BreakdownType bt,
                const Item & item,
                InventorySlotType ist);

        CBreakdownsPane* m_pPane;
        std::vector<std::list<Effect>> m_weaponFeatEffects;
        std::vector<std::list<Effect>> m_weaponItemEffects;
        std::vector<std::list<Effect>> m_weaponEnhancementEffects;
        MfcControls::CTreeListCtrl * m_pTreeList;
        HTREEITEM m_hItem;
        BreakdownItemWeapon* m_pMainHandWeapon;
        BreakdownItemWeapon* m_pOffHandWeapon;
};
