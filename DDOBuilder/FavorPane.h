// CFavorPane form view
#pragma once
#include "Resource.h"
#include "Character.h"
#include "SortHeaderCtrl.h"
#include "FavorProgressBar.h"
#include "FavorListCtrl.h"
#include "FavorTypes.h"

class CDDOBuilderDoc;

class CFavorPane :
    public CFormView,
    CharacterObserver
{
public:
    CFavorPane();           // protected constructor used by dynamic creation
    virtual ~CFavorPane();
    enum { IDD = IDD_FAVOR_PANE };

    virtual void UpdateActiveBuildChanged(Character*) override;
protected:
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg LRESULT OnNewDocument(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnLoadComplete(WPARAM, LPARAM);
    afx_msg void OnColumnclickListQuests(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnRightClickListQuests(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnUpdateFavorNone(CCmdUI * pCmdUi);
    afx_msg void OnUpdateFavorCasual(CCmdUI * pCmdUi);
    afx_msg void OnUpdateFavorNormal(CCmdUI * pCmdUi);
    afx_msg void OnUpdateFavorHard(CCmdUI * pCmdUi);
    afx_msg void OnUpdateFavorElite(CCmdUI * pCmdUi);
    afx_msg void OnUpdateFavorReaper1(CCmdUI * pCmdUi);
    afx_msg void OnUpdateFavorReaper2(CCmdUI * pCmdUi);
    afx_msg void OnUpdateFavorReaper3(CCmdUI * pCmdUi);
    afx_msg void OnUpdateFavorReaper4(CCmdUI * pCmdUi);
    afx_msg void OnUpdateFavorReaper5(CCmdUI * pCmdUi);
    afx_msg void OnUpdateFavorReaper6(CCmdUI * pCmdUi);
    afx_msg void OnUpdateFavorReaper7(CCmdUI * pCmdUi);
    afx_msg void OnUpdateFavorReaper8(CCmdUI * pCmdUi);
    afx_msg void OnUpdateFavorReaper9(CCmdUI * pCmdUi);
    afx_msg void OnUpdateFavorReaper10(CCmdUI * pCmdUi);

    virtual void DoDataExchange(CDataExchange* pDX) override;
    virtual void OnInitialUpdate() override;

    DECLARE_DYNCREATE(CFavorPane)
    DECLARE_MESSAGE_MAP()

private:
    static int CALLBACK SortCompareFunction(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
    void PopulateQuestList();
    unsigned int GetCheckedState(FavorType ft);
    bool GetEnabledState(FavorType ft);

    CDDOBuilderDoc* m_pDoc;
    Character* m_pCharacter;
    bool m_bLoadComplete;
    CFavorListCtrl m_listQuests;
    CSortHeaderCtrl m_sortHeader;
    CFavorProgressBar m_favorItems[MAX_FAVOR_ITEMS];
    size_t m_patronCount;
    bool m_bHadIntialise;
    CMenu m_favorMenu;
};


