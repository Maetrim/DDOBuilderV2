// BreakdownItemWeaponDamageBonus.cpp
//
#include "stdafx.h"
#include "BreakdownItemWeaponDamageBonus.h"

#include "GlobalSupportFunctions.h"

BreakdownItemWeaponDamageBonus::BreakdownItemWeaponDamageBonus(
        BreakdownType type,
        EffectType effect,
        const CString & title,
        MfcControls::CTreeListCtrl * treeList,
        HTREEITEM hItem,
        bool bOffhandWeapon,
        bool bCriticalEffects) :
    BreakdownItem(type, treeList, hItem),
    m_title(title),
    m_effect(effect),
    m_bOffhandWeapon(bOffhandWeapon),
    m_bCriticalEffects(bCriticalEffects)
{
}

BreakdownItemWeaponDamageBonus::~BreakdownItemWeaponDamageBonus()
{
}

// required overrides
CString BreakdownItemWeaponDamageBonus::Title() const
{
    return m_title;
}

CString BreakdownItemWeaponDamageBonus::Value() const
{
    CString value;
    value.Format(
            "%+3d",
            (int)Total());
    return value;
}

void BreakdownItemWeaponDamageBonus::CreateOtherEffects()
{
    if (m_pCharacter != NULL)
    {
        m_otherEffects.clear();

        // by default all weapons use Strength as their base stat for bonus damage
        // but other stats may also be allowed for this particular weapon. look through
        // the list of those available and get the one with the largest value
        AbilityType ability = LargestStatBonus();
        if (ability != Ability_Unknown)
        {
            double multiplier = 1.0;        // assume
            BreakdownItem * pBI = FindBreakdown(StatToBreakdown(ability));
            ASSERT(pBI != NULL);
            int bonus = BaseStatToBonus(pBI->Total());
            if (m_bOffhandWeapon)
            {
                pBI = FindBreakdown(Breakdown_DamageAbilityMultiplierOffhand);
                multiplier = pBI->Total();
            }
            else
            {
                pBI = FindBreakdown(Breakdown_DamageAbilityMultiplier);
                multiplier = pBI->Total();
            }
            if (bonus != 0) // only add to list if non zero
            {
                // should now have the best option
                std::string bonusName;
                if (multiplier != 1.0)
                {
                    CString text;
                    text.Format(
                            "%d%% of Ability bonus (%s)",
                            (int)(multiplier * 100),
                            (LPCTSTR)EnumEntryText(ability, abilityTypeMap));
                    bonusName = text;
                    bonus = (int)(bonus * multiplier);
                }
                else
                {
                    bonusName = "Ability bonus (" + EnumEntryText(ability, abilityTypeMap) + ")";
                }
                Effect abilityBonus(
                        Effect_AbilityBonus,
                        bonusName,
                        "Ability",
                        bonus);
                AddOtherEffect(abilityBonus);
            }
        }
    }
}

bool BreakdownItemWeaponDamageBonus::AffectsUs(const Effect & effect) const
{
    bool isUs = false;
    if (effect.IsType(m_effect))
    {
        // if its the right effect its for us as our holder class determines whether
        // it is the right weapon target type
        isUs = true;
    }
    if (effect.IsType(Effect_Weapon_AttackAndDamage))
    {
        isUs = true;
    }
    if (effect.IsType(Effect_Weapon_DamageAbility))
    {
        // weapon enchantments affect us if specific weapon
        isUs = true;
    }
    if (effect.IsType(Effect_Weapon_Enchantment))
    {
        // weapon enchantments affect us if specific weapon
        isUs = true;
    }
    if (m_bCriticalEffects)
    {
        if (effect.IsType(Effect_WeaponDamageBonusCriticalClass))
        {
            isUs = true;
        }
        if (effect.IsType(Effect_WeaponDamageBonusCriticalDamageType))
        {
            isUs = true;
        }
    }
    else
    {
        if (effect.IsType(Effect_WeaponDamageBonusClass))
        {
            isUs = true;
        }
        if (effect.IsType(Effect_WeaponDamageBonusDamageType))
        {
            isUs = true;
        }
    }
    return isUs;
}

void BreakdownItemWeaponDamageBonus::ClassChanged(
    Build* pBuild,
    const std::string& classFrom,
    const std::string& classTo,
    size_t level)
{
    BreakdownItem::ClassChanged(pBuild, classFrom, classTo, level);
    // if a class has changed, then the BAB may have changed
    // can also affect attack bonus due to favored Soul levels with any feat
    // of "Grace of Battle" or "Knowledge of Battle"
    CreateOtherEffects();
}

