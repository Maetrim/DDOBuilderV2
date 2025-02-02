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
    CharacterObserver,
    LifeObserver,
    BuildObserver
{
public:
    CBuildsPane();           // protected constructor used by dynamic creation
    virtual ~CBuildsPane();
    enum { IDD = IDD_BUILDS_PANE };

    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg void OnButtonNewLife();
    afx_msg void OnButtonDeleteLife();
    afx_msg void OnButtonAddBuild();
    afx_msg void OnButtonDeleteBuild();
    afx_msg LRESULT OnNewDocument(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnLoadComplete(WPARAM, LPARAM);
    afx_msg void OnSelchangedTreeBuilds(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnBeginlabeleditTreeBuilds(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnEndlabeleditTreeBuilds(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnDblclkTreeBuilds(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnClickTreeBuilds(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnRButtonDownTreeBuilds(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnUpdateBuildLevel(CCmdUI* pCmdUI);
    afx_msg LRESULT OnStartLabelEdit(WPARAM, LPARAM);
    afx_msg void OnUpdateCopyLifeToClipboard(CCmdUI* pCmdUi);
    afx_msg void OnUpdatePasteLife(CCmdUI* pCmdUi);
    afx_msg void OnPasteLife();
    afx_msg void OnSetBuildLevel(UINT nID);

    virtual void DoDataExchange(CDataExchange* pDX) override;
    virtual void OnInitialUpdate() override;
    virtual BOOL PreTranslateMessage(MSG* pMsg) override;

    Build* OnNewLife();
    Build* OnNewImportLife();
    void PopulateBuildsList();

    void ReselectCurrentItem();

    // CharacterObserver
    virtual void UpdateActiveBuildPositionChanged(Character*) override;

    // Life observer
    virtual void UpdateRaceChanged(Life*, const std::string&) override;

    // Build observer
    virtual void UpdateBuildLevelChanged(Build* pBuild) override;
    virtual void UpdateClassChanged(
        Build* pBuild,
        const std::string& classFrom,
        const std::string& classTo,
        size_t level) override;

    DECLARE_DYNCREATE(CBuildsPane)
    DECLARE_MESSAGE_MAP()

private:
    void SelectTreeItem(
        TreeEntryItem type,
        size_t lifeIndex,
        size_t buildIndex);
    void SelectTreeItem(DWORD itemData);
    void CopyLifeToClipboard(DWORD itemData);
    void SaveLifeToNewFile(DWORD itemData);
    void ImportLivesFromOtherFile();

    CButton m_buttonNewLife;
    CButton m_buttonDeleteLife;
    CButton m_buttonAddBuild;
    CButton m_buttonDeleteBuild;
    CTreeCtrl m_treeBuilds;
    CDDOBuilderDoc* m_pDoc;
    Character* m_pCharacter;
    HTREEITEM m_hPopupMenuItem;
    bool m_bEscape;
    bool m_bLoadComplete;
    bool m_bEditInProgress;
};


