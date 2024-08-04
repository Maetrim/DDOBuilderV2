// ReaperEnhancementsPane.cpp
//
#include "stdafx.h"
#include "ReaperEnhancementsPane.h"

#include "Build.h"
#include "Character.h"
#include "GlobalSupportFunctions.h"

#include "EnhancementtreeDialog.h"

namespace
{
    // enhancement window size
    const size_t c_sizeX = 299;
    const size_t c_sizeY = 466;
}

IMPLEMENT_DYNCREATE(CReaperEnhancementsPane, CFormView)

CReaperEnhancementsPane::CReaperEnhancementsPane() :
    CFormView(CReaperEnhancementsPane::IDD),
    m_pDoc(NULL),
    m_pCharacter(NULL),
    m_bHadinitialUpdate(false)
{
}

CReaperEnhancementsPane::~CReaperEnhancementsPane()
{
    for (size_t vi = 0; vi < m_treeViews.size(); ++vi)
    {
        delete m_treeViews[vi];
    }
}

void CReaperEnhancementsPane::DoDataExchange(CDataExchange* pDX)
{
    CFormView::DoDataExchange(pDX);
}

#pragma warning(push)
#pragma warning(disable: 4407) // warning C4407: cast between different pointer to member representations, compiler may generate incorrect code
BEGIN_MESSAGE_MAP(CReaperEnhancementsPane, CFormView)
    ON_WM_SIZE()
    ON_WM_ERASEBKGND()
    ON_REGISTERED_MESSAGE(UWM_NEW_DOCUMENT, OnNewDocument)
    ON_REGISTERED_MESSAGE(UWM_LOAD_COMPLETE, OnLoadComplete)
END_MESSAGE_MAP()
#pragma warning(pop)

void CReaperEnhancementsPane::OnInitialUpdate()
{
    if (!m_bHadinitialUpdate)
    {
        m_bHadinitialUpdate = true;
        CFormView::OnInitialUpdate();
        UpdateWindowTitle();
    }
}

void CReaperEnhancementsPane::OnSize(UINT nType, int cx, int cy)
{
    CWnd::OnSize(nType, cx, cy);
    if (m_treeViews.size() > 0
            && IsWindow(m_treeViews[0]->GetSafeHwnd()))
    {
        // we can position and show all the visible enhancement windows
        // with scrolls bars as required

        // default location of first enhancement tree
        CRect itemRect(
                c_controlSpacing,
                c_controlSpacing,
                c_sizeX + c_controlSpacing,
                c_sizeY + c_controlSpacing);

        ASSERT(m_treeViews.size() == MST_Number);
        std::vector<bool> isShown(MST_Number, false);  // gets set to true when displayed
        for (size_t ti = 0; ti < m_visibleTrees.size(); ++ti)
        {
            size_t index = m_visibleTrees[ti];
            ASSERT(index >= 0 && index < MST_Number);
            // move the window to the correct location
            m_treeViews[index]->MoveWindow(itemRect, false);
            m_treeViews[index]->ShowWindow(SW_SHOW);        // ensure visible
            isShown[index] = true;
            // now move the rectangle to the next tree location
            itemRect += CPoint(itemRect.Width() + c_controlSpacing, 0);
        }
        SetScrollSizes(
                MM_TEXT,
                CSize(itemRect.left, itemRect.bottom + c_controlSpacing));
    }
}

LRESULT CReaperEnhancementsPane::OnLoadComplete(WPARAM, LPARAM)
{
    UpdateWindowTitle();

    // cause a resize after all sub windows have updated
    CRect rect;
    GetClientRect(&rect);
    PostMessage(WM_SIZE, SIZE_RESTORED, MAKELONG(rect.Width(), rect.Height()));
    return 0;
}

LRESULT CReaperEnhancementsPane::OnNewDocument(WPARAM wParam, LPARAM lParam)
{
    // wParam is the document pointer
    CDDOBuilderDoc* pDoc = (CDDOBuilderDoc*)(wParam);
    m_pDoc = pDoc;
    // lParam is the character pointer
    Character * pCharacter = (Character *)(lParam);
    if (pCharacter != NULL)
    {
        m_pCharacter = pCharacter;
        m_pCharacter->AttachObserver(this);
        if (IsWindow(GetSafeHwnd()))
        {
            SetScrollPos(SB_HORZ, 0, TRUE);
            SetScrollPos(SB_VERT, 0, TRUE);
        }
        // trees definitely change if the character has changed
        m_availableTrees = DetermineTrees();
        DestroyEnhancementWindows();
        CreateEnhancementWindows();
    }
    else
    {
        if (m_pCharacter != NULL)
        {
            m_pCharacter->DetachObserver(this);
            m_pCharacter = NULL;
        }
        DestroyEnhancementWindows();
    }
    UpdateWindowTitle();
    return 0L;
}

BOOL CReaperEnhancementsPane::OnEraseBkgnd(CDC* pDC)
{
    static int controlsNotToBeErased[] =
    {
        0 // end marker
    };

    // exclude any visible enhancement view windows
    for (size_t i = 0; i < m_treeViews.size(); ++i)
    {
        if (::IsWindow(m_treeViews[i]->GetSafeHwnd())
                && ::IsWindowVisible(m_treeViews[i]->GetSafeHwnd()))
        {
            CRect rctWnd;
            m_treeViews[i]->GetWindowRect(&rctWnd);
            ScreenToClient(&rctWnd);
            pDC->ExcludeClipRect(&rctWnd);
        }
    }

    return OnEraseBackground(this, pDC, controlsNotToBeErased);
}

