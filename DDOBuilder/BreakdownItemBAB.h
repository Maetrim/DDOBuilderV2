// BreakdownItemBAB.h
//
#pragma once
#include "BreakdownItem.h"

class CBreakdownsPane;

class BreakdownItemBAB :
        public BreakdownItem
{
    public:
        BreakdownItemBAB(
                CBreakdownsPane* pPane,
                MfcControls::CTreeListCtrl * treeList,
                HTREEITEM hItem);
        virtual ~BreakdownItemBAB();

        // required overrides
        virtual CString Title() const override;
        virtual CString Value() const override;
        virtual void CreateOtherEffects() override;
        virtual bool AffectsUs(const Effect & effect) const override;
    protected:
        // BuildObserver overrides
        virtual void ClassChanged(Build *, const std::string& classFrom, const std::string& classTo, size_t level) override;
        virtual void BuildLevelChanged(Build* pBuild) override;

        size_t m_overrideBabCount;
};
