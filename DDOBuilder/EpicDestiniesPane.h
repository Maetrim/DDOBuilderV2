// EpicDestiniesPane.h
//
#pragma once
#include "Resource.h"
#include <vector>
#include "Character.h"
#include "EnhancementTree.h"
#include "InfoTip.h"

class CEpicDestiniesPane :
    public CFormView,
    public CharacterObserver,
    public LifeObserver,
    public BuildObserver
{
    public:
        enum { IDD = IDD_EPIC_DESTINIES_PANE };
        DECLARE_DYNCREATE(CEpicDestiniesPane)

#ifdef _DEBUG
        virtual void AssertValid() const;
    #ifndef _WIN32_WCE
    virtual void Dump(CDumpContext& dc) const;
#endif
#endif
        virtual void OnInitialUpdate();
    protected:
        CEpicDestiniesPane();           // protected constructor used by dynamic creation
        virtual ~CEpicDestiniesPane();

        virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
        afx_msg void OnSize(UINT nType, int cx, int cy);
        afx_msg BOOL OnEraseBkgnd(CDC* pDC);
        afx_msg LRESULT OnNewDocument(WPARAM wParam, LPARAM lParam);
        afx_msg LRESULT OnLoadComplete(WPARAM, LPARAM);
        afx_msg void OnDestinySelect();
        afx_msg void OnButtonMakeActiveDestiny();
        afx_msg void OnButtonClaimDestinies();
        afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
        afx_msg LRESULT OnMouseEnter(WPARAM wParam, LPARAM lParam);
        afx_msg LRESULT OnMouseLeave(WPARAM wParam, LPARAM lParam);
        afx_msg void OnHScroll(UINT, UINT, CScrollBar*);
        afx_msg void OnVScroll( UINT, UINT, CScrollBar* );
        DECLARE_MESSAGE_MAP()

        // Character Observer overrides
        virtual void UpdateActiveLifeChanged(Character*) override;
        virtual void UpdateActiveBuildChanged(Character*) override;

        // BuildObserver overrides
        //void UpdateEnhancementTrained(Build*, const std::string & enhancementName, const std::string & selection, bool isTier5) override;
        //void UpdateEnhancementRevoked(Build*, const std::string & enhancementName,const std::string & selection,  bool isTier5) override;
        //void UpdateEnhancementTreeReset(Build*) override;
        void UpdateFatePointsChanged(Build*) override;
        //void UpdateEpicCompletionistChanged(Build*) override;
        void UpdateActionPointsChanged(Build*) override;
    private:
        std::vector<EnhancementTree> DetermineTrees();
        void CreateEnhancementWindows();
        void DestroyEnhancementWindows();
        void PopulateCombobox();
        void EnableControls();
        void UpdateMouseHooks();
        CSize MoveFatePointControls();
        void ShowTip(const EnhancementTreeItem & item, CRect itemRect);
        void HideTip();
        void SetTooltipText(const EnhancementTreeItem & item, CPoint tipTopLeft, CPoint tipAlternate);

        CDocument * m_pDocument;
        Character * m_pCharacter;
        std::vector<EnhancementTree> m_availableEpicDestinies;
        std::vector<CDialog *> m_treeViews;
        CComboBox m_comboEpicDestinySelect;
        CButton m_buttonMakeActive;
        CMFCButton m_buttonClaimDestinies;
        CStatic m_labelTwistsOfFate;
        CStatic m_fatePointsSpent;
        std::vector<CDialog *> m_twistsOfFate;
        UINT m_hookTwistHandles[MAX_TWISTS];
        CInfoTip m_tooltip;
        bool m_showingTip;
        bool m_tipCreated;
        bool m_bHadInitialUpdate;
};
