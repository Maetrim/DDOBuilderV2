// BreakdownItemEnergyResistance.cpp
//
#include "stdafx.h"
#include "BreakdownItemEnergyResistance.h"

#include "BreakdownsPane.h"
#include "GlobalSupportFunctions.h"

BreakdownItemEnergyResistance::BreakdownItemEnergyResistance(
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

BreakdownItemEnergyResistance::~BreakdownItemEnergyResistance()
{
}

// required overrides
CString BreakdownItemEnergyResistance::Title() const
{
    return m_title;
}

CString BreakdownItemEnergyResistance::Value() const
{
    CString value;
    value.Format(
            "%3d",
            (int)Total());
    return value;
}

void BreakdownItemEnergyResistance::CreateOtherEffects()
{
    m_otherEffects.clear();
}

bool BreakdownItemEnergyResistance::AffectsUs(const Effect & effect) const
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
