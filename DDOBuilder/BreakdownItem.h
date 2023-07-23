// BreakdownItem.h
//
// A standard base class that all items that can have breakdowns calculated
// for them inherit from to allow a common interface to the CBreakdownsView
#pragma once

#include "BreakdownTypes.h"
#include "Effect.h"
#include "ObserverSubject.h"
#include "WeaponTypes.h"
#include "EffectCallbackItem.h"

namespace MfcControls
{
    class CTreeListCtrl;
};

enum ColumnOrder
{
    CO_Source = 0,
    CO_Stacks,
    CO_Value,
    CO_BonusType,

    CO_count
};

class CBreakdownsView;
class BreakdownItem;

class BreakdownObserver :
    public Observer<BreakdownItem>
{
    public:
        virtual void UpdateTotalChanged(BreakdownItem *, BreakdownType) {};
};

// this is the base class for all items that can have a break down done for them
// it provides the common interface that allows information from feats, enhancements
// and items to filter in and be collated.
class BreakdownItem :
    public EffectCallbackItem,
    public Subject<BreakdownObserver>,
    public BreakdownObserver
{
    public:
        BreakdownItem(
                BreakdownType type,
                MfcControls::CTreeListCtrl * treeList,
                HTREEITEM hItem);
        virtual ~BreakdownItem();

        virtual void BuildChanged(Character * charData);
        virtual void BuildChangeComplete();

        // EffectCallbackItem
        virtual void AbilityTomeChanged(Life*, AbilityType) override {};
        virtual void AbilityValueChanged(Build*, AbilityType) override {};
        virtual void AbilityTotalChanged(Build*, AbilityType) override;
        virtual void BuildLevelChanged(Build*) override;
        virtual void ClassChanged(Build*, const std::string& , const std::string&, size_t) override {};
        virtual void FeatTrained(Build*, const std::string&) override;
        virtual void FeatRevoked(Build*, const std::string&) override;
        virtual void RaceChanged(Life*, const std::string&) override {};
        virtual void SkillTomeChanged(Life*, SkillType) override {};
        virtual void SkillSpendChanged(Build*, size_t, SkillType) override {};

        virtual void FeatEffectApplied(Build*, const Effect& effect) override;
        virtual void FeatEffectRevoked(Build*, const Effect& effect) override;
        virtual void ItemEffectApplied(Build*, const Effect& effect) override;
        virtual void ItemEffectRevoked(Build*, const Effect& effect) override;
        virtual void ItemEffectApplied(Build*, const Effect& effect, WeaponType wt) override;
        virtual void ItemEffectRevoked(Build*, const Effect& effect, WeaponType wt) override;
        virtual void EnhancementTrained(Build*, const EnhancementItemParams& item) override;
        virtual void EnhancementRevoked(Build*, const EnhancementItemParams& item) override;
        virtual void EnhancementEffectApplied(Build*, const Effect& effect) override;
        virtual void EnhancementEffectRevoked(Build*, const Effect& effect) override;
        virtual void StanceActivated(Build* pBuild, const std::string& stanceName) override;
        virtual void StanceDeactivated(Build* pBuild, const std::string& stanceName) override;
        virtual void SliderChanged(Build* pBuild, const std::string& sliderName, int newValue) override;
        virtual void GearChanged(Build* pBuild, InventorySlotType slot) override;

        BreakdownType Type() const;
        virtual CString Title() const = 0;
        virtual CString Value() const = 0;
        virtual double Multiplier() const;
        void Populate();            // updates the HTREEITEM
        void PopulateBreakdownControl(CListCtrl * pControl);
        void AddItems(CListCtrl* pControl);
        virtual double Total() const;
        virtual double CappedTotal() const;
        virtual double ReplacementTotal() const { return Total(); };

        void AddAbility(AbilityType ability);
        void RemoveFirstAbility(AbilityType ability);
        void AddAbility(AbilityType ability, const Requirements& requirements, WeaponType wt);
        void RemoveFirstAbility(AbilityType ability, const Requirements& requirements, WeaponType wt);

        void SetWeapon(WeaponType wt, size_t weaponCriticalMultiplier);
        WeaponType Weapon() const;

        double GetEffectValue(const std::string& bonusType) const;
    protected:
        void AddOtherEffect(const Effect & effect);
        void AddFeatEffect(const Effect & effect);
        void AddEnhancementEffect(const Effect & effect);
        void AddItemEffect(const Effect & effect);

        void RevokeOtherEffect(const Effect & effect);
        void RevokeFeatEffect(const Effect & effect);
        void RevokeEnhancementEffect(const Effect & effect);
        void RevokeItemEffect(const Effect & effect);

        bool UpdateTreeItemTotals();
        bool UpdateTreeItemTotals(std::list<Effect> * list);
        bool UpdateEffectAmounts(std::list<Effect> * list, BreakdownType bt);
        bool UpdateEffectAmounts(std::list<Effect> * list, const std::string& className);

        virtual bool StacksByMultiplication() const { return false;};

        virtual void CreateOtherEffects() = 0;
        virtual bool AffectsUs(const Effect & effect) const = 0;

        virtual void AppendItems(CListCtrl *) {};  // base does nothing

        // BreakdownObserver overrides (may be specialised in inheriting classes)
        virtual void UpdateTotalChanged(BreakdownItem * item, BreakdownType type) override;

        void NotifyTotalChanged();
        void SetHTreeItem(HTREEITEM hItem);
        std::list<Effect> AllActiveEffects() const;

        AbilityType LargestStatBonus();
        double DoPercentageEffects(const std::list<Effect>& effects, double total) const;
        void RemoveInactive(std::list<Effect>* effects, std::list<Effect>* inactiveEffects) const;
        void RemoveNonStacking(std::list<Effect>* effects, std::list<Effect>* nonStackingEffects) const;
        void RemoveTemporary(std::list<Effect>* effects, std::list<Effect>* temporaryEffects) const;
        void UpdateEnhancementTreeEffects(const std::string& treeName);
        void UpdateSliderEffects(const std::string& sliderName, int newValue);
        struct AbilityStance
        {
            AbilityStance() : ability(Ability_Unknown), weapon(Weapon_Unknown) {};
            AbilityType ability;
            WeaponType weapon;
            Requirements requirements;
            bool operator==(const AbilityStance& other) const
            {
                return ability == other.ability
                    && requirements == other.requirements
                    && weapon == other.weapon;
            }
        };
        std::vector<AbilityStance> m_mainAbility; // ability types that are used for this breakdown (highest of those listed)
        std::vector<AbilityStance> m_baseAbility; // restore list on change of character
        bool m_bHasWeapon;
        WeaponType m_weapon;
        size_t m_weaponCriticalMultiplier;
        bool m_bAddEnergies;
        Character * m_pCharacter;
        std::list<Effect> m_otherEffects;       // these always stack
        std::list<Effect> m_effects;            // these always stack
        std::list<Effect> m_itemEffects;        // highest of a given type counts
    private:
        void AddActiveItems(const std::list<Effect> & effects, CListCtrl * pControl, bool bShowMultiplier);
        void AddActivePercentageItems(const std::list<Effect> & effects, CListCtrl * pControl);
        void AddDeactiveItems(const std::list<Effect> & effects, CListCtrl * pControl, bool bShowMultiplier);
        virtual double SumItems(const std::list<Effect> & effects, bool bApplyMultiplier) const;
        void AddEffect(std::list<Effect> * effectList, const Effect & effect);
        void RevokeEffect(std::list<Effect> * effectList, const Effect & effect);
        MfcControls::CTreeListCtrl * m_pTreeList;
        HTREEITEM m_hItem;
        BreakdownType m_type;

        friend class BreakdownItemWeapon;
};
