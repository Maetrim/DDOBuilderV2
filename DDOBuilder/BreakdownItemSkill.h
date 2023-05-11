// BreakdownItemSkill.h
//
#pragma once
#include "BreakdownItem.h"

class CBreakdownsPane;

class BreakdownItemSkill :
        public BreakdownItem
{
    public:
        BreakdownItemSkill(
                CBreakdownsPane* pPane,
                SkillType skill,
                BreakdownType type,
                MfcControls::CTreeListCtrl * treeList,
                HTREEITEM hItem,
                BreakdownItem * pAbility);
        virtual ~BreakdownItemSkill();

        SkillType Skill() const;

        // required overrides
        virtual CString Title() const override;
        virtual CString Value() const override;
        virtual void CreateOtherEffects() override;
        virtual bool AffectsUs(const Effect & effect) const override;
    protected:
        virtual void SkillTomeChanged(Life*, SkillType skill) override;
        virtual void SkillSpendChanged(Build*, size_t level, SkillType skill) override;
        // BreakdownObserver overrides
        virtual void UpdateTotalChanged(BreakdownItem * item, BreakdownType type) override;
    private:
        SkillType m_skill;
        BreakdownItem * m_pAbility;
};
