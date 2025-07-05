// SkillsPane.cpp
//
#include "stdafx.h"
#include "SkillsPane.h"
#include "GlobalSupportFunctions.h"

IMPLEMENT_DYNCREATE(CSkillsPane, CFormView)

CSkillsPane::CSkillsPane() :
    CFormView(CSkillsPane::IDD),
    m_pCharacter(NULL),
    m_pDocument(NULL)
{
    //{{AFX_DATA_INIT(CSkillsPane)
    //}}AFX_DATA_INIT
}

CSkillsPane::~CSkillsPane()
{
}

void CSkillsPane::DoDataExchange(CDataExchange* pDX)
{
    CFormView::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CSkillsPane)
    DDX_Control(pDX, IDC_SKILL_SPEND, m_skillCtrl);
    //}}AFX_DATA_MAP
}

#pragma warning(push)
#pragma warning(disable: 4407) // warning C4407: cast between different pointer to member representations, compiler may generate incorrect code
BEGIN_MESSAGE_MAP(CSkillsPane, CFormView)
    ON_WM_SIZE()
    ON_WM_ERASEBKGND()
    ON_REGISTERED_MESSAGE(UWM_NEW_DOCUMENT, OnNewDocument)
    ON_REGISTERED_MESSAGE(UWM_LOAD_COMPLETE, OnLoadComplete)
END_MESSAGE_MAP()
#pragma warning(pop)

void CSkillsPane::OnInitialUpdate()
{
    CFormView::OnInitialUpdate();
}

LRESULT CSkillsPane::OnLoadComplete(WPARAM, LPARAM)
{
    m_skillCtrl.LoadComplete();
    return 0;
}

void CSkillsPane::OnSize(UINT nType, int cx, int cy)
{
    if (IsWindow(m_skillCtrl.GetSafeHwnd())
            && IsWindowVisible())
    {
        CSize requiredSize = m_skillCtrl.RequiredSize();
        if (requiredSize == CSize(0, 0))
        {
            // no size, make it fit the window
            requiredSize = CSize(cx, cy);
        }
        int scrollX = GetScrollPos(SB_HORZ);
        int scrollY = GetScrollPos(SB_VERT);
        CRect rctControl(0, 0, requiredSize.cx, requiredSize.cy);
        rctControl -= CPoint(scrollX, scrollY);
        m_skillCtrl.MoveWindow(rctControl, TRUE);
        SetScrollSizes(MM_TEXT, requiredSize);
    }
    CFormView::OnSize(nType, cx, cy);
}

LRESULT CSkillsPane::OnNewDocument(WPARAM wParam, LPARAM lParam)
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
    m_skillCtrl.SetCharacter(m_pCharacter);
    if (m_pCharacter != NULL)
    {
        m_pCharacter->AttachObserver(this);
        Build * pBuild = m_pCharacter->ActiveBuild();
        if (pBuild != NULL)
        {
            pBuild->AttachObserver(this);
        }
    }
    return 0L;
}

BOOL CSkillsPane::OnEraseBkgnd(CDC* pDC)
{
    static int controlsNotToBeErased[] =
    {
        IDC_SKILL_SPEND,
        0 // end marker
    };

    return OnEraseBackground(this, pDC, controlsNotToBeErased);
}

void CSkillsPane::UpdateActiveBuildChanged(Character *)
{
    Life * pLife = m_pCharacter->ActiveLife();
    if (pLife != NULL)
    {
        pLife->AttachObserver(this);
    }
    Build * pBuild = m_pCharacter->ActiveBuild();
    if (pBuild != NULL)
    {
        pBuild->AttachObserver(this);
    }
    m_skillCtrl.SetCharacter(m_pCharacter);
    CRect rect;
    GetClientRect(&rect);
    PostMessage(WM_SIZE, SIZE_RESTORED, MAKELONG(rect.Width(), rect.Height()));
}

void CSkillsPane::UpdateAbilityTomeChanged(Life*, AbilityType ability)
{
    if (ability == Ability_Intelligence)
    {
        m_skillCtrl.Invalidate(FALSE);
    }
}

void CSkillsPane::UpdateRaceChanged(Life*, const std::string&)
{
    m_skillCtrl.Invalidate(FALSE);
}

void CSkillsPane::UpdateBuildLevelChanged(Build *)
{
    m_skillCtrl.Invalidate(FALSE);
    CRect rect;
    GetClientRect(&rect);
    PostMessage(WM_SIZE, SIZE_RESTORED, MAKELONG(rect.Width(), rect.Height()));
}

void CSkillsPane::UpdateClassChanged(
        Build *,
        const std::string&,
        const std::string&,
        size_t)
{
    m_skillCtrl.Invalidate(FALSE);
}

void CSkillsPane::UpdateSkillSpendChanged(
        Build *,
        size_t,
        SkillType)
{
    m_skillCtrl.Invalidate(FALSE);
}

void CSkillsPane::UpdateAbilityValueChanged(Build*, AbilityType ability)
{
    if (ability == Ability_Intelligence)
    {
        m_skillCtrl.Invalidate(FALSE);
    }
}
