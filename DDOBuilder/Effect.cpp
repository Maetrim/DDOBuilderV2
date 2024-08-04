// Effect.cpp
//
#include "StdAfx.h"
#include "Effect.h"
#include "XmlLib\SaxWriter.h"

#include "Character.h"
#include "Build.h"

//#include "Feat.h"
#include "Bonus.h"
#include "BreakdownItem.h"
#include "Class.h"
#include "GlobalSupportFunctions.h"
//#include "Spell.h"
#include "DamageReductionTypes.h"
#include "SaveTypes.h"
#include "SpellPowerTypes.h"
#include "SpellSchoolTypes.h"
#include "WeaponClassTypes.h"
#include "WeaponDamageTypes.h"
#include "WeaponTypes.h"

#include "LogPane.h"

#define DL_ELEMENT Effect

namespace
{
    const wchar_t f_saxElementName[] = L"Effect";
    DL_DEFINE_NAMES(Effect_PROPERTIES)

    const unsigned f_verCurrent = 1;
    const int f_noSpecificAmountSizeRequired = -1;
}

Effect::Effect() :
    XmlLib::SaxContentElement(f_saxElementName, f_verCurrent),
    m_stacks(1),
    m_percentValue(0.0),
    m_pBuild(NULL)
{
    DL_INIT(Effect_PROPERTIES)
}

Effect::Effect(EffectType t, const std::string& name, const std::string& group) :
    XmlLib::SaxContentElement(f_saxElementName, f_verCurrent),
    m_stacks(1),
    m_percentValue(0.0),
    m_pBuild(NULL)
{
    // this constructor is used by EnhancementSelection to create special effects
    // used for enhancement eclusion only.
    DL_INIT(Effect_PROPERTIES)
    m_Type.push_back(t);
    m_DisplayName = name;
    m_hasDisplayName = true;
    m_Bonus = "Don't care";
    m_hasBonus = true;
    m_AType = Amount_NotNeeded,
    m_hasAType = true;
    m_Item.push_back(name);
    m_Item.push_back(group);
}

Effect::Effect(EffectType t, const std::string& name, const std::string& bonusType, double amount) :
    XmlLib::SaxContentElement(f_saxElementName, f_verCurrent),
    m_stacks(1),
    m_percentValue(0.0),
    m_pBuild(NULL)
{
    DL_INIT(Effect_PROPERTIES)
    m_Type.push_back(t);
    m_DisplayName = name;
    m_hasDisplayName = true;
    m_Bonus = bonusType;
    m_hasBonus = true;
    m_AType = Amount_Simple,
    m_hasAType = true;
    m_Amount.push_back(amount);
    m_hasAmount = true;
}

void Effect::SetBuild(const Build* pBuild)
{
    m_pBuild = pBuild;
}

DL_DEFINE_ACCESS(Effect_PROPERTIES)

XmlLib::SaxContentElementInterface * Effect::StartElement(
        const XmlLib::SaxString & name,
        const XmlLib::SaxAttributes & attributes)
{
    XmlLib::SaxContentElementInterface * subHandler =
            SaxContentElement::StartElement(name, attributes);

    DL_START(Effect_PROPERTIES)
    return subHandler;
}

void Effect::EndElement()
{
    SaxContentElement::EndElement();
    DL_END(Effect_PROPERTIES)
}

void Effect::Write(XmlLib::SaxWriter * writer) const
{
    writer->StartElement(ElementName());//, VersionAttributes());
    DL_WRITE(Effect_PROPERTIES)
    writer->EndElement();
}

