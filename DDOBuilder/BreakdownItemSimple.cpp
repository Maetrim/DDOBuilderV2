// BreakdownItemSimple.cpp
//
#include "stdafx.h"
#include "BreakdownItemSimple.h"
#include "BreakdownsPane.h"

BreakdownItemSimple::BreakdownItemSimple(
        CBreakdownsPane* pPane,
        BreakdownType type,
        EffectType effect,
        const CString & title,
        MfcControls::CTreeListCtrl * treeList,
        HTREEITEM hItem,
        bool bRegisterCallback) :
    BreakdownItem(type, treeList, hItem),
    m_title(title),
    m_effect(effect)
{
    if (bRegisterCallback)
    {
        // register ourselves for effects that affect us
        pPane->RegisterBuildCallbackEffect(effect, this);
    }
}

BreakdownItemSimple::~BreakdownItemSimple()
{
}

// required overrides
CString BreakdownItemSimple::Title() const
{
    return m_title;
}

CString BreakdownItemSimple::Value() const
{
    CString value;
    value.Format(
            "%.2f",
            Total());
    return value;
}

void BreakdownItemSimple::CreateOtherEffects()
{
    // no other effects for this simple item
    m_otherEffects.clear();
}

bool BreakdownItemSimple::AffectsUs(const Effect & effect) const
{
    bool isUs = false;
    if (effect.IsType(m_effect))
    {
        isUs = true;
    }
    return isUs;
}
