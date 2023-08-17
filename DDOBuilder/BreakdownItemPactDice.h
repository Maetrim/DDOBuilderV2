// BreakdownItemPactDice.h
//
#pragma once
#include "BreakdownItem.h"

class CBreakdownsPane;

class BreakdownItemPactDice :
        public BreakdownItem
{
    public:
        BreakdownItemPactDice(
                CBreakdownsPane* pPane,
                BreakdownType type,
                EffectType effect,
                const CString& Title,
                MfcControls::CTreeListCtrl * treeList,
                HTREEITEM hItem,
                size_t diceSize);
        virtual ~BreakdownItemPactDice();

        // required overrides
        virtual CString Title() const override;
        virtual CString Value() const override;
        virtual void CreateOtherEffects() override;
        virtual bool AffectsUs(const Effect & effect) const override;
    private:
        CString m_title;
        EffectType m_effect;
        size_t m_diceSize;
};
