// BreakdownItemSave.h
//
#pragma once
#include "BreakdownItem.h"
#include "BreakdownItemSimple.h"
#include "SaveTypes.h"

class CBreakdownsPane;

class BreakdownItemSave:
        public BreakdownItem
{
    public:
        BreakdownItemSave(
                CBreakdownsPane* pPane,
                BreakdownType type,
                SaveType st,
                MfcControls::CTreeListCtrl * treeList,
                HTREEITEM hItem,
                AbilityType ability,
                BreakdownItemSave * pBaseItem,
                bool bCreateNoFail = true);
        virtual ~BreakdownItemSave();

        virtual void BuildChanged(Character* charData) override;

        virtual void AppendItems(CListCtrl * pControl) override;

        // required overrides
        virtual CString Title() const override;
        virtual CString Value() const override;
        virtual void CreateOtherEffects() override;
        virtual bool AffectsUs(const Effect & effect) const override;

        bool HasNoFailOn1() const;
    protected:
        virtual void BuildLevelChanged(Build*) override;
        virtual void ClassChanged(Build*, const std::string& classFrom, const std::string& classTo,size_t level) override;
        virtual void FeatEffectApplied(Build*, const Effect& effect) override;
        virtual void FeatEffectRevoked(Build*, const Effect& effect) override;
        virtual void ItemEffectApplied(Build*, const Effect& effect) override;
        virtual void ItemEffectRevoked(Build*, const Effect& effect) override;
        virtual void ItemEffectApplied(Build*, const Effect& effect, WeaponType wt) override;
        virtual void ItemEffectRevoked(Build*, const Effect& effect, WeaponType wt) override;
        virtual void EnhancementEffectApplied(Build*, const Effect& effect) override;
        virtual void EnhancementEffectRevoked(Build*, const Effect& effect) override;

        // BreakdownObserver overrides
        virtual void UpdateTotalChanged(BreakdownItem * item, BreakdownType type) override;

    private:
        bool IsUs(const Effect & effect) const;
        void AddClassSaves();
        bool AddDivineGrace();
        void AddHalfElfDivineGrace();

        SaveType m_saveType;
        AbilityType m_ability;
        BreakdownItemSave * m_pBaseBreakdown;
        BreakdownItemSave * m_pNoFailOnOne;
};