std::vector<EnhancementTree> CReaperEnhancementsPane::DetermineTrees()
{
    // the three standard reaper trees are always the ones available
    std::vector<EnhancementTree> trees;
    const std::list<EnhancementTree> & allTrees = EnhancementTrees();
    std::list<EnhancementTree>::const_iterator it = allTrees.begin();
    while (it != allTrees.end())
    {
        // get all the trees that are compatible with the race/class setup
        if ((*it).HasIsReaperTree())
        {
            // yes this is one of our tree's add it
            trees.push_back((*it));
        }
        ++it;
    }
    return trees;
}

void CReaperEnhancementsPane::CreateEnhancementWindows()
{
    LockWindowUpdate();
    // all enhancement windows are a set size and max of MST_Number available to the user
    CRect itemRect(
            c_controlSpacing,
            c_controlSpacing,
            c_sizeX + c_controlSpacing,
            c_sizeY + c_controlSpacing);
    Build* pBuild = m_pCharacter->ActiveBuild();
    if (pBuild != NULL)
    {
        for (size_t i = 0; i < MST_Number; ++i)
        {
            // create the tree dialog
            // show an enhancement dialog
            CEnhancementTreeDialog * dlg = new CEnhancementTreeDialog(
                    this,
                    m_pCharacter,
                    EnhancementTree::GetTree(m_availableTrees[i].Name()),
                    TT_reaper);
            dlg->Create(CEnhancementTreeDialog::IDD, this);
            dlg->MoveWindow(&itemRect);
            dlg->ShowWindow(SW_SHOW);
            m_treeViews.push_back(dlg);
            m_visibleTrees.push_back(m_visibleTrees.size());
        }
        UnlockWindowUpdate();
        // reposition and show the windows (handled in OnSize)
        CRect rctWnd;
        GetClientRect(&rctWnd);
        OnSize(SIZE_RESTORED, rctWnd.Width(), rctWnd.Height());
    }
}

void CReaperEnhancementsPane::DestroyEnhancementWindows()
{
    LockWindowUpdate();
    // the user has changed either a race or class and the available enhancement trees
    // has changed
    for (size_t i = 0; i < m_treeViews.size(); ++i)
    {
        m_treeViews[i]->DestroyWindow();
        delete m_treeViews[i];
        m_treeViews[i] = NULL;
    }
    m_treeViews.clear();
    m_visibleTrees.clear();
    UnlockWindowUpdate();
}

void CReaperEnhancementsPane::UpdateWindowTitle()
{
    CString text("Reaper Enhancements");
    if (m_pCharacter != NULL)
    {
        Build* pBuild = m_pCharacter->ActiveBuild();
        if (pBuild != NULL)
        {
            // total up the total number of spent reaper points
            size_t totalReaperPoints = 0;
            for (size_t i = 0; i < m_availableTrees.size(); ++i)
            {
                totalReaperPoints += pBuild->APSpentInTree(m_availableTrees[i].Name());
            }
            // calculate the required reaper xp
            size_t reaperXp = 0;
            for (size_t i = 0; i < totalReaperPoints; ++i)
            {
                reaperXp += (i * 2 + 1);
            }
            text.Format("Reaper Enhancements - %d RAPs, Requires %dk Reaper XP",
                    totalReaperPoints,
                    reaperXp);
        }
    }
    GetParent()->SetWindowText(text);
}

void CReaperEnhancementsPane::UpdateEnhancementTrained(
        Build*,
        const EnhancementItemParams&)
{
    UpdateWindowTitle();
}

void CReaperEnhancementsPane::UpdateEnhancementRevoked(
        Build*,
        const EnhancementItemParams&)
{
    UpdateWindowTitle();
}

void CReaperEnhancementsPane::UpdateActiveLifeChanged(Character*)
{
    Life *pLife = m_pCharacter->ActiveLife();
    Build *pBuild = m_pCharacter->ActiveBuild();
    if (pLife != NULL
            && pBuild != NULL)
    {
        pLife->AttachObserver(this);
        pBuild->AttachObserver(this);
    }
    m_availableTrees = DetermineTrees();
    DestroyEnhancementWindows();
    CreateEnhancementWindows();
    UpdateWindowTitle();
}

void CReaperEnhancementsPane::UpdateActiveBuildChanged(Character*)
{
    Life *pLife = m_pCharacter->ActiveLife();
    Build *pBuild = m_pCharacter->ActiveBuild();
    if (pLife != NULL
            && pBuild != NULL)
    {
        pLife->AttachObserver(this);
        pBuild->AttachObserver(this);
    }
    m_availableTrees = DetermineTrees();
    DestroyEnhancementWindows();
    CreateEnhancementWindows();
    UpdateWindowTitle();
}

void CReaperEnhancementsPane::UpdateActionPointsChanged(Life*)
{
    UpdateWindowTitle();
}
