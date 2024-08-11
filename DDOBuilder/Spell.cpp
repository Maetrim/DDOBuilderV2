// Spell.cpp
//
#include "StdAfx.h"
#include "Spell.h"
#include "XmlLib\SaxWriter.h"
#include "GlobalSupportFunctions.h"
#include "LogPane.h"
#include "Build.h"
#include "ClassSpell.h"
#include "Class.h"
#include "BreakdownItem.h"
//#include "BreakdownItemAbility.h"
//#include "BreakdownItemSpellSchool.h"

#define DL_ELEMENT Spell

namespace
{
    const wchar_t f_saxElementName[] = L"Spell";
    DL_DEFINE_NAMES(Spell_PROPERTIES)

    const unsigned f_verCurrent = 1;
}

Spell::Spell() :
    XmlLib::SaxContentElement(f_saxElementName, f_verCurrent),
    m_iconIndex(0)
{
    DL_INIT(Spell_PROPERTIES)
}

DL_DEFINE_ACCESS(Spell_PROPERTIES)

XmlLib::SaxContentElementInterface * Spell::StartElement(
        const XmlLib::SaxString & name,
        const XmlLib::SaxAttributes & attributes)
{
    XmlLib::SaxContentElementInterface * subHandler =
            SaxContentElement::StartElement(name, attributes);

    DL_START(Spell_PROPERTIES)

    return subHandler;
}

void Spell::EndElement()
{
    SaxContentElement::EndElement();
    DL_END(Spell_PROPERTIES)
}

void Spell::Write(XmlLib::SaxWriter * writer) const
{
    writer->StartElement(ElementName());//, VersionAttributes());
    DL_WRITE(Spell_PROPERTIES)
    writer->EndElement();
}

bool Spell::operator<(const Spell& other) const
{
    // (assumes all spell names are unique)
    // sort by name
    return (Name() < other.Name());
}

size_t Spell::MetamagicCount() const
{
    size_t count = 0;
    if (HasAccelerate()) count++;
    if (HasEmbolden()) count++;
    if (HasEmpower()) count++;
    if (HasEmpowerHealing()) count++;
    if (HasEnlarge()) count++;
    if (HasExtend()) count++;
    if (HasHeighten()) count++;
    if (HasIntensify()) count++;
    if (HasMaximize()) count++;
    if (HasQuicken()) count++;
    return count;
}

void Spell::VerifyObject() const
{
    bool ok = true;
    std::stringstream ss;
    ss << "=====" << m_Name << "=====\n";
    if (!ImageFileExists("DataFiles\\SpellImages\\", Icon()))
    {
        ss << "Spell is missing image file \"" << Icon() << "\"\n";
        ok = false;
    }
    // check the spell effects also
    for (auto&& eit: m_Effects)
    {
        ok &= eit.VerifyObject(&ss);
    }
    // check the spell damage effects also
    for (auto&& sdit : m_SpellDamageEffects)
    {
        ok &= sdit.VerifyObject(&ss, HasMaxCasterLevel());
    }
    // check the Dcs also
    for (auto&& dcit: m_DCs)
    {
        ok &= dcit.VerifyObject(&ss);
    }
    if (!ok)
    {
        GetLog().AddLogEntry(ss.str().c_str());
    }
}

size_t Spell::DC(
        const Build& build) const
{
    size_t dc = 0;
    if (DCs().size() > 0)
    {
        dc = DCs().front().CalculateSpellDC(&build, *this);
    }
    return dc;
}

std::list<Effect> Spell::UpdatedEffects(size_t castingLevel) const
{
    UNREFERENCED_PARAMETER(castingLevel);
    // create a copy of the spell effects with the amount field dependent on the
    // the caster level in use.
    std::list<Effect> effects = m_Effects;
    // look at each effect and update if required
    //for (size_t i = 0; i < effects.size(); ++i)
    //{
    //    if (effects[i].HasAmount())
    //    {
    //        // look up the amount to use for this casting level
    //        // note that its ok for a caster level to be larger than the
    //        // vector supplied, in such cases we just use the value from the last element
    //        std::vector<double> amounts = effects[i].Amount();
    //        castingLevel = min(castingLevel, amounts.size() - 1);   // limit range
    //        effects[i].Set_Amount(amounts[castingLevel]);
    //        effects[i].Clear_AmountVector();    // no longer has amount vector
    //    }
    //}
    return effects;
}

void Spell::UpdateSpell(const ClassSpell& cs, const std::string& ct, int spellLevel)
{
    SetClass(ct);
    Set_Level(cs.Level());
    if (cs.HasCost()) Set_Cost(cs.Cost()); else Set_Cost(5 * spellLevel);
    if (cs.HasMaxCasterLevel()) Set_MaxCasterLevel(cs.MaxCasterLevel());
}

