// BreakdownItemEnergyAbsorption.h
//
#pragma once
#include "BreakdownItem.h"
#include "EnergyTypes.h"

class CBreakdownsPane;

class BreakdownItemEnergyAbsorption :
        public BreakdownItem
{
    public:
        BreakdownItemEnergyAbsorption(
                CBreakdownsPane* pPane,
                BreakdownType type,
                EffectType effect,
                EnergyType ssType,
                const CString & Title,
                MfcControls::CTreeListCtrl * treeList,
                HTREEITEM hItem);
        virtual ~BreakdownItemEnergyAbsorption();

        // required overrides
        virtual CString Title() const override;
        virtual CString Value() const override;
        virtual void CreateOtherEffects() override;
        virtual bool AffectsUs(const Effect & effect) const override;
    private:
        virtual double SumItems(const std::list<Effect> & effects, bool bApplyMultiplier) const override;
        virtual double Total() const override;
        CString m_title;
        EffectType m_effect;
        EnergyType m_energyType;
};
