// DCPane.cpp
//
#include "stdafx.h"
#include "DCPane.h"
#include "GlobalSupportFunctions.h"

namespace
{
    const int c_windowSizeX = 38;
    const int c_windowSizeY = 48;
}

IMPLEMENT_DYNCREATE(CDCPane, CFormView)

CDCPane::CDCPane() :
    CFormView(CDCPane::IDD),
    m_pCharacter(NULL),
    m_pDocument(NULL),
    m_tipCreated(false),
    m_pTooltipItem(NULL),
    m_showingTip(false),
    m_nextDcId(IDC_SPECIALFEAT_0)
{
}

CDCPane::~CDCPane()
{
    DestroyAllDCs();
}

void CDCPane::DoDataExchange(CDataExchange* pDX)
{
    CFormView::DoDataExchange(pDX);
}

#pragma warning(push)
#pragma warning(disable: 4407) // warning C4407: cast between different pointer to member representations, compiler may generate incorrect code
BEGIN_MESSAGE_MAP(CDCPane, CFormView)
    ON_WM_SIZE()
    ON_WM_ERASEBKGND()
    ON_REGISTERED_MESSAGE(UWM_NEW_DOCUMENT, OnNewDocument)
    ON_WM_MOUSEMOVE()
    ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
END_MESSAGE_MAP()
#pragma warning(pop)

