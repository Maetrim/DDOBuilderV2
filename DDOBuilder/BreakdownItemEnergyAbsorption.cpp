// BreakdownItemEnergyAbsorption.cpp
//
#include "stdafx.h"
#include "BreakdownItemEnergyAbsorption.h"

#include "BreakdownsPane.h"
#include "GlobalSupportFunctions.h"

BreakdownItemEnergyAbsorption::BreakdownItemEnergyAbsorption(
        CBreakdownsPane* pPane,
        BreakdownType type,
        EffectType effect,
        EnergyType ssType,
        const CString & title,
        MfcControls::CTreeListCtrl * treeList,
        HTREEITEM hItem) :
    BreakdownItem(type, treeList, hItem),
    m_title(title),
    m_effect(effect),
    m_energyType(ssType)
{
    // register ourselves for effects that affect us
    pPane->RegisterBuildCallbackEffect(effect, this);
    m_bAddEnergies = false; // change base class behavior
}

BreakdownItemEnergyAbsorption::~BreakdownItemEnergyAbsorption()
{
}

// required overrides
CString BreakdownItemEnergyAbsorption::Title() const
{
    return m_title;
}

CString BreakdownItemEnergyAbsorption::Value() const
{
    CString value;
    value.Format(
            "%.1f %% (*)",
            Total());
    return value;
}

void BreakdownItemEnergyAbsorption::CreateOtherEffects()
{
    m_otherEffects.clear();
}

bool BreakdownItemEnergyAbsorption::AffectsUs(const Effect & effect) const
{
    bool isUs = false;
    for (auto&& iit: effect.Item())
    {
        EnergyType et = TextToEnumEntry(iit, energyTypeMap);
        if (et == Energy_All
                || et == m_energyType)
        {
            isUs |= true;
        }
    }
    return isUs;
}

double BreakdownItemEnergyAbsorption::Total() const
{
    // to sum the multiplication total, just get the contributions of all the stacking effects
    double total = 100.0;
    if (m_pCharacter != NULL)
    {
        total *= SumItems(m_otherEffects, false) / 100.0;
        total *= SumItems(m_effects, false) / 100.0;

        std::list<Effect> itemEffects = m_itemEffects;
        std::list<Effect> inactiveEffects;
        std::list<Effect> nonStackingEffects;
        RemoveInactive(&itemEffects, &inactiveEffects);
        RemoveNonStacking(&itemEffects, &nonStackingEffects);
        total *= SumItems(itemEffects, true) / 100.0;
        double baseTotal = total;

        // TBD Future: Handle percentage effects for items that stack by multiplication
        // there should not currently be any cases for this

        // now apply percentage effects. Note percentage effects do not stack.
        // a test on live shows two percentage bonus's to hp adds two lots
        // of the base total (before percentages) to the total
        total += DoPercentageEffects(m_otherEffects, baseTotal);
        total += DoPercentageEffects(m_effects, baseTotal);
        // make sure we update listed items
        DoPercentageEffects(m_itemEffects, baseTotal);
        total += DoPercentageEffects(itemEffects, baseTotal);

        // and finally invert to actual percentage
        total = 100.0 - total;
    }
    return total;
}

double BreakdownItemEnergyAbsorption::SumItems(
        const std::list<Effect> & effects,
        bool bApplyMultiplier) const
{
    double total = 100.0;
    std::list<Effect>::const_iterator it = effects.begin();
    while (it != effects.end())
    {
        // only count the active items in the total
        if ((*it).IsActive(*m_pCharacter, m_wtMain, m_wtOffhand))
        {
            if (!(*it).HasPercent())
            {
                double amount = (*it).TotalAmount(true);
                if (bApplyMultiplier)
                {
                    amount *= Multiplier();
                }
                // fractional multiplication
                total *= ((100.0 - amount) / 100.0);
            }
        }
        ++it;
    }
    return total;
}
