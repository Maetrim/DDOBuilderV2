// Requirement.cpp
//
#include "StdAfx.h"
#include "Requirement.h"
#include "XmlLib\SaxWriter.h"
#include "Build.h"
#include "Class.h"
#include "EnhancementTreeItem.h"
#include "GlobalSupportFunctions.h"
#include "InventorySlotTypes.h"
#include "Item.h"
#include "Race.h"
#include "WeaponTypes.h"

// A requirement object can handle each of the individual requirement types
// that can be encountered for a Feat/Enhancement/effect etc

#define DL_ELEMENT Requirement

namespace
{
    const wchar_t f_saxElementName[] = L"Requirement";
    DL_DEFINE_NAMES(Requirement_PROPERTIES)

    const unsigned f_verCurrent = 1;
}

Requirement::Requirement() :
    XmlLib::SaxContentElement(f_saxElementName, f_verCurrent)
{
    DL_INIT(Requirement_PROPERTIES)
}

Requirement::Requirement(
    RequirementType type) :
    XmlLib::SaxContentElement(f_saxElementName, f_verCurrent)
{
    DL_INIT(Requirement_PROPERTIES)
    m_Type = type;
    m_hasType = true;
}

Requirement::Requirement(
        RequirementType type,
        const std::string& item) :
    XmlLib::SaxContentElement(f_saxElementName, f_verCurrent)
{
    DL_INIT(Requirement_PROPERTIES)
    m_Type = type;
    m_hasType = true;
    m_Item.push_back(item);
}

Requirement::Requirement(
        RequirementType type,
        const std::string& item,
        size_t value) :
    XmlLib::SaxContentElement(f_saxElementName, f_verCurrent)
{
    DL_INIT(Requirement_PROPERTIES)
    m_Type = type;
    m_hasType = true;
    m_Item.push_back(item);
    m_Value = value;
    m_hasValue = true;
}

Requirement::Requirement(
        RequirementType type,
        const std::string& item1,
        const std::string& item2) :
    XmlLib::SaxContentElement(f_saxElementName, f_verCurrent)
{
    DL_INIT(Requirement_PROPERTIES)
    m_Type = type;
    m_hasType = true;
    m_Item.push_back(item1);
    m_Item.push_back(item2);
}

DL_DEFINE_ACCESS(Requirement_PROPERTIES)

XmlLib::SaxContentElementInterface * Requirement::StartElement(
        const XmlLib::SaxString & name,
        const XmlLib::SaxAttributes & attributes)
{
    XmlLib::SaxContentElementInterface * subHandler =
            SaxContentElement::StartElement(name, attributes);

    DL_START(Requirement_PROPERTIES)

    return subHandler;
}

void Requirement::EndElement()
{
    SaxContentElement::EndElement();
    DL_END(Requirement_PROPERTIES)
}

void Requirement::Write(XmlLib::SaxWriter * writer) const
{
    writer->StartElement(ElementName());// , VersionAttributes());
    DL_WRITE(Requirement_PROPERTIES)
    writer->EndElement();
}

