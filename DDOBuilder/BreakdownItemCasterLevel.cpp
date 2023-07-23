// BreakdownItemCasterLevel.cpp
//
#include "stdafx.h"
#include "BreakdownItemCasterLevel.h"
#include "BreakdownsPane.h"

#include "GlobalSupportFunctions.h"

BreakdownItemClassCasterLevel::BreakdownItemClassCasterLevel(
        CBreakdownsPane* pPane,
        const std::string& classType,
        BreakdownType type,
        EffectType effectType,
        MfcControls::CTreeListCtrl * treeList,
        HTREEITEM hItem) :
    BreakdownItem(type, treeList, hItem),
    m_class(classType),
    m_effectType(effectType)
{
    // register ourselves for effects that affect us
    pPane->RegisterBuildCallbackEffect(effectType, this);
}

BreakdownItemClassCasterLevel::~BreakdownItemClassCasterLevel()
{
}

// required overrides
CString BreakdownItemClassCasterLevel::Title() const
{
    CString text;
    if (m_effectType == Effect_CasterLevel)
    {
        text.Format("%s Caster level", m_class.c_str());
    }
    else
    {
        text.Format("Max %s Caster level", m_class.c_str());
    }
    return text;
}

CString BreakdownItemClassCasterLevel::Value() const
{
    CString value;
    value.Format(
            "%3d",          // Caster level values are always whole numbers
            (int)Total());
    return value;
}

void BreakdownItemClassCasterLevel::CreateOtherEffects()
{
    if (m_pCharacter != NULL)
    {
        m_otherEffects.clear();
        Life* pLife = m_pCharacter->ActiveLife();
        Build* pBuild = m_pCharacter->ActiveBuild();
        if (pLife != NULL
            && pBuild != NULL)
        {
            size_t classLevels = pBuild->ClassLevels(m_class, pBuild->Level()-1);
            if (classLevels != 0)
            {
                CString text;
                text.Format("%s(%d)",
                        m_class.c_str(),
                        classLevels);
                Effect levels(
                        Effect_Unknown,
                        (LPCTSTR)text,
                        (LPCTSTR)text,
                        classLevels);
                AddOtherEffect(levels);
            }
        }
    }
}

bool BreakdownItemClassCasterLevel::AffectsUs(const Effect & effect) const
{
    bool isUs = false;
    for (auto&& iit : effect.Item())
    {
        if (iit == m_class
            || iit == "All")
        {
            isUs |= true;
        }
    }
    return isUs;
}

void BreakdownItemClassCasterLevel::ClassChanged(
        Build* pBuild,
        const std::string& classFrom,
        const std::string& classTo,
        size_t level)
{
    BreakdownItem::ClassChanged(pBuild, classFrom, classTo, level);
    if (classFrom == m_class
            || classTo == m_class)
    {
        CreateOtherEffects();
        Populate();
    }
}

void BreakdownItemClassCasterLevel::BuildLevelChanged(Build* pBuild)
{
    CreateOtherEffects();
    BreakdownItem::BuildLevelChanged(pBuild);
}
