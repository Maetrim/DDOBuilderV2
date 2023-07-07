// CustomTabbedPane.cpp
//
#include "stdafx.h"
#include "CustomTabbedPane.h"

//---------------------------------------------------------------------------
#pragma warning(push)
#pragma warning(disable: 4407) // warning C4407: cast between different pointer to member representations, compiler may generate incorrect code
BEGIN_MESSAGE_MAP(CCustomTabbedPane, CTabbedPane)
END_MESSAGE_MAP()
#pragma warning(pop)

IMPLEMENT_SERIAL(CCustomTabbedPane, CTabbedPane, VERSIONABLE_SCHEMA | 2)

//---------------------------------------------------------------------------
CCustomTabbedPane::CCustomTabbedPane() :
    CTabbedPane(),
    m_hIcon(0)
{
}

CCustomTabbedPane::~CCustomTabbedPane()
{
}

void CCustomTabbedPane::DrawCaption(CDC* pDC, CRect rectCaption)
{
    int iTab = m_pTabWnd->GetActiveTab();
    if (iTab >= 0)
    {
        HICON hIcon = m_pTabWnd->GetTabHicon(iTab);
        ::DrawIconEx(
                pDC->GetSafeHdc(),
                rectCaption.left+2,
                rectCaption.top+2,
                hIcon,
                ::GetSystemMetrics(SM_CXSMICON),
                ::GetSystemMetrics(SM_CYSMICON),
                0,
                NULL,
                DI_NORMAL);
    }
    CRect rctIcon(rectCaption);
    rctIcon.bottom += 2;
    rctIcon.right = rctIcon.bottom;
    pDC->Draw3dRect(rctIcon,
                ::GetSysColor(COLOR_BTNHIGHLIGHT),
                ::GetSysColor(COLOR_BTNSHADOW));
    // make sure we don't over draw the icon we just drew
    rectCaption.left += ::GetSystemMetrics(SM_CXSMICON) + 4;
    // let the base class draw the rest
    CTabbedPane::DrawCaption(pDC, rectCaption);
}
