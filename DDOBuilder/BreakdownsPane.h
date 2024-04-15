// BreakdownsPane.h
//
#pragma once
#include "Resource.h"

#include "BreakdownTypes.h"
#include "BreakdownItem.h"
#include "Build.h"
#include "Character.h"
#include "EnergyTypes.h"
#include "Life.h"
#include "SpellSchoolTypes.h"
#include "TacticalTypes.h"
#include "TreeListCtrl.h"
#include "GroupLine.h"
#include <map>

class BreakdownItemWeaponEffects;

class CBreakdownsPane :
    public CFormView,
    public CharacterObserver,
    public LifeObserver,
    public BuildObserver,
    BreakdownObserver
{
    public:
        enum { IDD = IDD_BREAKDOWNS_PANE };
        DECLARE_DYNCREATE(CBreakdownsPane)

        virtual void OnInitialUpdate();
        BreakdownItem * FindBreakdown(BreakdownType type) const;
        void RegisterBuildCallbackEffect(EffectType type, EffectCallbackItem* pItem);
        void UpdateBreakdown();

    protected:
        CBreakdownsPane();           // protected constructor used by dynamic creation
        virtual ~CBreakdownsPane();

        virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
        afx_msg void OnSize(UINT nType, int cx, int cy);
        afx_msg LRESULT OnNewDocument(WPARAM wParam, LPARAM lParam);
        afx_msg LRESULT OnLoadComplete(WPARAM, LPARAM);
        afx_msg void OnSelChangedBreakdownTree(NMHDR* pNMHDR, LRESULT* pResult);
        afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
        afx_msg void OnDividerClicked();
        afx_msg void OnMouseMove(UINT nFlags, CPoint point);
        afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
        afx_msg void OnEndtrackBreakdownList(NMHDR* pNMHDR, LRESULT* pResult);
        afx_msg void OnButtonClipboardCopy();
        DECLARE_MESSAGE_MAP()
    private:
        // Character Observer overrides
        virtual void UpdateActiveLifeChanged(Character*) override;
        virtual void UpdateActiveBuildChanged(Character*) override;

        // LifeObserver overrides
        virtual void UpdateRaceChanged(Life*, const std::string&) override;
        virtual void UpdateAbilityTomeChanged(Life*, AbilityType) override;
        virtual void UpdateSkillTomeChanged(Life*, SkillType skill) override;

        // BuildObserver overrides
        virtual void UpdateBuildLevelChanged(Build*) override;
        virtual void UpdateClassChanged(Build*, const std::string& classFrom, const std::string& classTo, size_t level) override;
        virtual void UpdateAbilityValueChanged(Build*, AbilityType) override;
        virtual void UpdateFeatEffectApplied(Build*, const Effect& effect) override;
        virtual void UpdateFeatEffectRevoked(Build*, const Effect& effect) override;
        virtual void UpdateEnhancementTrained(Build*, const EnhancementItemParams& item) override;
        virtual void UpdateEnhancementRevoked(Build*, const EnhancementItemParams& item) override;
        virtual void UpdateEnhancementEffectApplied(Build*, const Effect& effect) override;
        virtual void UpdateEnhancementEffectRevoked(Build*, const Effect& effect) override;
        virtual void UpdateFeatTrained(Build*, const std::string& featName) override;
        virtual void UpdateFeatRevoked(Build*, const std::string& featName) override;
        virtual void UpdateSkillSpendChanged(Build*, size_t level, SkillType skill) override;
        virtual void UpdateStanceActivated(Build*, const std::string& stanceName) override;
        virtual void UpdateStanceDeactivated(Build*, const std::string& stanceName) override;
        virtual void UpdateSliderChanged(Build*, const std::string& sliderName, int newValue) override;
        virtual void UpdateItemEffectApplied(Build*, const Effect& effect) override;
        virtual void UpdateItemEffectRevoked(Build*, const Effect& effect) override;
        virtual void UpdateItemWeaponEffectApplied(Build*, const Effect& effect, WeaponType wt, InventorySlotType ist) override;
        virtual void UpdateItemWeaponEffectRevoked(Build*, const Effect& effect, WeaponType wt, InventorySlotType ist) override;
        virtual void UpdateGearChanged(Build*, InventorySlotType slot) override;

        // BreakdownObserver overrides
        virtual void UpdateTotalChanged(BreakdownItem *, BreakdownType) override;

        void CreateBreakdowns();
        void CreateAbilityBreakdowns();
        void CreateSavesBreakdowns();
        void CreateSkillBreakdowns();
        void CreatePhysicalBreakdowns();
        void CreateMagicalBreakdowns();
        void CreateSongBreakdowns();
        void CreateTurnUndeadBreakdowns();
        void CreateEnergyResistancesBreakdowns();
        void CreateWeaponBreakdowns();
        void CreateHirelingBreakdowns();

        void AddKiBreakdowns(HTREEITEM hParent);
        void AddClassCasterLevels(HTREEITEM hParent);
        void AddEnergyCasterLevels(HTREEITEM hParent);
        void AddEnergyCasterLevels(
                HTREEITEM hParent,
                BreakdownType ecl,
                BreakdownType emcl,
                EnergyType et);
        void AddSchoolCasterLevels(HTREEITEM hParent);
        void AddSchoolCasterLevels(
            HTREEITEM hParent,
            BreakdownType scl,
            BreakdownType smcl,
            SpellSchoolType st);
        void AddTacticalItem(BreakdownType bt, TacticalType tt, const std::string& name, HTREEITEM hParent);
        void AddSpellPower(BreakdownType bt, SpellPowerType type, const std::string& name, HTREEITEM hParent);
        void AddSpellCriticalChance(BreakdownType bt, SpellPowerType type, const std::string& name, HTREEITEM hParent);
        void AddSpellCriticalMultiplier(BreakdownType bt, SpellPowerType type, const std::string& name, HTREEITEM hParent);
        void AddSpellSchool(BreakdownType bt, SpellSchoolType type, const std::string& name, HTREEITEM hParent);
        void AddEnergyResistance(BreakdownType bt, EnergyType type, const std::string& name, HTREEITEM hParent);
        void AddEnergyAbsorption(BreakdownType bt, EnergyType type, const std::string& name, HTREEITEM hParent);
        void CalculatePercent(CPoint point);
        Character * m_pCharacter;

        std::vector<BreakdownItem *> m_items;
        MfcControls::CTreeListCtrl m_itemBreakdownTree;
        MfcControls::CGroupLine m_divider;
        CMFCButton m_buttonClipboard;
        CListCtrl m_itemBreakdownList;
        bool m_bDraggingDivider;
        int m_treeSizePercent;             // percentage of space used
        BreakdownItemWeaponEffects* m_pWeaponEffects;
        HTREEITEM m_hWeaponsHeader;
        bool m_bUpdateBreakdowns;
        std::map<EffectType, std::list<EffectCallbackItem*> > m_mapBuildCallbacks;
        bool m_bHadInitialUpdate;
};