bool Effect::VerifyObject(std::stringstream * ss) const
{
    bool ok = true;

    // check bonus type is known
    if (FindBonus(Bonus()).Name() == "")
    {
        (*ss) << "Effects has bad bonus type field\n";
        ok = false;
    }
    // now check all the effect types listed
    for (auto&& eit: m_Type)
    {
        bool bRequiresAmountField = false;
        int requiredAmountSize = f_noSpecificAmountSizeRequired;
        ok &= CheckAType(ss, &bRequiresAmountField, &requiredAmountSize);
        switch (eit)
        {
            case Effect_Unknown:
                (*ss) << "Has unknown effect type\n";
                ok = false;
                break;
            case Effect_ACBonus:
                // all these effect types work the same, we just need to check that the
                // items are setup correctly (done after switch)
                break;
            case Effect_AddGroupWeapon:
                if (m_Item.size() < 1)
                {
                    (*ss) << "AddGroupWeapon effect missing required element\n";
                    ok = false;
                }
                else
                {
                    auto it = m_Item.begin();
                    ++it;   // 1st item is group, cannot be checked
                    while (it != m_Item.end())
                    {
                        if (TextToEnumEntry((*it), weaponTypeMap, false) == Weapon_Unknown)
                        {
                            (*ss) << "AddGroupWeapon effect has bad enum value\n";
                            ok = false;
                        }
                        ++it;
                    }
                }
                break;
            case Effect_CreateSlider:
                if (m_Item.size() != 1)
                {
                    (*ss) << "CreateSlider effect missing required element\n";
                    ok = false;
                }
                break;
            case Effect_AbilityBonus:
                if (m_Item.size() == 0)
                {
                    (*ss) << "Ability effect missing ability field\n";
                    ok = false;
                }
                else
                {
                    auto it = m_Item.begin();
                    while (it != m_Item.end())
                    {
                        if (TextToEnumEntry((*it), abilityTypeMap, false) == Ability_Unknown)
                        {
                            (*ss) << "Ability effect has bad enum value\n";
                            ok = false;
                        }
                        ++it;
                    }
                }
                break;
            case Effect_DRBypass:
                if (m_Item.size() == 0)
                {
                    (*ss) << "DRBypass effect missing weapon type Item\n";
                    ok = false;
                    if (!HasValue())
                    {
                        (*ss) << "DRBypass effect missing Value DR field\n";
                    }
                }
                else
                {
                    if (!HasValue())
                    {
                        (*ss) << "DRBypass effect missing Value DR field\n";
                        ok = false;
                    }
                    else
                    {
                        if (TextToEnumEntry(Value(), drTypeMap, false) == DR_Unknown)
                        {
                            (*ss) << "DRBypass effect missing Value DR field\n";
                            ok = false;
                        }
                    }
                    for (auto&& it: m_Item)
                    {
                        if (TextToEnumEntry(it, weaponTypeMap, false) == Weapon_Unknown)
                        {
                            (*ss) << "DRBypass effect has bad enum Item value\n";
                            ok = false;
                        }
                    }
                }
                break;
            case Effect_Weapon_AttackAbility:
            case Effect_Weapon_DamageAbility:
                if (m_Item.size() == 0)
                {
                    (*ss) << "Weapon Ability effect missing ability field\n";
                    ok = false;
                }
                else
                {
                    auto it = m_Item.begin();
                    if (TextToEnumEntry((*it), abilityTypeMap, false) == Ability_Unknown)
                    {
                        (*ss) << "WeaponAbility effect has bad enum value\n";
                        ok = false;
                    }
                    it++;
                    while (it != m_Item.end())
                    {
                        if (TextToEnumEntry((*it), weaponTypeMap, false) == Weapon_Unknown)
                        {
                            (*ss) << "WeaponAbility effect has bad enum value\n";
                            ok = false;
                        }
                        ++it;
                    }
                }
                break;
            case Effect_WeaponAttackAbilityClass:
            case Effect_WeaponDamageAbilityClass:
                if (m_Item.size() != 2)
                {
                    (*ss) << "WeaponAbilityClass effect has wrong number of field\n";
                    ok = false;
                }
                else
                {
                    // 1st is ability
                    if (TextToEnumEntry(m_Item.front(), abilityTypeMap, false) == Ability_Unknown)
                    {
                        (*ss) << "WeaponAbilityClass effect has bad ability field\n";
                        ok = false;
                    }
                    // 2nd is weapon class
                    if (TextToEnumEntry(m_Item.back(), weaponClassTypeMap, false) == WeaponClass_Unknown)
                    {
                        (*ss) << "WeaponAbilityClass effect has bad weapon class field\n";
                        ok = false;
                    }
                }
                break;
            case Effect_EnergyAbsorbance:
            case Effect_EnergyResistance:
                // NYI
                break;
            case Effect_OverrideBAB:
                break;
            case Effect_SaveBonusAbility:
                if (m_Item.size() == 0)
                {
                    (*ss) << "Save effect missing ability field\n";
                    ok = false;
                }
                else
                {
                    auto it = m_Item.begin();
                    if (TextToEnumEntry((*it), abilityTypeMap, false) == Ability_Unknown)
                    {
                        (*ss) << "Save effect has bad enum value\n";
                        ok = false;
                    }
                    ++it;
                    while (it != m_Item.end())
                    {
                        if (TextToEnumEntry((*it), saveTypeMap, false) == Save_Unknown)
                        {
                            (*ss) << "Save effect has bad enum value\n";
                            ok = false;
                        }
                        ++it;
                    }
                }
                break;
            case Effect_SaveBonus:
                if (m_Item.size() == 0)
                {
                    (*ss) << "Save effect missing save field\n";
                    ok = false;
                }
                else
                {
                    auto it = m_Item.begin();
                    while (it != m_Item.end())
                    {
                        if (TextToEnumEntry((*it), saveTypeMap, false) == Save_Unknown)
                        {
                            (*ss) << "Save effect has bad enum value\n";
                            ok = false;
                        }
                        ++it;
                    }
                }
                break;
            case Effect_SkillBonus:
                if (m_Item.size() == 0)
                {
                    (*ss) << "SkillBonus effect missing skill field\n";
                    ok = false;
                }
                else
                {
                    for (auto&& iit: m_Item)
                    {
                        if (TextToEnumEntry(iit, skillTypeMap, false) == Skill_Unknown)
                        {
                            (*ss) << "SkillBonus effect has bad enum value\n";
                            ok = false;
                        }
                    }
                }
                break;
            case Effect_SkillBonusAbility:
                if (m_Item.size() == 0)
                {
                    (*ss) << "SkillBonusAbility effect missing skill field\n";
                    ok = false;
                }
                else
                {
                    // 1st item must the ability that supplies the bonus
                    if (TextToEnumEntry(m_Item.front(), abilityTypeMap, false) == Ability_Unknown)
                    {
                        (*ss) << "SkillBonusAbility effect has bad enum value\n";
                        ok = false;
                    }
                    auto it = m_Item.begin();
                    it++;       // skip first item
                    while (it != m_Item.end())
                    {
                        if (TextToEnumEntry((*it), abilityTypeMap, false) == Ability_Unknown)
                        {
                            (*ss) << "SkillBonusAbility effect has bad enum value\n";
                            ok = false;
                        }
                        ++it;
                    }
                }
                break;
            case Effect_TacticalDC:
                break;
            case Effect_SpellDC:
                if (m_Item.size() == 0)
                {
                    (*ss) << "SpellDC effect missing field\n";
                    ok = false;
                }
                else
                {
                    auto it = m_Item.begin();
                    while (it != m_Item.end())
                    {
                        if (TextToEnumEntry((*it), spellSchoolTypeMap, false) == SpellSchool_Unknown)
                        {
                            (*ss) << "SpellDC effect has bad enum value\n";
                            ok = false;
                        }
                        ++it;
                    }
                }
                break;
            case Effect_SpellPower:
                if (m_Item.size() == 0)
                {
                    (*ss) << "SpellPower effect missing field\n";
                    ok = false;
                }
                else
                {
                    auto it = m_Item.begin();
                    while (it != m_Item.end())
                    {
                        if (TextToEnumEntry((*it), spellPowerTypeMap, false) == SpellPower_Unknown)
                        {
                            (*ss) << "SpellPower effect has bad enum value\n";
                            ok = false;
                        }
                        ++it;
                    }
                }
                break;
            case Effect_SpellPowerReplacement:
                if (m_Item.size() != 2)
                {
                    (*ss) << "SpellPowerReplacement effect missing field\n";
                    ok = false;
                }
                else
                {
                    auto it = m_Item.begin();
                    while (it != m_Item.end())
                    {
                        if (TextToEnumEntry((*it), spellPowerTypeMap, false) == SpellPower_Unknown)
                        {
                            (*ss) << "SpellPowerReplacement effect has bad enum value\n";
                            ok = false;
                        }
                        ++it;
                    }
                }
                break;
            case Effect_WeaponProficiencyClass:
            case Effect_WeaponBaseDamageBonusClass:
                if (m_Item.size() == 0)
                {
                    (*ss) << "WeaponProficiencyClass missing enum value\n";
                    ok = false;
                }
                else
                {
                    for (auto&& iit : m_Item)
                    {
                        if (TextToEnumEntry(iit, weaponClassTypeMap, false) == WeaponClass_Unknown)
                        {
                            (*ss) << "WeaponProficiencyClass effect has bad enum value\n";
                            ok = false;
                        }
                    }
                }
                break;
            case Effect_WeaponAlacrityClass:
                if (m_Item.size() == 0)
                {
                    (*ss) << "WeaponAlacrityClass missing enum value\n";
                    ok = false;
                }
                else
                {
                    for (auto&& iit : m_Item)
                    {
                        if (TextToEnumEntry(iit, weaponClassTypeMap, false) == WeaponClass_Unknown)
                        {
                            (*ss) << "WeaponAlacrityClass effect has bad enum value\n";
                            ok = false;
                        }
                    }
                }
                break;
            case Effect_WeaponDamageBonusStat:
            case Effect_WeaponDamageBonusCriticalStat:
                if (m_Item.size() < 2)
                {
                    (*ss) << "WeaponDamageBonusStat effect missing required value\n";
                    ok = false;
                }
                else
                {
                    // first item has to be an ability
                    auto it = m_Item.begin();
                    if (TextToEnumEntry((*it), abilityTypeMap, false) == Ability_Unknown)
                    {
                        (*ss) << "WeaponDamageBonusStat effect has bad enum value\n";
                        ok = false;
                    }
                    ++it;   // rest of items are weapon types
                    while (it != m_Item.end())
                    {
                        if (TextToEnumEntry((*it), weaponTypeMap, false) == Weapon_Unknown)
                        {
                            (*ss) << "WeaponDamageBonusStat effect has bad enum value\n";
                            ok = false;
                        }
                        ++it;
                    }
                }
                break;
/*            case Effect_WeaponAttackBonusDamageType:
            case Effect_WeaponAttackBonusCriticalDamageType:
            case Effect_WeaponDamageBonusDamageType:
            case Effect_WeaponDamageBonusCriticalDamageType:
            case Effect_WeaponKeenDamageType:
            case Effect_WeaponCriticalMultiplierDamageType:
            case Effect_WeaponCriticalRangeDamageType:
                if (m_Item.size() == 0)
                {
                    (*ss) << "Weapon effect missing damage type value\n";
                    ok = false;
                }
                else
                {
                    for (auto&& iit : m_Item)
                    {
                        if (TextToEnumEntry(iit, weaponDamageTypeMap, false) == WeaponDamage_Unknown)
                        {
                            (*ss) << "Weapon effect has bad enum value\n";
                            ok = false;
                        }
                    }
                }
                break;*/
/*            case Effect_WeaponVorpalRangeClass:
            case Effect_WeaponAttackBonusClass:
            case Effect_WeaponAttackBonusCriticalClass:
            case Effect_WeaponDamageBonusClass:
            case Effect_WeaponDamageBonusCriticalClass:
            case Effect_WeaponKeenClass:
            case Effect_WeaponCriticalMultiplierClass:
            case Effect_WeaponCriticalRangeClass:
            case Effect_WeaponDamageBonusDiceClass:
            case Effect_WeaponCriticalDamageBonusDiceClass:
            case Effect_WeaponEnchantmentClass:
                    if (m_Item.size() == 0)
                {
                    (*ss) << "Weapon effect missing class value\n";
                    ok = false;
                }
                else
                {
                    for (auto&& iit : m_Item)
                    {
                        if (TextToEnumEntry(iit, weaponClassTypeMap, false) == WeaponClass_Unknown)
                        {
                            (*ss) << "Weapon effect has bad enum value\n";
                            ok = false;
                        }
                    }
                }
                break;*/
            case Effect_Weapon_Alacrity:
            case Effect_Weapon_BaseDamage:
            case Effect_Weapon_CriticalMultiplier:
            case Effect_Weapon_CriticalMultiplier19To20:
            case Effect_Weapon_CriticalRange:
            case Effect_Weapon_Enchantment:
            case Effect_Weapon_Keen:
            case Effect_Weapon_VorpalRange:
            case Effect_Weapon_OtherDamageBonus:
            case Effect_Weapon_OtherDamageBonusCritical:

            case Effect_Weapon_Attack:
            case Effect_Weapon_AttackAndDamage:
            case Effect_Weapon_Damage:

            case Effect_Weapon_AttackCritical:
            case Effect_Weapon_AttackAndDamageCritical:
            case Effect_Weapon_DamageCritical:
            case Effect_ShieldEnchantment:
                if (m_Item.size() == 0)
                {
                    (*ss) << (LPCTSTR)EnumEntryText(eit, effectTypeMap) << " effect missing weapon value\n";
                    ok = false;
                }
                else
                {
                    for (auto&& iit : m_Item)
                    {
                        if (TextToEnumEntry(iit, weaponTypeMap, false) == Weapon_Unknown)
                        {
                            (*ss) << "Weapon effect has bad enum value\n";
                            ok = false;
                        }
                    }
                }
                break;
            case Effect_GrantSpell:
            case Effect_SpellLikeAbility:
                requiredAmountSize = 3;     // we expect 3 amounts: Level, Cost, MCL
                // Look up the spell entry and ensure it exists
                if (m_Item.size() != 2)
                {
                    (*ss) << (LPCTSTR)EnumEntryText(eit, effectTypeMap) << " effect missing/extra Item fields\n";
                    ok = false;
                }
                else
                {
                    Spell spell = FindSpellByName(m_Item.front(), true);
                    if (spell.Name() != m_Item.front())
                    {
                        spell = FindItemClickieByName(m_Item.front(), true);
                        if (spell.Name() != m_Item.front())
                        {
                            (*ss) << (LPCTSTR)EnumEntryText(eit, effectTypeMap) << " effect references unknown spell\n";
                            ok = false;
                        }
                    }
                    const ::Class& c = FindClass(m_Item.back());
                    if (c.Name() == ""
                            && m_Item.back() != "None")
                    {
                        (*ss) << (LPCTSTR)EnumEntryText(eit, effectTypeMap) << " effect references unknown class\n";
                        ok = false;
                    }
                }
                break;
            case Effect_Immunity:
                // must have at least 1 item
                if (m_Item.size() == 0)
                {
                    (*ss) << "Immunity effect missing field\n";
                    ok = false;
                }
                break;
            case Effect_CasterLevel:
                if (m_Item.size() == 0)
                {
                    (*ss) << "CasterLevel effect missing class value\n";
                    ok = false;
                }
                //else
                //{
                //    for (auto&& iit : m_Item)
                //    {
                //        const ::Class & c = FindClass(iit);
                //        if (c.Name() == "")
                //        {
                //            (*ss) << "CasterLevel effect has bad class value\n";
                //            ok = false;
                //        }
                //    }
                //}
                break;
        }
        if (HasRequirementsToBeActive())
        {
            ok &= m_RequirementsToBeActive.VerifyObject(ss);
        }
        if (bRequiresAmountField)
        {
            if (HasAmount())
            {
                // realize the vector to catch size/data mismatches
                std::vector<double> d = m_Amount;
                if (requiredAmountSize != f_noSpecificAmountSizeRequired
                        && requiredAmountSize != static_cast<int>(d.size()))
                {
                    (*ss) << "Effect has incorrect Amount field size\n";
                    ok = false;
                }
            }
            else
            {
                (*ss) << "Effect missing Amount field\n";
                ok = false;
            }
        }
        else
        {
            if (HasAmount())
            {
                (*ss) << "Effect has erroneous Amount field\n";
                ok = false;
            }
        }
    }
    return ok;
}

