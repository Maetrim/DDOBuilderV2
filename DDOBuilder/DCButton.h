// DCButton.h
//

#pragma once

#include "Resource.h"
#include <string>
#include "DC.h"
#include "Character.h"

// this window handles selection of stances and enhancement sub-option
class CDCButton :
    public CStatic,
    public LifeObserver,
    public BuildObserver
{
    public:
        CDCButton(Character * charData, const DC & dc);

        const DC & GetDCItem() const;

        void AddStack();
        void RevokeStack();
        size_t NumStacks() const;
        bool IsYou(const DC & dc);

    ///////////////////////////////////////////////////////////////////
    // MFC

    public:
        //{{AFX_DATA(CDCButton)
        //}}AFX_DATA

    protected:
        //{{AFX_VIRTUAL(CDCButton)
        //}}AFX_VIRTUAL

        //virtual void UpdateAbilityTomeChanged(Character * charData, AbilityType ability) override;
        //virtual void UpdateRaceChanged(Character * charData, RaceType race) override;
        //virtual void UpdateGrantedFeatsChanged(Character * charData) override;
        //virtual void UpdateGearChanged(Character * charData, InventorySlotType slot) override;

        virtual void UpdateBuildLevelChanged(Build*) override;
        virtual void UpdateClassChanged(Build*, const std::string& classFrom, const std::string& classTo, size_t level) override;
        virtual void UpdateFeatTrained(Build *, const std::string& featName) override;
        virtual void UpdateFeatRevoked(Build *, const std::string& featName) override;
        virtual void UpdateFeatEffectApplied(Build*, const Effect& effect) override;
        virtual void UpdateFeatEffectRevoked(Build*, const Effect& effect) override;
        virtual void UpdateStanceActivated(Build *, const std::string& stanceName) override;
        virtual void UpdateStanceDeactivated(Build *, const std::string& stanceName) override;
        virtual void UpdateSkillSpendChanged(Build *, size_t level, SkillType skill) override;
        virtual void UpdateAbilityValueChanged(Build*, AbilityType ability) override;
        virtual void UpdateEnhancementTrained(Build*, const EnhancementItemParams& item) override;
        virtual void UpdateEnhancementRevoked(Build*, const EnhancementItemParams& item) override;
        virtual void UpdateEnhancementEffectApplied(Build*, const Effect& effect) override;
        virtual void UpdateEnhancementEffectRevoked(Build*, const Effect& effect) override;
        virtual void UpdateEnhancementTreeOrderChanged(Build*) override;
        virtual void UpdateItemEffectApplied(Build*, const Effect& effect) override;
        virtual void UpdateItemEffectRevoked(Build*, const Effect& effect) override;
        //{{AFX_MSG(CDCButton)
        afx_msg BOOL OnEraseBkgnd(CDC* pDC);
        afx_msg void OnPaint();
        //}}AFX_MSG
        DECLARE_MESSAGE_MAP()

    private:
        Character* m_pCharacter;
        DC m_dc;
        CImage m_image;
        size_t m_stacks;
};

//{{AFX_INSERT_LOCATION}}
