// ReaperEnhancementsPane.h
//
#pragma once
#include "Resource.h"
#include <vector>
#include "EnhancementTree.h"
#include "Character.h"

class CDDOBuilderDoc;
class Character;
class CEnhancementTreeDialog;

class CReaperEnhancementsPane :
    public CFormView,
    public CharacterObserver,
    public LifeObserver,
    public BuildObserver
{
public:
    CReaperEnhancementsPane();           // protected constructor used by dynamic creation
    virtual ~CReaperEnhancementsPane();
    enum { IDD = IDD_REAPER_ENHANCEMENTS_PANE };

    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual void OnInitialUpdate();
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg LRESULT OnNewDocument(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnLoadComplete(WPARAM, LPARAM);
    DECLARE_DYNCREATE(CReaperEnhancementsPane)
    DECLARE_MESSAGE_MAP()

private:
    // Character Observer overrides
    virtual void UpdateActiveLifeChanged(Character*) override;
    virtual void UpdateActiveBuildChanged(Character*) override;

    // Life Observer overrides
    virtual void UpdateActionPointsChanged(Life*) override;

    // Build Observer overrides
    virtual void UpdateEnhancementTrained(Build*, const EnhancementItemParams& item) override;
    virtual void UpdateEnhancementRevoked(Build*, const EnhancementItemParams& item) override;
    virtual void UpdateEnhancementTreeOrderChanged(Build*, enum TreeType tt) override;

    std::vector<EnhancementTree> DetermineTrees();
    void CreateEnhancementWindows();
    void DestroyEnhancementWindows();
    void UpdateWindowTitle();
    void UpdateTrees(const std::string& enhancementName);
    enum MaxSupportTrees
    {
        MST_Number = MAX_REAPER_TREES  // only 3 reaper trees
    };
    std::vector<EnhancementTree> m_availableTrees;
    std::vector<CEnhancementTreeDialog *> m_treeViews;
    std::vector<size_t> m_visibleTrees; // only some trees will be visible (indexes into m_treeViews)
    CDDOBuilderDoc* m_pDoc;
    Character* m_pCharacter;
    bool m_bHadinitialUpdate;
};
