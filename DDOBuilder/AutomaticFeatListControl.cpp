// AutomaticFeatListControl.cpp : implementation file
//
#include "stdafx.h"
#include "AutomaticFeatListControl.h"

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

// CAutomaticFeatListControl
IMPLEMENT_DYNAMIC(CAutomaticFeatListControl, CWnd)

CAutomaticFeatListControl::CAutomaticFeatListControl() :
    m_pCharacter(NULL),
    m_headerItemSize(0, 0),
    m_featItemSize(0, 0),
    m_level(c_noSelection),
    m_selectedItem(c_noSelection),
    m_showingTip(false),
    m_tipCreated(false)
{
}

CAutomaticFeatListControl::~CAutomaticFeatListControl()
{
}

void CAutomaticFeatListControl::SetCharacter(Character * pCharacter)
{
    m_pCharacter = pCharacter;
    SetupControl();
    // make sure our parent knows our new size
    CRect rctParent;
    GetParent()->GetClientRect(&rctParent);
    GetParent()->PostMessage(WM_SIZE, SIZE_RESTORED, MAKELONG(rctParent.Width(), rctParent.Height()));
}

#pragma warning(push)
#pragma warning(disable: 4407) // warning C4407: cast between different pointer to member representations, compiler may generate incorrect code
BEGIN_MESSAGE_MAP(CAutomaticFeatListControl, CWnd)
    ON_WM_PAINT()
    ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
    ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()
#pragma warning(pop)

CSize CAutomaticFeatListControl::RequiredSize()
{
    CSize requiredSize(0, 0);
    SetupControl();

    CDC screenDC;
    screenDC.Attach(::GetDC(NULL));
    screenDC.SaveDC();
    screenDC.SelectStockObject(DEFAULT_GUI_FONT);

    // required Width is always the same
    // calculate the width of the control required
    m_headerItemSize = screenDC.GetTextExtent("This is the width of a standard item");
    m_featItemSize = CSize(m_headerItemSize.cx, 34);    // standard icon height + border

    requiredSize.cx = m_headerItemSize.cx;
    requiredSize.cy = m_headerItemSize.cy
            + (m_featItemSize.cy * m_feats.size());
    screenDC.RestoreDC(-1);
    ::ReleaseDC(NULL, screenDC.Detach());
    return requiredSize;
}

void CAutomaticFeatListControl::SetupControl()
{
    if (!m_tipCreated)
    {
        m_tooltip.Create(this);
        m_tipCreated = true;
    }
}

void CAutomaticFeatListControl::SetAutomaticFeats(
        const std::string& label,
        const std::list<TrainedFeat>& feats,
        int level)
{
    SetupControl();
    m_label = label;
    m_feats = feats;
    m_level = level;
    Invalidate(FALSE);
}

