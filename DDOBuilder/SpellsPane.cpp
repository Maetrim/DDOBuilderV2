// SpellsPane.cpp
//
#include "stdafx.h"
#include "SpellsPane.h"
#include "GlobalSupportFunctions.h"
#include <numeric>
#include "Class.h"

#include "SpellsPage.h"
#include "SpellLikeAbilityPage.h"

IMPLEMENT_DYNCREATE(CSpellsPane, CFormView)

CSpellsPane::CSpellsPane() :
    CFormView(CSpellsPane::IDD),
    m_pCharacter(NULL),
    m_pDocument(NULL),
    m_bHadInitialUpdate(false)
{
    // a NULL pointer for every class
    // we maintain this vector at this size to avoid page re-creation/destruction
    // on each class change
    m_pagePointers.resize(1, NULL); // resized in OnLoadComplete
}

CSpellsPane::~CSpellsPane()
{
    for (size_t i = 0; i < m_pagePointers.size(); ++ i)
    {
        delete m_pagePointers[i];
        m_pagePointers[i] = NULL;
    }
}

void CSpellsPane::DoDataExchange(CDataExchange* pDX)
{
    CFormView::DoDataExchange(pDX);
}

#pragma warning(push)
#pragma warning(disable: 4407) // warning C4407: cast between different pointer to member representations, compiler may generate incorrect code
BEGIN_MESSAGE_MAP(CSpellsPane, CFormView)
    ON_WM_SIZE()
    ON_WM_ERASEBKGND()
    ON_REGISTERED_MESSAGE(UWM_NEW_DOCUMENT, OnNewDocument)
    ON_REGISTERED_MESSAGE(UWM_LOAD_COMPLETE, OnLoadComplete)
END_MESSAGE_MAP()
#pragma warning(pop)

LRESULT CSpellsPane::OnLoadComplete(WPARAM, LPARAM)
{
    m_pagePointers.resize(Classes().size() + 1, NULL);  // 0 is SLAs
    // Create the property sheet ready for adding the pages to
    m_spellsSheet.m_psh.dwFlags |= PSH_HASHELP;
    m_spellsSheet.Construct("", this);

    // must have at least 1 page to construct correctly (dummy page added)
    DetermineSpellViews();

    // Display the property sheet
    m_spellsSheet.Create(this, WS_CHILD | WS_VISIBLE, 0);
    // it looks like this line is needed to make sure PrintPreview does not hang
    // when a ShowWindow(SW_HIDE) gets called within the guts of the MFC code.
    // As the code is looking for a window to deactivate/activate it cycles through
    // controls starting from the active window. As the property sheet is a child
    // of this view, it needs to be included in the tab ordering of controls so
    // that the MFC iteration can make it to the correct item without becoming stuck.
    m_spellsSheet.ModifyStyleEx(0, WS_EX_CONTROLPARENT);
    // ensure property sheet is positioned correctly
    CRect rctParent;
    GetClientRect(&rctParent);
    PostMessage(WM_SIZE, SIZE_RESTORED, MAKELONG(rctParent.Width(), rctParent.Height()));
    m_spellsSheet.RedrawWindow();
    return 0;
}

void CSpellsPane::OnInitialUpdate()
{
    if (!m_bHadInitialUpdate)
    {
        m_bHadInitialUpdate = true;
        CFormView::OnInitialUpdate();
    }
}

void CSpellsPane::OnSize(UINT nType, int cx, int cy)
{
    CWnd::OnSize(nType, cx, cy);
    if (IsWindow(m_spellsSheet.GetSafeHwnd())
            && IsWindowVisible())
    {
        // takes up the entire space
        CRect rctSheet(0, 0, cx, cy);
        m_spellsSheet.MoveWindow(rctSheet);
        CRect tabRect(0, 0, rctSheet.Width(), rctSheet.Height());
        m_spellsSheet.GetTabControl()->MoveWindow(tabRect);
        m_spellsSheet.GetTabControl()->AdjustRect(FALSE, &tabRect);
        m_spellsSheet.GetActivePage()->MoveWindow(tabRect);
    }
}