bool Effect::CheckAType(
    std::stringstream* ss,
    bool* bRequiresAmount,
    int* requiredAmountElements) const
{
    bool ok = true;
    switch (m_AType)
    {
    default:
    case Amount_Unknown:
        (*ss) << "Has unknown effect AType\n";
        ok = false;
        break;
    case Amount_NotNeeded:
        // no Amount fields required, other m_Item checked elsewhere
        break;
    case Amount_Simple:
        *bRequiresAmount = true;            // its a a direct number * stacks
        *requiredAmountElements = 1;        // single element
        break;
    case Amount_AbilityValue:       // all handled the same for checking
    case Amount_AbilityMod:         // all handled the same for checking
    case Amount_HalfAbilityMod:     // all handled the same for checking
    case Amount_ThirdAbilityMod:    // all handled the same for checking
        if (!HasStackSource())
        {
            (*ss) << "Ability Effect missing StackSource\n";
            ok = false;
        }
        else
        {
            if (TextToEnumEntry(StackSource(), abilityTypeMap, false) == Ability_Unknown
                    && TextToEnumEntry(StackSource(), abilitySnapshotTypeMap, false) == Ability_Unknown)
            {
                (*ss) << "Ability effect has bad enum value\n";
                ok = false;
            }
        }
        break;
    case Amount_Slider:
        *bRequiresAmount = true;            // Amount specifies default, min and mx
        *requiredAmountElements = 3;        // 3 elements expected
        break;
    case Amount_SliderValue:
        *bRequiresAmount = true;           // no values
        *requiredAmountElements = 1;        // 1 element expected
        break;
    case Amount_APCount:
        *bRequiresAmount = true;           // no values
        *requiredAmountElements = 1;        // 1 element expected
        break;
    case Amount_BaseClassLevel:
        // expect a single Item thats a class and 20 Amount Items
        *bRequiresAmount = true;                    // Amount specifies amount at each class level
        *requiredAmountElements = MAX_CLASS_LEVEL + 1; // 20 elements expected
        if (!HasStackSource())
        {
            (*ss) << "BaseClassLevel effect missing StackSource field\n";
            ok = false;
        }
        else
        {
            const ::Class & c = FindClass(StackSource());
            if (c.Name() == "")
            {
                (*ss) << "BaseClassLevel effect has bad class StackSource field\n";
                ok = false;
            }
        }
        break;
    case Amount_ClassLevel:
        // expect a single Item thats a class and 20 Amount Items
        *bRequiresAmount = true;                    // Amount specifies amount at each class level
        *requiredAmountElements = MAX_CLASS_LEVEL + 1; // 20 elements expected
        if (!HasStackSource())
        {
            (*ss) << "ClassLevel effect missing StackSource field\n";
            ok = false;
        }
        else
        {
            const ::Class& c = FindClass(StackSource());
            if (c.Name() == "")
            {
                (*ss) << "ClassLevel effect has bad class StackSource field\n";
                ok = false;
            }
        }
        break;
    case Amount_ClassCasterLevel:
        // expect a single Item thats a class and 20 Amount Items
        *bRequiresAmount = true;                    // Amount specifies amount at each class level
        *requiredAmountElements = MAX_CLASS_LEVEL + 1; // 20 elements expected
        if (!HasStackSource())
        {
            (*ss) << "ClassCasterLevel effect missing StackSource field\n";
            ok = false;
        }
        else
        {
            const ::Class & c = FindClass(StackSource());
            if (c.Name() == "")
            {
                (*ss) << "ClassCasterLevel effect has bad class StackSource field\n";
                ok = false;
            }
        }
        break;
    case Amount_TotalLevel:
        *bRequiresAmount = true;                            // Amount specifies amount at each level
        *requiredAmountElements = MAX_BUILDER_LEVEL;        // 40 elements expected
        break;

    case Amount_Stacks:
        *bRequiresAmount = true;                    // Amount specifies amount at each stack level
        // unknown Amount field size, checked at run time when effects stack
        break;

    case Amount_FeatCount:
        *bRequiresAmount = true;                    // Amount specifies amount at each stack level
        // unknown Amount field size, checked at run time when effects stack
        if (!HasStackSource())
        {
            (*ss) << "Effect missing StackSource\n";
            ok = false;
        }
        else
        {
            if (FindFeat(StackSource()).Name() == "")
            {
                (*ss) << "Effect has bad feat StackSource\n";
                ok = false;
            }
        }
        break;
    case Amount_SpellInfo:
    case Amount_SLA:
        *bRequiresAmount = true;                    // Amount specifies level,cost,mcl
        *requiredAmountElements = 3;                // 3 elements expected
        break;

    case Amount_Dice:
    case Amount_CriticialDice:
        *bRequiresAmount = false;
        *requiredAmountElements = 0;
        break;
    }

    return ok;
}

