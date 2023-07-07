// CFavorPane form view
#pragma once
#include "Resource.h"
#include "Character.h"

class CDDOBuilderDoc;

class CFavorPane :
    public CFormView,
    CharacterObserver
{
public:
    CFavorPane();           // protected constructor used by dynamic creation
    virtual ~CFavorPane();
    enum { IDD = IDD_FAVOR_PANE };

protected:
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg LRESULT OnNewDocument(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnLoadComplete(WPARAM, LPARAM);

    virtual void DoDataExchange(CDataExchange* pDX) override;
    virtual void OnInitialUpdate() override;

    DECLARE_DYNCREATE(CFavorPane)
    DECLARE_MESSAGE_MAP()

private:
    CDDOBuilderDoc* m_pDoc;
    Character* m_pCharacter;
    bool m_bLoadComplete;
};


