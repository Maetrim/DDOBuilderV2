// SpellsPage.h
//
#pragma once

#include "Resource.h"
#include "AbilityTypes.h"
#include "SpellsControl.h"

class CSpellsPage :
    public CPropertyPage,
    public BuildObserver
{
    public:
        CSpellsPage(const std::string& ct, AbilityType ability);
        ~CSpellsPage();

        void SetCharacter(Character * pCharacter);
        void SetTrainableSpells(const std::vector<size_t> & spellsPerLevel);

        CSpellsControl * SpellsControl();
        bool IsClassType(const std::string& ct) const;

    protected:
        virtual void UpdateFeatEffectApplied(Build*, const Effect& effect) override;
        virtual void UpdateFeatEffectRevoked(Build*, const Effect& effect) override;
        virtual void UpdateEnhancementEffectApplied(Build*, const Effect& effect) override;
        virtual void UpdateEnhancementEffectRevoked(Build*, const Effect& effect) override;
        virtual void UpdateItemEffectApplied(Build*, const Effect& effect) override;
        virtual void UpdateItemEffectRevoked(Build*, const Effect& effect) override;
        //virtual void UpdateSpellTrained(Character * charData, const TrainedSpell & spell) override;
        //virtual void UpdateSpellRevoked(Character * charData, const TrainedSpell & spell) override;

    private:
        //{{AFX_VIRTUAL(CSpellsPage)
        virtual BOOL OnInitDialog();
        virtual void DoDataExchange(CDataExchange* pDX);
        //}}AFX_VIRTUAL

        //{{AFX_MSG(CSpellsPage)
        afx_msg void OnSize(UINT nType, int cx, int cy);
        afx_msg BOOL OnEraseBkgnd(CDC* pDC);
        //}}AFX_MSG
        DECLARE_MESSAGE_MAP()

        //{{AFX_DATA(CSpellsPage)
        enum { IDD = IDD_SPELLS_PAGE };
        CSpellsControl m_spells;
        //}}AFX_DATA

        Character * m_pCharacter;
        std::string m_classType;
        AbilityType m_abilityType;
};

//{{AFX_INSERT_LOCATION}}