bool Effect::IsType(EffectType type) const
{
    bool is = false;
    for (auto&& it : m_Type)
    {
        is |= (it == type);
    }
    return is;
}

bool Effect::HasSpellPower(SpellPowerType type) const
{
    bool is = false;
    for (auto&& it : m_Item)
    {
        SpellPowerType st = TextToEnumEntry(it, spellPowerTypeMap, false);
        is |= ((st == type)
            || (st == SpellPower_All));
    }
    return is;
}

bool Effect::HasSpellSchool(SpellSchoolType type) const
{
    bool is = false;
    for (auto&& it : m_Item)
    {
        SpellSchoolType st = TextToEnumEntry(it, spellSchoolTypeMap, false);
        is |= ((st == type)
            || (st == SpellSchool_All));
    }
    return is;
}

bool Effect::HasTacticalType(TacticalType type) const
{
    bool is = false;
    for (auto&& it : m_Item)
    {
        TacticalType tt = TextToEnumEntry(it, tacticalTypeMap, false);
        is |= (tt == type);
    }
    return is;
}

void Effect::SetDisplayName(const std::string displayName)
{
    Set_DisplayName(displayName);
}

bool Effect::operator==(const Effect & other) const
{
    return (m_hasDisplayName == other.m_hasDisplayName)
            && (m_DisplayName == other.m_DisplayName)
            && (m_Type == other.m_Type)
            && (m_Bonus == other.m_Bonus)
            && (m_Item == other.m_Item)
            && (m_AType == other.m_AType)
            && (m_hasAmount == other.m_hasAmount)
            && (m_Amount == other.m_Amount)
            && (m_hasValue == other.m_hasValue)
            && (m_Value == other.m_Value)
            && (m_hasStackSource == other.m_hasStackSource)
            && (m_StackSource == other.m_StackSource)
            && (m_hasPercent == other.m_hasPercent)
            && (m_hasWeapon1 == other.m_hasWeapon1)
            && (m_hasWeapon2 == other.m_hasWeapon2)
            && (m_hasApplyAsItemEffect == other.m_hasApplyAsItemEffect)
            && (m_hasUpdateAutomaticEffects == other.m_hasUpdateAutomaticEffects)
            && (m_hasRare == other.m_hasRare)
            && (m_RequirementsToBeActive == other.m_RequirementsToBeActive)
            //&& (m_hasRank == other.m_hasRank)
            //&& (m_Rank == other.m_Rank)
            && (m_hasDamageDice == other.m_hasDamageDice)
            && (m_DamageDice == other.m_DamageDice)
            && (m_hasDamage == other.m_hasDamage)
            && (m_Damage == other.m_Damage)
            && (m_Bonus != "Unique");
}

