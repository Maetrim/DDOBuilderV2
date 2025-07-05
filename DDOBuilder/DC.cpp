// DC.cpp
//
#include "StdAfx.h"
#include "DC.h"
#include "XmlLib\SaxWriter.h"
#include "GlobalSupportFunctions.h"
#include "BreakdownItem.h"
#include "Build.h"
#include "Class.h"

#define DL_ELEMENT DC

namespace
{
    const wchar_t f_saxElementName[] = L"DC";
    DL_DEFINE_NAMES(DC_PROPERTIES)

    const unsigned f_verCurrent = 1;
}

DC::DC() :
    XmlLib::SaxContentElement(f_saxElementName, f_verCurrent),
    m_stacks(1)
{
    DL_INIT(DC_PROPERTIES)
}

DL_DEFINE_ACCESS(DC_PROPERTIES)

XmlLib::SaxContentElementInterface * DC::StartElement(
        const XmlLib::SaxString & name,
        const XmlLib::SaxAttributes & attributes)
{
    XmlLib::SaxContentElementInterface * subHandler =
            SaxContentElement::StartElement(name, attributes);

    DL_START(DC_PROPERTIES)

    return subHandler;
}

void DC::EndElement()
{
    SaxContentElement::EndElement();
    DL_END(DC_PROPERTIES)
}

void DC::Write(XmlLib::SaxWriter * writer) const
{
    writer->StartElement(ElementName());//, VersionAttributes());
    DL_WRITE(DC_PROPERTIES)
    writer->EndElement();
}

bool DC::VerifyObject(std::stringstream * ss) const
{
    bool ok = true;
    std::list<AbilityType>::const_iterator fait = m_FullAbility.begin();
    while (fait != m_FullAbility.end())
    {
        if ((*fait) == Ability_Unknown)
        {
            *ss << "---DC has bad FullAbility value type\r\n";
            ok = false;
        }
        ++fait;
    }
    std::list<AbilityType>::const_iterator mait = m_ModAbility.begin();
    while (mait != m_ModAbility.end())
    {
        if ((*mait) == Ability_Unknown)
        {
            *ss << "---DC has bad ModAbility value type\r\n";
            ok = false;
        }
        ++mait;
    }
    if (!ImageFileExists("DataFiles\\FeatImages\\", Icon())
            && !ImageFileExists("DataFiles\\EnhancementImages\\", Icon())
            && !ImageFileExists("DataFiles\\ItemImages\\", Icon())
            && !ImageFileExists("DataFiles\\UIImages\\", Icon())
            && !ImageFileExists("DataFiles\\SpellImages\\", Icon()))
    {
        *ss << "DC is missing image file \"" << Icon() << "\"\n";
        ok = false;
    }
    if (HasTactical2() && !HasTactical())
    {
        *ss << "DC is missing Tactical field with tactical 2 present\n";
        ok = false;
    }
    return ok;
}

