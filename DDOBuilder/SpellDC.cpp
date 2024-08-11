// SpellDC.cpp
//
#include "StdAfx.h"
#include "SpellDC.h"
#include "XmlLib\SaxWriter.h"
#include "GlobalSupportFunctions.h"
#include "BreakdownItem.h"
#include "Build.h"
#include "Class.h"
#include "Spell.h"

#define DL_ELEMENT SpellDC

namespace
{
    const wchar_t f_saxElementName[] = L"SpellDC";
    DL_DEFINE_NAMES(SpellDC_PROPERTIES)

    const unsigned f_verCurrent = 1;
}

SpellDC::SpellDC() :
    XmlLib::SaxContentElement(f_saxElementName, f_verCurrent)
{
    DL_INIT(SpellDC_PROPERTIES)
}

DL_DEFINE_ACCESS(SpellDC_PROPERTIES)

XmlLib::SaxContentElementInterface * SpellDC::StartElement(
        const XmlLib::SaxString & name,
        const XmlLib::SaxAttributes & attributes)
{
    XmlLib::SaxContentElementInterface * subHandler =
            SaxContentElement::StartElement(name, attributes);

    DL_START(SpellDC_PROPERTIES)

    return subHandler;
}

void SpellDC::EndElement()
{
    SaxContentElement::EndElement();
    DL_END(SpellDC_PROPERTIES)
}

void SpellDC::Write(XmlLib::SaxWriter * writer) const
{
    writer->StartElement(ElementName());//, VersionAttributes());
    DL_WRITE(SpellDC_PROPERTIES)
    writer->EndElement();
}

bool SpellDC::VerifyObject(std::stringstream * ss) const
{
    UNREFERENCED_PARAMETER(ss);
    bool ok = true;
    return ok;
}

int SpellDC::CalculateSpellDC(const Build * build, const Spell& spell) const
{
    int value = 10;     // base amount for all spell DC's
    if (HasAmount())
    {
        value = Amount();
    }
    // add any casting stat breakdown
    if (HasCastingStatMod())
    {
        if (spell.Class() != "")
        {
            const ::Class& c = FindClass(spell.Class());
            if (HasCastingStatMod())
            {
                AbilityType at = c.ClassCastingStat();
                BreakdownType bt = StatToBreakdown(at);
                BreakdownItem* pBI = FindBreakdown(bt);
                if (pBI != NULL)
                {
                    int abilityBonus = BaseStatToBonus(pBI->Total());
                    value += abilityBonus;
                }
            }
            // add any casting stat breakdown
            if (spell.HasLevel())
            {
                if (spell.HasHeighten() && build->IsStanceActive("Heighten"))
                {
                    size_t amount = c.MaxSpellLevel(build->Level());
                    value += amount;
                }
                else
                {
                    value += spell.Level();
                }
            }
        }
    }
    if (ModAbility().size() > 0)
    {
        // use the largest of any ModAbility values
        int modAbilityBonus = 0;
        std::list<AbilityType>::const_iterator mait = m_ModAbility.begin();
        while (mait != m_ModAbility.end())
        {
            BreakdownType bt = StatToBreakdown(*mait);
            BreakdownItem* pBI = FindBreakdown(bt);
            int abilityBonus = BaseStatToBonus(pBI->Total());
            if (abilityBonus > modAbilityBonus)
            {
                modAbilityBonus = abilityBonus;
            }
            ++mait;
        }
        value += modAbilityBonus;
    }
    // spell school bonuses, all of these get added if present
    std::list<SpellSchoolType>::const_iterator sit = m_School.begin();
    while (sit != m_School.end())
    {
        BreakdownType bt = SchoolToBreakdown(*sit);
        BreakdownItem * pBI = FindBreakdown(bt);
        int schoolBonus = (int)pBI->Total();
        value += schoolBonus;
        ++sit;
    }
    return value;
}

std::string SpellDC::SpellDCBreakdown(const Build * build, const Spell& spell) const
{
    std::stringstream ss;
    if (HasAmount())
    {
        if (m_hasOther)
        {
            ss << m_DCVersus << " vs " << m_Other << " + " << CalculateSpellDC(build, spell) << " : ";
        }
        else
        {
            ss << m_DCVersus << " vs " << CalculateSpellDC(build, spell) << " : ";
        }
        ss << Amount();
    }
    if (HasCastingStatMod())
    {
        const ::Class& c = FindClass(spell.Class());
        if (m_hasOther)
        {
            ss << m_DCVersus << " vs " << m_Other << " + " << CalculateSpellDC(build, spell) << " : ";
        }
        else
        {
            ss << m_DCVersus << " vs " << CalculateSpellDC(build, spell) << " : ";
        }
        if (m_hasOther)
        {
            ss << m_Other;
        }
        else
        {
            ss << "10";
        }

        if (spell.Class() != "")
        {
            // add any casting stat breakdown
            if (HasCastingStatMod())
            {
                AbilityType at = c.ClassCastingStat();
                BreakdownType bt = StatToBreakdown(at);
                BreakdownItem* pBI = FindBreakdown(bt);
                if (pBI != NULL)
                {
                    int abilityBonus = BaseStatToBonus(pBI->Total());
                    std::string name = EnumEntryText(at, abilityTypeMap);
                    name.resize(3);     // truncate to 1st 3 characters, e.g. "Strength" becomes "Str"
                    ss << " + " << name << " Mod(" << abilityBonus << ")";
                }
                else
                {
                    ss << " + Unknown Mod(0)";
                }
            }
            // add any casting stat breakdown
            if (spell.HasLevel())
            {
                size_t amount = spell.Level();
                ss << " + SpellLevel(" << amount << ")";
                if (spell.HasHeighten() && build->IsStanceActive("Heighten"))
                {
                    size_t classLevels = build->ClassLevels(spell.Class(), build->Level());
                    size_t maxAmount = c.MaxSpellLevel(classLevels);
                    if (maxAmount > amount)
                    {
                        ss << " + Heighten(" << (maxAmount - amount) << ")";
                    }
                }
            }
        }
    }
    if (m_ModAbility.size() > 0)
    {
        ss << " + Max Mod(";
        std::list<AbilityType>::const_iterator mait = m_ModAbility.begin();
        while (mait != m_ModAbility.end())
        {
            BreakdownType bt = StatToBreakdown(*mait);
            BreakdownItem* pBI = FindBreakdown(bt);
            int abilityBonus = BaseStatToBonus(pBI->Total());
            if (mait != m_ModAbility.begin())
            {
                ss << ", ";
            }
            std::string name = EnumEntryText(*mait, abilityTypeMap);
            name.resize(3);     // truncate to 1st 3 characters, e.g. "Strength" becomes "Str"
            ss << name << "(" << abilityBonus << ")";
            ++mait;
        }
        ss << ")";
    }
    // spell school bonuses, all of these get added if present
    std::list<SpellSchoolType>::const_iterator sit = m_School.begin();
    while (sit != m_School.end())
    {
        ss << " + ";
        BreakdownType bt = SchoolToBreakdown(*sit);
        BreakdownItem * pBI = FindBreakdown(bt);
        int schoolBonus = (int)pBI->Total();
        ss << EnumEntryText(*sit, spellSchoolTypeMap) << "(" << schoolBonus << ")";
        ++sit;
    }
    std::string description = ss.str();
    return description;
}
