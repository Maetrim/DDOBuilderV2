// SpellsPane.h
//
#pragma once
#include "Resource.h"
#include <vector>
#include "Character.h"

class CSpellsPage;
class CSLAControl;
class CSpellsControl;
class CSpellLikeAbilityPage;

class CSpellsPane :
    public CFormView,
    public CharacterObserver,
    public BuildObserver
{
    public:
        enum { IDD = IDD_SPELLS_PANE };
        DECLARE_DYNCREATE(CSpellsPane)

        virtual void OnInitialUpdate();
        virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
        CSpellLikeAbilityPage* GetSLAPage();
        CSpellsPage* GetClassSpells(const std::string& classType);

    protected:
        CSpellsPane();           // protected constructor used by dynamic creation
        virtual ~CSpellsPane();

        afx_msg void OnSize(UINT nType, int cx, int cy);
        afx_msg BOOL OnEraseBkgnd(CDC* pDC);
        afx_msg LRESULT OnNewDocument(WPARAM wParam, LPARAM lParam);
        afx_msg LRESULT OnLoadComplete(WPARAM, LPARAM);
        DECLARE_MESSAGE_MAP()

        // CharacterObserver
        virtual void UpdateActiveBuildChanged(Character * pCharacter) override;

        // Build Observer
        virtual void UpdateBuildLevelChanged(Build *) override;
        virtual void UpdateClassChanged(
                Build *,
                const std::string& classFrom,
                const std::string& classTo,
                size_t level) override;
    private:
        void DetermineSpellViews();

        CPropertySheet m_spellsSheet;
        std::vector<CPropertyPage*> m_pagePointers;
        CDocument * m_pDocument;
        Character * m_pCharacter;
        bool m_bHadInitialUpdate;
};