bool Effect::IsActive(
        const Character& c,
        InventorySlotType slot,
        WeaponType wtMain,
        WeaponType wtOffhand) const
{
    bool bActive = true;
    if (HasRequirementsToBeActive())
    {
        const Build* pBuild = c.ActiveBuild();
        if (pBuild != NULL)
        {
            bActive = RequirementsToBeActive().Met(*pBuild, pBuild->Level()-1, true, slot, wtMain, wtOffhand);
        }
    }
    return bActive;
}

size_t Effect::EffectStacks() const
{
    return m_stacks;
}

void Effect::AddEffectStack()
{
    ++m_stacks;
}

bool Effect::RevokeEffectStack()
{
    if (m_AType == Amount_SliderValue)
    {
    // have to clear all stack for this effect type
        m_stacks = 0;
    }
    else
    {
        --m_stacks;
    }
    return (m_stacks == 0);     // true if last stack gone
}

std::string Effect::StacksAsString() const
{
    std::stringstream ss;
    switch (m_AType)
    {
    case Amount_Simple:
    case Amount_Dice:
    case Amount_CriticialDice:
        // fall through
    case Amount_Stacks:
        ss << m_stacks;
        break;
    case Amount_TotalLevel:
        ss << m_pBuild->Level() << " Levels";
        break;
    case Amount_BaseClassLevel:
        ss << m_pBuild->BaseClassLevels(StackSource(), m_pBuild->Level() - 1);
        ss << " " << StackSource();
        break;
    case Amount_ClassLevel:
        ss << m_pBuild->ClassLevels(StackSource(), m_pBuild->Level() - 1);
        ss << " " << StackSource();
        break;
    case Amount_ClassCasterLevel:
        {
            const ::Class& c = FindClass(StackSource());
            BreakdownItem* pBI = FindBreakdown(static_cast<BreakdownType>(Breakdown_CasterLevel_First + c.Index()));
            if (pBI != NULL)
            {
                int level = static_cast<int>(pBI->Total());
                level = min(level, MAX_CLASS_LEVEL);
                ss << level << " " << StackSource() << " Caster Levels";
            }
            else
            {
                ss << "Not Found";
            }
        }
        break;
    case Amount_APCount:
        ss << m_pBuild->APSpentInTree(StackSource()) << " APs";
        break;
    case Amount_FeatCount:
    case Amount_AbilityValue:
    case Amount_AbilityMod:
    case Amount_Slider:
    case Amount_SliderValue:
        ss << m_stacks;
        break;
    case Amount_SetBonusCount:
        ss << m_pBuild->SetBonusCount(StackSource()) << " Set Bonus Count";
        break;
    }
    return ss.str();
}

