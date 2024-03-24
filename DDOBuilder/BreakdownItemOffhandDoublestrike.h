// BreakdownItemOffhandDoublestrike.h
//
#pragma once
#include "BreakdownItem.h"

class CBreakdownsPane;

class BreakdownItemOffhandDoublestrike :
        public BreakdownItem
{
    public:
        BreakdownItemOffhandDoublestrike(
                CBreakdownsPane* pPane,
                BreakdownType type,
                const CString & Title,
                MfcControls::CTreeListCtrl * treeList,
                HTREEITEM hItem);
        virtual ~BreakdownItemOffhandDoublestrike();

        // required overrides
        virtual CString Title() const override;
        virtual CString Value() const override;
        virtual void CreateOtherEffects() override;
        virtual bool AffectsUs(const Effect & effect) const override;
        // BreakdownObserver overrides
        virtual void UpdateTotalChanged(BreakdownItem* item, BreakdownType type) override;

        // EffectCallbackItem
        virtual void FeatTrained(Build*, const std::string&) override;
        virtual void FeatRevoked(Build*, const std::string&) override;
    private:
        CString m_title;
};
