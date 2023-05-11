// GrantedFeatListControl.cpp : implementation file
//
#include "stdafx.h"
#include "GrantedFeatListControl.h"

#include "GlobalSupportFunctions.h"
#include "Resource.h"

namespace
{
    COLORREF f_selectedColour = ::GetSysColor(COLOR_HIGHLIGHT);
    COLORREF f_backgroundColour = ::GetSysColor(COLOR_BTNFACE); // grey
    COLORREF f_white = RGB(255, 255, 255);                      // white
    COLORREF f_black = RGB(0, 0, 0);                            // black
    const int c_noSelection = -1;
}

// CGrantedFeatListControl
IMPLEMENT_DYNAMIC(CGrantedFeatListControl, CWnd)

CGrantedFeatListControl::CGrantedFeatListControl() :
    m_pCharacter(NULL),
    m_headerItemSize(0, 0),
    m_featItemSize(0, 0),
    m_selectedSection(c_noSelection),
    m_selectedSectionItem(c_noSelection),
    m_showingTip(false),
    m_tipCreated(false)
{
}

CGrantedFeatListControl::~CGrantedFeatListControl()
{
}

void CGrantedFeatListControl::SetCharacter(Character * pCharacter)
{
    m_pCharacter = pCharacter;
    SetupControl();
}

#pragma warning(push)
#pragma warning(disable: 4407) // warning C4407: cast between different pointer to member representations, compiler may generate incorrect code
BEGIN_MESSAGE_MAP(CGrantedFeatListControl, CWnd)
    ON_WM_PAINT()
    ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
    ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()
#pragma warning(pop)

CSize CGrantedFeatListControl::RequiredSize()
{
    CSize requiredSize(0, 0);

    CDC screenDC;
    screenDC.Attach(::GetDC(NULL));
    screenDC.SaveDC();
    screenDC.SelectStockObject(DEFAULT_GUI_FONT);

    // required Width is always the same
    // calculate the width of the control required
    m_headerItemSize = screenDC.GetTextExtent("This is the width of a standard item");
    m_featItemSize = CSize(m_headerItemSize.cx, 34);    // standard icon height + border

    size_t numSections = m_sections.size();
    size_t numSectionItems = 0;
    for (size_t iSection = 0; iSection < numSections; ++iSection)
    {
        numSectionItems += m_sections[iSection].feats.size();
    }
    requiredSize.cx = m_headerItemSize.cx;
    requiredSize.cy = (m_headerItemSize.cy * numSections)
            + (m_featItemSize.cy * numSectionItems);
    screenDC.RestoreDC(-1);
    ::ReleaseDC(NULL, screenDC.Detach());
    return requiredSize;
}

void CGrantedFeatListControl::Clear()
{
    m_sections.clear();
    Invalidate(FALSE);
}

void CGrantedFeatListControl::AddSection(
        const CString& name,
        const std::list<Feat>& feats)
{
    Section s;
    s.name = name;
    s.feats = feats;
    m_sections.push_back(s);
    Invalidate(FALSE);
}

void CGrantedFeatListControl::SetupControl()
{
    if (!m_tipCreated)
    {
        m_tooltip.Create(this);
        m_tipCreated = true;
    }
    m_sections.clear();
    Invalidate(FALSE);
}

void CGrantedFeatListControl::OnPaint()
{
    CPaintDC dc(this);
    CRect rctWindow;
    GetWindowRect(rctWindow);
    ScreenToClient(rctWindow);

    if (m_headerItemSize == CSize(0, 0))
    {
        RequiredSize();
    }

    // double buffer drawing to avoid flickering
    CBitmap bitmap;
    bitmap.CreateCompatibleBitmap(
            &dc,
            rctWindow.Width(),
            rctWindow.Height());
    // draw to a compatible device context and then splat to screen
    // to avoid flickering of the control
    CDC memoryDc;
    memoryDc.CreateCompatibleDC(&dc);
    memoryDc.SaveDC();
    memoryDc.SelectObject(&bitmap);
    memoryDc.SelectStockObject(DEFAULT_GUI_FONT);
    memoryDc.SetBkMode(TRANSPARENT);

    m_headerItemSize.cx = rctWindow.Width();

    // fill background
    memoryDc.FillSolidRect(rctWindow, f_backgroundColour);
    memoryDc.Draw3dRect(
            rctWindow,
            ::GetSysColor(COLOR_BTNHIGHLIGHT),
            ::GetSysColor(COLOR_BTNSHADOW));

    if (m_pCharacter != NULL)
    {
        size_t top = 0;
        for (size_t iSection = 0; iSection < m_sections.size(); ++iSection)
        {
            top = DrawSection(&memoryDc, iSection, top);
        }
    }
    // now draw to display
    dc.BitBlt(
            0,
            0,
            rctWindow.Width(),
            rctWindow.Height(),
            &memoryDc,
            0,
            0,
            SRCCOPY);
    memoryDc.RestoreDC(-1);
    memoryDc.DeleteDC();
}

