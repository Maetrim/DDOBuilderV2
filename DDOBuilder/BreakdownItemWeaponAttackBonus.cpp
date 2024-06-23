// BreakdownItemWeaponAttackBonus.cpp
//
#include "stdafx.h"
#include "BreakdownItemWeaponAttackBonus.h"

#include "GlobalSupportFunctions.h"
#include "Character.h"
#include "BreakdownItemWeaponEffects.h"

const std::string c_TWF = "Two Weapon Fighting";
const std::string c_OTWF = "Oversized Two Weapon Fighting";

BreakdownItemWeaponAttackBonus::BreakdownItemWeaponAttackBonus(
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

BreakdownItemWeaponAttackBonus::~BreakdownItemWeaponAttackBonus()
{
}

// required overrides
CString BreakdownItemWeaponAttackBonus::Title() const
{
    return m_title;
}

CString BreakdownItemWeaponAttackBonus::Value() const
{
    CString value;
    value.Format(
            "%+3d",
            (int)Total());
    return value;
}

void BreakdownItemWeaponAttackBonus::CreateOtherEffects()
{
    if (m_pCharacter != NULL)
    {
        m_otherEffects.clear();
        // all weapon to hits include BAB, which may be changed by a
        // enhancement
        BreakdownItem * pBBAB = FindBreakdown(Breakdown_BAB);
        ASSERT(pBBAB != NULL);
        pBBAB->AttachObserver(this);  // need to know about changes to this effect
        double amount = pBBAB->Total();
        if (amount > 0)
        {
            Effect amountTrained(
                    Effect_Unknown,         // doesn't matter
                    "Base Attack Bonus",
                    "Base",
                    amount);
            AddOtherEffect(amountTrained);
        }
        // must be proficient with the weapon
        if (!m_pCharacter->ActiveBuild()->IsWeaponInGroup("Proficiency", m_weapon))
        {
            // non-proficient penalty
            Effect nonProficient(
                Effect_Unknown,         // doesn't matter
                "Non proficient penalty",
                "Penalty",
                -4);
            AddOtherEffect(nonProficient);
        }

        // attack penalty due to negative levels
        BreakdownItem* pNL = FindBreakdown(Breakdown_NegativeLevels);
        if (pNL != NULL)
        {
            pNL->AttachObserver(this); // need to know about changes
            int negLevels = static_cast<int>(pNL->Total());
            if (negLevels != 0)
            {
                Effect negLevelsEffect(
                    Effect_Unknown,
                    "Negative Levels",
                    "Negative Levels",
                    -1 * negLevels);          // -1 per neg level
                AddOtherEffect(negLevelsEffect);
            }
        }

        //TBD - Not penalty applied if you are proficient with your
        // armor or shields

        // also apply any armor check penalty
        BreakdownItem* pBACP = FindBreakdown(Breakdown_ArmorCheckPenalty);
        ASSERT(pBACP != NULL);
        pBACP->AttachObserver(this);  // need to know about changes to this effect
        double total = max(0, pBACP->Total()); // ACP cannot be a bonus!
        if (total != 0)
        {
            Effect acp(
                    Effect_Unknown,         // doesn't matter
                    "Armor check penalty",
                    "Penalty",
                    -total);
            AddOtherEffect(acp);
        }

        // add any weapon enchantment
        BreakdownItem* pBI = FindBreakdown(Breakdown_WeaponEffectHolder);
        BreakdownItemWeaponEffects* pBIW = dynamic_cast<BreakdownItemWeaponEffects*>(pBI);
        if (pBIW != NULL)
        {
            BreakdownItem* pBWE = pBIW->GetWeaponBreakdown(!m_bOffhandWeapon, Breakdown_WeaponEnchantment);
            if (pBWE != NULL)
            {
                pBWE->AttachObserver(this);  // need to know about changes to this effect
                total = pBWE->Total();
                if (total != 0)
                {
                    Effect we(
                            Effect_Unknown,         // doesn't matter
                            "Weapon Enchantment",
                            "Weapon Enchantment",
                            total);
                    AddOtherEffect(we);
                }
            }
        }

        // by default all weapons use Strength as their base stat for attack bonus
        // but other stats may also be allowed for this particular weapon. look through
        // the list of those available and get the one with the largest value
        AbilityType ability = LargestStatBonus();
        if (ability != Ability_Unknown)
        {
            BreakdownItem* pBStat = FindBreakdown(StatToBreakdown(ability));
            ASSERT(pBStat != NULL);
            int bonus = BaseStatToBonus(pBStat->Total());
            if (bonus != 0) // only add to list if non zero
            {
                // should now have the best option
                std::string bonusName = "Ability bonus (" + EnumEntryText(ability, abilityTypeMap) + ")";
                Effect abilityBonus(
                        Effect_AbilityBonus,
                        bonusName,
                        "Ability",
                        bonus);
                AddOtherEffect(abilityBonus);
            }
        }

        Build* pBuild = m_pCharacter->ActiveBuild();
        if (pBuild->IsStanceActive("Two Weapon Fighting"))
        {
            int bonus = 0;
            // they are two weapon fighting, apply attack penalties to this weapon
            if (pBuild->IsFeatTrained(c_TWF))
            {
                // -4/-4 penalty when TWF with light off hand weapon
                bonus = -4;
            }
            else
            {
                // -6/-10 penalty when no TWF
                bonus = m_bOffhandWeapon ? -10: -6;
            }
            // heavy weapon off hand weapon penalty
            WeaponType wtOffhand = pBuild->OffhandWeapon();
            if (pBuild->IsWeaponInGroup("Light", wtOffhand)
                    || m_pCharacter->ActiveBuild()->IsFeatTrained(c_OTWF))
            {
                // 2 less penalty if off hand weapon is light
                // or over sized TWF is trained
                bonus += 2;
            }
            Effect twf(
                    Effect_Unknown,
                    "TWF attack penalty",
                    "Penalty",
                    bonus);
            AddOtherEffect(twf);
        }
    }
}

bool BreakdownItemWeaponAttackBonus::AffectsUs(const Effect & effect) const
{
    // note that we only get here if the effect is for this weapon type
    // we just need to filter on the effects we are interested in
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
    if (effect.IsType(Effect_Weapon_AttackAbility))
    {
        // weapon enchantments affect us if specific weapon
        isUs = true;
    }
    if (m_bCriticalEffects)
    {
        if (effect.IsType(Effect_Weapon_AttackBonusCritical))
        {
            isUs = true;
        }
        if (effect.IsType(Effect_WeaponAttackBonusCriticalClass))
        {
            isUs = true;
        }
        if (effect.IsType(Effect_WeaponAttackBonusCriticalDamageType))
        {
            isUs = true;
        }
    }
    else
    {
        if (effect.IsType(Effect_Weapon_AttackBonus))
        {
            isUs = true;
        }
        if (effect.IsType(Effect_WeaponAttackBonusClass))
        {
            isUs = true;
        }
        if (effect.IsType(Effect_WeaponAttackBonusDamageType))
        {
            isUs = true;
        }
    }
    return isUs;
}

void BreakdownItemWeaponAttackBonus::LinkUp()
{
    BreakdownItem* pBI = FindBreakdown(Breakdown_WeaponEffectHolder);
    BreakdownItemWeaponEffects* pBIW = dynamic_cast<BreakdownItemWeaponEffects*>(pBI);
    if (pBIW != NULL)
    {
        BreakdownItem* pBWE = pBIW->GetWeaponBreakdown(!m_bOffhandWeapon, Breakdown_WeaponEnchantment);
        if (pBWE != NULL)
        {
            pBWE->AttachObserver(this);  // need to know about changes to this effect
        }
    }
}

void BreakdownItemWeaponAttackBonus::ClassChanged(
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

void BreakdownItemWeaponAttackBonus::FeatEffectApplied(
    Build* pBuild,
    const Effect& effect)
{
    if (AffectsUs(effect))
    {
        // handle special affects that change our list of available stats
        if (effect.IsType(Effect_Weapon_AttackAbility))
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

void BreakdownItemWeaponAttackBonus::FeatEffectRevoked(
    Build* pBuild,
    const Effect& effect)
{
    if (AffectsUs(effect))
    {
        // handle special affects that change our list of available stats
        if (effect.IsType(Effect_Weapon_AttackAbility))
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

void BreakdownItemWeaponAttackBonus::ItemEffectApplied(
    Build* pBuild,
    const Effect& effect)
{
    if (AffectsUs(effect))
    {
        // handle special affects that change our list of available stats
        if (effect.IsType(Effect_Weapon_AttackAbility))
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

void BreakdownItemWeaponAttackBonus::ItemEffectRevoked(
    Build* pBuild,
    const Effect& effect)
{
    if (AffectsUs(effect))
    {
        // handle special affects that change our list of available stats
        if (effect.IsType(Effect_Weapon_AttackAbility))
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

void BreakdownItemWeaponAttackBonus::EnhancementEffectApplied(
    Build* pBuild,
    const Effect& effect)
{
    if (AffectsUs(effect))
    {
        // handle special affects that change our list of available stats
        if (effect.IsType(Effect_Weapon_AttackAbility))
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

void BreakdownItemWeaponAttackBonus::EnhancementEffectRevoked(
    Build* pBuild,
    const Effect& effect)
{
    if (AffectsUs(effect))
    {
        // handle special affects that change our list of available stats
        if (effect.IsType(Effect_Weapon_AttackAbility))
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

void BreakdownItemWeaponAttackBonus::UpdateTotalChanged(
        BreakdownItem * item,
        BreakdownType type)
{
    // ability stat for this skill has changed, update our skill total
    CreateOtherEffects();
    // do base class stuff also
    BreakdownItem::UpdateTotalChanged(item, type);
}

void BreakdownItemWeaponAttackBonus::FeatTrained(
    Build* pBuild,
    const std::string& featName)
{
    UNREFERENCED_PARAMETER(pBuild);
    // TWF and Oversized TWF affect attack penalty
    if (featName == c_TWF
            || featName == c_OTWF)
    {
        CreateOtherEffects();
    }
}

void BreakdownItemWeaponAttackBonus::FeatRevoked(
    Build* pBuild,
    const std::string& featName)
{
    UNREFERENCED_PARAMETER(pBuild);
    // TWF and Oversized TWF affect attack penalty
    if (featName == c_TWF
        || featName == c_OTWF)
    {
        CreateOtherEffects();
    }
}

