// BreakdownItemHitpoints.h
//
#pragma once
#include "BreakdownItem.h"

class CBreakdownsPane;

class BreakdownItemHitpoints :
        public BreakdownItem
{
    public:
        BreakdownItemHitpoints(
                CBreakdownsPane * pPane,
                MfcControls::CTreeListCtrl * treeList,
                HTREEITEM hItem,
                BreakdownItem * pCon);
        virtual ~BreakdownItemHitpoints();

        // required overrides
        virtual CString Title() const override;
        virtual CString Value() const override;
        virtual void CreateOtherEffects() override;
        virtual bool AffectsUs(const Effect & effect) const override;
    protected:
        // EffectCallbackItem overrides
        virtual void BuildLevelChanged(Build*) override;
        virtual void ClassChanged(Build*, const std::string& classFrom, const std::string& classTo, size_t level) override;

        // BreakdownObserver overrides
        virtual void UpdateTotalChanged(BreakdownItem * item, BreakdownType type) override;
    private:
        BreakdownItem * m_pConstitutionBreakdown;
};
