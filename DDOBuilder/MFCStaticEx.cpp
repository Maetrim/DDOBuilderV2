#include "stdafx.h"
#include "MFCStaticEx.h"

BEGIN_MESSAGE_MAP(CMFCStaticEx, CStatic)
    ON_WM_PAINT()
    ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

CMFCStaticEx::CMFCStaticEx() noexcept
{
    m_obTextColour = RGB(0, 0, 0);
}

//---------------------------------------------------------------------------
// ~CMFCStaticEx
//---------------------------------------------------------------------------
CMFCStaticEx::~CMFCStaticEx(
) noexcept
{
}

void CMFCStaticEx::SetTextColour(COLORREF clrText)
{
    m_obTextColour = clrText;
    Invalidate();
}

BOOL CMFCStaticEx::OnEraseBkgnd(
    CDC*
) noexcept
{
    //N.B. TRUE return will get parent to handle background, so it will 
    //     match theme
    return TRUE;
}

void CMFCStaticEx::OnPaint(
) noexcept
{
    CRect obClientRect;
    GetClientRect(&obClientRect);

    CPaintDC obDC(this);
    obDC.SaveDC();
    obDC.SetBkMode(TRANSPARENT);
    obDC.SelectStockObject(DEFAULT_GUI_FONT);
    obDC.SetTextColor(m_obTextColour);
    CString strText;
    GetWindowText(strText);

    if (_T("") != strText)
    {
        DWORD dwSetStyle = DT_VCENTER | DT_WORDBREAK | DT_SINGLELINE;
        DWORD dwRetrievedStyle = GetStyle();
        DWORD dwType = dwRetrievedStyle & SS_TYPEMASK;

        if ((SS_RIGHT & dwType) != 0)
        {
            dwSetStyle |= DT_RIGHT;
        }

        if ((SS_CENTER & dwType) != 0)
        {
            dwSetStyle |= DT_CENTER;
        }

        if ((SS_LEFT & dwType) != 0)
        {
            dwSetStyle |= DT_LEFT;
        }
        obDC.DrawText(strText, obClientRect, dwSetStyle);
    }
    obDC.RestoreDC(-1);
}
