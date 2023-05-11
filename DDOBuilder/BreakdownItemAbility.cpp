// BreakdownItemAbility.cpp
//
#include "stdafx.h"
#include "BreakdownItemAbility.h"
#include "BreakdownsPane.h"
#include "Race.h"

#include "GlobalSupportFunctions.h"

BreakdownItemAbility::BreakdownItemAbility(
        CBreakdownsPane* pPane,
        AbilityType ability,
        BreakdownType type,
        MfcControls::CTreeListCtrl * treeList,
        HTREEITEM hItem) :
    BreakdownItem(type, treeList, hItem),
    m_ability(ability)
{
    // register ourselves for effects that affect us
    pPane->RegisterBuildCallbackEffect(Effect_AbilityBonus, this);
}

BreakdownItemAbility::~BreakdownItemAbility()
{
}

// required overrides
CString BreakdownItemAbility::Title() const
{
    CString text = EnumEntryText(m_ability, abilityTypeMap);
    return text;
}

CString BreakdownItemAbility::Value() const
{
    CString value;
    int total = (int)Total();
    value.Format(
            "%3d (Mod %d)",          // ability values are always whole numbers
            total,
            BaseStatToBonus(total));
    return value;
}

void BreakdownItemAbility::CreateOtherEffects()
{
    if (m_pCharacter != NULL)
    {
        m_otherEffects.clear();
        Life *pLife = m_pCharacter->ActiveLife();
        Build* pBuild = m_pCharacter->ActiveBuild();
        if (pLife != NULL
                && pBuild != NULL)
        {
            const AbilitySpend & as = pBuild->BuildPoints();
            // basic build point spend
            int amount = 8 + as.GetAbilitySpend(m_ability);
            if (amount != 0)
            {
                Effect base(Effect_AbilityBonus, "Base", "Base", static_cast<double>(amount));
                AddOtherEffect(base);
            }
            // racial
            const Race & r = FindRace(pLife->Race());
            amount = r.RacialModifier(m_ability);
            if (amount != 0)
            {
                Effect racial(Effect_AbilityBonus, "Racial", "Racial", static_cast<double>(amount));
                AddOtherEffect(racial);
            }
            // tome
            amount = pLife->TomeAtLevel(m_ability, pBuild->Level()-1);
            if (amount > 0)
            {
                Effect tome(Effect_AbilityBonus, "Tome", "Inherent", static_cast<double>(amount));
                AddOtherEffect(tome);
            }
            // level ups
            amount = pLife->LevelUpsAtLevel(m_ability, pBuild->Level());
            if (amount > 0)
            {
                Effect levelUps(Effect_AbilityBonus, "Level Up", "Level Up", static_cast<double>(amount));
                AddOtherEffect(levelUps);
            }
        }
    }
}

void BreakdownItemAbility::BuildLevelChanged(Build*)
{
    // need to re-create other effects list
    CreateOtherEffects();
    Populate();
}

void BreakdownItemAbility::AbilityValueChanged(
        Build*,
        AbilityType ability)
{
    if (m_ability == ability)
    {
        // need to re-create other effects list
        CreateOtherEffects();
        Populate();
    }
}

void BreakdownItemAbility::AbilityTomeChanged(
        Life*,
        AbilityType ability)
{
    if (m_ability == ability)
    {
        // need to re-create other effects list
        CreateOtherEffects();
        Populate();
    }
}

void BreakdownItemAbility::RaceChanged(
        Life*,
        const std::string& race)
{
    UNREFERENCED_PARAMETER(race);
    // race change could affect any ability, always repopulate for this
    CreateOtherEffects();
    Populate();
}

bool BreakdownItemAbility::AffectsUs(const Effect & effect) const
{
    bool isUs = false;
    for (auto&& iit: effect.Item())
    {
        AbilityType at = TextToEnumEntry(iit, abilityTypeMap);
        if (at == Ability_All
            || at == m_ability)
        {
            isUs |= true;
        }
    }
    return isUs;
}
