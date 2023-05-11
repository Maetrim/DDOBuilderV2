// BreakdownItemEnergyCasterLevel.h
//
// A standard base class that all items that can have breakdowns calculated
// for them inherit from to allow a common interface to the CBreakdownsView
#pragma once

#include "BreakdownItem.h"
#include "EnergyTypes.h"

class CBreakdownsPane;

class BreakdownItemEnergyCasterLevel :
        public BreakdownItem
{
    public:
        BreakdownItemEnergyCasterLevel(
                CBreakdownsPane* pPane,
                EffectType et,
                EnergyType energyType,
                BreakdownType type,
                MfcControls::CTreeListCtrl * treeList,
                HTREEITEM hItem);
        virtual ~BreakdownItemEnergyCasterLevel();

        // required overrides
        virtual CString Title() const override;
        virtual CString Value() const override;
        virtual void CreateOtherEffects() override;
        virtual bool AffectsUs(const Effect & effect) const override;

    private:
        EnergyType m_energy;
};
