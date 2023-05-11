// BreakdownItemSchoolCasterLevel.h
//
// A standard base class that all items that can have breakdowns calculated
// for them inherit from to allow a common interface to the CBreakdownsView
#pragma once

#include "BreakdownItem.h"
#include "SpellSchoolTypes.h"
class CBreakdownsPane;

class BreakdownItemSchoolCasterLevel :
        public BreakdownItem
{
    public:
        BreakdownItemSchoolCasterLevel(
                CBreakdownsPane* pPane,
                EffectType et,
                SpellSchoolType schoolType,
                BreakdownType type,
                MfcControls::CTreeListCtrl * treeList,
                HTREEITEM hItem);
        virtual ~BreakdownItemSchoolCasterLevel();

        // required overrides
        virtual CString Title() const override;
        virtual CString Value() const override;
        virtual void CreateOtherEffects() override;
        virtual bool AffectsUs(const Effect & effect) const override;

    private:
        SpellSchoolType m_school;
};