size_t CGrantedFeatListControl::DrawSection(CDC* pDC, size_t iSection, size_t top)
{
    // draw the section header item
    CRect rctItem(0, top, m_headerItemSize.cx, top + m_headerItemSize.cy);
    pDC->Draw3dRect(
            rctItem,
            ::GetSysColor(COLOR_BTNHIGHLIGHT),
            ::GetSysColor(COLOR_BTNSHADOW));
    CString sectionName = m_sections[iSection].name;
    CSize strSize = pDC->GetTextExtent(sectionName);
    pDC->TextOut(
            rctItem.left + (rctItem.Width() - strSize.cx) / 2,
            rctItem.top + (rctItem.Height() - strSize.cy) / 2,
            sectionName);
    top += rctItem.Height();

    // now draw all the items in this section
    CImageList & il = Feat::FeatImageList();
    const std::list<Feat>& feats = m_sections[iSection].feats;
    size_t iItemIndex = 0;
    for (auto&& fit: feats)
    {
        rctItem = CRect(0, top, m_headerItemSize.cx, top + m_featItemSize.cy);
        pDC->Draw3dRect(
                rctItem,
                ::GetSysColor(COLOR_BTNHIGHLIGHT),
                ::GetSysColor(COLOR_BTNSHADOW));
        if ((int)iSection == m_selectedSection
                && (int)iItemIndex == m_selectedSectionItem)
        {
            CRect rctInterior(rctItem);
            rctInterior.DeflateRect(1, 1, 1, 1);
            pDC->FillSolidRect(rctInterior, f_selectedColour);
            pDC->SetTextColor(f_white);
        }
        // draw the feat icon
        il.Draw(pDC, fit.FeatImageIndex(), rctItem.TopLeft() + CPoint(1, 1), ILD_NORMAL);
        // now draw the feat name
        strSize = pDC->GetTextExtent(fit.Name().c_str());
        pDC->TextOut(
                rctItem.left + 34,
                rctItem.top + (rctItem.Height() - strSize.cy) / 2,  // centred vertically
                fit.Name().c_str());
        pDC->SetTextColor(f_black);
        top += rctItem.Height();
        ++iItemIndex;
    }
    return top;
}

LRESULT CGrantedFeatListControl::OnMouseLeave(WPARAM, LPARAM)
{
    m_selectedSection = c_noSelection;
    m_selectedSectionItem = c_noSelection;
    Invalidate(FALSE);
    HideTip();
    return 0;
}

void CGrantedFeatListControl::OnMouseMove(UINT nFlags, CPoint point)
{
    // determine which item the mouse is over (if any)
    int oldSection = m_selectedSection;
    int oldSectionItem = m_selectedSectionItem;

    // assume nothing under mouse
    m_selectedSection = c_noSelection;
    m_selectedSectionItem = c_noSelection;
    size_t top = 0;
    CRect rctItemTooltip(0, 0, 0, 0);
    for (size_t iSection = 0; iSection < m_sections.size()
            && m_selectedSection == c_noSelection; ++iSection)
    {
        // header items cannot be selected
        top += m_headerItemSize.cy;
        // now check each item
        for (size_t iSectionItem = 0; iSectionItem < m_sections[iSection].feats.size()
                && m_selectedSection == c_noSelection; ++iSectionItem)
        {
            CRect rctItem(0, top, m_headerItemSize.cx, top + m_featItemSize.cy);
            if (rctItem.PtInRect(point))
            {
                // this is the item we want to select
                m_selectedSection = iSection;
                m_selectedSectionItem = iSectionItem;
                rctItemTooltip = rctItem;
            }
            top += rctItem.Height();
        }
    }
    if (oldSection != m_selectedSection
            || oldSectionItem != m_selectedSectionItem)
    {
        // selection has changed, re-draw
        Invalidate(FALSE);
        HideTip();
        ShowTip(rctItemTooltip);
    }
    // track the mouse so we know when it leaves our window
    TRACKMOUSEEVENT tme;
    tme.cbSize = sizeof(tme);
    tme.hwndTrack = m_hWnd;
    tme.dwFlags = TME_LEAVE;
    tme.dwHoverTime = 1;
    _TrackMouseEvent(&tme);

    __super::OnMouseMove(nFlags, point);
}

void CGrantedFeatListControl::ShowTip(CRect rctItem)
{
    // tip is shown to the left or the right of the highlighted item
    if (m_selectedSection >= 0)
    {
        const std::list<Feat>& feats = m_sections[m_selectedSection].feats;
        std::list<Feat>::const_iterator fit = feats.begin();
        std::advance(fit, m_selectedSectionItem);

        ClientToScreen(rctItem);

        CPoint tipTopLeft(rctItem.left, rctItem.top);
        CPoint tipAlternate(rctItem.right, rctItem.top);
        m_tooltip.SetOrigin(tipTopLeft, tipAlternate, true);
        m_tooltip.SetFeatItem(
                *m_pCharacter->ActiveBuild(),
                &(*fit),
                false,
                0,
                false);
        m_tooltip.Show();
        m_showingTip = true;
    }
}

void CGrantedFeatListControl::HideTip()
{
    // tip not shown if not over a tip item
    if (m_tipCreated && m_showingTip)
    {
        m_tooltip.Hide();
        m_showingTip = false;
    }
}
