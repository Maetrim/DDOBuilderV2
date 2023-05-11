// SkillsPane.h
//
#pragma once
#include "Resource.h"

#include "Character.h"
#include "SkillSpendControl.h"

class CSkillsPane :
    public CFormView,
    public CharacterObserver,
    public LifeObserver,
    public BuildObserver
{
    public:
        enum { IDD = IDD_SKILLS_PANE };
        DECLARE_DYNCREATE(CSkillsPane)

        virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
        virtual void OnInitialUpdate();

    protected:
        CSkillsPane();           // protected constructor used by dynamic creation
        virtual ~CSkillsPane();

        afx_msg void OnSize(UINT nType, int cx, int cy);
        afx_msg BOOL OnEraseBkgnd(CDC* pDC);
        afx_msg LRESULT OnNewDocument(WPARAM wParam, LPARAM lParam);
        afx_msg LRESULT OnLoadComplete(WPARAM, LPARAM);
        DECLARE_MESSAGE_MAP()

        // CharacterObserver
        virtual void UpdateActiveBuildChanged(Character * pCharacter) override;

        // Life observer
        virtual void UpdateAbilityTomeChanged(Life*, AbilityType) override;
        virtual void UpdateRaceChanged(Life*, const std::string&) override;

        // Build Observer
        virtual void UpdateBuildLevelChanged(Build *) override;
        virtual void UpdateClassChanged(
                Build *,
                const std::string& classFrom,
                const std::string& classTo,
                size_t level) override;
        virtual void UpdateSkillSpendChanged(
                Build *,
                size_t level,
                SkillType skill) override;
        virtual void UpdateAbilityValueChanged(Build*, AbilityType ability) override;

    private:
        CDocument * m_pDocument;
        Character * m_pCharacter;
        CSkillSpendControl m_skillCtrl;
};