bool Requirement::VerifyObject(
        std::stringstream * ss) const
{
    bool ok = true;
    bool requiresValueField = false;
    switch (m_Type)
    {
        case Requirement_Ability:
            requiresValueField = true;
            if (m_Item.size() == 0)
            {
                (*ss) << "Requirement:" << EnumEntryText(m_Type, requirementTypeMap) << " missing/extra Ability item field\n";
                ok = false;
            }
            else
            {
                for (auto&& iit : m_Item)
                {
                    if (TextToEnumEntry(iit, abilityTypeMap) == Ability_Unknown)
                    {
                        (*ss) << "Requirement:" << EnumEntryText(m_Type, requirementTypeMap) << " bad Ability item field\n";
                    }
                }
            }
            break;
        case Requirement_AbilityGreaterCondition:
            if (m_Item.size() != 2)
            {
                (*ss) << "Requirement:" << EnumEntryText(m_Type, requirementTypeMap) << " missing/extra Ability item field\n";
                ok = false;
            }
            else
            {
                for (auto&& iit : m_Item)
                {
                    if (TextToEnumEntry(iit, abilityTypeMap) == Ability_Unknown)
                    {
                        (*ss) << "Requirement:" << EnumEntryText(m_Type, requirementTypeMap) << " bad Ability item field\n";
                    }
                }
            }
            break;
        case Requirement_Alignment:
            if (m_Item.size() == 0)
            {
                (*ss) << "Requirement:" << EnumEntryText(m_Type, requirementTypeMap) << " missing/extra Alignment item field\n";
                ok = false;
            }
            else
            {
                for (auto&& iit: m_Item)
                {
                    if (TextToEnumEntry(iit, alignmentTypeMap) == Alignment_Unknown)
                    {
                        (*ss) << "Requirement:" << EnumEntryText(m_Type, requirementTypeMap) << " bad Alignment item field\n";
                    }
                }
            }
            break;
        case Requirement_AlignmentType:
            if (m_Item.size() == 0)
            {
                (*ss) << "Requirement:" << EnumEntryText(m_Type, requirementTypeMap) << " missing/extra Alignment item field\n";
                ok = false;
            }
            else
            {
                for (auto&& iit : m_Item)
                {
                    if (TextToEnumEntry(iit, alignmentOptionsTypeMap) == AlignmentOption_Unknown)
                    {
                        (*ss) << "Requirement:" << EnumEntryText(m_Type, requirementTypeMap) << " bad AlignmentOptions item field\n";
                    }
                }
            }
            break;
        case Requirement_BAB:
            requiresValueField = true;  // just a Value field required
            break;
        case Requirement_ClassAtLevel:
        case Requirement_ClassMinLevel:
        case Requirement_BaseClassAtLevel:
        case Requirement_BaseClassMinLevel:
        case Requirement_BaseClass:
        case Requirement_Class:
            if (m_Item.size() != 1)
            {
                (*ss) << "Requirement:" << EnumEntryText(m_Type, requirementTypeMap) << " missing/extra Class item field\n";
                ok = false;
            }
            else
            {
                for (auto&& iit : m_Item)
                {
                    const ::Class& c = FindClass(iit);
                    if (c.Name() == "")
                    {
                        (*ss) << "Requirement:" << EnumEntryText(m_Type, requirementTypeMap) << " bad Class item field\n";
                    }
                }
            }
            break;
        case Requirement_Enhancement:
            if (m_Item.size() == 0)
            {
                (*ss) << "Requirement:" << EnumEntryText(m_Type, requirementTypeMap) << " missing Enhancement item field\n";
                ok = false;
            }
            break;
        case Requirement_Exclusive:
            if (m_Item.size() != 2)
            {
                (*ss) << "Requirement:" << EnumEntryText(m_Type, requirementTypeMap) << " missing Exclusive item field(s)\n";
                ok = false;
            }
            break;
        case Requirement_Feat:
            // fall through
        case Requirement_FeatAnySource:
            if (m_Item.size() == 0)
            {
                (*ss) << "Requirement:" << EnumEntryText(m_Type, requirementTypeMap) << " missing FeatAnySource item field\n";
                ok = false;
            }
            break;
        case Requirement_GroupMember:
        case Requirement_GroupMember2:
            if (m_Item.size() != 1 && m_Item.size() != 2)
            {
                (*ss) << "Requirement:" << EnumEntryText(m_Type, requirementTypeMap) << " missing group field\n";
                ok = false;
            }
            break;
        case Requirement_ItemTypeInSlot:
            // first item is the slot
            if (m_Item.size() == 0)
            {
                (*ss) << "Requirement:" << EnumEntryText(m_Type, requirementTypeMap) << " missing Slot item field\n";
                ok = false;
            }
            else
            {
                auto it = m_Item.begin();
                if (TextToEnumEntry((*it), InventorySlotTypeMap, false) == Inventory_Unknown)
                {
                    (*ss) << "Requirement:" << EnumEntryText(m_Type, requirementTypeMap) << " has bad slot field\n";
                    ok = false;
                }
                it++;
                for (auto&& iit : m_Item)
                {
                    const ::Race& r = FindRace(iit);
                    if (r.Name() == "")
                    {
                        (*ss) << "Requirement:" << EnumEntryText(m_Type, requirementTypeMap) << " bad Race item field\n";
                    }
                }
            }
            break;
        case Requirement_Level:
        case Requirement_SpecificLevel:
            requiresValueField = true;  // just a Value field required
            break;
        case Requirement_Race:
            if (m_Item.size() == 0)
            {
                (*ss) << "Requirement:" << EnumEntryText(m_Type, requirementTypeMap) << " missing Race item field\n";
                ok = false;
            }
            else
            {
                for (auto&& iit : m_Item)
                {
                    const ::Race& r = FindRace(iit);
                    if (r.Name() == "")
                    {
                        (*ss) << "Requirement:" << EnumEntryText(m_Type, requirementTypeMap) << " bad Race item field\n";
                    }
                }
            }
            break;
        case Requirement_NotConstruct:
        case Requirement_RaceConstruct:
            if (m_Item.size() != 0)
            {
                (*ss) << "Requirement:" << EnumEntryText(m_Type, requirementTypeMap) << " has erroneous item field\n";
                ok = false;
            }
            break;
        case Requirement_Skill:
            if (m_Item.size() != 1)
            {
                (*ss) << "Requirement:" << EnumEntryText(m_Type, requirementTypeMap) << " missing/extra Skill item field\n";
                ok = false;
            }
            else
            {
                for (auto&& iit : m_Item)
                {
                    if (TextToEnumEntry(iit, skillTypeMap) == Skill_Unknown)
                    {
                        (*ss) << "Requirement:" << EnumEntryText(m_Type, requirementTypeMap) << " bad Skill item field\n";
                    }
                }
            }
            requiresValueField = true;  // Value field also required
            break;
        case Requirement_Stance:
            if (m_Item.size() != 1)
            {
                (*ss) << "Requirement:" << EnumEntryText(m_Type, requirementTypeMap) << " missing/extra Stance item field\n";
                ok = false;
            }
            break;
        case Requirement_StartingWorld:
            if (m_Item.size() == 0)
            {
                (*ss) << "Requirement:" << EnumEntryText(m_Type, requirementTypeMap) << " missing item field\n";
                ok = false;
            }
            else
            {
                for (auto&& iit : m_Item)
                {
                    if (TextToEnumEntry(iit, startingWorldTypeMap) == StartingWorld_Unknown)
                    {
                        (*ss) << "Requirement:" << EnumEntryText(m_Type, requirementTypeMap) << " bad StartingWorld item field\n";
                    }
                }
            }
            break;
        case Requirement_EnemyType:
            // must have at least 1 item
            if (m_Item.size() == 0)
            {
                (*ss) << "Requirement:" << EnumEntryText(m_Type, requirementTypeMap) << " missing item field\n";
                ok = false;
            }
            break;
        case Requirement_WeaponTypesEquipped:
            if (m_Item.size() == 0)
            {
                (*ss) << "Requirement:" << EnumEntryText(m_Type, requirementTypeMap) << " missing item field(s)\n";
                ok = false;
            }
            else if (m_Item.size() > 2)
            {
                (*ss) << "Requirement:" << EnumEntryText(m_Type, requirementTypeMap) << " has too many Item fields\n";
                ok = false;
            }
            else
            {
                for (auto&& iit : m_Item)
                {
                    const ::WeaponType& wt = TextToEnumEntry(iit, weaponTypeMap);
                    if (wt == Weapon_Unknown)
                    {
                        (*ss) << "Requirement:" << EnumEntryText(m_Type, requirementTypeMap) << " bad WeaponType item field\n";
                    }
                }
            }
            break;
        case Requirement_WeaponClassMainHand:
            if (m_Item.size() != 1)
            {
                (*ss) << "Requirement:" << EnumEntryText(m_Type, requirementTypeMap) << " missing/extra item field\n";
                ok = false;
            }
            break;
        case Requirement_WeaponClassOffHand:
            if (m_Item.size() != 1)
            {
                (*ss) << "Requirement:" << EnumEntryText(m_Type, requirementTypeMap) << " missing/extra item field\n";
                ok = false;
            }
            break;
        default:
            (*ss) << "Requirement has bad Type field\n";
            ok = false;
            break;
    }
    if (requiresValueField && !HasValue())
    {
        (*ss) << "Requirement:" << EnumEntryText(m_Type, requirementTypeMap) << " was missing Value field\n";
        ok = false;
    }
    return ok;
}

