// BreakdownItemWeaponCriticalThreatRange.cpp
//
#include "stdafx.h"
#include "BreakdownItemWeaponCriticalThreatRange.h"

#include "BreakdownsPane.h"
#include "GlobalSupportFunctions.h"

BreakdownItemWeaponCriticalThreatRange::BreakdownItemWeaponCriticalThreatRange(
        BreakdownType type,
        MfcControls::CTreeListCtrl * treeList,
        HTREEITEM hItem) :
    BreakdownItem(type, treeList, hItem),
    m_keenCount(0),
    m_improvedCriticalCount(0)
{
}

BreakdownItemWeaponCriticalThreatRange::~BreakdownItemWeaponCriticalThreatRange()
{
}

// required overrides
CString BreakdownItemWeaponCriticalThreatRange::Title() const
{
    return "Critical Threat Range";
}

CString BreakdownItemWeaponCriticalThreatRange::Value() const
{
    CString value;
    int total = (int)Total();
    if (total > 1)
    {
        value.Format("%d-20", 21 - total);  // e.g. 19-20
    }
    else
    {
        value = "20";
    }
    return value;
}

void BreakdownItemWeaponCriticalThreatRange::CreateOtherEffects()
{
    if (m_pCharacter != NULL)
    {
        m_otherEffects.clear();
    }
}

bool BreakdownItemWeaponCriticalThreatRange::AffectsUs(const Effect & effect) const
{
    bool isUs = false;
    if (effect.IsType(Effect_Weapon_CriticalRange)
            || effect.IsType(Effect_Weapon_Keen)
            || effect.IsType(Effect_WeaponCriticalRangeClass))
    {
        // if its the right effect its for us as our holder class determines whether
        // it is the right weapon target type
        isUs = true;
    }
    if (effect.IsType(Effect_Weapon_KeenDamageType))
    {
        // needs to be correct weapon type
        std::string group = effect.Item().front();
        const Build* pBuild = m_pCharacter->ActiveBuild();
        if (pBuild != NULL
                && pBuild->IsWeaponInGroup(group, Weapon()))
        {
            isUs = true;
        }
    }
    return isUs;
}

void BreakdownItemWeaponCriticalThreatRange::FeatEffectApplied(
        Build* pBuild,
        const Effect & effect)
{
    // pass through to the base class
    BreakdownItem::FeatEffectApplied(pBuild, effect);
    HandleAddSpecialEffects(effect);
}

void BreakdownItemWeaponCriticalThreatRange::FeatEffectRevoked(
        Build* pBuild,
        const Effect & effect)
{
    // pass through to the base class
    HandleRevokeSpecialEffects(effect);
    BreakdownItem::FeatEffectRevoked(pBuild, effect);
}

void BreakdownItemWeaponCriticalThreatRange::ItemEffectApplied(
        Build* pBuild,
        const Effect & effect)
{
    // pass through to the base class
    HandleAddSpecialEffects(effect);
    BreakdownItem::ItemEffectApplied(pBuild, effect);
}

void BreakdownItemWeaponCriticalThreatRange::ItemEffectRevoked(
        Build* pBuild,
        const Effect & effect)
{
    // pass through to the base class
    HandleRevokeSpecialEffects(effect);
    BreakdownItem::ItemEffectRevoked(pBuild, effect);
}

void BreakdownItemWeaponCriticalThreatRange::EnhancementEffectApplied(
        Build* pBuild,
        const Effect & effect)
{
    // pass through to the base class
    HandleAddSpecialEffects(effect);
    BreakdownItem::EnhancementEffectApplied(pBuild, effect);
}

void BreakdownItemWeaponCriticalThreatRange::EnhancementEffectRevoked(
        Build* pBuild,
        const Effect & effect)
{
    // pass through to the base class
    HandleRevokeSpecialEffects(effect);
    BreakdownItem::EnhancementEffectRevoked(pBuild, effect);
}

void BreakdownItemWeaponCriticalThreatRange::HandleAddSpecialEffects(
    const Effect& effect)
{
    if (effect.IsType(Effect_Weapon_Keen))
    {
        ++m_keenCount;
        if (m_keenCount == 1)
        {
            double baseRange = WeaponBaseCriticalRange(Weapon());
            if (baseRange > 0)
            {
                // we have an effect that doubles this weapons base critical range
                Effect keenEffect(
                    Effect_Weapon_Keen,
                    "Keen/Impact weapon",
                    "Keen",     // keen/improved critical do not stack
                    baseRange);
                AddItemEffect(keenEffect);  // stacking rules only apply to Item effects
            }
        }
    }
    if (effect.IsType(Effect_Weapon_KeenDamageType))
    {
        ++m_improvedCriticalCount;
        if (m_improvedCriticalCount == 1)
        {
            double baseRange = WeaponBaseCriticalRange(Weapon());
            if (baseRange > 0)
            {
                // we have an effect that doubles this weapons base critical range
                Effect icEffect(
                    Effect_Weapon_Keen,
                    effect.DisplayName(),
                    "Keen",     // keen/improved critical do not stack
                    baseRange);
                AddItemEffect(icEffect);    // stacking rules only apply to Item effects
            }
        }
    }
}

void BreakdownItemWeaponCriticalThreatRange::HandleRevokeSpecialEffects(
    const Effect& effect)
{
    if (effect.IsType(Effect_Weapon_Keen))
    {
        --m_keenCount;
        if (m_keenCount == 0)
        {
            double baseRange = WeaponBaseCriticalRange(Weapon());
            if (baseRange > 0)
            {
                // we have an effect that doubles this weapons base critical range
                Effect keenEffect(
                    Effect_Weapon_Keen,
                    "Keen/Impact weapon",
                    "Keen",     // keen/improved critical do not stack
                    baseRange);
                RevokeItemEffect(keenEffect);  // stacking rules only apply to Item effects
            }
        }
    }
    if (effect.IsType(Effect_Weapon_KeenDamageType))
    {
        --m_improvedCriticalCount;
        if (m_improvedCriticalCount == 0)
        {
            double baseRange = WeaponBaseCriticalRange(Weapon());
            if (baseRange > 0)
            {
                // we have an effect that doubles this weapons base critical range
                Effect icEffect(
                    Effect_Weapon_Keen,
                    effect.DisplayName(),
                    "Keen",     // keen/improved critical do not stack
                    baseRange);
                RevokeItemEffect(icEffect);    // stacking rules only apply to Item effects
            }
        }
    }
}