void CAutomaticFeatListControl::OnPaint()
{
    CPaintDC dc(this);

    CRect rctWindow;
    GetWindowRect(&rctWindow);
    ScreenToClient(&rctWindow);

    // double buffer drawing to avoid flickering
    CBitmap bitmap;
    VERIFY(bitmap.CreateCompatibleBitmap(
            &dc,
            rctWindow.Width(),
            rctWindow.Height()) != 0);
    // draw to a compatible device context and then splat to screen
    // to avoid flickering of the control
    CDC memoryDc;
    memoryDc.CreateCompatibleDC(&dc);
    memoryDc.SaveDC();
    memoryDc.SelectObject(&bitmap);
    memoryDc.SelectStockObject(DEFAULT_GUI_FONT);
    memoryDc.SetBkMode(TRANSPARENT);

    // fill background
    memoryDc.FillSolidRect(rctWindow, f_backgroundColour);
    memoryDc.Draw3dRect(
            rctWindow,
            ::GetSysColor(COLOR_BTNHIGHLIGHT),
            ::GetSysColor(COLOR_BTNSHADOW));

    m_headerItemSize.cx = rctWindow.Width();
    m_featItemSize.cx = rctWindow.Width();

    // draw the section header item
    int top = 0;
    CRect rctItem(0, top, m_headerItemSize.cx, top + m_headerItemSize.cy);
    memoryDc.Draw3dRect(
            rctItem,
            ::GetSysColor(COLOR_BTNHIGHLIGHT),
            ::GetSysColor(COLOR_BTNSHADOW));
    CSize strSize = memoryDc.GetTextExtent(m_label.c_str());
    memoryDc.TextOut(
            rctItem.left + (rctItem.Width() - strSize.cx) / 2,
            rctItem.top + (rctItem.Height() - strSize.cy) / 2,
            m_label.c_str());
    top += rctItem.Height();

    // now draw all the items in this section
    CImageList & il = Feat::FeatImageList();
    size_t iItemIndex = 0;
    for (auto&& fit: m_feats)
    {
        rctItem = CRect(0, top, m_headerItemSize.cx, top + m_featItemSize.cy);
        memoryDc.Draw3dRect(
                rctItem,
                ::GetSysColor(COLOR_BTNHIGHLIGHT),
                ::GetSysColor(COLOR_BTNSHADOW));
        if ((int)iItemIndex == m_selectedItem)
        {
            CRect rctInterior(rctItem);
            rctInterior.DeflateRect(1, 1, 1, 1);
            memoryDc.FillSolidRect(rctInterior, f_selectedColour);
            memoryDc.SetTextColor(f_white);
        }
        // draw the feat icon
        const Feat& feat = FindFeat(fit.FeatName());
        il.Draw(&memoryDc,
                feat.FeatImageIndex(),
                rctItem.TopLeft() + CPoint(1, 1),
                ILD_NORMAL);
        // now draw the feat name
        strSize = memoryDc.GetTextExtent(fit.FeatName().c_str());
        memoryDc.TextOut(
                rctItem.left + 34,
                rctItem.top + (rctItem.Height() - strSize.cy) / 2,  // centred vertically
                fit.FeatName().c_str());
        memoryDc.SetTextColor(f_black);
        top += rctItem.Height();
        ++iItemIndex;
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


LRESULT CAutomaticFeatListControl::OnMouseLeave(WPARAM, LPARAM)
{
    m_selectedItem = c_noSelection;
    Invalidate(FALSE);
    HideTip();
    return 0;
}

void CAutomaticFeatListControl::OnMouseMove(UINT nFlags, CPoint point)
{
    UNREFERENCED_PARAMETER(nFlags);
    // determine which item the mouse is over (if any)
    int oldSelection = m_selectedItem;

    // assume nothing under mouse
    m_selectedItem = c_noSelection;
    size_t top = 0;
    CRect rctItemTooltip(0, 0, 0, 0);
    // header items cannot be selected
    top += m_headerItemSize.cy;
    // now check each item
    bool found = false;
    size_t itemCount = m_feats.size();
    for (size_t iItem = 0; iItem < itemCount && !found; ++iItem)
    {
        CRect rctItem(0, top, m_featItemSize.cx, top + m_featItemSize.cy);
        if (rctItem.PtInRect(point))
        {
            // this is the item we want to select
            m_selectedItem = iItem;
            rctItemTooltip = rctItem;
            found = true;
        }
        top += rctItem.Height();
    }
    if (oldSelection != m_selectedItem)
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

void CAutomaticFeatListControl::ShowTip(CRect rctItem)
{
    if (m_selectedItem != c_noSelection)
    {
        // tip is shown to the left or the right of the highlighted item
        std::list<TrainedFeat>::const_iterator fit = m_feats.begin();
        std::advance(fit, m_selectedItem);
        const Feat& feat = FindFeat((*fit).FeatName());

        ClientToScreen(rctItem);

        CPoint tipTopLeft(rctItem.left, rctItem.top);
        CPoint tipAlternate(rctItem.right, rctItem.top);
        m_tooltip.SetOrigin(tipTopLeft, tipAlternate, true);
        m_tooltip.SetFeatItem(
                *m_pCharacter->ActiveBuild(),
                &feat,
                false,
                m_level,
                false);
        m_tooltip.Show();
        m_showingTip = true;
    }
}

void CAutomaticFeatListControl::HideTip()
{
    // tip not shown if not over a tip item
    if (m_tipCreated && m_showingTip)
    {
        m_tooltip.Hide();
        m_showingTip = false;
    }
}
