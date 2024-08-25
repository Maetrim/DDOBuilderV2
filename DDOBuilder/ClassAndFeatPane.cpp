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
    m_pDocument(NULL),
    m_bHadInitialise(false)
{
}

CClassAndFeatPane::~CClassAndFeatPane()
{
}

void CClassAndFeatPane::DoDataExchange(CDataExchange* pDX)
{
    CFormView::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_COMBO_BUILD_LEVEL, m_comboBuildLevel);
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
    ON_CBN_SELENDOK(IDC_COMBO_BUILD_LEVEL, &CClassAndFeatPane::OnComboBuildLevelSelect)
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
    if (!m_bHadInitialise)
    {
        m_bHadInitialise = true;
        CFormView::OnInitialUpdate();
        CString strLevel;
        for (size_t level = 1; level <= MAX_BUILDER_LEVEL; ++level)
        {
            strLevel.Format("%d", level);
            m_comboBuildLevel.AddString(strLevel);
        }
    }
}

void CClassAndFeatPane::OnSize(UINT nType, int cx, int cy)
{
    CFormView::OnSize(nType, cx, cy);
    if (IsWindow(m_featsAndClasses.GetSafeHwnd()))
    {
        CRect rctStatic;
        CRect rctCombo;
        GetDlgItem(IDC_STATIC_BUILD_LEVEL)->GetWindowRect(&rctStatic);
        m_comboBuildLevel.GetWindowRect(rctCombo);
        rctStatic -= rctStatic.TopLeft();
        rctStatic += CPoint(c_controlSpacing, c_controlSpacing);
        GetDlgItem(IDC_STATIC_BUILD_LEVEL)->MoveWindow(rctStatic);
        rctCombo -= rctCombo.TopLeft();
        rctCombo += CPoint(rctStatic.right + c_controlSpacing, c_controlSpacing);
        m_comboBuildLevel.MoveWindow(rctCombo);
        CSize requiredSize = m_featsAndClasses.RequiredSize();
        if (requiredSize == CSize(0, 0))
        {
            // no size, make it fit the window under the level controls
            requiredSize = CSize(cx, cy);
        }
        int scrollX = GetScrollPos(SB_HORZ);
        int scrollY = GetScrollPos(SB_VERT);
        CRect rctControl(0, rctStatic.bottom + c_controlSpacing, requiredSize.cx, rctStatic.bottom + c_controlSpacing + requiredSize.cy);
        rctControl -= CPoint(scrollX, scrollY);
        m_featsAndClasses.MoveWindow(rctControl, TRUE);
        SetScrollSizes(MM_TEXT, CSize(rctControl.right, rctControl.bottom));
    }
}

LRESULT CClassAndFeatPane::OnNewDocument(WPARAM wParam, LPARAM lParam)
{
    // wParam is the document pointer
    CDocument * pDoc = (CDocument*)(wParam);
    m_pDocument = pDoc;
    // lParam is the character pointer
    Character * pCharacter = (Character *)(lParam);
    m_pCharacter = pCharacter;
    m_featsAndClasses.SetCharacter(m_pCharacter);
    m_comboBuildLevel.EnableWindow(FALSE);
    if (m_pCharacter != NULL)
    {
        m_pCharacter->AttachObserver(this);
        Build* pBuild = m_pCharacter->ActiveBuild();
        if (pBuild != NULL)
        {
            pBuild->AttachObserver(this);
            m_comboBuildLevel.SetCurSel(m_pCharacter->ActiveBuild()->Level()-1);
            m_comboBuildLevel.EnableWindow(TRUE);
        }
    }
    return 0L;
}

BOOL CClassAndFeatPane::OnEraseBkgnd(CDC* pDC)
{
    static int controlsNotToBeErased[] =
    {
        IDC_STATIC_BUILD_LEVEL,
        IDC_COMBO_BUILD_LEVEL,
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

void CClassAndFeatPane::UpdateActiveBuildChanged(Character*)
{
    m_comboBuildLevel.EnableWindow(FALSE);
    if (m_pCharacter != NULL)
    {
        Build* pBuild = m_pCharacter->ActiveBuild();
        if (pBuild != NULL)
        {
            pBuild->AttachObserver(this);
            m_comboBuildLevel.SetCurSel(m_pCharacter->ActiveBuild()->Level()-1);
            m_comboBuildLevel.EnableWindow(TRUE);
        }
    }
}

void CClassAndFeatPane::UpdateBuildLevelChanged(Build* pBuild)
{
    if (pBuild != NULL)
    {
        m_comboBuildLevel.SetCurSel(m_pCharacter->ActiveBuild()->Level()-1);
        m_comboBuildLevel.EnableWindow(TRUE);
    }
}

void CClassAndFeatPane::OnComboBuildLevelSelect()
{
    int sel = m_comboBuildLevel.GetCurSel();
    if (sel != CB_ERR)
    {
        size_t level = sel + 1;
        m_pCharacter->ActiveBuild()->SetLevel(level);
    }
}
