// SelfAndPartyBuffsPane.h
//
#pragma once
#include "Resource.h"
#include "Character.h"
#include "InfoTip.h"

class CSelfAndPartyBuffsPane :
    public CFormView,
    public CharacterObserver
{
// Construction
public:
    CSelfAndPartyBuffsPane();
    virtual ~CSelfAndPartyBuffsPane();
    enum { IDD = IDD_SELFANDPARTYBUFFS_PANE
    };

    void UpdateFonts();

protected:
    virtual void OnInitialUpdate() override;
    virtual void DoDataExchange(CDataExchange* pDX) override;
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg LRESULT OnNewDocument(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnLoadComplete(WPARAM, LPARAM);
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg void OnItemchangedListBuffs(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnHoverListBuffs(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg LRESULT OnMouseLeave(WPARAM wParam, LPARAM lParam);

    DECLARE_DYNCREATE(CSelfAndPartyBuffsPane)
    DECLARE_MESSAGE_MAP()

private:
    // CharacterObserver
    virtual void UpdateActiveBuildChanged(Character* pCharacter) override;
    void CheckSelectedItems();
    void ShowTip(const std::string& name, CRect itemRect);
    void HideTip();

    CDocument* m_pDocument;
    Character* m_pCharacter;
    CListCtrl m_listBuffs;
    bool m_bPopulating;
// item tooltips
    CInfoTip m_tooltip;
    bool m_showingTip;
    bool m_tipCreated;
    int m_hoverItem;
    UINT m_hoverHandle;
};