bool Requirement::RequiresEnhancement(const std::string& name) const
{
    bool bRequires = false;
    if (m_Type == Requirement_Enhancement)
    {
        bRequires = (name == m_Item.front());
    }
    return bRequires;
}

bool Requirement::RequiresAnEnhancement() const
{
    return (m_Type == Requirement_Enhancement);
}

bool Requirement::Met(
        const Build & build,
        size_t level,  // this is 0 based
        bool includeTomes,
        WeaponType wtMainHand,
        WeaponType wtOffHand) const
{
    bool met = true;
    switch (m_Type)
    {
    case Requirement_Ability:           met = EvaluateAbility(build, level, includeTomes); break;
    case Requirement_AbilityGreaterCondition:   met = EvaluateAbilityGreaterCondition(build, level, includeTomes); break;
    case Requirement_Alignment:         met = EvaluateAlignment(build); break;
    case Requirement_AlignmentType:     met = EvaluateAlignmentType(build); break;
    case Requirement_BAB:               met = EvaluateBAB(build, level, includeTomes); break;
    case Requirement_BaseClass:         met = EvaluateBaseClass(build, level, includeTomes); break;
    case Requirement_BaseClassAtLevel:  met = EvaluateBaseClassAtLevel(build, level, includeTomes); break;
    case Requirement_BaseClassMinLevel: met = EvaluateBaseClassMinLevel(build, level, includeTomes); break;
    case Requirement_Class:             met = EvaluateClass(build, level, includeTomes); break;
    case Requirement_ClassAtLevel:      met = EvaluateClassAtLevel(build, level, includeTomes); break;
    case Requirement_ClassMinLevel:     met = EvaluateClassMinLevel(build, level, includeTomes); break;
    case Requirement_EnemyType:         met = false; break;
    case Requirement_Enhancement:       met = EvaluateEnhancement(build); break;
    case Requirement_Exclusive:         met = EvaluateExclusive(build, level, includeTomes); break;
    case Requirement_Feat:              met = EvaluateFeat(build, level, includeTomes); break;
    case Requirement_FeatAnySource:     met = EvaluateFeatAnySource(build, level, includeTomes); break;
    case Requirement_GroupMember:       met = EvaluateWeaponGroupMember(build, wtMainHand, wtOffHand, true); break;
    case Requirement_GroupMember2:      met = EvaluateWeaponGroupMember(build, wtMainHand, wtOffHand, false); break;
    case Requirement_ItemTypeInSlot:    met = EvaluateItemInSlot(build); break;
    case Requirement_Level:             met = EvaluateLevel(build, level, includeTomes); break;
    case Requirement_NotConstruct:      met = EvaluateNotConstruct(build); break;
    case Requirement_Race:              met = EvaluateRace(build, level, includeTomes); break;
    case Requirement_RaceConstruct:     met = EvaluateRaceConstruct(build); break;
    case Requirement_Skill:             met = EvaluateSkill(build, level, includeTomes); break;
    case Requirement_SpecificLevel:     met = EvaluateSpecificLevel(build, level, includeTomes); break;
    case Requirement_Stance:            met = EvaluateStance(build, level, includeTomes); break;
    case Requirement_StartingWorld:     met = EvaluateStartingWorld(build); break;
    case Requirement_WeaponClassMainHand: met = EvaluateWeaponGroupMember(build, wtMainHand, wtOffHand, true); break;
    case Requirement_WeaponClassOffHand: met = EvaluateWeaponGroupMember(build, wtMainHand, wtOffHand, false); break;
    default:                            met = false; break;
    }
    return met;
}

