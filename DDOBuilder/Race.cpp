// Race.cpp
//
#include "StdAfx.h"
#include "Race.h"
#include "XmlLib\SaxWriter.h"
#include "Feat.h"
#include "GlobalSupportFunctions.h"
#include "LogPane.h"

// Races need to define a few things to be fully setup:
// Name          - The name of the race
// Description   - What is shown in the UI for this race in a tooltip
// IsConstruct   - So that the builder can tell which races need access to
//                 Warforged feats and Docents as equipment
// NoPastLife      Present if a race is a variant of a main race (e.g. Wood Elf)
// BuildPoints   - The number of build points at set number of TR levels
// IconicClass   - Not present for non-iconic races. This is the class that
//                 the builder will warn for if it is not level 1
// <AbilityName> - optional bonus/penalty amount to each given stat for this race
// SkillPoints   - The number of bonus/penalty skill points the character gets
//                 at each Heroic level
// Skill         - The skill that gets a bonus for a single racial TR in this
//                 Race. Not present for Iconic races.
// Ability       - The Ability that gets a bonus for a 2nd racial TR in this
//                 Race. Not present for Iconic races
// AutoBuySkill  - Any skills that this race should prioritize on an auto buy
//                 action in the skills pane
// RacialFeats     Normal feat definitions for feats specific to this race only
// GrantedFeats    List of feats that should be auto granted to a character of this
//                 race at level 1.
//
// The missing IconicClass, present Skill and Ability fields are used to
// automatically generate the Racial past life feats for non-iconic races,
// and to also setup the "Racial Completionist" feat automatically.

#define DL_ELEMENT Race

namespace
{
    const wchar_t f_saxElementName[] = L"Race";
    DL_DEFINE_NAMES(Race_PROPERTIES)

    const unsigned f_verCurrent = 1;
}

Race::Race() :
    XmlLib::SaxContentElement(f_saxElementName, f_verCurrent)
{
    DL_INIT(Race_PROPERTIES)
}

DL_DEFINE_ACCESS(Race_PROPERTIES)

XmlLib::SaxContentElementInterface * Race::StartElement(
        const XmlLib::SaxString & name,
        const XmlLib::SaxAttributes & attributes)
{
    XmlLib::SaxContentElementInterface * subHandler =
            SaxContentElement::StartElement(name, attributes);

    DL_START(Race_PROPERTIES)

    return subHandler;
}

void Race::EndElement()
{
    // now check out our object
    SaxContentElement::EndElement();
    DL_END(Race_PROPERTIES)
    if (m_BuildPoints.size() != 4)
    {
        SAXASSERT(false, "Race Build Points element incorrect size");
    }
}

void Race::Write(XmlLib::SaxWriter * writer) const
{
    writer->StartElement(ElementName());//, VersionAttributes());
    DL_WRITE(Race_PROPERTIES)
    writer->EndElement();
}

bool Race::IsIconic() const
{
    return HasIconicClass();
}

void Race::AddFeatSlots(
    size_t level,
    std::vector<FeatSlot> * tfts) const
{
    // look through the list of FeatSlots and add those which are for the level in question
    for (auto&& rfli : m_RaceSpecificFeat)
    {
        if (rfli.Level() == level + 1)   // 1 based in file, count (1 based) in code
        {
            // need to add this one
            tfts->push_back(rfli);
        }
    }
}

void Race::VerifyObject() const
{
    bool ok = true;
    std::stringstream ss;
    ss << "=====" << m_Name << "=====\n";

    if (m_BuildPoints.size() != 4)
    {
        ss << "---has incorrect BuildPoints size\n";
    }

    if (!ok)
    {
        GetLog().AddLogEntry(ss.str().c_str());
    }
}

int Race::RacialModifier(const AbilityType ability) const
{
    int modifier = 0;
    switch (ability)
    {
    case Ability_Strength:      if (HasStrength())      modifier = Strength(); break;
    case Ability_Dexterity:     if (HasDexterity())     modifier = Dexterity(); break;
    case Ability_Constitution:  if (HasConstitution())  modifier = Constitution(); break;
    case Ability_Intelligence:  if (HasIntelligence())  modifier = Intelligence(); break;
    case Ability_Wisdom:        if (HasWisdom())        modifier = Wisdom(); break;
    case Ability_Charisma:      if (HasCharisma())      modifier = Charisma(); break;
    }
    return modifier;
}

void Race::ClearRacialFeats()
{
    m_RacialFeats.clear();
}