void Effect::SetEffectStacks(size_t count)
{
    m_stacks = count;
}

double Effect::TotalAmount(bool allowTruncate) const
{
    UNREFERENCED_PARAMETER(allowTruncate);
    double total = 0.0;
    // the way the total amount is calculated depends on the type
    switch (m_AType)
    {
        case Amount_Simple:
            // it a straight value * stacks
            switch (m_Amount.size())
            {
                default:
                {
                    std::stringstream ss;
                    ss << "Effect \"" << DisplayName() << "\" has missing/bad Amount field.";
                    GetLog().AddLogEntry(ss.str().c_str());
                    break;
                }
                case 1: total = m_Amount[0] * m_stacks;
                    break;
            }
            break;
        case Amount_Stacks:
        {
            // vector lookup
            int vi = min(m_stacks - 1, m_Amount.size() - 1);
            if (vi < 0
                || m_stacks > m_Amount.size())
            {
                std::stringstream ss;
                ss << "Effect \"" << DisplayName() << "\" has incorrect Amount vector size. Stacks present are " << m_stacks;
                GetLog().AddLogEntry(ss.str().c_str());
            }
            total = m_Amount[vi];
            break;
        }
        case Amount_TotalLevel:
            {
                size_t level = m_pBuild->Level();
                // vector lookup
                int vi = min(level - 1, m_Amount.size() - 1);
                if (vi < 0
                    || level > m_Amount.size())
                {
                    std::stringstream ss;
                    ss << "Effect \"" << DisplayName() << "\" has incorrect Amount vector size. Stacks present are " << level;
                    GetLog().AddLogEntry(ss.str().c_str());
                }
                total = m_Amount[vi] * m_stacks;
                break;
            }
        case Amount_BaseClassLevel:
            // StackSource specifies the class in question
            {
                if (!HasStackSource())
                {
                    std::stringstream ss;
                    ss << "Effect \"" << DisplayName() << "\" missing StackSource";
                    GetLog().AddLogEntry(ss.str().c_str());
                }
                size_t level = m_pBuild->BaseClassLevels(StackSource(), m_pBuild->Level()-1);
                // vector lookup
                int vi = min(level, m_Amount.size() - 1);
                if (vi < 0
                    || level > m_Amount.size())
                {
                    std::stringstream ss;
                    ss << "Effect \"" << DisplayName() << "\" has incorrect Amount vector size. Stacks present are " << level;
                    GetLog().AddLogEntry(ss.str().c_str());
                }
                total = m_Amount[vi] * m_stacks;
                break;
            }
        case Amount_ClassLevel:
            // StackSource specifies the class in question
            {
                if (!HasStackSource())
                {
                    std::stringstream ss;
                    ss << "Effect \"" << DisplayName() << "\" missing StackSource";
                    GetLog().AddLogEntry(ss.str().c_str());
                }
                size_t level = m_pBuild->ClassLevels(StackSource(), m_pBuild->Level()-1);
                // vector lookup
                int vi = min(level, m_Amount.size() - 1);
                if (vi < 0
                    || level > m_Amount.size())
                {
                    std::stringstream ss;
                    ss << "Effect \"" << DisplayName() << "\" has incorrect Amount vector size. Stacks present are " << level;
                    GetLog().AddLogEntry(ss.str().c_str());
                }
                total = m_Amount[vi] * m_stacks;
                break;
            }
        case Amount_SetBonusCount:
            // StackSource specifies the set bonus in question
            {
                if (!HasStackSource())
                {
                    std::stringstream ss;
                    ss << "Effect \"" << DisplayName() << "\" missing StackSource";
                    GetLog().AddLogEntry(ss.str().c_str());
                }
                size_t count = m_pBuild->SetBonusCount(StackSource());
                // vector lookup
                int vi = min(count, m_Amount.size() - 1);
                if (vi < 0
                    || count > m_Amount.size())
                {
                    std::stringstream ss;
                    ss << "Effect \"" << DisplayName() << "\" has incorrect Amount vector size. Stacks present are " << count;
                    GetLog().AddLogEntry(ss.str().c_str());
                }
                total = m_Amount[vi] * m_stacks;
                break;
            }
        case Amount_ClassCasterLevel:
            // StackSource specifies the class in question
            {
                if (!HasStackSource())
                {
                    std::stringstream ss;
                    ss << "Effect \"" << DisplayName() << "\" missing StackSource";
                    GetLog().AddLogEntry(ss.str().c_str());
                }
                size_t level = 0;
                const ::Class& c = FindClass(StackSource());
                BreakdownItem* pBI = FindBreakdown(static_cast<BreakdownType>(Breakdown_CasterLevel_First + c.Index()));
                if (pBI != NULL)
                {
                    level = static_cast<size_t>(pBI->Total());
                    level = min(level, MAX_CLASS_LEVEL);
                }
                // vector lookup
                int vi = min(level, m_Amount.size() - 1);
                if (vi < 0
                    || level > m_Amount.size())
                {
                    std::stringstream ss;
                    ss << "Effect \"" << DisplayName() << "\" has incorrect Amount vector size. Stacks present are " << level;
                    GetLog().AddLogEntry(ss.str().c_str());
                }
                total = m_Amount[vi] * m_stacks;
                break;
            }
        case Amount_APCount:
            // StackSource is the enhancement tree
            {
                size_t aps = m_pBuild->APSpentInTree(StackSource());
                total = m_Amount[0] * aps;
                break;
            }
        case Amount_AbilityValue:
            // stack source is the ability value
            {
                AbilityType ability = TextToEnumEntry(StackSource(), abilityTypeMap, false);
                AbilityType snapshotAbility = TextToEnumEntry(StackSource(), abilitySnapshotTypeMap, false);
                if (ability != Ability_Unknown)
                {
                    total = m_pBuild->AbilityAtLevel(ability, m_pBuild->Level()-1, true);
                }
                else
                {
                    total = m_pBuild->SnapshotAbilityValue(snapshotAbility);
                }
                break;
            }
        case Amount_AbilityMod:
            // stack source is the ability mod value
            {
                AbilityType ability = TextToEnumEntry(StackSource(), abilityTypeMap, false);
                AbilityType snapshotAbility = TextToEnumEntry(StackSource(), abilitySnapshotTypeMap, false);
                if (ability != Ability_Unknown)
                {
                    BreakdownItem* pBreakdown = FindBreakdown(StatToBreakdown(ability));
                    total = pBreakdown->Total();
                }
                else
                {
                    total = m_pBuild->SnapshotAbilityValue(snapshotAbility);
                }
                total = BaseStatToBonus(total);
                break;
            }
        case Amount_HalfAbilityMod:
            // stack source is half the ability mod value
            {
                AbilityType ability = TextToEnumEntry(StackSource(), abilityTypeMap, false);
                AbilityType snapshotAbility = TextToEnumEntry(StackSource(), abilitySnapshotTypeMap, false);
                if (ability != Ability_Unknown)
                {
                    BreakdownItem* pBreakdown = FindBreakdown(StatToBreakdown(ability));
                    total = pBreakdown->Total();
                }
                else
                {
                    total = m_pBuild->SnapshotAbilityValue(snapshotAbility);
                }
                total = (int)(BaseStatToBonus(total) / 2.0);
                break;
            }
        case Amount_ThirdAbilityMod:
            // stack source is third the ability mod value
            {
                AbilityType ability = TextToEnumEntry(StackSource(), abilityTypeMap, false);
                AbilityType snapshotAbility = TextToEnumEntry(StackSource(), abilitySnapshotTypeMap, false);
                if (ability != Ability_Unknown)
                {
                    BreakdownItem* pBreakdown = FindBreakdown(StatToBreakdown(ability));
                    total = pBreakdown->Total();
                }
                else
                {
                    total = m_pBuild->SnapshotAbilityValue(snapshotAbility);
                }
                total = (int)(BaseStatToBonus(total) / 3.0);
                break;
            }
        case Amount_FeatCount:
            {
                size_t count = m_pBuild->FeatTrainedCount(StackSource());
                if (count > m_Amount.size())
                {
                    std::stringstream ss;
                    ss << "Effect \"" << DisplayName() << "\" has incorrect Amount vector size. Stacks present are " << count;
                    GetLog().AddLogEntry(ss.str().c_str());
                    count = m_Amount.size() - 1;
                }
                total = m_Amount[count] * m_stacks;
                break;
            }
        case Amount_Slider:
        case Amount_SliderValue:
            total = m_Amount[0] * m_stacks;
            break;
        case Amount_Dice:
            if (!HasDamageDice())
            {
                std::stringstream ss;
                ss << "Effect \"" << DisplayName() << "\" is missing DamageDice field. Stacks present are " << m_stacks;
                GetLog().AddLogEntry(ss.str().c_str());
            }
            break;
        case Amount_CriticialDice:
            if (!HasDamageDice())
            {
                std::stringstream ss;
                ss << "Effect \"" << DisplayName() << "\" is missing DamageDice field. Stacks present are " << m_stacks;
                GetLog().AddLogEntry(ss.str().c_str());
            }
            else if (m_stacks >= m_DamageDice.MaxSize())
            {
                std::stringstream ss;
                ss << "Effect \"" << DisplayName() << "\" has incorrect DamageDice field. Stacks present are " << m_stacks;
                GetLog().AddLogEntry(ss.str().c_str());
            }
            break;
    }
    return total;
}