bool Requirement::CanTrainEnhancement(
    const Build& build,
    size_t /*trainedRanks*/) const
{
    size_t level = build.Level() - 1;
    bool includeTomes = true;
    bool met = true;
    switch (m_Type)
    {
    case Requirement_Ability:           met = EvaluateAbility(build, level, includeTomes); break;
    case Requirement_AbilityGreaterCondition:   met = EvaluateAbilityGreaterCondition(build, level, includeTomes); break;
    case Requirement_Alignment:         met = EvaluateAlignment(build); break;
    case Requirement_AlignmentType:     met = EvaluateAlignmentType(build); break;
    case Requirement_BAB:               met = EvaluateBAB(build, level, includeTomes); break;
    case Requirement_BaseClass:         met = EvaluateBaseClass(build, level, includeTomes); break;
    case Requirement_BaseClassAtLevel:  met = EvaluateBaseClassAtLevel(build, level, includeTomes); break;
    case Requirement_BaseClassMinLevel: met = EvaluateBaseClassMinLevel(build, level, includeTomes); break;
    case Requirement_Class:             met = EvaluateClass(build, level, includeTomes); break;
    case Requirement_ClassAtLevel:      met = EvaluateClassAtLevel(build, level, includeTomes); break;
    case Requirement_ClassMinLevel:     met = EvaluateClassMinLevel(build, level, includeTomes); break;
    case Requirement_EnemyType:         met = false; break;
    case Requirement_Enhancement:       met = EvaluateEnhancement(build); break;
    case Requirement_Exclusive:         met = EvaluateExclusive(build, level, includeTomes); break;
    case Requirement_Feat:              met = EvaluateFeat(build, level, includeTomes); break;
    case Requirement_FeatAnySource:     met = EvaluateFeatAnySource(build, level, includeTomes); break;
    case Requirement_GroupMember:       met = EvaluateWeaponGroupMember(build, Weapon_Unknown, Weapon_Unknown, true); break;
    case Requirement_GroupMember2:      met = EvaluateWeaponGroupMember(build, Weapon_Unknown, Weapon_Unknown, false); break;
    case Requirement_ItemTypeInSlot:    met = false; break;
    case Requirement_Level:             met = EvaluateLevel(build, level, includeTomes); break;
    case Requirement_NotConstruct:      met = EvaluateNotConstruct(build); break;
    case Requirement_Race:              met = EvaluateRace(build, level, includeTomes); break;
    case Requirement_RaceConstruct:     met = EvaluateRaceConstruct(build); break;
    case Requirement_Skill:             met = EvaluateSkill(build, level, includeTomes); break;
    case Requirement_SpecificLevel:     met = EvaluateSpecificLevel(build, level, includeTomes); break;
    case Requirement_Stance:            met = EvaluateStance(build, level, includeTomes); break;
    case Requirement_StartingWorld:     met = EvaluateStartingWorld(build); break;
    default:                            met = false; break;
    }
    return met;
}

bool Requirement::MetHardRequirements(
    const Build& build,
    size_t level,
    bool includeTomes) const
{
    bool met = true;
    switch (m_Type)
    {
    case Requirement_Ability:       met = EvaluateAbility(build, level, includeTomes); break;
    case Requirement_AbilityGreaterCondition:   met = EvaluateAbilityGreaterCondition(build, level, includeTomes); break;
    case Requirement_Alignment:     met = EvaluateAlignment(build); break;
    case Requirement_AlignmentType:     met = EvaluateAlignmentType(build); break;
    case Requirement_BAB:           met = EvaluateBAB(build, level, includeTomes); break;
    case Requirement_BaseClass:         met = EvaluateBaseClass(build, level, includeTomes); break;
    case Requirement_BaseClassAtLevel:  met = EvaluateBaseClassAtLevel(build, level, includeTomes); break;
    case Requirement_BaseClassMinLevel: met = EvaluateBaseClassMinLevel(build, level, includeTomes); break;
    case Requirement_Class:         met = EvaluateClass(build, level, includeTomes); break;
    case Requirement_ClassAtLevel:  met = EvaluateClassAtLevel(build, level, includeTomes); break;
    case Requirement_ClassMinLevel: met = EvaluateClassMinLevel(build, level, includeTomes); break;
    //case Requirement_EnemyType:     met = false; break;
    //case Requirement_Enhancement:   met = EvaluateEnhancement(build); break;
    case Requirement_Exclusive:     met = EvaluateExclusive(build, level, includeTomes); break;
    case Requirement_Feat:          met = EvaluateFeat(build, level, includeTomes); break;
    case Requirement_FeatAnySource: met = EvaluateFeatAnySource(build, level, includeTomes); break;
    //case Requirement_GroupMember:   met = false; break;
    //case Requirement_GroupMember2:  met = false; break;
    //case Requirement_ItemTypeInSlot:met = false; break;
    case Requirement_Level:         met = EvaluateLevel(build, level, includeTomes); break;
    case Requirement_NotConstruct:  met = EvaluateNotConstruct(build); break;
    case Requirement_Race:          met = EvaluateRace(build, level, includeTomes); break;
    case Requirement_RaceConstruct: met = EvaluateRaceConstruct(build); break;
    //case Requirement_Skill:         met = EvaluateSkill(build, level, includeTomes); break;
    case Requirement_SpecificLevel: met = EvaluateSpecificLevel(build, level, includeTomes); break;
    //case Requirement_Stance:        met = EvaluateStance(build, level, includeTomes); break;
    case Requirement_StartingWorld:     met = EvaluateStartingWorld(build); break;
    //default:                        met = false; break;
    }
    return met;
}

