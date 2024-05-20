// GrantedFeatsPane.h
//
#pragma once
#include "Resource.h"
#include "Character.h"
#include "EffectCallbackItem.h"
#include "GrantedFeatListControl.h"

class CGrantedFeatsPane :
    public CFormView,
    public CharacterObserver,
    public EffectCallbackItem
{
    public:
        enum { IDD = IDD_GRANTED_FEATS_PANE };
        DECLARE_DYNCREATE(CGrantedFeatsPane)

        virtual void OnInitialUpdate();
        bool IsGrantedFeat(const std::string& featName) const;

        const std::list<Effect>& GrantedFeats() const;
    protected:
        CGrantedFeatsPane();           // protected constructor used by dynamic creation
        virtual ~CGrantedFeatsPane();

        virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
        afx_msg void OnSize(UINT nType, int cx, int cy);
        afx_msg LRESULT OnNewDocument(WPARAM wParam, LPARAM lParam);
        afx_msg LRESULT OnLoadComplete(WPARAM, LPARAM);
        afx_msg BOOL OnEraseBkgnd(CDC* pDC);
        DECLARE_MESSAGE_MAP()
    private:
        // Character Observer overrides
        virtual void UpdateActiveLifeChanged(Character*) override;
        virtual void UpdateActiveBuildChanged(Character*) override;

        // Build Observer overrides

        // EffectCallbackItem
        virtual void AbilityTomeChanged(Life*, AbilityType) override {};
        virtual void AbilityValueChanged(Build*, AbilityType) override {};
        virtual void AbilityTotalChanged(Build*, AbilityType) override {};
        virtual void BuildLevelChanged(Build*) override;
        virtual void ClassChanged(Build*, const std::string&, const std::string&, size_t) override;
        virtual void FeatTrained(Build*, const std::string&) override;
        virtual void FeatRevoked(Build*, const std::string&) override;
        virtual void RaceChanged(Life*, const std::string&) override;
        virtual void SkillTomeChanged(Life*, SkillType) override {};
        virtual void SkillSpendChanged(Build*, size_t, SkillType) override {};

        virtual void FeatEffectApplied(Build*, const Effect& effect) override;
        virtual void FeatEffectRevoked(Build*, const Effect& effect) override;
        virtual void ItemEffectApplied(Build*, const Effect& effect) override;
        virtual void ItemEffectRevoked(Build*, const Effect& effect) override;
        virtual void ItemEffectApplied(Build*, const Effect& effect, WeaponType wt) override;
        virtual void ItemEffectRevoked(Build*, const Effect& effect, WeaponType wt) override;
        virtual void EnhancementTrained(Build*, const EnhancementItemParams& item) override;
        virtual void EnhancementRevoked(Build*, const EnhancementItemParams& item) override;
        virtual void EnhancementEffectApplied(Build*, const Effect& effect) override;
        virtual void EnhancementEffectRevoked(Build*, const Effect& effect) override;
        virtual void StanceActivated(Build* charData, const std::string& stanceName) override;
        virtual void StanceDeactivated(Build* charData, const std::string& stanceName) override;
        virtual void SliderChanged(Build* charData, const std::string& sliderName, int newValue) override;
        virtual void GearChanged(Build* pBuild, InventorySlotType slot) override;

        void PopulateGrantedFeatsList();
        void AddGrantedFeatEffect(const Effect& effect);
        void RevokeGrantedFeatEffect(const Effect& effect);

        CGrantedFeatListControl m_listGrantedFeats;

        Character * m_pCharacter;
        bool m_bHadInitialUpdate;
        UINT m_grantedHandle;
        std::list<Effect> m_grantedFeats;
        std::list<bool> m_grantedNotifyState;
};
