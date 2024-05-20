// FavorProgressBar.cpp
#include "stdafx.h"
#include "FavorProgressBar.h"
#include "GlobalSupportFunctions.h"
#include <algorithm>

/////////////////////////////////////////////////////////////////////////////
// CFavorProgressBar
IMPLEMENT_DYNAMIC(CFavorProgressBar, CWnd)

CFavorProgressBar::CFavorProgressBar() :
    m_maxFavor(0),
    m_currentFavor(0),
    m_maxValue(0),
    m_favorHeight(0),
    m_selectedItem(c_noSelection),
    m_showingTip(false),
    m_tipCreated(false)
{
}

CFavorProgressBar::~CFavorProgressBar()
{
}

void CFavorProgressBar::SetPatronName(const CString& patronName)
{
    m_patronName = patronName;
    if (!m_tipCreated)
    {
        m_tooltip.Create(this);
        m_tipCreated = true;
    }
}

void CFavorProgressBar::SetFavorTiers(
    const std::vector<int>& favorTiers,
    int maxFavor)
{
    m_favorTiers = favorTiers;
    m_maxFavor = maxFavor;
    m_maxValue = max(m_maxFavor, *std::max_element(m_favorTiers.begin(), m_favorTiers.end()));
    m_currentFavor = 0;
}

void CFavorProgressBar::SetCurrentFavor(int currentFavor)
{
    m_currentFavor = currentFavor;
}

BEGIN_MESSAGE_MAP(CFavorProgressBar, CWnd)
    //{{AFX_MSG_MAP(CFavorProgressBar)
    ON_WM_PAINT()
    ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
    ON_WM_MOUSEMOVE()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CFavorProgressBar message handlers

void CFavorProgressBar::OnPaint() 
{
    CDC* pDC = GetDC();
    pDC->SaveDC();

    CRect rect;
    GetClientRect(&rect);
    CBrush brush(GetSysColor(COLOR_HIGHLIGHT));
    pDC->FillRect(rect, &brush);

    // stop text drawing beyond the control edge on a resize
    CRect rctAreaRight(rect);
    rctAreaRight += CPoint(rect.Width(), 0);
    pDC->ExcludeClipRect(rctAreaRight);

    // favor tiers shown in small font
    LOGFONT lf;
    ZeroMemory((PVOID)&lf, sizeof(LOGFONT));
    strcpy_s(lf.lfFaceName, "Consolas");
    lf.lfHeight = 9;
    CFont smallFont;
    smallFont.CreateFontIndirect(&lf);
    pDC->SelectObject(&smallFont);
    pDC->SetBkMode(TRANSPARENT); // don't erase the text background
    pDC->SetTextColor(COLORREF(RGB(255, 255, 255)));   // white
    CPen penWhite(PS_SOLID, 1, COLORREF(RGB(255, 255, 255)));
    pDC->SelectObject(&penWhite);

    m_favorTiersPoints.clear();
    CRect rectRed(rect);
    CRect rectWhite(rect);
    for (size_t tier = 0; tier < m_favorTiers.size(); ++tier)
    {
        CString tierText;
        tierText.Format("%d", m_favorTiers[tier]);
        // measure the text so its centered horizontally
        CSize textSize = pDC->GetTextExtent(tierText);
        int xPos = ((rect.Width() * m_favorTiers[tier]) / m_maxValue);
        pDC->MoveTo(xPos, 0);
        pDC->LineTo(xPos, 2);
        m_favorTiersPoints.push_back(xPos);
        // now position the text
        xPos -= textSize.cx / 2;
        // stop going off left/right hand sides
        if (xPos < 0) xPos = 0;
        else if (xPos + textSize.cx > rect.Width()) xPos = rect.right - textSize.cx - 1;
        pDC->TextOut(xPos, 0, tierText);
        rectWhite.top = textSize.cy;
        rectRed.top = textSize.cy;
    }
    m_favorHeight = rectWhite.top;

    int xCF = ((rect.Width() * m_currentFavor) / m_maxValue);
    rectRed.right = xCF - 1;
    if (rectRed.right > 0)
    {
        CBrush brushRed(COLORREF(RGB(255, 0, 0)));
        pDC->FillRect(rectRed, &brushRed);
    }
    CBrush brush2(GetSysColor(COLOR_BTNFACE));
    rectWhite.left = xCF;
    pDC->FillRect(rectWhite, &brush2);

    CPen pen(PS_SOLID, 1, COLORREF(RGB(0, 0, 0)));
    pDC->SelectObject(pen);
    pDC->MoveTo(xCF-1, rectWhite.top);
    pDC->LineTo(xCF-1, rectWhite.bottom + 1);

    CFont* pDefaultGUIFont = CFont::FromHandle(
            (HFONT)GetStockObject(DEFAULT_GUI_FONT));
    pDC->SelectObject(pDefaultGUIFont);
    CString favorText;
    favorText.Format("%s (%d / %d)", (LPCTSTR)m_patronName, m_currentFavor, m_maxValue);
    CSize textSize = pDC->GetTextExtent(favorText);
    // one colour text for the unselected background
    pDC->SetTextColor(COLORREF(RGB(0, 0, 0)));
    pDC->TextOut((rect.Width() - textSize.cx) / 2, rect.bottom - textSize.cy, favorText);
    // highlighted text colour for the selected background
    pDC->ExcludeClipRect(rectWhite);
    pDC->SetTextColor(GetSysColor(COLOR_HIGHLIGHTTEXT));
    pDC->TextOut((rect.Width() - textSize.cx) / 2, rect.bottom - textSize.cy, favorText);

    pDC->RestoreDC(-1);
    ReleaseDC(pDC);
    ValidateRect(NULL);
}