LRESULT CSpellsPane::OnNewDocument(WPARAM wParam, LPARAM lParam)
{
    if (m_pCharacter != NULL)
    {
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
    }
    DetermineSpellViews();
    // set the character for each page displayed
    for (size_t i = 0; i < m_pagePointers.size(); ++i)
    {
        if (m_pagePointers[i] != NULL)
        {
            CSpellLikeAbilityPage * pSLAPage = dynamic_cast<CSpellLikeAbilityPage*>(m_pagePointers[i]);
            if (pSLAPage != NULL)
            {
                pSLAPage->SetCharacter(m_pCharacter);
            }
            CSpellsPage * pSpellPage = dynamic_cast<CSpellsPage*>(m_pagePointers[i]);
            if (pSpellPage != NULL)
            {
                pSpellPage->SetCharacter(m_pCharacter);
            }
        }
    }
    return 0L;
}

CSpellLikeAbilityPage* CSpellsPane::GetSLAPage()
{
    CSpellLikeAbilityPage* pPage = NULL;
    for (size_t i = 0; i < m_pagePointers.size(); ++i)
    {
        if (m_pagePointers[i] != NULL)
        {
            CSpellLikeAbilityPage* pSLAPage = dynamic_cast<CSpellLikeAbilityPage*>(m_pagePointers[i]);
            if (pSLAPage != NULL)
            {
                pPage = pSLAPage;
            }
        }
    }
    return pPage;
}

CSpellsPage* CSpellsPane::GetClassSpells(const std::string& classType)
{
    CSpellsPage* pPage = NULL;
    for (size_t i = 0; i < m_pagePointers.size(); ++i)
    {
        if (m_pagePointers[i] != NULL)
        {
            CSpellsPage* pSpellsPage = dynamic_cast<CSpellsPage*>(m_pagePointers[i]);
            if (pSpellsPage != NULL)
            {
                if (pSpellsPage->IsClassType(classType))
                {
                    pPage = pSpellsPage;
                }
            }
        }
    }
    return pPage;
}

BOOL CSpellsPane::OnEraseBkgnd(CDC* pDC)
{
    static int controlsNotToBeErased[] =
    {
        0 // end marker
    };

    return OnEraseBackground(this, pDC, controlsNotToBeErased);
}

void CSpellsPane::UpdateActiveBuildChanged(Character *)
{
    Build * pBuild = m_pCharacter->ActiveBuild();
    if (pBuild != NULL)
    {
        pBuild->AttachObserver(this);
    }
    DetermineSpellViews();
}

void CSpellsPane::UpdateBuildLevelChanged(Build*)
{
    // if a class has changed or the number of levels in a given
    // class has changed then we may need to change which spell selection
    // views are displayed
    DetermineSpellViews();
}

void CSpellsPane::UpdateClassChanged(
        Build *,
        const std::string&,
        const std::string&,
        size_t)
{
    // if a class has changed or the number of levels in a given
    // class has changed then we may need to change which spell selection
    // views are displayed
    DetermineSpellViews();
}

