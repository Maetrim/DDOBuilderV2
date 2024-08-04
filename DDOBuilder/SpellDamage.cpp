// SpellDamage.cpp
//
#include "StdAfx.h"
#include "SpellDamage.h"
#include "XmlLib\SaxWriter.h"

#include "Build.h"
#include "BreakdownItem.h"
#include "GlobalSupportFunctions.h"

#define DL_ELEMENT SpellDamage

namespace
{
    const wchar_t f_saxElementName[] = L"SpellDamage";
    DL_DEFINE_NAMES(SpellDamage_PROPERTIES)

    const unsigned f_verCurrent = 1;
}

SpellDamage::SpellDamage() :
    XmlLib::SaxContentElement(f_saxElementName, f_verCurrent)
{
    DL_INIT(SpellDamage_PROPERTIES)
}

DL_DEFINE_ACCESS(SpellDamage_PROPERTIES)

XmlLib::SaxContentElementInterface * SpellDamage::StartElement(
        const XmlLib::SaxString & name,
        const XmlLib::SaxAttributes & attributes)
{
    XmlLib::SaxContentElementInterface * subHandler =
            SaxContentElement::StartElement(name, attributes);

    DL_START(SpellDamage_PROPERTIES)

    return subHandler;
}

void SpellDamage::EndElement()
{
    SaxContentElement::EndElement();
    DL_END(SpellDamage_PROPERTIES)
}

void SpellDamage::Write(XmlLib::SaxWriter * writer) const
{
    writer->StartElement(ElementName());//, VersionAttributes());
    DL_WRITE(SpellDamage_PROPERTIES)
    writer->EndElement();
}

CString SpellDamage::SpellDamageText(const Build&, size_t casterLevel) const
{
    CString totalText;

    double spellPower = 1.0;        // assume
    double spellCriticalChance = 0.0;
    double spellCriticalMultiplier = 2.0;       // default 2* damage
    CString spellPowerName("Unknown");
    if (HasSpellPower())
    {
        SpellPowerType spt = SpellPower();
        spellPowerName = EnumEntryText(spt, spellPowerTypeMap);
        BreakdownItem* pBI = FindBreakdown(SpellPowerToBreakdown(spt));
        if (pBI != NULL)
        {
            // make sure we use the highest of any replacement spell power also
            spellPower = ((int)pBI->ReplacementTotal() / 100.0);
        }
        pBI = FindBreakdown(SpellPowerToCriticalChanceBreakdown(spt));
        if (pBI != NULL)
        {
            // make sure we use the highest of any replacement spell power also
            spellCriticalChance = pBI->ReplacementTotal();
        }
        pBI = FindBreakdown(SpellPowerToCriticalMultiplierBreakdown(spt));
        if (pBI != NULL)
        {
            // make sure we use the highest of any replacement spell power also
            spellCriticalMultiplier += (pBI->ReplacementTotal() / 100.0);
        }
    }

    CString txt;
    txt.Format("%s Spell Power: %.0f\r\n", (LPCSTR)spellPowerName, spellPower * 100.0);
    totalText += txt;

    txt.Format("Critical chance = %d%%, Multiplier = %.2f\r\n",
        (int)spellCriticalChance,
        spellCriticalMultiplier);
    totalText += txt;

    txt = DamageDice().AverageDamageText(1 + spellPower, casterLevel);
    totalText += txt;

    txt = DamageDice().CriticalDamageText(1 + spellPower, casterLevel, spellCriticalMultiplier);
    totalText += txt;

    return totalText;
}

CString SpellDamage::DiceDamageText() const
{
    CString fullText;
    // show the spell base damage as:
    // Spell Damage: xDy +6 + (aDb + c per n caster levels)
    if (DamageDice().HasStandardDice())
    {
        CString dt = DamageDice().StandardDice().DiceAsText();
        if (DamageDice().HasDicePerCasterLevel())
        {
            CString bdt = DamageDice().DicePerCasterLevel().DiceAsText();
            if (DamageDice().HasPerCasterLevels()
                && DamageDice().PerCasterLevels() > 1)
            {
                CString bt;
                bt.Format(" per %d caster levels", DamageDice().PerCasterLevels());
                bdt += bt;
            }
            else
            {
                bdt += " per caster level";
            }
            fullText.Format("Spell Damage: %s + (%s)", (LPCSTR)dt, (LPCSTR)bdt);
        }
        else
        {
            fullText.Format("Spell Damage: %s", (LPCSTR)dt);
        }
    }
    else
    {
        if (DamageDice().HasDicePerCasterLevel())
        {
            CString bdt = DamageDice().DicePerCasterLevel().DiceAsText();
            if (DamageDice().HasPerCasterLevels()
                && DamageDice().PerCasterLevels() > 1)
            {
                CString bt;
                bt.Format(" per %d caster levels", DamageDice().PerCasterLevels());
                bdt += bt;
            }
            else
            {
                bdt += " per caster level";
            }
            fullText.Format("Spell Damage: %s", (LPCSTR)bdt);
        }
    }
    return fullText;
}

