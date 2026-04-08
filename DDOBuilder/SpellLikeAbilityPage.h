// SpellLikeAbilityPage.h
//
#pragma once

#include "Resource.h"
#include "Build.h"
#include "SLAControl.h"

class CSpellLikeAbilityPage :
    public CPropertyPage,
    public CharacterObserver,
    public BuildObserver
{
    public:
        CSpellLikeAbilityPage();
        ~CSpellLikeAbilityPage();

        void SetCharacter(Character * pCharacter);
        CSLAControl* Control();

    protected:
        // CharacterObserver
        virtual void UpdateActiveBuildChanged(Character* pCharacter) override;

        virtual void UpdateFeatEffectApplied(Build*, const Effect& effect) override;
        virtual void UpdateFeatEffectRevoked(Build*, const Effect& effect) override;
        virtual void UpdateItemEffectApplied(Build*, const Effect& effect) override;
        virtual void UpdateItemEffectRevoked(Build*, const Effect& effect) override;
        virtual void UpdateEnhancementEffectApplied(Build*, const Effect& effect) override;
        virtual void UpdateEnhancementEffectRevoked(Build*, const Effect& effect) override;

    private:
        //{{AFX_VIRTUAL(CSpellLikeAbilityPage)
        virtual BOOL OnInitDialog();
        virtual void DoDataExchange(CDataExchange* pDX);
        //}}AFX_VIRTUAL

        //{{AFX_MSG(CSpellLikeAbilityPage)
        afx_msg void OnSize(UINT nType, int cx, int cy);
        afx_msg BOOL OnEraseBkgnd(CDC* pDC);
        //}}AFX_MSG
        DECLARE_MESSAGE_MAP()

        //{{AFX_DATA(CSpellLikeAbilityPage)
        enum { IDD = IDD_SLA_PAGE };
        CSLAControl m_slas;
        //}}AFX_DATA

        Character * m_pCharacter;
};

//{{AFX_INSERT_LOCATION}}
