// ClassAndFeatPane.h
//
#pragma once
#include "Resource.h"
#include "Character.h"
#include "FeatsClassControl.h"

class CClassAndFeatPane :
    public CFormView,
    public CharacterObserver,
    public BuildObserver
{
    public:
        enum { IDD = IDD_CLASS_AND_FEAT_PANE };
        DECLARE_DYNCREATE(CClassAndFeatPane)

#ifdef _DEBUG
        virtual void AssertValid() const;
    #ifndef _WIN32_WCE
    virtual void Dump(CDumpContext& dc) const;
#endif
#endif
        virtual void OnInitialUpdate();

    protected:
        CClassAndFeatPane();           // protected constructor used by dynamic creation
        virtual ~CClassAndFeatPane();

        //virtual void UpdateClassChoiceChanged(Character * charData) override;

        virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
        afx_msg void OnSize(UINT nType, int cx, int cy);
        afx_msg BOOL OnEraseBkgnd(CDC* pDC);
        afx_msg LRESULT OnNewDocument(WPARAM wParam, LPARAM lParam);
        afx_msg void OnHScroll(UINT, UINT, CScrollBar*);
        afx_msg void OnVScroll(UINT, UINT, CScrollBar*);
        afx_msg void OnComboBuildLevelSelect();
        DECLARE_MESSAGE_MAP()


        // CharacterObserver overrides
        virtual void UpdateActiveBuildChanged(Character*) override;
        // BuildObserver overrides
        virtual void UpdateBuildLevelChanged(Build*) override;

    private:
        CDocument * m_pDocument;
        Character * m_pCharacter;
        CComboBox m_comboBuildLevel;
        CFeatsClassControl m_featsAndClasses;
        bool m_bHadInitialise;
        bool m_bDone1stResize;
        int m_nOffset;
};
