// DCPane.h
//
#pragma once
#include "Resource.h"

#include "Character.h"
#include "DCButton.h"
#include "InfoTip.h"

class CDCPane :
    public CFormView,
    public CharacterObserver,
    public LifeObserver,
    public BuildObserver
{
    public:
        enum { IDD = IDD_DC_PANE };
        DECLARE_DYNCREATE(CDCPane)

#ifdef _DEBUG
        virtual void AssertValid() const;
    #ifndef _WIN32_WCE
    virtual void Dump(CDumpContext& dc) const;
#endif
#endif
        virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
        virtual void OnInitialUpdate();

        const std::vector<CDCButton *> & DCs() const;
    protected:
        CDCPane();           // protected constructor used by dynamic creation
        virtual ~CDCPane();

        afx_msg void OnSize(UINT nType, int cx, int cy);
        afx_msg BOOL OnEraseBkgnd(CDC* pDC);
        afx_msg LRESULT OnNewDocument(WPARAM wParam, LPARAM lParam);
        afx_msg LRESULT OnLoadComplete(WPARAM, LPARAM);
        afx_msg void OnMouseMove(UINT nFlags, CPoint point);
        afx_msg LRESULT OnMouseLeave(WPARAM wParam, LPARAM lParam);
        DECLARE_MESSAGE_MAP()

    private:
        // CharacterObserver
        virtual void UpdateActiveBuildChanged(Character * pCharacter) override;

        // BuildObserver
        virtual void UpdateNewDC(Build* pBuild, const DC& dc) override;
        virtual void UpdateRevokeDC(Build* pBuild, const DC& dc) override;

        void AddDC(const DC & dc);
        void RevokeDC(const DC & dc);
        void DestroyAllDCs();

        void ShowTip(const CDCButton & item, CRect itemRect);
        void HideTip();
        void SetTooltipText(const CDCButton & item, CPoint tipTopLeft, CPoint tipAlternate);

        CDocument * m_pDocument;
        Character * m_pCharacter;
        std::vector<CDCButton *> m_dcButtons;
        CInfoTip m_tooltip;
        bool m_showingTip;
        bool m_tipCreated;
        const CDCButton * m_pTooltipItem;
        int m_nextDcId;
};