bool Requirement::MetEnhancements(
    const Build& build,
    size_t trainedRanks) const
{
    bool met = true;
    if (Type() == Requirement_Enhancement)
    {
        std::string enhancement = m_Item.front();
        std::string selection;
        if (m_Item.size() == 2)
        {
            selection = m_Item.back();
        }
        const TrainedEnhancement* te = build.IsTrained(enhancement, selection);
        if (te != NULL)
        {
            const EnhancementTreeItem* pTreeItem = FindEnhancement(enhancement);
            size_t maxRanks = pTreeItem->Ranks(selection);
            size_t ranksTrained = te->Ranks();
            if (maxRanks != 1)
            {
                if (HasValue())
                {
                    // must have at least this many ranks
                    met = (te->Ranks() >= Value());
                }
                else
                {
                    // must have lower trained ranks
                    met = (trainedRanks < ranksTrained);
                }
            }
            else
            {
                // if it only has 1 ranks and its trained, were good to always be trained
                met = true;
            }
        }
        else
        {
            // if its not trained at all, requirements are not met
            met = false;
        }
    }
    return met;
}

bool Requirement::EvaluateAbility(
    const Build& build,
    size_t level,  // this is 0 based
    bool includeTomes) const
{
    AbilityType ability = TextToEnumEntry(m_Item.front(), abilityTypeMap);
    size_t value = build.AbilityAtLevel(ability, level, includeTomes);
    bool met = (value >= Value());
    return met;
}

bool Requirement::EvaluateAbilityGreaterCondition(
    const Build& build,
    size_t level,
    bool includeTomes) const
{
    ASSERT(m_Item.size() == 2);
    AbilityType ability1 = TextToEnumEntry(m_Item.front(), abilityTypeMap);
    AbilityType ability2 = TextToEnumEntry(m_Item.back(), abilityTypeMap);
    size_t value1 = build.AbilityAtLevel(ability1, level, includeTomes);
    size_t value2 = build.AbilityAtLevel(ability2, level, includeTomes);
    bool met = (value1 > value2);
    return met;
}

bool Requirement::EvaluateAlignment(
    const Build& build) const
{
    AlignmentType a = build.Alignment();
    bool met = false;
    auto it = m_Item.begin();
    while (!met && it != m_Item.end())
    {
        AlignmentType at = TextToEnumEntry(*m_Item.begin(), alignmentTypeMap);
        met = (at == a);
        ++it;
    }
    return met;
}

bool Requirement::EvaluateAlignmentType(
    const Build& build) const
{
    AlignmentType a = build.Alignment();
    bool met = false;
    auto it = m_Item.begin();
    while (!met && it != m_Item.end())
    {
        AlignmentOptions ao = TextToEnumEntry(*m_Item.begin(), alignmentOptionsTypeMap);
        switch (ao)
        {
            case AlignmentOption_Lawful:
                met = (a == Alignment_LawfulGood
                        || a == Alignment_LawfulNeutral
                        || a == Alignment_LawfulEvil);
                break;
            case AlignmentOption_Chaotic:
                met = (a == Alignment_ChaoticNeutral
                        || a == Alignment_ChaoticGood
                        || a == Alignment_ChaoticEvil);
                break;
            case AlignmentOption_TrueNeutral:
                met = (a == Alignment_TrueNeutral);
                break;
            case AlignmentOption_Good:
                met = (a == Alignment_LawfulGood
                        || a == Alignment_NeutralGood
                        || a == Alignment_ChaoticGood);
                break;
            case AlignmentOption_Evil:
                met = (a == Alignment_LawfulEvil
                        || a == Alignment_NeutralEvil
                        || a == Alignment_ChaoticEvil);
                break;
            case AlignmentOption_PartNeutral:
                met = (a == Alignment_LawfulNeutral
                        || a == Alignment_NeutralGood
                        || a == Alignment_ChaoticNeutral
                        || a == Alignment_NeutralEvil);
                break;
        }
        ++it;
    }
    return met;
}