LRESULT CFavorProgressBar::OnMouseLeave(WPARAM, LPARAM)
{
    m_selectedItem = c_noSelection;
    Invalidate(FALSE);
    HideTip();
    return 0;
}

void CFavorProgressBar::OnMouseMove(UINT nFlags, CPoint point)
{
    UNREFERENCED_PARAMETER(nFlags);
    // determine which item the mouse is over (if any)
    int oldSelection = m_selectedItem;

    // assume nothing under mouse
    m_selectedItem = c_noSelection;
    CRect rctItemTooltip(0, 0, 0, 0);

    for (size_t index = 0; index < m_favorTiersPoints.size(); ++index)
    {
        int xDistance = abs(point.x - m_favorTiersPoints[index]);
        if (xDistance <= 3)
        {
            m_selectedItem = index;
        }
    }
    if (m_selectedItem != c_noSelection)
    {
        // make sure we match the correct y location also
        if (point.y > m_favorHeight)
        {
            // not in the correct location
            m_selectedItem = c_noSelection;
        }
        else
        {
            // make sure the tooltip appears in the correct location
            CRect rctWindow;
            GetWindowRect(rctWindow);
            rctItemTooltip += CPoint(m_favorTiersPoints[m_selectedItem], rctWindow.Height());
        }
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

void CFavorProgressBar::ShowTip(CRect rctItem)
{
    // tip is shown under the favor bar
    if (m_selectedItem >= 0)
    {
        ClientToScreen(rctItem);
        const Patron& patron = FindPatron((LPCTSTR)m_patronName);

        CPoint tipTopLeft(rctItem.left, rctItem.top);
        CPoint tipAlternate(rctItem.right, rctItem.top);
        m_tooltip.SetOrigin(tipTopLeft, tipAlternate, true);
        m_tooltip.SetFavorItem(patron, m_selectedItem);
        m_tooltip.Show();
        m_showingTip = true;
    }
}

void CFavorProgressBar::HideTip()
{
    // tip not shown if not over a tip item
    if (m_tipCreated && m_showingTip)
    {
        m_tooltip.Hide();
        m_showingTip = false;
    }
}
