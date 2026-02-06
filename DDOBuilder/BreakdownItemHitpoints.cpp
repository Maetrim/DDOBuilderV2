// BreakdownItemHitpoints.cpp
//
#include "stdafx.h"
#include "BreakdownItemHitpoints.h"
#include "BreakdownsPane.h"
#include "Class.h"

#include "GlobalSupportFunctions.h"

BreakdownItemHitpoints::BreakdownItemHitpoints(
        CBreakdownsPane * pPane,
        MfcControls::CTreeListCtrl * treeList,
        HTREEITEM hItem,
        BreakdownItem * pCon) :
    BreakdownItem(Breakdown_Hitpoints, treeList, hItem),
    m_pConstitutionBreakdown(pCon)
{
    // register ourselves for effects that affect us
    pPane->RegisterBuildCallbackEffect(Effect_Hitpoints, this);
    // need to know when total constitution changes
    m_pConstitutionBreakdown->AttachObserver(this);
}

BreakdownItemHitpoints::~BreakdownItemHitpoints()
{
}

// required overrides
CString BreakdownItemHitpoints::Title() const
{
    CString text;
    text = "Hitpoints";
    return text;
}

CString BreakdownItemHitpoints::Value() const
{
    CString value;

    value.Format(
            "%4d",
            (int)Total());
    return value;
}

void BreakdownItemHitpoints::CreateOtherEffects()
{
    // add class hit points
    if (m_pCharacter != NULL)
    {
        size_t classHitpoints = 0;      // needed for style bonus
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
                    const ::Class & c = ClassFromIndex(ci);
                    std::stringstream ss;
                    ss << c.Name() << " " << classLevels[ci] << " Levels";
                    std::string className = ss.str();
                    Effect classBonus(
                            Effect_Unknown,
                            className,
                            className,
                            static_cast<double>(classLevels[ci]) * c.HitPoints());
                    AddOtherEffect(classBonus);
                    if (c.Name() != "Epic" && c.Name() != "Legendary")
                    {
                        // full bonus
                        classHitpoints += c.HitPoints() * classLevels[ci];
                    }
                    else
                    {
                        // epic / legendary give 50% bonus
                        classHitpoints += c.HitPoints() * classLevels[ci] / 2;
                    }
                }
            }

            // hp bonus due to fate points (active at level 20+)
            if (pBuild->Level() >= MAX_CLASS_LEVEL)
            {
                BreakdownItem* pBD = FindBreakdown(Breakdown_FatePoints);
                if (pBD != NULL)
                {
                    pBD->AttachObserver(this); // need to know about changes
                    int fatePoints = static_cast<int>(pBD->Total());
                    if (fatePoints != 0)
                    {
                        Effect fateBonus(
                            Effect_Unknown,
                            "Fate Points bonus",
                            "Fate Points bonus",
                            static_cast<double>(2.0 * fatePoints));          // 2hp per fate point
                        AddOtherEffect(fateBonus);
                    }
                }
            }

            // hp penalty due to negative levels
            BreakdownItem* pBD = FindBreakdown(Breakdown_NegativeLevels);
            if (pBD != NULL)
            {
                pBD->AttachObserver(this); // need to know about changes
                int negLevels = static_cast<int>(pBD->Total());
                if (negLevels != 0)
                {
                    Effect negLevelsEffect(
                        Effect_Unknown,
                        "Negative Levels",
                        "Negative Levels",
                        static_cast<double>(-5.0 * negLevels));          // -5hp per neg level
                    AddOtherEffect(negLevelsEffect);
                }
            }

            // bonus HP due to constitution
            // this has to be based on the final constitution value from the breakdown
            double con = m_pConstitutionBreakdown->Total();
            double bonus = BaseStatToBonus(con);
            if (bonus != 0)
            {
                Effect conBonus(
                        Effect_AbilityBonus,
                        "Constitution bonus",
                        "Constitution bonus",
                        bonus * static_cast<int>(pBuild->Level()));
                AddOtherEffect(conBonus);
            }

            // fighting style bonus
            BreakdownItem* pBreakdown = FindBreakdown(Breakdown_StyleBonusFeats);
            pBreakdown->AttachObserver(this);
            bonus = pBreakdown->Total();
            if (bonus > 0)
            {
                // 25% per style feat, max 100%
                bonus = 0.25 * min(4, bonus) * classHitpoints;
                Effect styleBonus(
                    Effect_HitpointsStyleBonus,
                    "Combat Style",
                    "Combat Style",
                    static_cast<int>(bonus));
                AddOtherEffect(styleBonus);
            }

            // add the false life bonus
            pBreakdown = FindBreakdown(Breakdown_FalseLife);
            pBreakdown->AttachObserver(this);
            bonus = pBreakdown->Total();
            if (bonus != 0)
            {
                Effect falseLifeBonus(
                    Effect_FalseLife,
                    "False Life",
                    "False Life",
                    static_cast<int>(bonus));
                AddOtherEffect(falseLifeBonus);
            }

            // add reaper hitpoints which are level gated
            pBreakdown = FindBreakdown(Breakdown_ReaperHitpoints);
            pBreakdown->AttachObserver(this);
            bonus = pBreakdown->Total();
            if (bonus != 0)
            {
                size_t level = m_pCharacter->ActiveBuild()->Level() + 1;    // 1 based
                double hpCap = 0;
                if (level <= 5) hpCap = 50;
                else if (level <= 10) hpCap = 100;
                else if (level <= 15) hpCap = 200;
                else if (level <= 20) hpCap = 400;
                else if (level <= 25) hpCap = 800;
                else hpCap = 1e99;
                bonus = min(bonus, hpCap);

                Effect reaperBonus(
                    Effect_FalseLife,
                    "Reaper Bonus",
                    "Reaper Bonus",
                    static_cast<int>(bonus));
                Requirements req;
                Requirement reqStance(Requirement_Stance, "Reaper");
                req.AddRequirement(reqStance);
                reaperBonus.SetRequirements(req);
                AddOtherEffect(reaperBonus);
            }
        }
    }
}

bool BreakdownItemHitpoints::AffectsUs(const Effect &) const
{
    return true;
}

void BreakdownItemHitpoints::BuildLevelChanged(Build* pBuild)
{
    BreakdownItem::BuildLevelChanged(pBuild);
    // need to re-create other effects list (class hp)
    CreateOtherEffects();
    Populate();
}

void BreakdownItemHitpoints::ClassChanged(
        Build * pBuild,
        const std::string& classFrom,
        const std::string& classTo,
        size_t level)
{
    BreakdownItem::ClassChanged(pBuild, classFrom, classTo, level);
    // need to re-create other effects list (class hp)
    CreateOtherEffects();
    Populate();
}

void BreakdownItemHitpoints::UpdateTotalChanged(
        BreakdownItem * item,
        BreakdownType type)
{
    // total constitution has changed, update
    CreateOtherEffects();
    // do base class stuff also
    BreakdownItem::UpdateTotalChanged(item, type);
}