bool Requirement::EvaluateBAB(
    const Build& build,
    size_t level,  // this is 0 based
    bool includeTomes) const
{
    UNREFERENCED_PARAMETER(includeTomes);
    size_t bab = build.BaseAttackBonus(level);
    bool met = (bab >= Value());
    return met;
}

bool Requirement::EvaluateBaseClass(
    const Build& build,
    size_t level,  // this is 0 based
    bool includeTomes) const
{
    UNREFERENCED_PARAMETER(includeTomes);
    std::string c = m_Item.front();
    size_t classLevel = build.BaseClassLevels(c, level);
    bool met = HasValue()
        ? (classLevel >= Value())
        : (classLevel > 0);
    return met;
}

bool Requirement::EvaluateBaseClassAtLevel(
    const Build& build,
    size_t level,  // this is 0 based
    bool includeTomes) const
{
    UNREFERENCED_PARAMETER(includeTomes);
    bool met = false;
    std::string c = m_Item.front();
    if (HasValue())
    {
        size_t classLevel = build.BaseClassLevels(c, level);
        met = (classLevel == Value());
    }
    else
    {
        // must have this class at the level in question
        met = (c == build.BaseClassAtLevel(level));
    }
    return met;
}

bool Requirement::EvaluateBaseClassMinLevel(
    const Build& build,
    size_t level,  // this is 0 based
    bool includeTomes) const
{
    UNREFERENCED_PARAMETER(includeTomes);
    std::string c = m_Item.front();
    size_t classLevel = build.BaseClassLevels(c, level);
    bool met = (classLevel >= Value());
    return met;
}

bool Requirement::EvaluateClass(
    const Build& build,
    size_t level,  // this is 0 based
    bool includeTomes) const
{
    UNREFERENCED_PARAMETER(includeTomes);
    std::string c = m_Item.front();
    size_t classLevel = build.ClassLevels(c, level);
    bool met = HasValue()
        ? (classLevel >= Value())
        : (classLevel > 0);
    return met;
}

bool Requirement::EvaluateClassAtLevel(
    const Build& build,
    size_t level,  // this is 0 based
    bool includeTomes) const
{
    UNREFERENCED_PARAMETER(includeTomes);
    bool met = false;
    std::string c = m_Item.front();
    if (HasValue())
    {
        size_t classLevel = build.ClassLevels(c, level);
        met = (classLevel == Value());
    }
    else
    {
        // must have this class at the level in question
        met = (c == build.ClassAtLevel(level));
    }
    return met;
}

bool Requirement::EvaluateClassMinLevel(
    const Build& build,
    size_t level,  // this is 0 based
    bool includeTomes) const
{
    UNREFERENCED_PARAMETER(includeTomes);
    std::string c = m_Item.front();
    size_t classLevel = build.ClassLevels(c, level);
    bool met = (classLevel >= Value());
    return met;
}

bool Requirement::EvaluateEnhancement(
    const Build& build) const
{
    std::string enhancement = m_Item.front();
    std::string selection;
    if (m_Item.size() == 2)
    {
        selection = m_Item.back();
    }
    const TrainedEnhancement* te = build.IsTrained(enhancement, selection);
    bool met = (te != NULL);
    if (HasValue())
    {
        met &= (te != NULL && te->Ranks() >= Value());
    }
    return met;
}

bool Requirement::EvaluateExclusive(
    const Build& build,
    size_t level,
    bool includeTomes) const
{
    UNREFERENCED_PARAMETER(level);
    UNREFERENCED_PARAMETER(includeTomes);
    bool met = build.IsExclusiveEnhancement(
            m_Item.front(),             // enhancement id InternalName
            m_Item.back());             // enhancement name (e.g. "Maximize")
    return met;
}

bool Requirement::EvaluateFeat(
    const Build& build,
    size_t level,  // this is 0 based
    bool includeTomes) const
{
    UNREFERENCED_PARAMETER(includeTomes);
    std::string feat = m_Item.front();
    std::list<TrainedFeat> currentFeats = build.CurrentFeats(level);
    size_t countNormal = TrainedCount(currentFeats, feat);
    size_t countSpecial = build.GetSpecialFeatTrainedCount(feat);
    size_t numNeeded = 1;
    if (HasValue())
    {
        numNeeded = Value();
    }
    bool met = (countNormal >= numNeeded
        || countSpecial >= numNeeded);
    return met;
}

bool Requirement::EvaluateFeatAnySource(
    const Build& build,
    size_t level,  // this is 0 based
    bool includeTomes) const
{
    UNREFERENCED_PARAMETER(includeTomes);
    std::string feat = m_Item.front();
    std::list<TrainedFeat> currentFeats = build.CurrentFeats(level);
    size_t count = TrainedCount(currentFeats, feat);
    size_t numNeeded = 1;
    if (HasValue())
    {
        numNeeded = Value();
    }
    bool met = (count >= numNeeded);
    if (!met)
    {
        // need to also evaluate Granted Feats
        met = build.IsGrantedFeat(feat);
    }
    return met;
}

