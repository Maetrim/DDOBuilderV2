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
            || effect.IsType(Effect_Weapon_Keen))
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
    BreakdownItem::FeatEffectRevoked(pBuild, effect);
    HandleRevokeSpecialEffects(effect);
}

void BreakdownItemWeaponCriticalThreatRange::ItemEffectApplied(
        Build* pBuild,
        const Effect & effect)
{
    // pass through to the base class
    BreakdownItem::ItemEffectApplied(pBuild, effect);
    HandleAddSpecialEffects(effect);
}

void BreakdownItemWeaponCriticalThreatRange::ItemEffectRevoked(
        Build* pBuild,
        const Effect & effect)
{
    // pass through to the base class
    BreakdownItem::ItemEffectRevoked(pBuild, effect);
    HandleRevokeSpecialEffects(effect);
}

void BreakdownItemWeaponCriticalThreatRange::EnhancementEffectApplied(
        Build* pBuild,
        const Effect & effect)
{
    // pass through to the base class
    BreakdownItem::EnhancementEffectApplied(pBuild, effect);
    HandleAddSpecialEffects(effect);
}

void BreakdownItemWeaponCriticalThreatRange::EnhancementEffectRevoked(
        Build* pBuild,
        const Effect & effect)
{
    // pass through to the base class
    BreakdownItem::EnhancementEffectRevoked(pBuild, effect);
    HandleRevokeSpecialEffects(effect);
}

void BreakdownItemWeaponCriticalThreatRange::HandleAddSpecialEffects(
    const Effect& effect)
{
    if (effect.IsType(Effect_Weapon_Keen))
    {
        ++m_keenCount;
        if (m_keenCount == 1)
        {
            double baseRange = WeaponBaseCriticalRange();
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
            double baseRange = WeaponBaseCriticalRange();
            if (baseRange > 0)
            {
                // we have an effect that doubles this weapons base critical range
                Effect icEffect(
                    Effect_Weapon_Keen,
                    "Improved Critical Feat",
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
            double baseRange = WeaponBaseCriticalRange();
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
            double baseRange = WeaponBaseCriticalRange();
            if (baseRange > 0)
            {
                // we have an effect that doubles this weapons base critical range
                Effect icEffect(
                    Effect_Weapon_Keen,
                    "Improved Critical Feat",
                    "Keen",     // keen/improved critical do not stack
                    baseRange);
                RevokeItemEffect(icEffect);    // stacking rules only apply to Item effects
            }
        }
    }
}

size_t BreakdownItemWeaponCriticalThreatRange::WeaponBaseCriticalRange() const
{
    // from the wiki: This feat adds 1, 2, or 3 to critical threat
    // range based on the weapon type's unmodified threat range.
    // +3 for falchion, great crossbow, kukri, rapier, and scimitar.
    // +2 for bastard sword, dagger, greatsword, heavy crossbow, khopesh,
    // light crossbow, long sword, repeating heavy crossbow,
    // repeating light crossbow, short sword, and throwing dagger.
    // +1 to all other weapons.
    // Shields do not benefit from Improved Critical.
    size_t baseRange = 0;     // assume unknown
    switch (Weapon())
    {
        case Weapon_Falchion:
        case Weapon_GreatCrossbow:
        case Weapon_Kukri:
        case Weapon_Rapier:
        case Weapon_Scimitar:
            baseRange = +3;
            break;
        case Weapon_BastardSword:
        case Weapon_Dagger:
        case Weapon_GreatSword:
        case Weapon_HeavyCrossbow:
        case Weapon_Khopesh:
        case Weapon_LightCrossbow:
        case Weapon_Longsword:
        case Weapon_RepeatingHeavyCrossbow:
        case Weapon_RepeatingLightCrossbow:
        case Weapon_Shortsword:
        case Weapon_ThrowingDagger:
            baseRange = +2;
            break;
        case Weapon_ShieldBuckler:
        case Weapon_ShieldSmall:
        case Weapon_ShieldLarge:
        case Weapon_ShieldTower:
        case Weapon_Orb:
        case Weapon_RuneArm:
            baseRange = 0;
            break;
        default:
            // all other weapon types
            baseRange = +1;
            break;
    }
    return baseRange;
}

