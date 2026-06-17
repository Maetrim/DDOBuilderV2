// BreakdownItemMRR.h
//
#pragma once
#include "BreakdownItem.h"

class CBreakdownsPane;

class BreakdownItemMRR :
        public BreakdownItem
{
    public:
        BreakdownItemMRR(
            CBreakdownsPane* pPane,
            BreakdownType type,
                const CString& Title,
                MfcControls::CTreeListCtrl* treeList,
                HTREEITEM hItem);
        virtual ~BreakdownItemMRR();

        // required overrides
        virtual CString Title() const override;
        virtual CString Value() const override;
        virtual void CreateOtherEffects() override;
        virtual bool AffectsUs(const Effect& effect) const override;
    private:
        // BreakdownObserver overrides
        virtual void UpdateTotalChanged(BreakdownItem* item, BreakdownType type) override;
        CString m_title;
};
