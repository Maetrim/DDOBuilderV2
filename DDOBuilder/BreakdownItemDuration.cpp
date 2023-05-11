// BreakdownItemDuration.cpp
//
#include "stdafx.h"
#include "BreakdownItemDuration.h"
#include "BreakdownsPane.h"

BreakdownItemDuration::BreakdownItemDuration(
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
    pPane->RegisterBuildCallbackEffect(m_effect, this);
}

BreakdownItemDuration::~BreakdownItemDuration()
{
}

// required overrides
CString BreakdownItemDuration::Title() const
{
    return m_title;
}

CString BreakdownItemDuration::Value() const
{
    CString value;
    int total = (int)Total(); // this is in seconds
    int seconds = total % 60;
    total /= 60;
    int minutes = total % 60;
    total /= 60;
    int hours = total;
    if (hours > 0)
    {
        // just hours:minutes:seconds
        value.Format(
                "%d:%02d:%02d",
                hours,
                minutes,
                seconds);
    }
    else if (minutes > 0)
    {
        // just minutes:seconds
        value.Format(
                "%d:%02d",
                minutes,
                seconds);
    }
    else
    {
        // just seconds
        value.Format(
                "%d Seconds",
                seconds);
    }
    return value;
}

void BreakdownItemDuration::CreateOtherEffects()
{
    // no other effects for this simple item
    m_otherEffects.clear();
}

bool BreakdownItemDuration::AffectsUs(const Effect &) const
{
    return true;
}
