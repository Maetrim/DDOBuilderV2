// BreakdownItemSpellPoints.cpp
//
#include "stdafx.h"
#include "BreakdownItemSpellPoints.h"
#include "BreakdownsPane.h"
#include "Class.h"
#include "GlobalSupportFunctions.h"

BreakdownItemSpellPoints::BreakdownItemSpellPoints(
        CBreakdownsPane* pPane,
        BreakdownType type,
        EffectType effect,
        const CString & title,
        MfcControls::CTreeListCtrl * treeList,
        HTREEITEM hItem) :
    BreakdownItem(type, treeList, hItem),
    m_title(title),
    m_effect(effect)
{
    // register ourselves for effects that affect us
    pPane->RegisterBuildCallbackEffect(effect, this);
}

BreakdownItemSpellPoints::~BreakdownItemSpellPoints()
{
}

// required overrides
CString BreakdownItemSpellPoints::Title() const
{
    return m_title;
}

CString BreakdownItemSpellPoints::Value() const
{
    CString value;
    value.Format(
            "%3d",
            (int)Total());
    return value;
}

void BreakdownItemSpellPoints::CreateOtherEffects()
{
    m_otherEffects.clear();
    // create the class specific spell point contributions
    if (m_pCharacter != NULL)
    {
        Life* pLife = m_pCharacter->ActiveLife();
        Build* pBuild = m_pCharacter->ActiveBuild();
        if (pLife != NULL
            && pBuild != NULL)
        {
            std::vector<size_t> classLevels = pBuild->ClassLevels(pBuild->Level()-1);
            for (size_t ci = 0; ci < classLevels.size(); ++ci)
            {
                // class level spell points
                if (classLevels[ci] > 0) // must have some levels in the class
                {
                    const ::Class& c = ClassFromIndex(ci);
                    //?? should this be based of the breakdown for this class caster level?
                    size_t classSpellPoints = c.SpellPointsAtLevel(classLevels[ci]-1);
                    if (classSpellPoints > 0)
                    {
                        CString bonusName;
                        bonusName.Format("%s(%d) Spell points",
                                c.Name().c_str(),
                                classLevels[ci]);
                        Effect classSps(
                                Effect_Unknown,
                                (LPCSTR)bonusName,
                                (LPCSTR)bonusName,
                                classSpellPoints);
                        AddOtherEffect(classSps);
                        // if we have class spell points we must have bonus class ability spell points
                        // from the DDOWiki:
                        // Your casting ability grants you a number of bonus spell points equal
                        // to (caster level + 9) * (casting ability modifier)
                        AbilityType at = c.ClassCastingStat();
                        BreakdownItem * pBI = FindBreakdown(StatToBreakdown(at));
                        ASSERT(pBI != NULL);
                        pBI->AttachObserver(this);  // need to know about changes to this stat
                        int abilityBonus = BaseStatToBonus(pBI->Total());
                        int abilitySps = (classLevels[ci] + 9) * abilityBonus;
                        if (abilitySps != 0)
                        {
                            bonusName.Format("%s(%s) ability Spell points",
                                    c.Name().c_str(),
                                    (LPCTSTR)EnumEntryText(at, abilityTypeMap));
                            Effect abilitySpBonus(
                                    Effect_Unknown,
                                    (LPCTSTR)bonusName,
                                    (LPCTSTR)bonusName,
                                    abilitySps);
                            AddOtherEffect(abilitySpBonus);
                        }
                    }
                }
            }
        }
   }
}

bool BreakdownItemSpellPoints::AffectsUs(const Effect &) const
{
    return true;
}

void BreakdownItemSpellPoints::ClassChanged(
        Build* pBuild,
        const std::string& classFrom,
        const std::string& classTo,
        size_t level)
{
    BreakdownItem::ClassChanged(pBuild, classFrom, classTo, level);
    // if a class has changed, just recreate our other effects
    // this will update spell point totals
    CreateOtherEffects();
}

void BreakdownItemSpellPoints::UpdateTotalChanged(
        BreakdownItem*,
        BreakdownType)
{
    // just re-create our other effects
    CreateOtherEffects();
}

double BreakdownItemSpellPoints::Multiplier() const
{
    double factor = 1.0;
    // Note that favored souls and sorcerers get up to double spell points
    // from item effects
    Life* pLife = m_pCharacter->ActiveLife();
    Build* pBuild = m_pCharacter->ActiveBuild();
    if (pLife != NULL
        && pBuild != NULL)
    {
        size_t fvsLevels = pBuild->ClassLevels("Favored Soul", pBuild->Level()-1);
        size_t sorcLevels = pBuild->ClassLevels("Sorcerer", pBuild->Level()-1);
        factor = 1.0 + (double)(fvsLevels + sorcLevels) / (double)pBuild->Level();
    }
    return factor;
}

