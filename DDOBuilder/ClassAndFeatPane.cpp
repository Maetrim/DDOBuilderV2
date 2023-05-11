// ClassAndFeatPane.cpp
//
#include "stdafx.h"
#include "ClassAndFeatPane.h"
#include "GlobalSupportFunctions.h"
#include "MainFrm.h"

IMPLEMENT_DYNCREATE(CClassAndFeatPane, CFormView)

CClassAndFeatPane::CClassAndFeatPane() :
    CFormView(CClassAndFeatPane::IDD),
    m_pCharacter(NULL),
    m_pDocument(NULL)
{
}

CClassAndFeatPane::~CClassAndFeatPane()
{
}

void CClassAndFeatPane::DoDataExchange(CDataExchange* pDX)
{
    CFormView::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_CLASS_AND_FEAT_LIST, m_featsAndClasses);
}

#pragma warning(push)
#pragma warning(disable: 4407) // warning C4407: cast between different pointer to member representations, compiler may generate incorrect code
BEGIN_MESSAGE_MAP(CClassAndFeatPane, CFormView)
    ON_WM_SIZE()
    ON_WM_ERASEBKGND()
    ON_REGISTERED_MESSAGE(UWM_NEW_DOCUMENT, OnNewDocument)
    ON_WM_VSCROLL()
    ON_WM_HSCROLL()
END_MESSAGE_MAP()
#pragma warning(pop)

#ifdef _DEBUG
void CClassAndFeatPane::AssertValid() const
{
    CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CClassAndFeatPane::Dump(CDumpContext& dc) const
{
    CFormView::Dump(dc);
}
#endif
#endif //_DEBUG

void CClassAndFeatPane::OnInitialUpdate()
{
    CFormView::OnInitialUpdate();
}

void CClassAndFeatPane::OnSize(UINT nType, int cx, int cy)
{
    CFormView::OnSize(nType, cx, cy);
    if (IsWindow(m_featsAndClasses.GetSafeHwnd()))
    {
        CSize requiredSize = m_featsAndClasses.RequiredSize();
        if (requiredSize == CSize(0, 0))
        {
            // no size, make it fit the window
            requiredSize = CSize(cx, cy);
        }
        int scrollX = GetScrollPos(SB_HORZ);
        int scrollY = GetScrollPos(SB_VERT);
        CRect rctControl(0, 0, requiredSize.cx, requiredSize.cy);
        rctControl -= CPoint(scrollX, scrollY);
        m_featsAndClasses.MoveWindow(rctControl, TRUE);
        SetScrollSizes(MM_TEXT, requiredSize);
    }
}

LRESULT CClassAndFeatPane::OnNewDocument(WPARAM wParam, LPARAM lParam)
{
    if (m_pCharacter != NULL)
    {
        Build * pBuild = m_pCharacter->ActiveBuild();
        if (pBuild != NULL)
        {
            pBuild->DetachObserver(this);
        }
        m_pCharacter->DetachObserver(this);
    }

    // wParam is the document pointer
    CDocument * pDoc = (CDocument*)(wParam);
    m_pDocument = pDoc;
    // lParam is the character pointer
    Character * pCharacter = (Character *)(lParam);
    m_pCharacter = pCharacter;
    if (m_pCharacter != NULL)
    {
        m_pCharacter->AttachObserver(this);
        Build * pBuild = m_pCharacter->ActiveBuild();
        if (pBuild != NULL)
        {
            pBuild->AttachObserver(this);
        }
    }
    m_featsAndClasses.SetCharacter(m_pCharacter);
    PostUpdateMessage();
    return 0L;
}

BOOL CClassAndFeatPane::OnEraseBkgnd(CDC* pDC)
{
    static int controlsNotToBeErased[] =
    {
        IDC_CLASS_AND_FEAT_LIST,
        0 // end marker
    };

    return OnEraseBackground(this, pDC, controlsNotToBeErased);
}

void CClassAndFeatPane::OnHScroll(UINT p1, UINT p2, CScrollBar* p3)
{
    // call base class
    CFormView::OnHScroll(p1, p2, p3);
}

void CClassAndFeatPane::OnVScroll(UINT p1, UINT p2, CScrollBar* p3)
{
    // call base class
    CFormView::OnVScroll(p1, p2, p3);
}

void CClassAndFeatPane::UpdateActiveBuildChanged(Character *)
{
    Build * pBuild = m_pCharacter->ActiveBuild();
    if (pBuild != NULL)
    {
        pBuild->AttachObserver(this);
    }
    // cause a resize after all sub windows have updated
    PostUpdateMessage();
}

void CClassAndFeatPane::PostUpdateMessage()
{
    // cause a resize after all sub windows have updated
    CRect rect;
    GetClientRect(&rect);
    PostMessage(WM_SIZE, SIZE_RESTORED, MAKELONG(rect.Width(), rect.Height()));
}

void CClassAndFeatPane::UpdateBuildLevelChanged(Build *)
{
    // cause a resize after all sub windows have updated
    PostUpdateMessage();
}

void CClassAndFeatPane::UpdateClassChoiceChanged(Build *)
{
    // cause a resize after all sub windows have updated
    PostUpdateMessage();
}

void CClassAndFeatPane::UpdateClassChanged(
        Build *,
        const std::string&,
        const std::string&,
        size_t)
{
    // cause a resize after all sub windows have updated
    PostUpdateMessage();
}
