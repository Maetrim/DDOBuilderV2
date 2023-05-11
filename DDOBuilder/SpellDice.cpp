// SpellDice.cpp
//
#include "StdAfx.h"
#include "SpellDice.h"
#include "XmlLib\SaxWriter.h"
#include "GlobalSupportFunctions.h"

#define DL_ELEMENT SpellDice

namespace
{
    const wchar_t f_saxElementName[] = L"SpellDice";
    DL_DEFINE_NAMES(SpellDice_PROPERTIES)

    const unsigned f_verCurrent = 1;
}

SpellDice::SpellDice() :
    XmlLib::SaxContentElement(f_saxElementName, f_verCurrent)
{
    DL_INIT(SpellDice_PROPERTIES)
}

DL_DEFINE_ACCESS(SpellDice_PROPERTIES)

XmlLib::SaxContentElementInterface * SpellDice::StartElement(
        const XmlLib::SaxString & name,
        const XmlLib::SaxAttributes & attributes)
{
    XmlLib::SaxContentElementInterface * subHandler =
            SaxContentElement::StartElement(name, attributes);

    DL_START(SpellDice_PROPERTIES)

    return subHandler;
}

void SpellDice::EndElement()
{
    SaxContentElement::EndElement();
    DL_END(SpellDice_PROPERTIES)
}

void SpellDice::Write(XmlLib::SaxWriter * writer) const
{
    writer->StartElement(ElementName());//, VersionAttributes());
    DL_WRITE(SpellDice_PROPERTIES)
    writer->EndElement();
}

CString SpellDice::AverageDamageText(
    double spellPower,
    int casterLevel) const
{
    double standardDiceMin = 0.0;
    double standardDiceMax = 0.0;
    double standardDiceAverage = 0.0;
    if (HasStandardDice())
    {
        standardDiceMin = StandardDice().MinDamage();
        standardDiceMax = StandardDice().MaxDamage();
        standardDiceAverage = StandardDice().AverageDamage();
    }
    double perCasterLevelDiceMin = 0.0;
    double perCasterLevelDiceMax = 0.0;
    double perCasterLevelDiceAverage = 0.0;
    size_t perCasterLevel = 1;
    if (HasDicePerCasterLevel())
    {
        perCasterLevel = HasPerCasterLevels() ? PerCasterLevels() : 1;
        perCasterLevelDiceMin = DicePerCasterLevel().MinDamage();
        perCasterLevelDiceMax = DicePerCasterLevel().MaxDamage();
        perCasterLevelDiceAverage = DicePerCasterLevel().AverageDamage();
    }

    double totalMin = standardDiceMin + perCasterLevelDiceMin * floor(casterLevel / perCasterLevel);
    double totalMax = standardDiceMax + perCasterLevelDiceMax * floor(casterLevel / perCasterLevel);
    double totalAv = standardDiceAverage + perCasterLevelDiceAverage * floor(casterLevel / perCasterLevel);

    totalMin = floor(totalMin * spellPower);
    totalMax = floor(totalMax * spellPower);
    totalAv = floor(totalAv * spellPower);

    CString txt;
    txt.Format("Average Damage: %d - %d, Average %d\r\n",
        (int)totalMin,
        (int)totalMax,
        (int)totalAv);
    return txt;
}

CString SpellDice::CriticalDamageText(
    double spellPower,
    int casterLevel,
    double criticalMultiplier) const
{
    double standardDiceMin = 0.0;
    double standardDiceMax = 0.0;
    double standardDiceAverage = 0.0;
    if (HasStandardDice())
    {
        standardDiceMin = StandardDice().MinDamage();
        standardDiceMax = StandardDice().MaxDamage();
        standardDiceAverage = StandardDice().AverageDamage();
    }
    double perCasterLevelDiceMin = 0.0;
    double perCasterLevelDiceMax = 0.0;
    double perCasterLevelDiceAverage = 0.0;
    size_t perCasterLevel = 1;
    if (HasDicePerCasterLevel())
    {
        perCasterLevel = HasPerCasterLevels() ? PerCasterLevels() : 1;
        perCasterLevelDiceMin = DicePerCasterLevel().MinDamage();
        perCasterLevelDiceMax = DicePerCasterLevel().MaxDamage();
        perCasterLevelDiceAverage = DicePerCasterLevel().AverageDamage();
    }

    double totalMin = standardDiceMin + perCasterLevelDiceMin * floor(casterLevel / perCasterLevel);
    double totalMax = standardDiceMax + perCasterLevelDiceMax * floor(casterLevel / perCasterLevel);
    double totalAv = standardDiceAverage + perCasterLevelDiceAverage * floor(casterLevel / perCasterLevel);

    totalMin = floor(totalMin * spellPower * criticalMultiplier);
    totalMax = floor(totalMax * spellPower * criticalMultiplier);
    totalAv = floor(totalAv * spellPower * criticalMultiplier);

    CString txt;
    txt.Format("Average Critical Damage: %d - %d, Average %d",
        (int)totalMin,
        (int)totalMax,
        (int)totalAv);
    return txt;
}
