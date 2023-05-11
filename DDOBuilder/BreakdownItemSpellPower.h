// BreakdownItemSpellPower.h
//
#pragma once
#include "BreakdownItem.h"

class CBreakdownsPane;

class BreakdownItemSpellPower :
        public BreakdownItem
{
    public:
        BreakdownItemSpellPower(
                CBreakdownsPane* pPane,
                BreakdownType type,
                EffectType effect,
                SpellPowerType sptype,
                const CString & Title,
                MfcControls::CTreeListCtrl * treeList,
                HTREEITEM hItem);
        virtual ~BreakdownItemSpellPower();

        // required overrides
        virtual CString Title() const override;
        virtual CString Value() const override;
        virtual double ReplacementTotal() const override;
        virtual void CreateOtherEffects() override;
        virtual bool AffectsUs(const Effect & effect) const override;
        virtual void AppendItems(CListCtrl* pControl) override;

        // BreakdownObserver overrides
        virtual void UpdateTotalChanged(BreakdownItem * item, BreakdownType type) override;
        // CharacterObserver overrides
        //virtual void UpdateEnhancementTrained(
        //        Character * charData,
        //        const std::string& enhancementName,
        //        const std::string& selection,
        //        bool isTier5) override;
        //virtual void UpdateEnhancementRevoked(
        //        Character * charData,
        //        const std::string& enhancementName,
        //        const std::string& selection,
        //        bool isTier5) override;
        //virtual void UpdateGearChanged(Character * charData, InventorySlotType slot);
    private:
        BreakdownType SpellPowerBreakdown() const;
        double IterateList(
            const std::list<Effect>& effects,
            double value,
            SpellPowerType* usedSpt) const;
        void AppendList(
            const std::list<Effect>& effects,
            std::list<SpellPowerType>* pSPL) const;

        CString m_title;
        EffectType m_effect;
        SpellPowerType m_spellPowerType;
        mutable SpellPowerType m_replacementSpellPower;
};
