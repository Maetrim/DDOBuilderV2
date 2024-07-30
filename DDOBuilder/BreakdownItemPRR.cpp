// BreakdownItemPRR.cpp
//
#include "stdafx.h"
#include "BreakdownItemPRR.h"
#include "BreakdownsPane.h"
#include "GlobalSupportFunctions.h"

BreakdownItemPRR::BreakdownItemPRR(
        CBreakdownsPane* pPane,
        BreakdownType type,
        const CString & title,
        MfcControls::CTreeListCtrl * treeList,
        HTREEITEM hItem) :
    BreakdownItem(type, treeList, hItem),
    m_title(title)
{
    // register ourselves for effects that affect us
    pPane->RegisterBuildCallbackEffect(Effect_PRR, this);
}

BreakdownItemPRR::~BreakdownItemPRR()
{
}

// required overrides
CString BreakdownItemPRR::Title() const
{
    return m_title;
}

CString BreakdownItemPRR::Value() const
{
    // Example 133 (57.1%)
    CString value;
    double decrease = 100.0 - (100.0 / (100.0 + Total())) * 100;
    value.Format(
            "%3d (%.1f%%)",
            (int)Total(),
            decrease);
    return value;
}

void BreakdownItemPRR::CreateOtherEffects()
{
    m_otherEffects.clear();
    if (m_pCharacter != NULL)
    {
        Build* pBuild = m_pCharacter->ActiveBuild();
        if (pBuild != NULL)
        {
            // the armor equipped affects the PRR bonus from BAB
            BreakdownItem * pBBAB = FindBreakdown(Breakdown_BAB);
            ASSERT(pBBAB != NULL);
            pBBAB->AttachObserver(this);  // need to know about changes to this effect
            double amount = pBBAB->Total();
            // Armor type           PRR
            // Cloth armor          0
            // Light armor          BAB * 1
            // Medium armor         BAB * 1.5
            // Heavy armor          BAB * 2
            //
            // Docents              PRR 
            // Composite Plating    0
            // Mithral Body         BAB * 1
            // (no medium armor comparable feat)
            // Adamantine Body      BAB * 2
            // note that the Mithral body and Adamantine body feats control the state
            // of the stances, so we only need to check them

            if (pBuild->IsStanceActive("Light Armor")
                && (pBuild->IsFeatTrained("Light Armor Proficiency")
                    || pBuild->IsGrantedFeat("Light Armor Proficiency")))
            {
                Effect prr(
                        Effect_Unknown,
                        "Light Armor PRR (BAB * 1)",
                        "Light Armor PRR (BAB * 1)",
                        amount);
                AddOtherEffect(prr);
            }
            if (pBuild->IsFeatTrained("Mithral Body"))
            {
                Effect prr(
                        Effect_Unknown,
                        "Mithral Body PRR (BAB * 1)",
                        "Mithral Body PRR (BAB * 1)",
                        amount);
                AddOtherEffect(prr);
            }
            if (pBuild->IsStanceActive("Medium Armor")
                    && (pBuild->IsFeatTrained("Medium Armor Proficiency")
                        || pBuild->IsGrantedFeat("Medium Armor Proficiency")))
            {
                Effect prr(
                        Effect_Unknown,
                        "Medium Armor PRR (BAB * 1.5)",
                        "Medium Armor PRR (BAB * 1.5)",
                        (int)(amount * 1.5));       // drop fractions
                AddOtherEffect(prr);
            }
            if (pBuild->IsStanceActive("Heavy Armor")
                && (pBuild->IsFeatTrained("Heavy Armor Proficiency")
                    || pBuild->IsGrantedFeat("Heavy Armor Proficiency")))
            {
                Effect prr(
                        Effect_Unknown,
                        "Heavy Armor PRR (BAB * 2)",
                        "Heavy Armor PRR (BAB * 2)",
                        amount * 2.0);
                AddOtherEffect(prr);
            }
            if (pBuild->IsFeatTrained("Adamantine Body"))
            {
                Effect prr(
                        Effect_Unknown,
                        "Adamantine Body PRR (BAB * 2)",
                        "Adamantine Body PRR (BAB * 2)",
                        amount * 2.0);
                AddOtherEffect(prr);
            }
        }
    }
}

bool BreakdownItemPRR::AffectsUs(const Effect &) const
{
    return true;
}

void BreakdownItemPRR::StanceActivated(
        Build* pBuild,
        const std::string& stanceName)
{
    CreateOtherEffects();
    // and finally call the base class
    BreakdownItem::StanceActivated(pBuild, stanceName);
}

void BreakdownItemPRR::StanceDeactivated(
        Build* pBuild,
        const std::string& stanceName)
{
    CreateOtherEffects();
    // and finally call the base class
    BreakdownItem::StanceDeactivated(pBuild, stanceName);
}

// BreakdownObserver overrides
void BreakdownItemPRR::UpdateTotalChanged(
        BreakdownItem * item,
        BreakdownType type)
{
    // BAB has changed
    CreateOtherEffects();
    // and finally call the base class
    BreakdownItem::UpdateTotalChanged(item, type);
    Populate();
}

void BreakdownItemPRR::FeatTrained(Build* pBuild, const std::string& feat)
{
    CreateOtherEffects();           // armor PRR bonuses may have changed
    // and finally call the base class
    BreakdownItem::FeatTrained(pBuild, feat);
    Populate();
}

void BreakdownItemPRR::FeatRevoked(Build* pBuild, const std::string& feat)
{
    CreateOtherEffects();           // armor PRR bonuses may have changed
    // and finally call the base class
    BreakdownItem::FeatRevoked(pBuild, feat);
    Populate();
}

void BreakdownItemPRR::EnhancementEffectApplied(Build* pBuild, const Effect& effect)
{
    if (effect.IsType(Effect_GrantFeat))
    {
        CreateOtherEffects();           // armor PRR bonuses may have changed
    }
    // and finally call the base class
    BreakdownItem::EnhancementEffectApplied(pBuild, effect);
    Populate();
}

void BreakdownItemPRR::EnhancementEffectRevoked(Build* pBuild, const Effect& effect)
{
    if (effect.IsType(Effect_GrantFeat))
    {
        CreateOtherEffects();           // armor PRR bonuses may have changed
    }
    // and finally call the base class
    BreakdownItem::EnhancementEffectRevoked(pBuild, effect);
    Populate();
}
