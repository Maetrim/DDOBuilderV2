// TrainedFeat.cpp
//
#include "StdAfx.h"
#include "TrainedFeat.h"
#include "XmlLib\SaxWriter.h"
#include "GlobalSupportFunctions.h"

#define DL_ELEMENT TrainedFeat

namespace
{
    const wchar_t f_saxElementName[] = L"TrainedFeat";
    DL_DEFINE_NAMES(TrainedFeat_PROPERTIES)

    const unsigned f_verCurrent = 1;
}

TrainedFeat::TrainedFeat() :
    XmlLib::SaxContentElement(f_saxElementName, f_verCurrent),
    m_count(1)
{
    DL_INIT(TrainedFeat_PROPERTIES)
}

TrainedFeat::TrainedFeat(
        const std::string featName,
        const std::string& type,
        size_t level,
        bool bFeatSwapWarning) :
    XmlLib::SaxContentElement(f_saxElementName, f_verCurrent),
    m_count(1)
{
    DL_INIT(TrainedFeat_PROPERTIES)
    m_hasFeatName = true;
    m_FeatName = featName;
    m_hasType = true;
    m_Type = type;
    m_hasLevelTrainedAt = true;
    m_LevelTrainedAt = level;
    if (bFeatSwapWarning)
    {
        Set_FeatSwapWarning();
    }
}

DL_DEFINE_ACCESS(TrainedFeat_PROPERTIES)

XmlLib::SaxContentElementInterface * TrainedFeat::StartElement(
        const XmlLib::SaxString & name,
        const XmlLib::SaxAttributes & attributes)
{
    XmlLib::SaxContentElementInterface * subHandler =
            SaxContentElement::StartElement(name, attributes);

    DL_START(TrainedFeat_PROPERTIES)

    return subHandler;
}

void TrainedFeat::EndElement()
{
    SaxContentElement::EndElement();
    DL_END(TrainedFeat_PROPERTIES)

    TranslateOldFeatNames();
    // translate old feat slot names of V1 builder to their new names in V2
    if (m_Type == "AasimarBond") m_Type = "Aasimar Bond";
    if (m_Type == "AlchemistBonus") m_Type = "Alchemist Bonus Feat";
    if (m_Type == "ArtificerBonus") m_Type = "Artificer Bonus Feat";
    if (m_Type == "BelovedOf") m_Type = "Beloved Of";
    if (m_Type == "BlightCasterToggleImbue") m_Type = "Toggle Imbue";
    if (m_Type == "ChildOf") m_Type = "Child Of";
    if (m_Type == "DarkHunterFavoredEnemy") m_Type = "Favored Enemy ";
    if (m_Type == "DamageReduction") m_Type = "Damage Reduction";
    if (m_Type == "DomainFeat") m_Type = "Domain Feat";
    if (m_Type == "DragonbornRacial") m_Type = "Dragonborn Racial";
    if (m_Type == "DruidWildShape") m_Type = "Wild Shape";
    if (m_Type == "EnergyResistance") m_Type = "Energy Resistance";
    if (m_Type == "EpicDestinyFeat") m_Type = "Epic Destiny Feat";
    if (m_Type == "EpicFeat") m_Type = "Epic Feat";
    if (m_Type == "FavoredEnemy") m_Type = "Favored Enemy";
    if (m_Type == "FavoredSoulBattle") m_Type = "Battle Feat";
    if (m_Type == "FighterBonus") m_Type = "Fighter Bonus Feat";
    if (m_Type == "FollowerOf") m_Type = "Follower Of";
    if (m_Type == "FavoredSoulHeart") m_Type = "Heart Feat";
    if (m_Type == "HumanBonus") m_Type = "Human Bonus Feat";
    if (m_Type == "KinForm") m_Type = "Kin Form";
    if (m_Type == "Legendary") m_Type = "Legendary Feat";
    if (m_Type == "MajorDragonLordAura") m_Type = "Major Draconic Aura";
    if (m_Type == "Metamagic") m_Type = "Metamagic Feat";
    if (m_Type == "MinorDragonLordAura") m_Type = "Minor Draconic Aura";
    if (m_Type == "RogueSpecialAbility") m_Type = "Rogue Special Ability";
    if (m_Type == "MonkBonus") m_Type = "Monk Bonus";
    if (m_Type == "MonkBonus6") m_Type = "Monk Bonus";
    if (m_Type == "MonkPhilosophy") m_Type = "Monk Philosophy";
    if (m_Type == "PDKBonus") m_Type = "Purple Dragon Knight Bonus Feat";
    if (m_Type == "SuperiorDragonLordAura") m_Type = "Superior Draconic Aura";
    if (m_Type == "TruePact") m_Type = "True Patron";
    if (m_Type == "WarlockPactAbility") m_Type = "Pact Ability";
    if (m_Type == "WarlockResistance") m_Type = "Warlock Resistance";
    if (m_Type == "WarlockPactSaveBonus") m_Type = "Pact Save Bonus";
    if (m_Type == "WarlockPactSpell") m_Type = "Pact Spell";
    if (m_Type == "WarlockPact") m_Type = "Warlock Pact";

    // all Favored Enemy feats changed their name
    m_FeatName = ReplaceAll(m_FeatName, "Favored enemy", "Favored Enemy");
}

