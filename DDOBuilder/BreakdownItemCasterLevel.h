// BreakdownItemClassCasterLevel.h
//
// A standard base class that all items that can have breakdowns calculated
// for them inherit from to allow a common interface to the CBreakdownsView
#pragma once

#include "BreakdownItem.h"

class CBreakdownsPane;

class BreakdownItemClassCasterLevel :
        public BreakdownItem
{
    public:
        BreakdownItemClassCasterLevel(
                CBreakdownsPane* pPane,
                const std::string& classType,
                BreakdownType type,
                EffectType effect,
                MfcControls::CTreeListCtrl * treeList,
                HTREEITEM hItem);
        virtual ~BreakdownItemClassCasterLevel();

        // required overrides
        virtual CString Title() const override;
        virtual CString Value() const override;
        virtual void CreateOtherEffects() override;
        virtual bool AffectsUs(const Effect & effect) const override;
    protected:
        virtual void ClassChanged(Build *, const std::string& classFrom, const std::string& classTo, size_t level) override;
        virtual void BuildLevelChanged(Build*) override;
private:
        std::string m_class;
        EffectType m_effectType;
};
