// AutomaticFeatsPane.cpp
//
#include "stdafx.h"
#include "AutomaticFeatsPane.h"

#include "BreakdownsPane.h"
#include "GlobalSupportFunctions.h"
#include "MainFrm.h"

IMPLEMENT_DYNCREATE(CAutomaticFeatsPane, CFormView)

CAutomaticFeatsPane::CAutomaticFeatsPane() :
    CFormView(CAutomaticFeatsPane::IDD),
    m_pCharacter(NULL),
    m_bHadInitialUpdate(false)
{
}

CAutomaticFeatsPane::~CAutomaticFeatsPane()
{
}

void CAutomaticFeatsPane::DoDataExchange(CDataExchange* pDX)
{
    CFormView::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_AUTOMATICFEATS, m_automaticFeats);
}

#pragma warning(push)
#pragma warning(disable: 4407) // warning C4407: cast between different pointer to member representations, compiler may generate incorrect code
BEGIN_MESSAGE_MAP(CAutomaticFeatsPane, CFormView)
    ON_WM_SIZE()
    ON_REGISTERED_MESSAGE(UWM_NEW_DOCUMENT, OnNewDocument)
    ON_REGISTERED_MESSAGE(UWM_LOAD_COMPLETE, OnLoadComplete)
    ON_REGISTERED_MESSAGE(UWM_UPDATE, OnUpdate)
    ON_WM_ERASEBKGND()
END_MESSAGE_MAP()
#pragma warning(pop)

LRESULT CAutomaticFeatsPane::OnNewDocument(WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(wParam);
    if (m_pCharacter != NULL)
    {
        m_pCharacter->DetachObserver(this);
    }
    // wParam is the document pointer
    // lParam is the character pointer
    Character * pCharacter = (Character *)(lParam);
    if (pCharacter != m_pCharacter)
    {
        m_pCharacter = pCharacter;
        m_automaticFeats.SetCharacter(m_pCharacter);
        SetAutofeats(0);        // default to level 1 automatic feats

        if (pCharacter != NULL)
        {
            // need to know about gear changes
            pCharacter->AttachObserver(this);
        }
    }
    return 0L;
}

void CAutomaticFeatsPane::UpdateActiveLifeChanged(Character*)
{
    Life* pLife = m_pCharacter->ActiveLife();
    Build* pBuild = m_pCharacter->ActiveBuild();
    if (pLife != NULL
        && pBuild != NULL)
    {
    }
}

void CAutomaticFeatsPane::UpdateActiveBuildChanged(Character*)
{
    Life* pLife = m_pCharacter->ActiveLife();
    Build* pBuild = m_pCharacter->ActiveBuild();
    if (pLife != NULL
            && pBuild != NULL)
    {
    }
}

void CAutomaticFeatsPane::OnInitialUpdate()
{
    if (!m_bHadInitialUpdate)
    {
        m_bHadInitialUpdate = true;
        CFormView::OnInitialUpdate();
    }
}

LRESULT CAutomaticFeatsPane::OnLoadComplete(WPARAM, LPARAM)
{
    return 0;
}

void CAutomaticFeatsPane::OnSize(UINT nType, int cx, int cy)
{
    CWnd::OnSize(nType, cx, cy);
    if (IsWindow(m_automaticFeats.GetSafeHwnd()))
    {
        CSize requiredSize = m_automaticFeats.RequiredSize();
        int scrollY = GetScrollPos(SB_VERT);
        CRect rctControl(0, 0, cx, requiredSize.cy);
        rctControl -= CPoint(0, scrollY);
        m_automaticFeats.MoveWindow(rctControl);
        if (requiredSize.cy > cy)
        {
            cx -= (GetSystemMetrics(SM_CXVSCROLL) + 1);
        }
        // show scroll bars if required
        SetScrollSizes(
                MM_TEXT,
                CSize(cx, requiredSize.cy));
    }
}

BOOL CAutomaticFeatsPane::OnEraseBkgnd(CDC* pDC)
{
    static int controlsNotToBeErased[] =
    {
        IDC_AUTOMATICFEATS,
        0 // end marker
    };

    return OnEraseBackground(this, pDC, controlsNotToBeErased);
}

LRESULT CAutomaticFeatsPane::OnUpdate(WPARAM wParam, LPARAM)
{
    SetAutofeats(wParam);       // wParam is the level to show for
    return 0L;
}

void CAutomaticFeatsPane::SetAutofeats(size_t level)
{
    std::list<TrainedFeat> emptyList;
    m_automaticFeats.SetAutomaticFeats("Level ? Unknown", emptyList, -1);
    if (m_pCharacter != NULL)
    {
        Build* pBuild = m_pCharacter->ActiveBuild();
        if (pBuild != NULL)
        {
            size_t maxLevel = pBuild->Level();
            if (level < maxLevel)
            {
                // only add this section if its got some automatic feats
                const LevelTraining& lt = pBuild->LevelData(level);
                std::string cn = lt.HasClass() ? lt.Class() : Class_Unknown;
                std::stringstream ss;
                ss << " Level " << level + 1 << " " << cn; // e.g. " Level 1 Fighter"
                m_automaticFeats.SetAutomaticFeats(ss.str(), lt.AutomaticFeats(), level);
            }
        }
    }
    CRect rctWindow;
    GetClientRect(&rctWindow);
    OnSize(SIZE_RESTORED, rctWindow.Width(), rctWindow.Height());
}

