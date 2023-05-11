// SetBonusButton.cpp
//

#include "stdafx.h"
#include "SetBonusButton.h"
#include "Character.h"
#include "GlobalSupportFunctions.h"

namespace
{
    COLORREF f_selectedColor = RGB(128, 0, 0);
}

#pragma warning(push)
#pragma warning(disable: 4407) // warning C4407: cast between different pointer to member representations, compiler may generate incorrect code
BEGIN_MESSAGE_MAP(CSetBonusButton, CStatic)
    //{{AFX_MSG_MAP(CSetBonusButton)
    ON_WM_ERASEBKGND()
    ON_WM_PAINT()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()
#pragma warning(pop)

CSetBonusButton::CSetBonusButton(Character * charData, const SetBonus& setBonus) :
    m_pCharacter(charData),
    m_setBonus(setBonus),
    m_stacks(0)         // stack count updated later
{
    //{{AFX_DATA_INIT(CSetBonusButton)
    //}}AFX_DATA_INIT
    if (S_OK != LoadImageFile("DataFiles\\FiligreeImages\\", m_setBonus.Icon(), &m_image, CSize(32, 32), false))
    {
        if (S_OK != LoadImageFile("DataFiles\\SetBonusImages\\", m_setBonus.Icon(), &m_image, CSize(32, 32), false))
        {
            LoadImageFile("DataFiles\\UIImages\\", "Unknown", &m_image, CSize(32, 32));
        }
    }
    m_image.SetTransparentColor(c_transparentColour);
}

BOOL CSetBonusButton::OnEraseBkgnd(CDC* pDC)
{
    UNREFERENCED_PARAMETER(pDC);
    return 0;
}

void CSetBonusButton::OnPaint()
{
    CPaintDC pdc(this); // validates the client area on destruction
    pdc.SaveDC();

    CRect rect;
    GetWindowRect(&rect);
    rect -= rect.TopLeft(); // convert to client rectangle

    // fill the background
    pdc.FillSolidRect(rect, GetSysColor(COLOR_BTNFACE));
    m_image.TransparentBlt(
            pdc.GetSafeHdc(),
            (rect.Width() - 32) / 2,
            (rect.Height() - 32) / 2,
            32,
            32);
    pdc.RestoreDC(-1);
    // set bonuses debug
    CString stacks;
    stacks.Format("%d", m_stacks);
    pdc.TextOut(0, 0, stacks);
}

const SetBonus& CSetBonusButton::GetSetBonus() const
{
    return m_setBonus;
}

void CSetBonusButton::AddStack()
{
    ++m_stacks;
    // set bonus debug
    if (IsWindow(GetSafeHwnd()))
    {
        Invalidate();
    }
}

void CSetBonusButton::RevokeStack()
{
    --m_stacks;
    // set bonus debug
    if (IsWindow(GetSafeHwnd()))
    {
        Invalidate();
    }
}

size_t CSetBonusButton::NumStacks() const
{
    return m_stacks;
}

bool CSetBonusButton::IsYou(const SetBonus& setBonus)
{
    return (setBonus == m_setBonus);
}