#ifdef _DEBUG
void CDCPane::AssertValid() const
{
    CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CDCPane::Dump(CDumpContext& dc) const
{
    CFormView::Dump(dc);
}
#endif
#endif //_DEBUG

void CDCPane::OnInitialUpdate()
{
    CFormView::OnInitialUpdate();
    if (!m_tipCreated)
    {
        m_tooltip.Create(this);
        m_tipCreated = true;
    }
}

void CDCPane::OnSize(UINT nType, int cx, int cy)
{
    CWnd::OnSize(nType, cx, cy);
    int lastY = 0;
   if (m_dcButtons.size() > 0)
    {
        CRect wndClient;
        GetClientRect(&wndClient);
        CRect itemRect(
                c_controlSpacing,
                c_controlSpacing,
                c_windowSizeX + c_controlSpacing,
                c_windowSizeY + c_controlSpacing);
        // move each DC button
        for (size_t i = 0; i < m_dcButtons.size(); ++i)
        {
            m_dcButtons[i]->MoveWindow(itemRect, TRUE);
            // move rectangle across for next set of controls
            itemRect += CPoint(itemRect.Width() + c_controlSpacing, 0);
            lastY = itemRect.bottom + c_controlSpacing;
            if (itemRect.right > (wndClient.right - c_controlSpacing))
            {
                // oops, not enough space in client area here
                // move down and start the next row of controls
                itemRect -= CPoint(itemRect.left, 0);
                itemRect += CPoint(c_controlSpacing, itemRect.Height() + c_controlSpacing);
            }
        }
    }
    for (size_t i = 0; i < m_dcButtons.size(); ++i)
    {
        m_dcButtons[i]->Invalidate(TRUE);
    }
    SetScrollSizes(MM_TEXT, CSize(cx, max(cy, lastY)));
}

LRESULT CDCPane::OnNewDocument(WPARAM wParam, LPARAM lParam)
{
    DestroyAllDCs();

    // wParam is the document pointer
    CDocument * pDoc = (CDocument*)(wParam);
    m_pDocument = pDoc;
    // lParam is the character pointer
    Character * pCharacter = (Character *)(lParam);
    m_pCharacter = pCharacter;
    return 0L;
}

BOOL CDCPane::OnEraseBkgnd(CDC* pDC)
{
    static int controlsNotToBeErased[] =
    {
        0 // end marker
    };

    return OnEraseBackground(this, pDC, controlsNotToBeErased);
}

void CDCPane::UpdateNewDC(Build*, const DC & dc)
{
    AddDC(dc);
}

void CDCPane::UpdateRevokeDC(Build*, const DC & dc)
{
    RevokeDC(dc);
}

void CDCPane::AddDC(const DC & dc)
{
    // only add the stance if it is not already present
    bool found = false;
    for (size_t i = 0; i < m_dcButtons.size(); ++i)
    {
        if (m_dcButtons[i]->IsYou(dc))
        {
            found = true;
            m_dcButtons[i]->AddStack();
        }
    }
    if (!found)
    {
        // position the created windows left to right until
        // they don't fit then move them down a row and start again
        // each stance window is c_windowSizeX * c_windowSizeY pixels
        CRect wndClient;
        GetClientRect(&wndClient);
        CRect itemRect(
                c_controlSpacing,
                c_controlSpacing,
                c_windowSizeX + c_controlSpacing,
                c_windowSizeY + c_controlSpacing);

        // now create the new user DC control
        m_dcButtons.push_back(new CDCButton(m_pCharacter, dc));
        // create a parent window that is c_windowSizeX by c_windowSizeY pixels in size
        m_dcButtons.back()->Create(
                "",
                WS_CHILD | WS_VISIBLE,
                itemRect,
                this,
                m_nextDcId++);
        m_dcButtons.back()->AddStack();
        if (IsWindow(GetSafeHwnd()))
        {
            // now force an on size event to position everything
            CRect rctWnd;
            GetClientRect(&rctWnd);
            OnSize(SIZE_RESTORED, rctWnd.Width(), rctWnd.Height());
        }
    }
}

void CDCPane::RevokeDC(const DC & dc)
{
    // only revoke the stance if it is not already present and its the last stack of it
    for (size_t i = 0; i < m_dcButtons.size(); ++i)
    {
        if (m_dcButtons[i]->IsYou(dc))
        {
            m_dcButtons[i]->RevokeStack();
            if (m_dcButtons[i]->NumStacks() == 0)
            {
                // all instances of this stance are gone, remove the button
                m_dcButtons[i]->DestroyWindow();
                delete m_dcButtons[i];
                m_dcButtons[i] = NULL;
                // clear entries from the array
                std::vector<CDCButton *>::iterator it = m_dcButtons.begin() + i;
                m_dcButtons.erase(it);
                // now force an on size event
                CRect rctWnd;
                GetClientRect(&rctWnd);
                OnSize(SIZE_RESTORED, rctWnd.Width(), rctWnd.Height());
                break;
            }
        }
    }
}

void CDCPane::DestroyAllDCs()
{
    for (size_t i = 0; i < m_dcButtons.size(); ++i)
    {
        m_dcButtons[i]->DestroyWindow();
        delete m_dcButtons[i];
        m_dcButtons[i] = NULL;
    }
    m_dcButtons.clear();
}

void CDCPane::OnMouseMove(UINT, CPoint point)
{
    // determine which stance the mouse may be over
    CWnd * pWnd = ChildWindowFromPoint(point);
    CDCButton * pDC = dynamic_cast<CDCButton*>(pWnd);
    if (pDC != NULL
            && pDC != m_pTooltipItem)
    {
        CRect itemRect;
        pDC->GetWindowRect(&itemRect);
        ScreenToClient(itemRect);
        // over a new item or a different item
        m_pTooltipItem = pDC;
        ShowTip(*pDC, itemRect);
    }
    else
    {
        if (m_showingTip
                && pDC != m_pTooltipItem)
        {
            // no longer over the same item
            HideTip();
        }
    }
}

LRESULT CDCPane::OnMouseLeave(WPARAM, LPARAM)
{
    // hide any tooltip when the mouse leaves the area its being shown for
    HideTip();
    return 0;
}

void CDCPane::ShowTip(const CDCButton & item, CRect itemRect)
{
    if (m_showingTip)
    {
        m_tooltip.Hide();
    }
    ClientToScreen(&itemRect);
    CPoint tipTopLeft(itemRect.left, itemRect.bottom + 2);
    CPoint tipAlternate(itemRect.left, itemRect.top - 2);
    SetTooltipText(item, tipTopLeft, tipAlternate);
    m_showingTip = true;
    // track the mouse so we know when it leaves our window
    TRACKMOUSEEVENT tme;
    tme.cbSize = sizeof(tme);
    tme.hwndTrack = m_hWnd;
    tme.dwFlags = TME_LEAVE;
    tme.dwHoverTime = 1;
    _TrackMouseEvent(&tme);
}

void CDCPane::HideTip()
{
    // tip not shown if not over an assay
    if (m_tipCreated && m_showingTip)
    {
        m_tooltip.Hide();
        m_showingTip = false;
        m_pTooltipItem = NULL;
    }
}

void CDCPane::SetTooltipText(
        const CDCButton & item,
        CPoint tipTopLeft,
        CPoint tipAlternate)
{
    m_tooltip.SetOrigin(tipTopLeft, tipAlternate, false);
    m_tooltip.SetDCItem(*m_pCharacter->ActiveBuild(), &item.GetDCItem());
    m_tooltip.Show();
}

const std::vector<CDCButton *> & CDCPane::DCs() const
{
    return m_dcButtons;
}

void CDCPane::BuildChanging()
{
    DestroyAllDCs();

    Life *pLife = m_pCharacter->ActiveLife();
    Build *pBuild = m_pCharacter->ActiveBuild();
    if (pLife != NULL
        && pBuild != NULL)
    {
        pLife->AttachObserver(this);
        pBuild->AttachObserver(this);
    }
}
