// BreakdownItemTurnUndeadHitDice.h
//
#pragma once
#include "BreakdownItem.h"

class CBreakdownsPane;

class BreakdownItemTurnUndeadHitDice:
        public BreakdownItem
{
    public:
        BreakdownItemTurnUndeadHitDice(
                CBreakdownsPane* pPane,
                BreakdownType type,
                MfcControls::CTreeListCtrl * treeList,
                HTREEITEM hItem);
        virtual ~BreakdownItemTurnUndeadHitDice();

        // required overrides
        virtual CString Title() const override;
        virtual CString Value() const override;
        virtual void CreateOtherEffects() override;
        virtual bool AffectsUs(const Effect & effect) const override;
    private:
        virtual void ClassChanged(Build*, const std::string& classFrom, const std::string& classTo, size_t level) override;
        //virtual void UpdateSpellTrained(Character * charData, const TrainedSpell & spell) override;
        //virtual void UpdateSpellRevoked(Character * charData, const TrainedSpell & spell) override;

        // BreakdownObserver overrides
        virtual void UpdateTotalChanged(BreakdownItem * item, BreakdownType type) override;

        bool IsUs(const Effect & effect) const;
};