void CSpellsPane::DetermineSpellViews()
{
    // first lock the window and destroy any existing views
    if (IsWindow(m_spellsSheet.GetSafeHwnd()))
    {
        m_spellsSheet.LockWindowUpdate();
    }

    // get the number of pages already inserted
    size_t numPages = m_spellsSheet.GetPageCount();

    // we have a page for spell like abilities
    if (m_pagePointers[0] == NULL)  // 0 is in effect Class_Unknown
    {
        CSpellLikeAbilityPage * page = new CSpellLikeAbilityPage();
        m_pagePointers[0] = page;
        m_spellsSheet.AddPage(page);
    }

    if (m_pCharacter != NULL)
    {
        Build* pBuild = m_pCharacter->ActiveBuild();
        if (pBuild != NULL)
        {
            // now determine the class pages to add to the view
            std::vector<size_t> classLevels = pBuild->ClassLevels(pBuild->Level()-1);
            const std::list<::Class>& classes = Classes();
            size_t ci = 0;
            for (auto&& cit: classes)
            {
                // get the number of spells available for this class at this level
                std::vector<size_t> spellSlots = cit.SpellSlotsAtLevel(classLevels[ci]);
                // must have at least 1 spell slot at any level to display the
                // required spell selection tab for this class
                size_t slotCount = std::accumulate(spellSlots.begin(), spellSlots.end(), 0);
                if (slotCount > 0)
                {
                    // yes there are spell slots for this class, create the controlling
                    // window for it
                    // does this spell page already exist?
                    if (m_pagePointers[ci+1] == NULL)   // 0 is SLAs
                    {
                        CSpellsPage * page = new CSpellsPage(cit.Name(), cit.CastingStat().front());
                        if (page != NULL)
                        {
                            CString text;
                            text.Format("%s Spells", cit.Name().c_str());
                            SetWindowText(text);
                            // add the page
                            m_pagePointers[ci+1] = page;
                            m_spellsSheet.AddPage(page);
                            CTabCtrl* pTabCtrl = m_spellsSheet.GetTabControl();
                            TCITEM tcItem;
                            tcItem.pszText = text.GetBuffer();
                            tcItem.mask = TCIF_TEXT;
                            pTabCtrl->SetItem(pTabCtrl->GetItemCount() - 1, &tcItem);
                            page->SetCharacter(m_pCharacter);
                            page->SetTrainableSpells(spellSlots);
                        }
                    }
                    else
                    {
                        // ensure the page is updated with the correct number of spells
                        CSpellsPage * page = dynamic_cast<CSpellsPage *>(m_pagePointers[ci+1]);
                        if (page != NULL)
                        {
                            page->SetCharacter(m_pCharacter);   // clears old fixed spells
                            page->SetTrainableSpells(spellSlots);
                        }
                    }
                }
                else
                {
                    // no spells for this class anymore, remove its page if present
                    if (m_pagePointers[ci+1] != NULL)
                    {
                        // work out the page index
                        for (size_t i = 0; i < numPages; ++i)
                        {
                            if (m_spellsSheet.GetPage(i) == m_pagePointers[ci+1])
                            {
                                // this is the one to remove
                                m_spellsSheet.RemovePage(i);
                                break; // skip the rest
                            }
                        }
                        // release the page memory
                        delete m_pagePointers[ci+1];
                        m_pagePointers[ci+1] = NULL;
                    }
                }
                ++ci;
            }
        }
        else
        {
            CSpellLikeAbilityPage* page = dynamic_cast<CSpellLikeAbilityPage*>(m_pagePointers[0]);
            page->SetCharacter(m_pCharacter);   // clears old SLAs
            // no build means no spell pages
            for (size_t pi = 1; pi < m_pagePointers.size(); ++pi)
            {
                if (m_pagePointers[pi] != NULL)
                {
                    // work out the page index
                    for (size_t i = 0; i < numPages; ++i)
                    {
                        if (m_spellsSheet.GetPage(i) == m_pagePointers[pi])
                        {
                            // this is the one to remove
                            m_spellsSheet.RemovePage(i);
                            break; // skip the rest
                        }
                    }
                    // release the page memory
                    delete m_pagePointers[pi];
                    m_pagePointers[pi] = NULL;
                }
            }
        }
    }
    if (IsWindow(m_spellsSheet.GetSafeHwnd()))
    {
        m_spellsSheet.UnlockWindowUpdate();
        CRect rctParent;
        GetClientRect(&rctParent);
        PostMessage(WM_SIZE, SIZE_RESTORED, MAKELONG(rctParent.Width(), rctParent.Height()));
    }
}