void Spell::UpdateSpell(const FixedSpell& fs, const std::string& ct)
{
    SetClass(ct);
    Set_Level(fs.Level());
    Set_Cost(fs.Cost());
    if (fs.MaxCasterLevel() >= 0) Set_MaxCasterLevel(fs.MaxCasterLevel());
}

const std::string& Spell::Class() const
{
    return m_class;
}

void Spell::SetClass(const std::string& ct)
{
    m_class = ct;
}

int Spell::ActualCasterLevel(const Build& build, const SpellDamage& sd) const
{
    UNREFERENCED_PARAMETER(sd);
    // work out the actual caster level
    size_t classCasterLevel = 0;
    BreakdownItem* pBI = NULL;
    pBI = CasterLevelBreakdown(Class());
    if (pBI != NULL)
    {
        classCasterLevel += static_cast<int>(pBI->Total());
    }
    // include any school caster level bonus
    if (HasSchool())
    {
        pBI = FindBreakdown(CasterLevelSchoolToBreakdown(School()));
        if (pBI != NULL)
        {
            classCasterLevel += static_cast<int>(pBI->Total());
        }
    }
    int bonus = build.BonusCasterLevels(Name());
    classCasterLevel += bonus;
    return classCasterLevel;
}

int Spell::ActualMaxCasterLevel(const Build& build, const SpellDamage& sd) const
{
    UNREFERENCED_PARAMETER(sd);
    int maxCasterLevel = 0;
    BreakdownItem* pBI = NULL;
    if (HasMaxCasterLevel())
    {
        maxCasterLevel = MaxCasterLevel();
        // also need to add any options that increase the max caster level here
        // any school level bonuses
        const ::Class& c = FindClass(Class());
        pBI = FindBreakdown(static_cast<BreakdownType>(Breakdown_MaxCasterLevel_First + c.Index()));
        if (pBI != NULL)
        {
            int classCasterLevelBonus = static_cast<int>(pBI->Total());
            maxCasterLevel += classCasterLevelBonus;
        }
        if (HasSchool())
        {
            pBI = FindBreakdown(MaxCasterLevelSchoolToBreakdown(School()));
            if (pBI != NULL)
            {
                maxCasterLevel += static_cast<int>(pBI->Total());
            }
        }
        int bonus = build.BonusMaxCasterLevels(Name());
        maxCasterLevel += bonus;
    }
    return maxCasterLevel;
}

CString Spell::ActualCasterLevelText(const Build& build, const SpellDamage& sd) const
{
    // work out the actual caster level
    CString totalText;
    CString t;
    int classCasterLevel = 0;
    int schoolCasterLevel = 0;
    BreakdownItem* pBI = CasterLevelBreakdown(Class());
    if (pBI != NULL)
    {
        classCasterLevel = static_cast<int>(pBI->Total());
        totalText.Format("Class Caster Level (%d)", classCasterLevel);
    }
    // include any school caster level bonus
    if (HasSchool())
    {
        pBI = FindBreakdown(CasterLevelSchoolToBreakdown(School()));
        if (pBI != NULL)
        {
            schoolCasterLevel = static_cast<int>(pBI->Total());
        }
        if (schoolCasterLevel != 0)
        {
            t.Format(" + School Bonus(%+d)", schoolCasterLevel);
            totalText += t;
            classCasterLevel += schoolCasterLevel;
        }
    }
    int bonus = build.BonusCasterLevels(Name());
    if (bonus != 0)
    {
        t.Format(" + Spell Specific Bonus(%+d)", bonus);
        totalText += t;
        classCasterLevel += bonus;
    }

    int maxCasterLevel = ActualMaxCasterLevel(build, sd);
    if (classCasterLevel > maxCasterLevel)
    {
        t.Format(" = %d (Capped from %d)\r\n", maxCasterLevel, classCasterLevel);
    }
    else
    {
        t.Format(" = %d\r\n", classCasterLevel);
    }
    totalText += t;
    return totalText;
}