CString SpellDamage::AverageDamageText(
        size_t casterLevel) const
{
    CString txt;

    double spellPower = 1.0;        // assume
    CString spellPowerName("Unknown");
    if (HasSpellPower())
    {
        SpellPowerType spt = SpellPower();
        spellPowerName = EnumEntryText(spt, spellPowerTypeMap);
        BreakdownItem* pBI = FindBreakdown(SpellPowerToBreakdown(spt));
        if (pBI != NULL)
        {
            // make sure we use the highest of any replacement spell power also
            spellPower = ((int)pBI->ReplacementTotal() / 100.0);
        }
    }

    txt = DamageDice().AverageDamageText(1 + spellPower, casterLevel);
    txt.Replace("Average Damage: ", "");
    txt.Replace("\r\n", "");
    return txt;
}

CString SpellDamage::CriticalDamageText(
        size_t casterLevel) const
{
    CString txt;

    double spellPower = 1.0;        // assume
    double spellCriticalChance = 0.0;
    double spellCriticalMultiplier = 2.0;       // default 2* damage
    CString spellPowerName("Unknown");
    if (HasSpellPower())
    {
        SpellPowerType spt = SpellPower();
        spellPowerName = EnumEntryText(spt, spellPowerTypeMap);
        BreakdownItem* pBI = FindBreakdown(SpellPowerToBreakdown(spt));
        if (pBI != NULL)
        {
            // make sure we use the highest of any replacement spell power also
            spellPower = ((int)pBI->ReplacementTotal() / 100.0);
        }
        pBI = FindBreakdown(SpellPowerToCriticalChanceBreakdown(spt));
        if (pBI != NULL)
        {
            // make sure we use the highest of any replacement spell power also
            spellCriticalChance = pBI->ReplacementTotal();
        }
        pBI = FindBreakdown(SpellPowerToCriticalMultiplierBreakdown(spt));
        if (pBI != NULL)
        {
            // make sure we use the highest of any replacement spell power also
            spellCriticalMultiplier += (pBI->ReplacementTotal() / 100.0);
        }
    }

    txt = DamageDice().CriticalDamageText(1 + spellPower, casterLevel, spellCriticalMultiplier);
    txt.Replace("Average Critical Damage: ", "");
    txt.Replace("\r\n", "");
    return txt;
}

bool SpellDamage::VerifyObject(std::stringstream* ss, bool bHasMaxcasterLevel) const
{
    bool ok = true;
    if (DamageDice().HasStandardDice())
    {
        if (DamageDice().StandardDice().HasNumber() && !DamageDice().StandardDice().HasSides())
        {
            (*ss) << "Spell StandardDice missing Sides field\n";
            ok = false;
        }
        if (!DamageDice().StandardDice().HasNumber() && DamageDice().StandardDice().HasSides())
        {
            (*ss) << "Spell StandardDice missing Number field\n";
            ok = false;
        }
    }
    if (DamageDice().HasDicePerCasterLevel())
    {
        if (DamageDice().DicePerCasterLevel().HasNumber() && !DamageDice().DicePerCasterLevel().HasSides())
        {
            (*ss) << "Spell DicePerCasterLevel missing Sides field\n";
            ok = false;
        }
        if (!DamageDice().DicePerCasterLevel().HasNumber() && DamageDice().DicePerCasterLevel().HasSides())
        {
            (*ss) << "Spell DicePerCasterLevel missing Number field\n";
            ok = false;
        }
    }
    if (DamageDice().HasDicePerCasterLevel()
        && !bHasMaxcasterLevel)
    {
        (*ss) << "Spell is missing MaxCasterLevel field\n";
        ok = false;
    }
    if (HasDamage())
    {
        if (Damage() == Damage_Unknown)
        {
            (*ss) << "Spell has bad SpellDamage bad damage type\n";
            ok = false;
        }
    }
    if (HasSpellPower())
    {
        if (SpellPower() == SpellPower_Unknown)
        {
            (*ss) << "Spell has bad SpellDamage bad SpellPower type\n";
            ok = false;
        }
    }
    return ok;
}

