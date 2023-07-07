// FavorPane.cpp : implementation file
//

#include "stdafx.h"
#include "FavorPane.h"
#include "GlobalSupportFunctions.h"
#include "DDOBuilderDoc.h"
#include "LogPane.h"

// CFavorPane

IMPLEMENT_DYNCREATE(CFavorPane, CFormView)
BEGIN_MESSAGE_MAP(CFavorPane, CFormView)
    ON_WM_SIZE()
    ON_WM_ERASEBKGND()
    ON_REGISTERED_MESSAGE(UWM_NEW_DOCUMENT, OnNewDocument)
    ON_REGISTERED_MESSAGE(UWM_LOAD_COMPLETE, OnLoadComplete)
END_MESSAGE_MAP()

CFavorPane::CFavorPane() :
    CFormView(CFavorPane::IDD),
    m_pDoc(NULL),
    m_pCharacter(NULL),
    m_bLoadComplete(false)
{
}

CFavorPane::~CFavorPane()
{
}

LRESULT CFavorPane::OnNewDocument(WPARAM wParam, LPARAM lParam)
{
    if (m_pCharacter != NULL)
    {
        m_pCharacter->DetachObserver(this);
    }

    // wParam is the document pointer
    CDDOBuilderDoc * pDoc = (CDDOBuilderDoc*)(wParam);
    m_pDoc = pDoc;
    // lParam is the character pointer
    Character * pCharacter = (Character *)(lParam);
    m_pCharacter = pCharacter;
    if (m_pCharacter != NULL)
    {
        m_pCharacter->AttachObserver(this);
    }
    return 0L;
}

LRESULT CFavorPane::OnLoadComplete(WPARAM, LPARAM)
{
    m_bLoadComplete = true;
    return 0;
}

void CFavorPane::DoDataExchange(CDataExchange* pDX)
{
    CFormView::DoDataExchange(pDX);
}

void CFavorPane::OnSize(UINT nType, int cx, int cy)
{
    CFormView::OnSize(nType, cx, cy);
    //if (IsWindow(m_treeFavor.GetSafeHwnd()))
    {
    }
    SetScaleToFitSize(CSize(cx, cy));
}

BOOL CFavorPane::OnEraseBkgnd(CDC* pDC)
{
    static int controlsNotToBeErased[] =
    {
        0 // end marker
    };

    return OnEraseBackground(this, pDC, controlsNotToBeErased);
}

void CFavorPane::OnInitialUpdate()
{
    __super::OnInitialUpdate();
}
