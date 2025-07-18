// DCButton.cpp
//

#include "stdafx.h"
#include "DCButton.h"
#include "Character.h"
#include "GlobalSupportFunctions.h"

#pragma warning(push)
#pragma warning(disable: 4407) // warning C4407: cast between different pointer to member representations, compiler may generate incorrect code
BEGIN_MESSAGE_MAP(CDCButton, CStatic)
    //{{AFX_MSG_MAP(CDCButton)
    ON_WM_ERASEBKGND()
    ON_WM_PAINT()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()
#pragma warning(pop)

CDCButton::CDCButton(Character * charData, const DC & dc) :
    m_pCharacter(charData),
    m_dc(dc),
    m_stacks(0)         // stack count updated later
{
    //{{AFX_DATA_INIT(CDCButton)
    //}}AFX_DATA_INIT
    if (S_OK != LoadImageFile("DataFiles\\EnhancementImages\\", dc.Icon(), &m_image, CSize(32, 32), false))
    {
        // see if its a feat icon we need to use
        if (S_OK != LoadImageFile("DataFiles\\FeatImages\\", dc.Icon(), &m_image, CSize(32, 32), false))
        {
            if (S_OK != LoadImageFile("DataFiles\\ItemImages\\", dc.Icon(), &m_image, CSize(32, 32), false))
            {
                LoadImageFile("DataFiles\\UiImages\\", dc.Icon(), &m_image, CSize(32, 32), true);
            }
        }
    }
    m_image.SetTransparentColor(c_transparentColour);
    m_pCharacter->ActiveBuild()->AttachObserver(this);
}

BOOL CDCButton::OnEraseBkgnd(CDC*)
{
    return 0;
}

void CDCButton::OnPaint()
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
            3,
            3,
            32,
            32);
    // also show the current DC value under the icon
    int dc = m_dc.CalculateDC(m_pCharacter->ActiveBuild());
    CString text;
    text.Format("DC: %d", dc);

    // shown in a small font
    LOGFONT lf;
    ZeroMemory((PVOID)&lf, sizeof(LOGFONT));
    strcpy_s(lf.lfFaceName, "Consolas");
    lf.lfHeight = 11;
    CFont smallFont;
    smallFont.CreateFontIndirect(&lf);
    pdc.SelectObject(&smallFont);

    // measure the text so its centered horizontally
    CSize textSize = pdc.GetTextExtent(text);
    pdc.SetBkMode(TRANSPARENT); // don't erase the text background
    pdc.TextOut(
            rect.left + (rect.Width() - textSize.cx) / 2,
            rect.bottom - 12,
            text);
    // were done, restore the dc to pristine condition
    pdc.RestoreDC(-1);
    // font resource destroyed automatically by destructor
}

const DC & CDCButton::GetDCItem() const
{
    return m_dc;
}

void CDCButton::AddStack()
{
    ++m_stacks;
    if (m_stacks != 1) m_dc.AddStack();
}

void CDCButton::RevokeStack()
{
    --m_stacks;
    if (m_stacks != 1) m_dc.RevokeStack();
}

size_t CDCButton::NumStacks() const
{
    return m_stacks;
}

bool CDCButton::IsYou(const DC & dc)
{
    return (dc == m_dc);
}

void CDCButton::UpdateBuildLevelChanged(Build*)
{
    Invalidate(TRUE);
}

void CDCButton::UpdateClassChanged(Build*, const std::string&, const std::string&, size_t)
{
    Invalidate(TRUE);
}

void CDCButton::UpdateFeatTrained(Build*, const std::string&)
{
    Invalidate(TRUE);
}

void CDCButton::UpdateFeatRevoked(Build*, const std::string&)
{
    Invalidate(TRUE);
}

void CDCButton::UpdateFeatEffectApplied(Build*, const Effect&)
{
    Invalidate(TRUE);
}

void CDCButton::UpdateFeatEffectRevoked(Build*, const Effect&)
{
    Invalidate(TRUE);
}

void CDCButton::UpdateStanceActivated(Build*, const std::string&)
{
    Invalidate(TRUE);
}

void CDCButton::UpdateStanceDeactivated(Build*, const std::string&)
{
    Invalidate(TRUE);
}

void CDCButton::UpdateSkillSpendChanged(Build*, size_t, SkillType)
{
    Invalidate(TRUE);
}

void CDCButton::UpdateAbilityValueChanged(Build*, AbilityType)
{
    Invalidate(TRUE);
}

void CDCButton::UpdateEnhancementTrained(Build*, const EnhancementItemParams&)
{
    Invalidate(TRUE);
}

void CDCButton::UpdateEnhancementRevoked(Build*, const EnhancementItemParams&)
{
    Invalidate(TRUE);
}

void CDCButton::UpdateEnhancementEffectApplied(Build*, const Effect&)
{
    Invalidate(TRUE);
}

void CDCButton::UpdateEnhancementEffectRevoked(Build*, const Effect&)
{
    Invalidate(TRUE);
}

void CDCButton::UpdateItemEffectApplied(Build*, const Effect&)
{
    Invalidate(TRUE);
}

void CDCButton::UpdateItemEffectRevoked(Build*, const Effect&)
{
    Invalidate(TRUE);
}