int DC::CalculateDC(const Build * build) const
{
    int value = 0;
    if (m_hasAmount)
    {
        if (m_stacks < m_Amount.size() + 1)
        {
            value += m_Amount[m_stacks-1];    // 0 based
        }
        else
        {
            // if we have more stacks than elements, use the last value
            value += m_Amount[m_Amount.size()-1];
        }
    }
    // use the largest of any FullAbility values
    int fullAbilityBonus = 0;
    std::list<AbilityType>::const_iterator fait = m_FullAbility.begin();
    while (fait != m_FullAbility.end())
    {
        BreakdownType bt = StatToBreakdown(*fait);
        BreakdownItem * pBI = FindBreakdown(bt);
        int abilityBonus = (int)pBI->Total();
        if (abilityBonus > fullAbilityBonus)
        {
            fullAbilityBonus = abilityBonus;
        }
        ++fait;
    }
    value += fullAbilityBonus;
    // use the largest of any ModAbility values
    int modAbilityBonus = -5;
    std::list<AbilityType>::const_iterator mait = m_ModAbility.begin();
    while (mait != m_ModAbility.end())
    {
        BreakdownType bt = StatToBreakdown(*mait);
        BreakdownItem * pBI = FindBreakdown(bt);
        int abilityBonus = BaseStatToBonus(pBI->Total());
        if (abilityBonus > modAbilityBonus)
        {
            modAbilityBonus = abilityBonus;
        }
        ++mait;
    }
    value += modAbilityBonus;
    // add any skill breakdown bonus
    if (m_hasSkill)
    {
        BreakdownType bt = SkillToBreakdown(m_Skill);
        BreakdownItem * pBI = FindBreakdown(bt);
        int skillBonus = (int)pBI->Total();
        value += skillBonus;
    }
    // add any tactical breakdown bonus
    if (m_hasTactical)
    {
        BreakdownType bt = TacticalToBreakdown(m_Tactical);
        BreakdownItem * pBI = FindBreakdown(bt);
        int tacticalBonus = (int)pBI->Total();
        value += tacticalBonus;
        if (m_hasTactical2)
        {
            bt = TacticalToBreakdown(m_Tactical2);
            pBI = FindBreakdown(bt);
            tacticalBonus = (int)pBI->Total();
            value += tacticalBonus;
        }
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
    // add any class level bonus
    double multiplier = 1;
    if (HasClassMultiplier())
    {
        if (m_stacks < m_ClassMultiplier.size() + 1)
        {
            multiplier = m_ClassMultiplier[m_stacks-1];    // 0 based
        }
    }
    if (m_hasClassLevel)
    {
        int classLevels = build->ClassLevels(m_ClassLevel, build->Level()-1);
        value += (int)(classLevels * multiplier);
    }
    // add any half class level bonus
    if (m_hasHalfClassLevel)
    {
        int classLevels = build->ClassLevels(m_HalfClassLevel, build->Level()-1);
        value += (int)((classLevels * multiplier) / 2); // round down
    }
    // add any base class level bonus
    if (m_hasBaseClassLevel)
    {
        int baseClassLevels = build->BaseClassLevels(m_BaseClassLevel, build->Level() - 1);
        value += (int)(baseClassLevels * multiplier);
    }
    // add any half base class level bonus
    if (m_hasHalfClassLevel)
    {
        int baseClassLevels = build->BaseClassLevels(m_HalfBaseClassLevel, build->Level() - 1);
        value += (int)((baseClassLevels * multiplier) / 2); // round down
    }
    return value;
}

std::string DC::DCBreakdown(const Build * build) const
{
    std::stringstream ss;
    if (m_hasOther)
    {
        ss << m_DCVersus << " vs " << m_Other << " + " << CalculateDC(build) << " : ";
    }
    else
    {
        ss << m_DCVersus << " vs " << CalculateDC(build) << " : ";
    }
    bool first = true;
    if (m_hasOther)
    {
        ss << m_Other;
        first = false;
    }
    if (m_hasAmount)
    {
        if (!first)
        {
            ss << " + ";
        }
        int value = 0;
        if (m_stacks < m_Amount.size() + 1)
        {
            value += m_Amount[m_stacks-1];    // 0 based
        }
        else
        {
            // if we have more stacks than elements, use the last value
            value += m_Amount[m_Amount.size()-1];
        }
        ss << value;
        first = false;
    }

    // Show any FullAbility values
    // Max Mod(Str 10, Wis 15) is multiple or Str(10) if one
    {
        std::list<AbilityType>::const_iterator fait = m_FullAbility.begin();
        if (m_FullAbility.size() == 1)
        {
            if (!first)
            {
                ss << " + ";
            }
            first = false;
            BreakdownType bt = StatToBreakdown(*fait);
            BreakdownItem * pBI = FindBreakdown(bt);
            int abilityBonus = (int)pBI->Total();
            std::string name = EnumEntryText(*fait, abilityTypeMap);
            name.resize(3);     // truncate to 1st 3 characters, e.g. "Strength" becomes "Str"
            ss << name << "(" << abilityBonus << ")";
        }
        else if (m_FullAbility.size() > 1)
        {
            if (!first)
            {
                ss << " + ";
            }
            ss << "Max(";
            while (fait != m_FullAbility.end())
            {
                BreakdownType bt = StatToBreakdown(*fait);
                BreakdownItem * pBI = FindBreakdown(bt);
                int abilityBonus = (int)pBI->Total();
                if (fait != m_FullAbility.begin())
                {
                    ss << ", ";
                }
                std::string name = EnumEntryText(*fait, abilityTypeMap);
                name.resize(3);     // truncate to 1st 3 characters, e.g. "Strength" becomes "Str"
                ss << name << "(" << abilityBonus << ")";
                ++fait;
            }
            ss << ")";
        }
    }
    // use the largest of any ModAbility values
    {
        std::list<AbilityType>::const_iterator mait = m_ModAbility.begin();
        if (m_ModAbility.size() == 1)
        {
            if (!first)
            {
                ss << " + ";
            }
            first = false;
            BreakdownType bt = StatToBreakdown(*mait);
            BreakdownItem * pBI = FindBreakdown(bt);
            int abilityBonus = BaseStatToBonus(pBI->Total());
            std::string name = EnumEntryText(*mait, abilityTypeMap);
            name.resize(3);     // truncate to 1st 3 characters, e.g. "Strength" becomes "Str"
            ss << name << " Mod(" << abilityBonus << ")";
        }
        else if (m_ModAbility.size() > 1)
        {
            if (!first)
            {
                ss << " + ";
            }
            ss << "Max Mod(";
            while (mait != m_ModAbility.end())
            {
                BreakdownType bt = StatToBreakdown(*mait);
                BreakdownItem * pBI = FindBreakdown(bt);
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
    }
    // add any skill breakdown bonus
    if (m_hasSkill)
    {
        if (!first)
        {
            ss << " + ";
        }
        first = false;
        BreakdownType bt = SkillToBreakdown(m_Skill);
        BreakdownItem * pBI = FindBreakdown(bt);
        int skillBonus = (int)pBI->Total();
        ss << EnumEntryText(m_Skill, skillTypeMap) << "(" << skillBonus << ")";
    }
    // add any tactical breakdown bonus
    if (m_hasTactical)
    {
        if (!first)
        {
            ss << " + ";
        }
        first = false;
        BreakdownType bt = TacticalToBreakdown(m_Tactical);
        BreakdownItem * pBI = FindBreakdown(bt);
        int tacticalBonus = (int)pBI->Total();
        ss << EnumEntryText(m_Tactical, tacticalTypeMap) << "(" << tacticalBonus << ")";
        if (m_hasTactical2)
        {
            if (!first)
            {
                ss << " + ";
            }
            first = false;
            bt = TacticalToBreakdown(m_Tactical2);
            pBI = FindBreakdown(bt);
            tacticalBonus = (int)pBI->Total();
            ss << EnumEntryText(m_Tactical2, tacticalTypeMap) << "(" << tacticalBonus << ")";
        }
    }
    // spell school bonuses, all of these get added if present
    std::list<SpellSchoolType>::const_iterator sit = m_School.begin();
    while (sit != m_School.end())
    {
        if (!first)
        {
            ss << " + ";
        }
        first = false;
        BreakdownType bt = SchoolToBreakdown(*sit);
        BreakdownItem * pBI = FindBreakdown(bt);
        int schoolBonus = (int)pBI->Total();
        ss << EnumEntryText(*sit, spellSchoolTypeMap) << "(" << schoolBonus << ")";
        ++sit;
    }
    // add any class level bonus
    double multiplier = 1;
    if (HasClassMultiplier())
    {
        if (m_stacks < m_ClassMultiplier.size() + 1)
        {
            multiplier = m_ClassMultiplier[m_stacks-1];    // 0 based
        }
    }
    if (multiplier > 0)
    {
        // add any class level bonus
        if (m_hasClassLevel)
        {
            if (!first)
            {
                ss << " + ";
            }
            first = false;
            std::string name = m_ClassLevel;
            int classLevels = build->ClassLevels(m_ClassLevel, build->Level()-1);
            if (multiplier != 1)
            {
                ss << multiplier << " ";
            }
            if (m_ClassLevel == "All")
            {
                name = "Character Level";
            }
            ss << name << "(" << (int)(classLevels * multiplier) << ")";
        }
        // add any half class level bonus
        if (m_hasHalfClassLevel)
        {
            if (!first)
            {
                ss << " + ";
            }
            first = false;
            int classLevels = (int)(build->ClassLevels(m_HalfClassLevel, build->Level()-1) / 2);
            std::string name = m_HalfClassLevel;
            if (multiplier != 1)
            {
                ss << multiplier << " ";
            }
            if (name == "All")
            {
                name = "Character Level";
            }
            ss << name << "/2(" << (int)(classLevels * multiplier) << ")";
        }
        // add any base class level bonus
        if (m_hasBaseClassLevel)
        {
            if (!first)
            {
                ss << " + ";
            }
            first = false;
            std::string name = m_BaseClassLevel;
            int classLevels = build->BaseClassLevels(m_BaseClassLevel, build->Level() - 1);
            if (multiplier != 1)
            {
                ss << multiplier << " ";
            }
            if (m_BaseClassLevel == "All")
            {
                name = "Character Level";
            }
            ss << name << "(" << (int)(classLevels * multiplier) << ")";
        }
        // add any half base class level bonus
        if (m_hasHalfBaseClassLevel)
        {
            if (!first)
            {
                ss << " + ";
            }
            first = false;
            int classLevels = (int)(build->BaseClassLevels(m_HalfBaseClassLevel, build->Level() - 1) / 2);
            std::string name = m_HalfBaseClassLevel;
            if (multiplier != 1)
            {
                ss << multiplier << " ";
            }
            if (name == "All")
            {
                name = "Character Level";
            }
            ss << name << "/2(" << (int)(classLevels * multiplier) << ")";
        }
    }
    std::string description = ss.str();
    description = ReplaceAll(description, "0.5", "�");
    return description;
}

bool DC::operator==(const DC & other) const
{
    return (m_Name == other.m_Name)
            && (m_Icon == other.m_Icon);
}

void DC::SetClass(const std::string& ct)
{
    m_class = ct;
}

void DC::AddStack()
{
    m_stacks++;
}

void DC::RevokeStack()
{
    m_stacks--;
}

size_t DC::NumStacks() const
{
    return m_stacks;
}