void Effect::SetPercentValue(double value) const
{
    m_percentValue = value;
}

double Effect::GetPercentValue() const
{
    return m_percentValue;
}

bool Effect::UpdateAbilityEffects(AbilityType at)
{
    bool bUpdate = false;
    if (m_AType == Amount_AbilityValue
            || m_AType == Amount_AbilityMod
            || m_AType == Amount_HalfAbilityMod
            || m_AType == Amount_ThirdAbilityMod)
    {
        AbilityType eat = TextToEnumEntry(StackSource(), abilityTypeMap, false);
        AbilityType eats = TextToEnumEntry(StackSource(), abilitySnapshotTypeMap, false);
        if (eat == at
            || eats == at)
        {
            //BreakdownItem* pBreakdown = FindBreakdown(StatToBreakdown(at));
            //if (pBreakdown != NULL)
            //{
            //    double value = 0;
            //    switch (m_AType)
            //    {
            //        case Amount_AbilityValue:
            //            value = pBreakdown->Total();
            //            break;
            //        case Amount_AbilityMod:
            //            value = pBreakdown->Total();
            //            value = (int)(BaseStatToBonus(total));
            //            break;
            //        case Amount_HalfAbilityMod:
            //            value = pBreakdown->Total();
            //            value = (int)(BaseStatToBonus(total) / 2.0);
            //            break;
            //        case Amount_ThirdAbilityMod:
            //            value = pBreakdown->Total();
            //            value = (int)(BaseStatToBonus(total) / 3.0);
            //            break;
            //    }
                bUpdate = true;
            //}
        }
    }
    return bUpdate;
}

