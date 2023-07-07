// CustomMultiPaneFrameWnd.cpp
//
#include "stdafx.h"
#include "CustomMultiPaneFrameWnd.h"

//---------------------------------------------------------------------------
#pragma warning(push)
#pragma warning(disable: 4407) // warning C4407: cast between different pointer to member representations, compiler may generate incorrect code
BEGIN_MESSAGE_MAP(CCustomMultiPaneFrameWnd, CMultiPaneFrameWnd)
    ON_WM_NCPAINT()
    ON_WM_NCACTIVATE()
END_MESSAGE_MAP()
#pragma warning(pop)

IMPLEMENT_SERIAL(CCustomMultiPaneFrameWnd, CMultiPaneFrameWnd, VERSIONABLE_SCHEMA | 2)

//---------------------------------------------------------------------------
CCustomMultiPaneFrameWnd::CCustomMultiPaneFrameWnd() :
    CMultiPaneFrameWnd()
{
}

CCustomMultiPaneFrameWnd::~CCustomMultiPaneFrameWnd()
{
}

void CCustomMultiPaneFrameWnd::OnNcPaint()
{
    CMultiPaneFrameWnd::OnNcPaint();

    CString strTitle;
    GetWindowText(strTitle);

    CRect rect;
    GetWindowRect(&rect);
    rect -= rect.TopLeft();

    CMFCVisualManager* pManager = CMFCVisualManager::GetInstance();
    COLORREF clrText = pManager->GetCaptionBarTextColor(NULL);

    CDC* pDC = GetWindowDC();
    pDC->SaveDC();
    int uCaptionHeight = ::GetSystemMetrics(SM_CYSIZE);
    COLORREF color = pDC->GetPixel(uCaptionHeight, 4);
    CBrush background;
    background.CreateSolidBrush(color);
    rect += CPoint(2, 4);
    rect.bottom = rect.top + uCaptionHeight - 3;
    rect.right -= 25;
    pDC->FillRect(rect, &background);

    HICON hIcon = GetIcon(FALSE);
    ::DrawIconEx(
            pDC->GetSafeHdc(),
            rect.left + 2,
            rect.top,
            hIcon,
            ::GetSystemMetrics(SM_CXSMICON),
            ::GetSystemMetrics(SM_CYSMICON),
            0,
            NULL,
            DI_NORMAL);
    CRect rctIcon(rect);
    rctIcon.top -= 2;
    rctIcon.bottom = rctIcon.top + ::GetSystemMetrics(SM_CYSMICON) + 4;
    rctIcon.right = rctIcon.bottom;
    pDC->Draw3dRect(rctIcon,
                ::GetSysColor(COLOR_BTNHIGHLIGHT),
                ::GetSysColor(COLOR_BTNSHADOW));
                pDC->SetBkMode(TRANSPARENT);
    CFont* pDefaultGUIFont = CFont::FromHandle(
            (HFONT)GetStockObject(DEFAULT_GUI_FONT));
    pDC->SelectObject(pDefaultGUIFont);
    pDC->SetTextColor(clrText);
    pDC->TextOut(rctIcon.right + 4, rctIcon.top + 4, strTitle);
    pDC->RestoreDC(-1);
    ReleaseDC(pDC);
}

BOOL CCustomMultiPaneFrameWnd::OnNcActivate(BOOL)
{
    RedrawWindow(NULL, NULL, RDW_UPDATENOW);
    return FALSE;
}