void BreakdownItemWeaponDamageBonus::FeatEffectApplied(
    Build* pBuild,
    const Effect& effect)
{
    if (AffectsUs(effect))
    {
        // handle special affects that change our list of available stats
        if (effect.IsType(Effect_Weapon_DamageAbility))
        {
            // add to the list of available stats for this weapon
            AbilityType at = TextToEnumEntry(effect.Item().front(), abilityTypeMap);
            if (effect.HasRequirementsToBeActive())
            {
                AddAbility(
                    at,
                    effect.RequirementsToBeActive(),
                    Weapon());  // duplicates are fine
            }
            else
            {
                AddAbility(at);
            }
            CreateOtherEffects();
        }
        else
        {
            // pass through to the base class
            BreakdownItem::FeatEffectApplied(pBuild, effect);
        }
    }
}

void BreakdownItemWeaponDamageBonus::FeatEffectRevoked(
    Build* pBuild,
    const Effect& effect)
{
    if (AffectsUs(effect))
    {
        // handle special affects that change our list of available stats
        if (effect.IsType(Effect_Weapon_DamageAbility))
        {
            // add to the list of available stats for this weapon
            AbilityType at = TextToEnumEntry(effect.Item().front(), abilityTypeMap);
            if (effect.HasRequirementsToBeActive())
            {
                RemoveFirstAbility(
                    at,
                    effect.RequirementsToBeActive(),
                    Weapon());  // duplicates are fine
            }
            else
            {
                RemoveFirstAbility(at);
            }
            CreateOtherEffects();
        }
        else
        {
            // pass through to the base class
            BreakdownItem::FeatEffectRevoked(pBuild, effect);
        }
    }
}

void BreakdownItemWeaponDamageBonus::ItemEffectApplied(
    Build* pBuild,
    const Effect& effect)
{
    if (AffectsUs(effect))
    {
        // handle special affects that change our list of available stats
        if (effect.IsType(Effect_Weapon_DamageAbility))
        {
            // add to the list of available stats for this weapon
            AbilityType at = TextToEnumEntry(effect.Item().front(), abilityTypeMap);
            if (effect.HasRequirementsToBeActive())
            {
                AddAbility(
                    at,
                    effect.RequirementsToBeActive(),
                    Weapon());  // duplicates are fine
            }
            else
            {
                AddAbility(at);
            }
            CreateOtherEffects();
        }
        else
        {
            // pass through to the base class
            BreakdownItem::ItemEffectApplied(pBuild, effect);
        }
    }
}

void BreakdownItemWeaponDamageBonus::ItemEffectRevoked(
    Build* pBuild,
    const Effect& effect)
{
    if (AffectsUs(effect))
    {
        // handle special affects that change our list of available stats
        if (effect.IsType(Effect_Weapon_DamageAbility))
        {
            // add to the list of available stats for this weapon
            AbilityType at = TextToEnumEntry(effect.Item().front(), abilityTypeMap);
            if (effect.HasRequirementsToBeActive())
            {
                RemoveFirstAbility(
                    at,
                    effect.RequirementsToBeActive(),
                    Weapon());  // duplicates are fine
            }
            else
            {
                RemoveFirstAbility(at);
            }
            CreateOtherEffects();
        }
        else
        {
            // pass through to the base class
            BreakdownItem::ItemEffectRevoked(pBuild, effect);
        }
    }
}

void BreakdownItemWeaponDamageBonus::EnhancementEffectApplied(
    Build* pBuild,
    const Effect& effect)
{
    if (AffectsUs(effect))
    {
        // handle special affects that change our list of available stats
        if (effect.IsType(Effect_Weapon_DamageAbility))
        {
            // add to the list of available stats for this weapon
            AbilityType at = TextToEnumEntry(effect.Item().front(), abilityTypeMap);
            if (effect.HasRequirementsToBeActive())
            {
                AddAbility(
                    at,
                    effect.RequirementsToBeActive(),
                    Weapon());  // duplicates are fine
            }
            else
            {
                AddAbility(at);
            }
            CreateOtherEffects();
        }
        else
        {
            // pass through to the base class
            BreakdownItem::EnhancementEffectApplied(pBuild, effect);
        }
    }
}

void BreakdownItemWeaponDamageBonus::EnhancementEffectRevoked(
    Build* pBuild,
    const Effect& effect)
{
    if (AffectsUs(effect))
    {
        // handle special affects that change our list of available stats
        if (effect.IsType(Effect_Weapon_DamageAbility))
        {
            // add to the list of available stats for this weapon
            AbilityType at = TextToEnumEntry(effect.Item().front(), abilityTypeMap);
            if (effect.HasRequirementsToBeActive())
            {
                RemoveFirstAbility(
                    at,
                    effect.RequirementsToBeActive(),
                    Weapon());  // duplicates are fine
            }
            else
            {
                RemoveFirstAbility(at);
            }
            CreateOtherEffects();
        }
        else
        {
            // pass through to the base class
            BreakdownItem::EnhancementEffectRevoked(pBuild, effect);
        }
    }
}

void BreakdownItemWeaponDamageBonus::UpdateTotalChanged(
        BreakdownItem * item,
        BreakdownType type)
{
    // ability stat for this skill has changed, update our skill total
    CreateOtherEffects();
    // do base class stuff also
    BreakdownItem::UpdateTotalChanged(item, type);
}
