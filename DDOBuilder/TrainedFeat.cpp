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

    // translate old feat slot names of V1 builder to thier new names in V2
    if (m_Type == "AasimarBond") m_Type = "Aasimar Bond";
    if (m_Type == "AlchemistBonus") m_Type = "Alchemist Bonus Feat";
    if (m_Type == "ArtificerBonus") m_Type = "Artificer Bonus Feat";
    if (m_Type == "BelovedOf") m_Type = "Beloved Of";
    if (m_Type == "BlightCasterToggleImbue") m_Type = "Toggle Imbue";
    if (m_Type == "ChildOf") m_Type = "Child Of";
    if (m_Type == "DarkHunterFavoredEnemy") m_Type = "Favored Enemy ";
    if (m_Type == "DamageReduction") m_Type = "Damage Reduction";
    if (m_Type == "DomainFeat") m_Type = "Domain Feat";
    if (m_Type == "DruidWildShape") m_Type = "Wild Shape";
    if (m_Type == "EnergyResistance") m_Type = "Energy Resistance";
    if (m_Type == "EpicDestinyFeat") m_Type = "Epic Destiny Feat";
    if (m_Type == "EpicFeat") m_Type = "Epic Feat";
    if (m_Type == "FavoredEnemy") m_Type = "Favored Enemy";
    if (m_Type == "FavoredSoulBattle") m_Type = "Battle Feat";
    if (m_Type == "FighterBonus") m_Type = "Fighter Bonus Feat";
    if (m_Type == "FollowerOf") m_Type = "Follower Of";
    if (m_Type == "FavoredSoulHeart") m_Type = "Heart Feat";
    if (m_Type == "KinForm") m_Type = "Kin Form";
    if (m_Type == "Legendary") m_Type = "Legendary Feat";
    if (m_Type == "MajorDragonLordAura") m_Type = "Major Draconic Aura";
    if (m_Type == "Metamagic") m_Type = "Metamagic Feat";
    if (m_Type == "MinorDragonLordAura") m_Type = "Minor Draconic Aura";
    if (m_Type == "MinorDragonLordAura") m_Type = "Rogue Special Ability";
    if (m_Type == "MonkBonus") m_Type = "Monk Bonus";
    if (m_Type == "MonkBonus6") m_Type = "Monk Bonus";
    if (m_Type == "MonkPhilosophy") m_Type = "Monk Philosophy";
    if (m_Type == "RogueSpecialAbility") m_Type = "Superior Draconic Aura";
    if (m_Type == "TruePact") m_Type = "True Patron";
    if (m_Type == "WarlockPactAbility") m_Type = "Pact Ability";
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
