// BreakdownItemAbility.h
//
// A standard base class that all items that can have breakdowns calculated
// for them inherit from to allow a common interface to the CBreakdownsView
#pragma once

#include "BreakdownItem.h"

class CBreakdownsPane;

class BreakdownItemAbility :
        public BreakdownItem
{
    public:
        BreakdownItemAbility(
                CBreakdownsPane* pPane,
                AbilityType skill,
                BreakdownType type,
                MfcControls::CTreeListCtrl * treeList,
                HTREEITEM hItem);
        virtual ~BreakdownItemAbility();

        // required overrides
        virtual CString Title() const override;
        virtual CString Value() const override;
        virtual void CreateOtherEffects() override;
        virtual bool AffectsUs(const Effect & effect) const override;
    protected:
        virtual void AbilityTomeChanged(
                Life*,
                AbilityType ability) override;
        virtual void RaceChanged(
                Life*,
                const std::string& race) override;
        virtual void BuildLevelChanged(Build*) override;
        virtual void AbilityValueChanged(
                Build*,
                AbilityType ability) override;
    private:
        AbilityType m_ability;
};
