// BreakdownItemEnergyCasterLevel.cpp
//
#include "stdafx.h"
#include "BreakdownItemEnergyCasterLevel.h"
#include "BreakdownsPane.h"

#include "GlobalSupportFunctions.h"

BreakdownItemEnergyCasterLevel::BreakdownItemEnergyCasterLevel(
        CBreakdownsPane* pPane,
        EffectType et,
        EnergyType energyType,
        BreakdownType type,
        MfcControls::CTreeListCtrl * treeList,
        HTREEITEM hItem) :
    BreakdownItem(type, treeList, hItem),
    m_energy(energyType)
{
    // register ourselves for effects that affect us
    pPane->RegisterBuildCallbackEffect(et, this);
}

BreakdownItemEnergyCasterLevel::~BreakdownItemEnergyCasterLevel()
{
}

// required overrides
CString BreakdownItemEnergyCasterLevel::Title() const
{
    return "";
}

CString BreakdownItemEnergyCasterLevel::Value() const
{
    CString value;
    value.Format(
            "%+3d",          // Caster level values are always whole numbers
            (int)Total());
    return value;
}

void BreakdownItemEnergyCasterLevel::CreateOtherEffects()
{
    m_otherEffects.clear();
    if (m_pCharacter != NULL)
    {
    }
}

bool BreakdownItemEnergyCasterLevel::AffectsUs(const Effect & effect) const
{
    bool isUs = false;
    for (auto&& iit : effect.Item())
    {
        EnergyType et = TextToEnumEntry(iit, energyTypeMap, false);
        if (et == m_energy)
        {
            isUs |= true;
        }
    }
    return isUs;
}