bool Requirement::EvaluateWeaponGroupMember(
        const Build& build,
        WeaponType wtMain,
        WeaponType wtOffhand,
        bool bMainHand) const
{
    bool bRet = false;
    const std::string& group = m_Item.front();
    if ((m_Type == Requirement_GroupMember
            || m_Type == Requirement_GroupMember2)
            && group == "One Handed"
            && wtMain == Weapon_Handwraps
            && wtOffhand == Weapon_Empty)
    {
        // special case for this requirement when handwraps in use
        bRet = true;
    }
    else
    {
        bRet = build.IsWeaponInGroup(group, bMainHand ? wtMain : wtOffhand);
    }
    return bRet;
}

bool Requirement::EvaluateItemInSlot(const Build& build) const
{
    bool met = false;
    // this is usually used for weapons etc
    InventorySlotType ist = TextToEnumEntry(m_Item.front(), InventorySlotTypeMap, false);
    if (build.ActiveGearSet().HasItemInSlot(ist))
    {
        ::Item item = build.ActiveGearSet().ItemInSlot(ist);
        if (item.HasWeapon())
        {
            WeaponType wt = TextToEnumEntry(m_Item.back(), weaponTypeMap, false);
            met = (wt == item.Weapon());
        }
        else if (item.HasArmor())
        {
            ArmorType at = TextToEnumEntry(m_Item.back(), armorTypeMap, false);
            met = (at == item.Armor());
        }
        else
        {
            throw "NYI";
        }
    }
    else
    {
        // special case, may be looking for an empty slot
        if (m_Item.back() == "Empty")
        {
            met = true;
        }
    }
    return met;
}

bool Requirement::EvaluateLevel(
    const Build& build,
    size_t level,  // this is 0 based
    bool includeTomes) const
{
    UNREFERENCED_PARAMETER(build);
    UNREFERENCED_PARAMETER(level);
    UNREFERENCED_PARAMETER(includeTomes);
    bool met = (level >= Value() - 1);  // value is 1 based
    return met;
}

bool Requirement::EvaluateNotConstruct(
    const Build& build) const
{
    std::string raceName = build.Race();
    const Race& race = FindRace(raceName);
    bool met = !race.HasIsConstruct();
    return met;
}

bool Requirement::EvaluateRace(
    const Build& build,
    size_t level,  // this is 0 based
    bool includeTomes) const
{
    UNREFERENCED_PARAMETER(level);
    UNREFERENCED_PARAMETER(includeTomes);
    std::string race = build.Race();
    bool met = false;
    auto it = m_Item.begin();
    while (!met && it != m_Item.end())
    {
        met = ((*it) == race);
        ++it;
    }
    return met;
}

bool Requirement::EvaluateRaceConstruct(
    const Build& build) const
{
    std::string raceName = build.Race();
    const Race& race = FindRace(raceName);
    bool met = race.HasIsConstruct();
    return met;
}

bool Requirement::EvaluateSkill(
    const Build& build,
    size_t level,  // this is 0 based
    bool includeTomes) const
{
    SkillType st = TextToEnumEntry(m_Item.front(), skillTypeMap);
    bool met = (build.SkillAtLevel(st, level, includeTomes) >= Value());
    return met;
}

bool Requirement::EvaluateSpecificLevel(
    const Build& build,
    size_t level,  // this is 0 based
    bool includeTomes) const
{
    UNREFERENCED_PARAMETER(build);
    UNREFERENCED_PARAMETER(level);
    UNREFERENCED_PARAMETER(includeTomes);
    bool met = (level >= Value() - 1);  // value is 1 based
    return met;
}

bool Requirement::EvaluateStance(
    const Build& build,
    size_t level,
    bool includeTomes) const
{
    // need to ask the stances view if this stance is active
    UNREFERENCED_PARAMETER(level);
    UNREFERENCED_PARAMETER(includeTomes);
    bool met = build.IsStanceActive(m_Item.front(), Weapon_Unknown);
    return met;
}

bool Requirement::EvaluateStartingWorld(
    const Build& build) const
{
    const Race& race = FindRace(build.Race());
    StartingWorldType swt = TextToEnumEntry(m_Item.front(), startingWorldTypeMap);
    bool met = (swt == race.StartingWorld());
    return met;
}

