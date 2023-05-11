// AutomaticFeatsPane.h
//
#pragma once
#include "Resource.h"
#include "Character.h"
#include "AutomaticFeatListControl.h"

class CAutomaticFeatsPane :
    public CFormView,
    public CharacterObserver
{
    public:
        enum { IDD = IDD_AUTOMATIC_FEATS_PANE };
        DECLARE_DYNCREATE(CAutomaticFeatsPane)

        virtual void OnInitialUpdate();

    protected:
        CAutomaticFeatsPane();           // protected constructor used by dynamic creation
        virtual ~CAutomaticFeatsPane();

        virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
        afx_msg void OnSize(UINT nType, int cx, int cy);
        afx_msg LRESULT OnNewDocument(WPARAM wParam, LPARAM lParam);
        afx_msg LRESULT OnLoadComplete(WPARAM, LPARAM);
        afx_msg LRESULT OnUpdate(WPARAM, LPARAM);
        afx_msg BOOL OnEraseBkgnd(CDC* pDC);
        DECLARE_MESSAGE_MAP()
    private:
        // Character Observer overrides
        virtual void UpdateActiveLifeChanged(Character*) override;
        virtual void UpdateActiveBuildChanged(Character*) override;

        void SetAutofeats(size_t level);
        CAutomaticFeatListControl m_automaticFeats;

        Character * m_pCharacter;
        bool m_bHadInitialUpdate;
};
