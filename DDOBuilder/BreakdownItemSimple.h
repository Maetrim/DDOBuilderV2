// BreakdownItemSimple.h
//
#pragma once
#include "BreakdownItem.h"

class CBreakdownsPane;

class BreakdownItemSimple :
        public BreakdownItem
{
    public:
        BreakdownItemSimple(
                CBreakdownsPane* pPane,
                BreakdownType type,
                EffectType effect,
                const CString & Title,
                MfcControls::CTreeListCtrl * treeList,
                HTREEITEM hItem,
                bool bRegisterCallback = true);
        virtual ~BreakdownItemSimple();

        // required overrides
        virtual CString Title() const override;
        virtual CString Value() const override;
        virtual void CreateOtherEffects() override;
        virtual bool AffectsUs(const Effect & effect) const override;
    private:
        CString m_title;
        EffectType m_effect;
};