CString Spell::ActualMaxCasterLevelText(const Build& build, const SpellDamage& sd) const
{
    UNREFERENCED_PARAMETER(sd);
    CString totalText;
    int maxCasterLevel = 0;
    BreakdownItem* pBI = NULL;
    CString t;
    if (HasMaxCasterLevel())
    {
        int schoolLevelBonus = 0;
        CString mcl;
        maxCasterLevel = MaxCasterLevel();

        mcl.Format("Max Caster Level(%d)", maxCasterLevel);

        const ::Class& c = FindClass(Class());
        pBI = FindBreakdown(static_cast<BreakdownType>(Breakdown_MaxCasterLevel_First + c.Index()));
        if (pBI != NULL)
        {
            int classCasterLevelBonus = static_cast<int>(pBI->Total());
            if (classCasterLevelBonus != 0)
            {
                t.Format(" + Epic MCL Bonus(%+d)", classCasterLevelBonus);
                mcl += t;
            }
            maxCasterLevel += classCasterLevelBonus;
        }
        // also need to add any options that increase the max caster level here
        // any school level bonuses
        if (HasSchool())
        {
            pBI = FindBreakdown(MaxCasterLevelSchoolToBreakdown(School()));
            if (pBI != NULL)
            {
                schoolLevelBonus = static_cast<int>(pBI->Total());
                if (schoolLevelBonus != 0)
                {
                    t.Format(" + School Bonus(%+d)", schoolLevelBonus);
                    mcl += t;
                }
            }
            maxCasterLevel += schoolLevelBonus;
        }
        int bonus = build.BonusMaxCasterLevels(Name());
        if (bonus != 0)
        {
            t.Format(" + Spell Specific Bonus(%+d)", bonus);
            mcl += t;
            maxCasterLevel += bonus;
        }
        t.Format(" = %d", maxCasterLevel);
        mcl += t;
        totalText += mcl;
    }
    return totalText;
}

void Spell::SetCost(int cost)
{
    Set_Cost(cost);
}

int Spell::TotalCost(const Build& build) const
{
    // determine the total cost to cast this spell based on what metamagics are
    // currently enabled. Note that SLAs get metamagics at no additional cost
    int cost = Cost();              // base cost
    // check each possible metamagic to see if it is active and add the cost is required
    if (HasAccelerate() && build.IsStanceActive("Accelerate Spell"))
    {
        BreakdownItem *pBI = FindBreakdown(Breakdown_Accelerate);
        if (pBI != NULL)
        {
            cost += pBI->Total();
        }
    }
    if (HasEmbolden() && build.IsStanceActive("Embolden Spell"))
    {
        BreakdownItem* pBI = FindBreakdown(Breakdown_Embolden);
        if (pBI != NULL)
        {
            cost += pBI->Total();
        }
    }
    if (HasEmpower() && build.IsStanceActive("Empower Spell"))
    {
        BreakdownItem* pBI = FindBreakdown(Breakdown_Empower);
        if (pBI != NULL)
        {
            cost += pBI->Total();
        }
    }
    if (HasEmpowerHealing() && build.IsStanceActive("Empower Healing Spell"))
    {
        BreakdownItem* pBI = FindBreakdown(Breakdown_EmpowerHealing);
        if (pBI != NULL)
        {
            cost += pBI->Total();
        }
    }
    if (HasEnlarge() && build.IsStanceActive("Enlarge Spell"))
    {
        BreakdownItem* pBI = FindBreakdown(Breakdown_Enlarge);
        if (pBI != NULL)
        {
            cost += pBI->Total();
        }
    }
    if (HasExtend() && build.IsStanceActive("Extend Spell"))
    {
        BreakdownItem* pBI = FindBreakdown(Breakdown_Extend);
        if (pBI != NULL)
        {
            cost += pBI->Total();
        }
    }
    if (HasHeighten() && build.IsStanceActive("Heighten Spell"))
    {
        BreakdownItem* pBI = FindBreakdown(Breakdown_Heighten);
        if (pBI != NULL)
        {
            // Special: This is amount per spell level of heightening
            const ::Class& c = FindClass(Class());
            size_t maxLevel = c.MaxSpellLevel(build.Level());
            size_t heightenedLevels = maxLevel - Level();
            if (heightenedLevels > 0)
            {
                cost += (pBI->Total() * heightenedLevels);
            }
        }
    }
    if (HasIntensify() && build.IsStanceActive("Intensify Spell"))
    {
        BreakdownItem* pBI = FindBreakdown(Breakdown_Intensify);
        if (pBI != NULL)
        {
            cost += pBI->Total();
        }
    }
    if (HasMaximize() && build.IsStanceActive("Maximize Spell"))
    {
        BreakdownItem* pBI = FindBreakdown(Breakdown_Maximize);
        if (pBI != NULL)
        {
            cost += pBI->Total();
        }
    }
    if (HasQuicken() && build.IsStanceActive("Quicken Spell"))
    {
        BreakdownItem* pBI = FindBreakdown(Breakdown_Quicken);
        if (pBI != NULL)
        {
            cost += pBI->Total();
        }
    }
    return cost;
}

void Spell::SetIconIndex(size_t index)
{
    m_iconIndex = index;
}

size_t Spell::IconIndex() const
{
    return m_iconIndex;
}
