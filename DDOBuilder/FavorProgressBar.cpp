// FavorProgressBar.cpp
#include "stdafx.h"
#include "FavorProgressBar.h"
#include <algorithm>

/////////////////////////////////////////////////////////////////////////////
// CFavorProgressBar
IMPLEMENT_DYNAMIC(CFavorProgressBar, CWnd)

CFavorProgressBar::CFavorProgressBar() :
    m_maxFavor(0),
    m_currentFavor(0),
    m_maxValue(0)
{
}

CFavorProgressBar::~CFavorProgressBar()
{
}

void CFavorProgressBar::SetPatronName(const CString& patronName)
{
    m_patronName = patronName;
}

void CFavorProgressBar::SetFavorTiers(
    const std::vector<int>& favorTiers,
    int maxFavor)
{
    m_favorTiers = favorTiers;
    m_maxFavor = maxFavor;
    m_maxValue = max(m_maxFavor, *std::max_element(m_favorTiers.begin(), m_favorTiers.end()));

    m_currentFavor = (rand() % m_maxValue);
}

void CFavorProgressBar::SetCurrentFavor(int currentFavor)
{
    m_currentFavor = currentFavor;
}

BEGIN_MESSAGE_MAP(CFavorProgressBar, CWnd)
    //{{AFX_MSG_MAP(CFavorProgressBar)
    ON_WM_PAINT()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CFavorProgressBar message handlers

void CFavorProgressBar::OnPaint() 
{
    CPaintDC dc(this); // device context for painting
    dc.SaveDC();
    CRect rect;
    GetClientRect(&rect);
    CBrush brush(GetSysColor(COLOR_HIGHLIGHT));
    dc.FillRect(rect, &brush);

    // favor tiers shown in small font
    LOGFONT lf;
    ZeroMemory((PVOID)&lf, sizeof(LOGFONT));
    strcpy_s(lf.lfFaceName, "Consolas");
    lf.lfHeight = 9;
    CFont smallFont;
    smallFont.CreateFontIndirect(&lf);
    dc.SelectObject(&smallFont);
    dc.SetBkMode(TRANSPARENT); // don't erase the text background
    dc.SetTextColor(COLORREF(RGB(255, 255, 255)));   // white
    CPen penWhite(PS_SOLID, 1, COLORREF(RGB(255, 255, 255)));
    dc.SelectObject(&penWhite);

    CRect rect2(rect);
    for (size_t tier = 0; tier < m_favorTiers.size(); ++tier)
    {
        CString tierText;
        tierText.Format("%d", m_favorTiers[tier]);
        // measure the text so its centered horizontally
        CSize textSize = dc.GetTextExtent(tierText);
        int xPos = ((rect.Width() * m_favorTiers[tier]) / m_maxValue);
        dc.MoveTo(xPos, 0);
        dc.LineTo(xPos, 2);
        // now position the text
        xPos -= textSize.cx / 2;
        // stop going off left/right hand sides
        if (xPos < 0) xPos = 0;
        else if (xPos + textSize.cx > rect.Width()) xPos = rect.right - textSize.cx - 1;
        dc.TextOut(xPos, 0, tierText);
        rect2.top = textSize.cy;
    }

    int xCF = ((rect.Width() * m_currentFavor) / m_maxValue);
    CBrush brush2(GetSysColor(COLOR_BTNFACE));
    rect2.left = xCF;
    dc.FillRect(rect2, &brush2);

    CFont* pDefaultGUIFont = CFont::FromHandle(
            (HFONT)GetStockObject(DEFAULT_GUI_FONT));
    dc.SelectObject(pDefaultGUIFont);
    CString favorText;
    favorText.Format("%s (%d / %d)", (LPCTSTR)m_patronName, m_currentFavor, m_maxValue);
    CSize textSize = dc.GetTextExtent(favorText);
    // one colour text for the unselected background
    dc.SetTextColor(COLORREF(RGB(0, 0, 0)));
    dc.TextOut((rect.Width() - textSize.cx) / 2, rect.bottom - textSize.cy, favorText);
    // highlighted text colour for the selected background
    dc.ExcludeClipRect(rect2);
    dc.SetTextColor(GetSysColor(COLOR_HIGHLIGHTTEXT));
    dc.TextOut((rect.Width() - textSize.cx) / 2, rect.bottom - textSize.cy, favorText);

    dc.RestoreDC(-1);
}
