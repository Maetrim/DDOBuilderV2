// BonusesPane.h
//
#pragma once
#include "Resource.h"
#include "Character.h"
#include "BreakdownItem.h"

class CBonusesPane :
    public CFormView,
    public CharacterObserver,
    public BreakdownObserver
{
// Construction
public:
    CBonusesPane();
    virtual ~CBonusesPane();
    enum { IDD = IDD_BONUSES_PANE };

    void UpdateFonts();

protected:
    virtual void OnInitialUpdate() override;
    virtual void DoDataExchange(CDataExchange* pDX) override;
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg LRESULT OnNewDocument(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnLoadComplete(WPARAM, LPARAM);
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg void OnAddBreakdown();
    afx_msg void OnRemoveBreakdown();
    afx_msg void OnMoveUpBreakdown();
    afx_msg void OnMoveDownBreakdown();
    afx_msg void OnItemchangedBonusesList(NMHDR* pNMHDR, LRESULT* pResult);

    DECLARE_DYNCREATE(CBonusesPane)
    DECLARE_MESSAGE_MAP()

    // CharacterObserver
    virtual void UpdateActiveBuildChanged(Character *) override;

    // BreakdownObserver overrides
    virtual void UpdateTotalChanged(BreakdownItem * item, BreakdownType type) override;

private:
    void ClearItems();
    void PopulateItems();

    CDocument* m_pDocument;
    Character* m_pCharacter;
    bool m_bHadInitialUpdate;
    bool m_bPopulating;
    CListCtrl m_listBonuses;
    CMFCButton m_buttonAdd;
    CMFCButton m_buttonRemove;
    CMFCButton m_buttonMoveUp;
    CMFCButton m_buttonMoveDown;
};

