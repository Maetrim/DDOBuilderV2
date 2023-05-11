// BreakdownItemBAB.cpp
//
#include "stdafx.h"
#include "BreakdownItemBAB.h"
#include "BreakdownsPane.h"
#include "Class.h"

#include "GlobalSupportFunctions.h"

BreakdownItemBAB::BreakdownItemBAB(
        CBreakdownsPane* pPane,
        MfcControls::CTreeListCtrl * treeList,
        HTREEITEM hItem) :
    BreakdownItem(Breakdown_BAB, treeList, hItem),
    m_overrideBabCount(0)
{
    // register ourselves for effects that affect us
    pPane->RegisterBuildCallbackEffect(Effect_OverrideBAB, this);
}

BreakdownItemBAB::~BreakdownItemBAB()
{
}

// required overrides
CString BreakdownItemBAB::Title() const
{
    CString text;
    text = "Base Attack Bonus";
    return text;
}

CString BreakdownItemBAB::Value() const
{
    CString value;

    value.Format(
            "%4d",
            (int)Total());
    return value;
}

void BreakdownItemBAB::CreateOtherEffects()
{
    // add class hit points
    if (m_pCharacter != NULL)
    {
        m_otherEffects.clear();
        Life* pLife = m_pCharacter->ActiveLife();
        Build* pBuild = m_pCharacter->ActiveBuild();
        if (pLife != NULL
            && pBuild != NULL)
        {
            std::vector<size_t> classLevels = pBuild->ClassLevels(pBuild->Level()-1);
            for (size_t ci = 0; ci < classLevels.size(); ++ci)
            {
                if (classLevels[ci] > 0)
                {
                    const ::Class& c = ClassFromIndex(ci);
                    std::string className = c.Name() + " Levels";
                    double classBab = (int)(c.BAB()[classLevels[ci]-1]); // round down
                    Effect classBonus(
                            Effect_Unknown,
                            className,
                            className,
                            classBab);
                    AddOtherEffect(classBonus);
                }
            }

            //if (m_overrideBabCount > 0)
            //{
            //    // have at least 1 enhancement that boosts BAB to Character level
            //    size_t currentBab = pBuild->BaseAttackBonus(pBuild->Level()-1);
            //    Effect amountTrained(
            //            Effect_Unknown,
            //            "BAB boost to character level",
            //            "BAB boost to character level",
            //            MAX_LEVEL - currentBab - 5);
            //    AddOtherEffect(amountTrained);
            //}
        }
    }
}

bool BreakdownItemBAB::AffectsUs(const Effect &) const
{
    return true;
}

void BreakdownItemBAB::ClassChanged(
        Build * pBuild,
        const std::string& classFrom,
        const std::string& classTo,
        size_t level)
{
    BreakdownItem::ClassChanged(pBuild, classFrom, classTo, level);
    // need to re-create other effects list
    CreateOtherEffects();
    Populate();
}

void BreakdownItemBAB::BuildLevelChanged(Build* pBuild)
{
    BreakdownItem::BuildLevelChanged(pBuild);
    // need to re-create other effects list
    CreateOtherEffects();
    Populate();
}