void TrainedFeat::Write(XmlLib::SaxWriter * writer) const
{
    writer->StartElement(ElementName());//, VersionAttributes());
    DL_WRITE(TrainedFeat_PROPERTIES)
    writer->EndElement();
}

bool TrainedFeat::operator<(const TrainedFeat & other) const
{
    if (Type() == other.Type())
    {
        return (FeatName() < other.FeatName());
    }
    else
    {
        return (Type() < other.Type());
    }
}

bool TrainedFeat::operator==(const TrainedFeat & other) const
{
    // ignore revoked feat for comparison
    return m_FeatName == other.m_FeatName
            && m_Type == other.m_Type
            && m_LevelTrainedAt == other.m_LevelTrainedAt;
}

size_t TrainedFeat::Count() const
{
    return m_count;
}

void TrainedFeat::IncrementCount()
{
    ++m_count;
}

void TrainedFeat::SetAlternateFeat(const std::string& featName)
{
    Set_AlternateFeatName(featName);
}

void TrainedFeat::TranslateOldFeatNames()
{
    static std::string nameTranslations[] =
    {
        // old feat name                        new feat name
        "Air Domain Tier I",                    "Domain of Air",
        "Air Domain Tier II",                   "Improved Domain of Air",
        "Air Domain Tier III",                  "Greater Domain of Air",
        "Air Domain Tier IV",                   "Master of the Domain of Air",

        "Animal Domain Tier I",                 "Domain of Animals",
        "Animal Domain Tier II",                "Improved Domain of Animals",
        "Animal Domain Tier III",               "Greater Domain of Animals",
        "Animal Domain Tier IV",                "Master of the Domain of Animals",

        "Chaos Domain Tier I",                  "Domain of Chaos",
        "Chaos Domain Tier II",                 "Improved Domain of Chaos",
        "Chaos Domain Tier III",                "Greater Domain of Chaos",
        "Chaos Domain Tier IV",                 "Master of the Domain of Chaos",

        "Death Domain Tier I",                  "Domain of Death",
        "Death Domain Tier II",                 "Improved Domain of Death",
        "Death Domain Tier III",                "Greater Domain of Death",
        "Death Domain Tier IV",                 "Master of the Domain of Death",

        "Destruction Domain Tier I",            "Domain of Destruction",
        "Destruction Domain Tier II",           "Improved Domain of Destruction",
        "Destruction Domain Tier III",          "Greater Domain of Destruction",
        "Destruction Domain Tier IV",           "Master of the Domain of Destruction",

        "Earth Domain Tier I",                  "Domain of Earth",
        "Earth Domain Tier II",                 "Improved Domain of Earth",
        "Earth Domain Tier III",                "Greater Domain of Earth",
        "Earth Domain Tier IV",                 "Master of the Domain of Earth",

        "Fire Domain Tier I",                   "Domain of Fire",
        //"Improved Domain of Fire",              "Improved Domain of Fire",
        //"Greater Domain of Fire",               "Greater Domain of Fire",
        "Fire Domain Tier IV",                  "Master of the Domain of Fire",

        "Good Domain Tier I",                   "Domain of Good",
        "Good Domain Tier II",                  "Improved Domain of Good",
        "Good Domain Tier III",                 "Greater Domain of Good",
        "Good Domain Tier IV",                  "Master of the Domain of Good",

        "Healing Domain Tier I",                "Domain of Healing",
        "Healing Domain Tier II",               "Improved Domain of Healing",
        "Healing Domain Tier III",              "Greater Domain of Healing",
        "Healing Domain Tier IV",               "Master of the Domain of Healing",

        "Knowledge Domain Tier I",              "Domain of Knowledge",
        "Knowledge Domain Tier II",             "Improved Domain of Knowledge",
        "Knowledge Domain Tier III",            "Greater Domain of Knowledge",
        "Knowledge Domain Tier IV",             "Master of the Domain of Knowledge",

        "Law Domain Tier I",                    "Domain of Law",
        "Law Domain Tier II",                   "Improved Domain of Law",
        "Law Domain Tier III",                  "Greater Domain of Law",
        "Law Domain Tier IV",                   "Master of the Domain of Law",

        "Luck Domain Tier I",                   "Domain of Luck",
        "Luck Domain Tier II",                  "Improved Domain of Luck",
        "Luck Domain Tier III",                 "Greater Domain of Luck",
        "Luck Domain Tier IV",                  "Master of the Domain of Luck",

        "Magic Domain Tier I",                  "Domain of Magic",
        "Magic Domain Tier II",                 "Improved Domain of Magic",
        "Magic Domain Tier III",                "Greater Domain of Magic",
        "Magic Domain Tier IV",                 "Master of the Domain of Magic",

        "Protection Domain Tier I",             "Domain of Protection",
        "Protection Domain Tier II",            "Improved Domain of Protection",
        "Protection Domain Tier III",           "Greater Domain of Protection",
        "Protection Domain Tier IV",            "Master of the Domain of Protection",

        "Strength Domain Tier I",               "Domain of Strength",
        "Strength Domain Tier II",              "Improved Domain of Strength",
        "Strength Domain Tier III",             "Greater Domain of Strength",
        "Strength Domain Tier IV",              "Master of the Domain of Strength",

        "Sun Domain Tier I",                    "Domain of Sun",
        "Sun Domain Tier II",                   "Improved Domain of Sun",
        "Sun Domain Tier III",                  "Greater Domain of Sun",
        "Sun Domain Tier IV",                   "Master of the Domain of Sun",

        "Trickery Domain Tier I",               "Domain of Trickery",
        "Trickery Domain Tier II",              "Improved Domain of Trickery",
        "Trickery Domain Tier III",             "Greater Domain of Trickery",
        "Trickery Domain Tier IV",              "Master of the Domain of Trickery",

        "War Domain Tier I",                    "Domain of War",
        "War Domain Tier II",                   "Improved Domain of War",
        "War Domain Tier III",                  "Greater Domain of War",
        "War Domain Tier IV",                   "Master of the Domain of War",

        "Water Domain Tier I",                  "Domain of Water",
        "Water Domain Tier II",                 "Improved Domain of Water",
        "Water Domain Tier III",                "Greater Domain of Water",
        "Water Domain Tier IV",                 "Master of the Domain of Water",

        "Warlock: Pact: Fey",                   "Pact: Fey",
        "Warlock: Pact: Fiend",                 "Pact: Fiend",
        "Warlock: Pact: Great Old One",         "Pact: Great Old One",
        "Warlock: Pact: Celestial",             "Pact: Celestial",
        "Warlock: Pact: The Abyss",             "Pact: The Abyss",
        "Warlock: Pact: The Carceri Storm",     "Pact: The Carceri Storm",

        "Past Life: Bard (Stormsinger)",        "Past Life: Bard - Stormsinger",
        "Past Life: Cleric (Dark Apostate)",    "Past Life: Cleric - Dark Apostate",
        "Past Life: Druid (Blight Caster)",     "Past Life: Druid - Blight Caster",
        "Past Life: Fighter (Dragon Lord)",     "Past Life: Fighter - Dragon Lord",
        "Past Life: Paladin (Sacred Fist)",     "Past Life: Paladin - Sacred Fist",
        "Past Life: Ranger (Dark Hunter)",      "Past Life: Ranger - Dark Hunter",
        "Past Life: Sorcerer (Wild Mage)",      "Past Life: Sorcerer - Wild Mage",
        "Past Life: Warlock (Acolyte of the Skin)", "Past Life: Warlock - Acolyte of the Skin"
    };
    size_t count = sizeof(nameTranslations) / sizeof(std::string);
    if (count % 2 != 0)
    {
        throw "Must be an multiple of 2";
    }
    for (size_t i = 0; i < count; i += 2)
    {
        if (m_FeatName == nameTranslations[i])
        {
            m_FeatName = nameTranslations[i + 1];
            break;
        }
    }
}
