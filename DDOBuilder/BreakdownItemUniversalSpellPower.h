// BreakdownItemUniversalSpellPower.h
//
#pragma once
#include "BreakdownItemSimple.h"

class CBreakdownsPane;

class BreakdownItemUniversalSpellPower :
        public BreakdownItemSimple
{
    public:
        BreakdownItemUniversalSpellPower(
                CBreakdownsPane * pPane,
                BreakdownType type,
                const CString & Title,
                MfcControls::CTreeListCtrl * treeList,
                HTREEITEM hItem);
        virtual ~BreakdownItemUniversalSpellPower();

        // required overrides
        virtual void CreateOtherEffects() override;

        // BreakdownObserver overrides
        virtual void UpdateTotalChanged(BreakdownItem * item, BreakdownType type) override;
        virtual void EnhancementTrained(Build*, const EnhancementItemParams& item) override;
        virtual void EnhancementRevoked(Build*, const EnhancementItemParams& item) override;
        virtual void StanceActivated(Build* pBuild, const std::string& stanceName) override;
        virtual void StanceDeactivated(Build* pBuild, const std::string& stanceName) override;
        virtual void GearChanged(Build* pBuild, InventorySlotType slot) override;
};