void Requirement::CreateRequirementStrings(
        const Build& build,
        size_t level,
        bool includeTomes,
        std::vector<CString>* requirementsArray,
        std::vector<bool>* metArray) const
{
    bool met = true;
    CString description;
    switch (m_Type)
    {
    case Requirement_Ability:
        {
            met = EvaluateAbility(build, level, includeTomes);
            description.Format(
                    "Requires: %s(%d)",
                    (LPCTSTR)m_Item.front().c_str(),
                    Value());
            break;
        }
    case Requirement_AbilityGreaterCondition:
        {
            description.Format(
                    "Requires: %s > %s",
                    (LPCTSTR)m_Item.front().c_str(),
                    (LPCTSTR)m_Item.back().c_str());
            break;
        }
    case Requirement_Alignment:
        {
            met = EvaluateAlignment(build);
            description = "Requires: ";
            auto it = m_Item.begin();
            while (it != m_Item.end())
            {
                description += (LPCTSTR)(*it).c_str();
                ++it;
                if (it != m_Item.end())
                {
                    description += ", ";
                }
            }
            break;
        }
    case Requirement_AlignmentType:
        {
            met = EvaluateAlignmentType(build);
            description = "Requires: ";
            auto it = m_Item.begin();
            while (it != m_Item.end())
            {
                description += (LPCTSTR)(*it).c_str();
                ++it;
                if (it != m_Item.end())
                {
                    description += ", ";
                }
            }
            description += " alignment";
            break;
        }
    case Requirement_BAB:
        {
            met = EvaluateBAB(build, level, includeTomes);
            description.Format("Requires: BAB(%d)", Value());
            break;
        }
    case Requirement_BaseClassAtLevel:
        {
            met = EvaluateBaseClassAtLevel(build, level, includeTomes);
            if (HasValue())
            {
                description.Format(
                        "Requires: Base Class %s(%d)",
                        m_Item.front().c_str(),
                        Value());
            }
            else
            {
                description.Format(
                        "Requires: Base Class %s(All levels)",
                        m_Item.front().c_str());
            }
            break;
        }
    case Requirement_ClassAtLevel:
        {
            met = EvaluateClassAtLevel(build, level, includeTomes);
            description.Format(
                    "Requires: Class %s(%d)",
                    m_Item.front().c_str(),
                    Value());
        break;
        }
    case Requirement_BaseClassMinLevel:
        {
            met = EvaluateBaseClassMinLevel(build, level, includeTomes);
            description.Format(
                    "Requires: Base Class %s(%d)",
                    m_Item.front().c_str(),
                    Value());
            break;
        }
    case Requirement_ClassMinLevel:
        {
            met = EvaluateClassMinLevel(build, level, includeTomes);
            description.Format(
                "Requires: Class %s(%d)",
                m_Item.front().c_str(),
                Value());
            break;
        }
    case Requirement_BaseClass:
        {
            met = EvaluateBaseClass(build, level, includeTomes);
            description.Format(
                    "Requires: Base Class %s",
                    m_Item.front().c_str());
            break;
        }
    case Requirement_Class:
        {
            met = EvaluateClass(build, level, includeTomes);
            description.Format(
                    "Requires: Class %s",
                    m_Item.front().c_str());
            break;
        }
    case Requirement_Enhancement:
        {
            met = EvaluateEnhancement(build);
            std::string name = "Unknown enhancement";
            const EnhancementTreeItem* item = FindEnhancement(m_Item.front());
            if (item != NULL)
            {
                name = item->Name();
            }
            if (m_Item.size() == 2)
            {
                description.Format(
                        "Requires: %s(%s)",
                        name.c_str(),
                        m_Item.back().c_str());
            }
            else
            {
                if (HasValue())
                {
                    description.Format(
                        "Requires: %s (%d Ranks)",
                        name.c_str(),
                        Value());
                }
                else
                {
                    description.Format(
                        "Requires: %s",
                        name.c_str());
                }
            }
            break;
        }
    case Requirement_Exclusive:
        {
            met = EvaluateExclusive(build, level, includeTomes);
            std::stringstream ss;
            ss << "Requires: No other \"" << m_Item.back() << "\" from another source.";
            description = ss.str().c_str();
        }
        break;
    case Requirement_Feat:
        {
            met = EvaluateFeat(build, level, includeTomes);
            if (HasValue())
            {
                description.Format("Requires: %s(%d)", m_Item.front().c_str(), Value());
            }
            else
            {
                description.Format("Requires: %s", m_Item.front().c_str());
            }
            break;
        }
    case Requirement_FeatAnySource:
        {
            met = EvaluateFeatAnySource(build, level, includeTomes);
            if (HasValue())
            {
                description.Format("Requires: %s(%d) (Any source)", m_Item.front().c_str(), Value());
            }
            else
            {
                description.Format("Requires: %s (Any source)", m_Item.front().c_str());
            }
            break;
        }
    case Requirement_Level:
        {
            met = EvaluateLevel(build, level, includeTomes);
            description.Format("Requires: Minimum level(%d)", Value());
            break;
        }
    case Requirement_Race:
        {
            met = EvaluateRace(build, level, includeTomes);
            description = "Requires: Race ";
            auto it = m_Item.begin();
            while (it != m_Item.end())
            {
                description += (*it).c_str();
                ++it;
                if (it != m_Item.end())
                {
                    description += ", ";
                }
            }
            break;
        }
    case Requirement_Skill:
        {
            met = EvaluateSkill(build, level, includeTomes);
            description.Format(
                    "Requires: %s(%d)",
                    (LPCTSTR)m_Item.front().c_str(),
                    Value());
            break;
        }
        case Requirement_Stance:
        {
            met = EvaluateStance(build, level, includeTomes);
            description.Format(
                    "Requires: %s",
                    m_Item.front().c_str());
            break;
        }
        case Requirement_StartingWorld:
        {
            met = EvaluateStartingWorld(build);
            description.Format(
                "Requires: Starting World of %s",
                m_Item.front().c_str());
            break;
        }
        default:
        {
            met = false;
            description = "Requires: Unknown";
            break;
        }
    }
    requirementsArray->push_back(description);
    metArray->push_back(met);
}

bool Requirement::operator==(const Requirement& other) const
{
    return (m_Type == other.m_Type)
            && (m_Item == other.m_Item)
            && (m_hasValue == other.m_hasValue)
            && (m_Value == other.m_Value);
}

