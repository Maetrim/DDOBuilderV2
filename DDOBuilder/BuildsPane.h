// CBuildsPane form view
#pragma once
#include "Resource.h"
#include "Character.h"

class CDDOBuilderDoc;

enum TreeEntryItem
{
    TEI_Life = 0,
    TEI_Build
};

class CBuildsPane :
    public CFormView,
    CharacterObserver
{
public:
    CBuildsPane();           // protected constructor used by dynamic creation
    virtual ~CBuildsPane();
    enum { IDD = IDD_BUILDS_PANE };

protected:
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg void OnButtonNewLife();
    afx_msg void OnButtonDeleteLife();
    afx_msg void OnButtonAddBuild();
    afx_msg void OnButtonDeleteBuild();
    afx_msg LRESULT OnNewDocument(WPARAM wParam, LPARAM lParam);
    afx_msg void OnSelchangedTreeBuilds(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnBeginlabeleditTreeBuilds(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnEndlabeleditTreeBuilds(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnDblclkTreeBuilds(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnUpdateBuildLevel(CCmdUI* pCmdUI);

    virtual void DoDataExchange(CDataExchange* pDX) override;
    virtual void OnInitialUpdate() override;
    // CharacterObserver
    virtual void UpdateNumBuildsChanged(Character * charData) override;

    DECLARE_DYNCREATE(CBuildsPane)
    DECLARE_MESSAGE_MAP()

private:
    void PopulateBuildsList();
    void SelectTreeItem(
            TreeEntryItem type,
            size_t lifeIndex,
            size_t buildIndex);
    void SelectTreeItem(DWORD itemData);

    CButton m_buttonNewLife;
    CButton m_buttonDeleteLife;
    CButton m_buttonAddBuild;
    CButton m_buttonDeleteBuild;
    CTreeCtrl m_treeBuilds;
    CDDOBuilderDoc * m_pDoc;
    Character * m_pCharacter;
    HTREEITEM m_hPopupMenuItem;
};


