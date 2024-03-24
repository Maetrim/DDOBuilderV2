// StanceButton.cpp
//

#include "stdafx.h"
#include "StanceButton.h"
#include "Character.h"
#include "GlobalSupportFunctions.h"
#include "StancesPane.h"
#include "MainFrm.h"

namespace
{
    COLORREF f_selectedColor = RGB(128, 0, 0);
}

#pragma warning(push)
#pragma warning(disable: 4407) // warning C4407: cast between different pointer to member representations, compiler may generate incorrect code
BEGIN_MESSAGE_MAP(CStanceButton, CStatic)
    //{{AFX_MSG_MAP(CStanceButton)
    ON_WM_ERASEBKGND()
    ON_WM_PAINT()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()
#pragma warning(pop)

CStanceButton::CStanceButton(Character * charData, const Stance & stance) :
    m_pCharacter(charData),
    m_stance(stance),
    m_bSelected(false),
    m_bDisabled(false),
    m_stacks(0)         // stack count updated later
{
    //{{AFX_DATA_INIT(CStanceButton)
    //}}AFX_DATA_INIT
    if (S_OK != LoadImageFile("DataFiles\\FeatImages\\", stance.Icon(), &m_image, CSize(32, 32), false))
    {
        if (S_OK != LoadImageFile("DataFiles\\EnhancementImages\\", stance.Icon(), &m_image, CSize(32, 32), false))
        {
            if (S_OK != LoadImageFile("DataFiles\\UIImages\\", stance.Icon(), &m_image, CSize(32, 32), false))
            {
                LoadImageFile("DataFiles\\UIImages\\", "Unknown", &m_image, CSize(32, 32));
            }
        }
    }
    m_image.SetTransparentColor(c_transparentColour);
    if (stance.HasActiveRequirements())
    {
        Evaluate(charData);
    }
}

void CStanceButton::CreateStanceWindow(CWnd* pParent, UINT nControlId)
{
    CStatic::Create(
            m_stance.Name().c_str(),
            WS_CHILD | WS_VISIBLE | BS_NOTIFY,
            CRect(0, 0, 32, 32),
            pParent,
            nControlId);
    AddStack();
}

const std::string& CStanceButton::Name() const
{
    return m_stance.Name();
}

bool CStanceButton::Evaluate(Character* charData)
{
    bool bChanged = false;
    bool bOldState = m_bSelected;
    bool bOldDisabled = m_bDisabled;
    Build* pBuild = charData->ActiveBuild();
    if (pBuild != NULL)
    {
        if (m_stance.HasActiveRequirements())
        {
            WeaponType wtMainHand = pBuild->MainHandWeapon();
            WeaponType wtOffHand = pBuild->OffhandWeapon();
            bool bState = m_stance.ActiveRequirements().Met(*pBuild, pBuild->Level()-1, false, wtMainHand, wtOffHand);
            if (m_stance.HasGroup()
                && m_stance.Group() == "Auto")
            {
                m_bSelected = bState;
                m_bDisabled = false;
            }
            else
            {
                m_bDisabled = !bState;
                if (m_bDisabled)
                {
                    m_bSelected = false;
                }
            }
            bChanged = (bOldState != m_bSelected)
                    || (bOldDisabled != m_bDisabled);
            if (bChanged)
            {
                CWnd* pWnd = AfxGetApp()->m_pMainWnd;
                CMainFrame* pMainWnd = dynamic_cast<CMainFrame*>(pWnd);
                CStancesPane* pStancesPane = dynamic_cast<CStancesPane*>(pMainWnd->GetPaneView(RUNTIME_CLASS(CStancesPane)));
                StanceGroup* pSG = pStancesPane->GetStanceGroup(m_stance.Group());
                if (m_bSelected)
                    pBuild->ActivateStance(m_stance, pSG);
                else if (m_bDisabled)
                    pBuild->DisableStance(m_stance, pSG);
                else
                    pBuild->DeactivateStance(m_stance, pSG);
            }
        }
    }
    return bChanged;
}

BOOL CStanceButton::OnEraseBkgnd(CDC* pDC)
{
    UNREFERENCED_PARAMETER(pDC);
    return 0;
}

void CStanceButton::OnPaint()
{
    CPaintDC pdc(this); // validates the client area on destruction
    pdc.SaveDC();

    CRect rect;
    GetWindowRect(&rect);
    rect -= rect.TopLeft(); // convert to client rectangle

    // fill the background
    if (m_bDisabled)
    {
        pdc.FillSolidRect(rect, RGB(255, 0, 0));
    }
    else if (m_bSelected)
    {
        pdc.FillSolidRect(rect, GetSysColor(COLOR_HIGHLIGHT));
    }
    else
    {
        pdc.FillSolidRect(rect, GetSysColor(COLOR_BTNFACE));
    }
    m_image.TransparentBlt(
            pdc.GetSafeHdc(),
            (rect.Width() - 32) / 2,
            (rect.Height() - 32) / 2,
            32,
            32);
    pdc.RestoreDC(-1);
    //// stances debug
    //CString stacks;
    //stacks.Format("%d", m_stacks);
    //pdc.TextOut(0, 0, stacks);
}

void CStanceButton::SetSelected(bool selected)
{
    if (selected != m_bSelected)
    {
        m_bSelected = selected;
        Invalidate();   // redraw on state change
    }
}

bool CStanceButton::IsSelected() const
{
    return m_bSelected;
}

bool CStanceButton::IsDisabled() const
{
    return m_bDisabled;
}

const Stance & CStanceButton::GetStance() const
{
    return m_stance;
}

void CStanceButton::AddStack()
{
    ++m_stacks;
    //// stances debug
    //if (IsWindow(GetSafeHwnd()))
    //{
    //    Invalidate();
    //}
}

void CStanceButton::RevokeStack()
{
    --m_stacks;
    //// stances debug
    //if (IsWindow(GetSafeHwnd()))
    //{
    //    Invalidate();
    //}
}

size_t CStanceButton::NumStacks() const
{
    return m_stacks;
}

bool CStanceButton::IsYou(const Stance & stance)
{
    return (stance == m_stance);
}