bool Effect::UpdateTreeEffects(const std::string& treeName)
{
    bool bUpdate = false;
    if (m_AType == Amount_APCount
            && treeName == StackSource())
    {
        bUpdate = true;
    }
    // also update if has requirements dependent on an enhancement
    if (HasRequirementsToBeActive())
    {
        bUpdate |= m_RequirementsToBeActive.RequiresAnEnhancement();
    }
    return bUpdate;
}

bool Effect::UpdateSliderEffects(const std::string& sliderName, int newValue)
{
    // slider effects can either be a stack source value or they can affect
    // a stance requirement
    bool bUpdate = false;
    // check for slider is the stack count
    if (m_AType == Amount_SliderValue
        && sliderName == StackSource())
    {
        bUpdate = true;
        m_stacks = newValue;
    }
    else
    {
        // check for slider affects stance state
        if (HasRequirementsToBeActive())
        {
            // just assume if we are stance controlled, then we need to update
            bUpdate = true;
        }
    }

    return bUpdate;
}

void Effect::SetType(EffectType t)
{
    m_Type.clear();
    m_Type.push_back(t);
}

void Effect::SetAType(AmountType t)
{
    Set_AType(t);
}

void Effect::SetAmount(double value)
{
    // called for effects that need values updated on the fly (e.g. cannith level augments)
    Clear_Amount();
    m_Amount.clear();
    m_hasAmount = true;
    m_Amount.push_back(value);
}

void Effect::SetBonus(const std::string& bonus)
{
    m_Bonus = bonus;
    m_hasBonus = true;
}

void Effect::SetItem(const std::list<std::string>& items)
{
    m_Item = items;
}

void Effect::AddItem(const std::string& item)
{
    m_Item.push_back(item);
}

void Effect::AddValue(const std::string& str)
{
    m_hasValue = true;
    m_Value = str;
}

void Effect::SetRequirements(const Requirements& req)
{
    m_RequirementsToBeActive = req;
    m_hasRequirementsToBeActive = true;
}

void Effect::ReplaceLastItem(const std::string& newEntry)
{
    if (m_Item.size() > 0)
    {
        m_Item.pop_back();
    }
    m_Item.push_back(newEntry);
}

void Effect::SetIsItemSpecific()
{
    Set_IsItemSpecific();
}

void Effect::SetApplyAsItemEffect()
{
    Set_ApplyAsItemEffect();
}

void Effect::SetWeapon1()
{
    Set_Weapon1();
}

void Effect::SetWeapon2()
{
    Set_Weapon2();
}

void Effect::SetStackSource(const std::string& source)
{
    Set_StackSource(source);
}
