// ContentPane.h
//
#pragma once
#include "Resource.h"
#include "Character.h"

class CContentPane :
    public CFormView,
    public CharacterObserver
{
// Construction
public:
    CContentPane();
    virtual ~CContentPane();
    enum { IDD = IDD_CONTENT_PANE };

    void UpdateFonts();

protected:
    virtual void OnInitialUpdate() override;
    virtual void DoDataExchange(CDataExchange* pDX) override;
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg LRESULT OnNewDocument(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnLoadComplete(WPARAM, LPARAM);
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg void OnItemchangedAdventurePack(NMHDR* pNMHDR, LRESULT* pResult);

    DECLARE_DYNCREATE(CContentPane)
    DECLARE_MESSAGE_MAP()

private:
    CDocument* m_pDocument;
    Character* m_pCharacter;
    bool m_bHadInitialUpdate;
    bool m_bPopulating;
    CListCtrl m_listContent;
};

