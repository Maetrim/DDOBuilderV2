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
        afx_msg void OnVScroll( UINT, UINT, CScrollBar* );
        DECLARE_MESSAGE_MAP()

        // CharacterObserver
        virtual void UpdateActiveBuildChanged(Character * pCharacter) override;

        // Build Observer
        virtual void UpdateBuildLevelChanged(Build *) override;
        virtual void UpdateClassChoiceChanged(Build *) override;
        virtual void UpdateClassChanged(
                Build *,
                const std::string& classFrom,
                const std::string& classTo,
                size_t level) override;

    private:
        void PostUpdateMessage();

        CDocument * m_pDocument;
        Character * m_pCharacter;
        CFeatsClassControl m_featsAndClasses;
};
