// SkillSpendControl.cpp : implementation file
//
#include "stdafx.h"
#include "SkillSpendControl.h"

#include "GlobalSupportFunctions.h"
#include <algorithm>
#include "Resource.h"
#include "Class.h"
#include "Race.h"
#include "BreakdownItem.h"

namespace
{
    COLORREF f_classSkillColour = RGB(152,251,152);             // pale green
    COLORREF f_classSkillColourHighlighted = RGB(52, 151, 52);  // darker green
    COLORREF f_totalSkillColour = RGB(200,200,255);             // pale blue
    COLORREF f_untrainableSkillColour = RGB(0xFF, 0xB6, 0xC1);  // light pink
    COLORREF f_selectedColour = ::GetSysColor(COLOR_HIGHLIGHT);
    COLORREF f_backgroundColour = ::GetSysColor(COLOR_BTNFACE); // grey
    COLORREF f_backgroundColourDark = RGB(83, 83, 83);
    COLORREF f_skillOverspendColour = RGB(0xFF, 0x00, 0x00);    // RED
    COLORREF f_white = RGB(255, 255, 255);                      // white
    COLORREF f_black = RGB(0, 0, 0);                            // black
    const int c_tomeColumn = 999;

    class WeightedSkill
    {
    public:
        WeightedSkill(SkillType skill) :
                m_skill(skill),
                m_weight(0)
            {
            }
        ~WeightedSkill() {};

        void AddWeighting(int amount) { if (amount < 100 || m_weight < 100) m_weight += amount; };
        SkillType Skill() const { return m_skill; };
        int Weight() const { return m_weight; };
    private:
        SkillType m_skill;
        int m_weight;
    };

    bool SortSkillDescending(const WeightedSkill & a, const WeightedSkill & b)
    {
        // sort on skill weighting
        return (a.Weight() > b.Weight());
    }
}

// CSkillSpendControl
IMPLEMENT_DYNAMIC(CSkillSpendControl, CWnd)

CSkillSpendControl::CSkillSpendControl() :
    m_pCharacter(NULL),
    m_selectedRow(c_noSelection),
    m_bSkillSelected(false),
    m_bAutoBuy(false),
    m_bClearSkills(false),
    m_selectedColumn(c_noSelection),
    m_highlightedColumn(c_noSelection),
    m_highlightSkill(Skill_Unknown),
    m_rctSkillNames(0, 0, 0, 0),
    m_rctSkills(0, 0, 0, 0),
    m_rctLevels(0, 0, 0, 0),
    m_rctTomeTitle(0, 0, 0, 0),
    m_rctTomes(0, 0, 0, 0),
    m_rctTotal(0, 0, 0, 0),
    m_rctAutoBuy(0, 0, 0, 0),
    m_rctClearSkills(0, 0, 0, 0),
    m_maxSkillTome(MAX_SKILL_TOME)
{
    // read the max skill tome value from the .ini file
    CSettingsStoreSP regSP;
    CSettingsStore& reg = regSP.Create(FALSE, TRUE);
    if (reg.Open("Settings"))
    {
        int local;
        if (reg.Read("MaxSkillTome", local))
        {
            m_maxSkillTome = (size_t)local;
        }
    }
}

CSkillSpendControl::~CSkillSpendControl()
{
}

void CSkillSpendControl::SetCharacter(Character * pCharacter)
{
    if (m_pCharacter != NULL)
    {
        m_pCharacter->DetachObserver(this);
    }
    m_pCharacter = pCharacter;
    if (m_pCharacter != NULL)
    {
        m_pCharacter->AttachObserver(this);
        // also need to know about any life changes
        Life* pLife = m_pCharacter->ActiveLife();
        if (pLife != NULL)
        {
            pLife->AttachObserver(this);
        }
        // also need to know about any build changes
        Build * pBuild = m_pCharacter->ActiveBuild();
        if (pBuild != NULL)
        {
            pBuild->AttachObserver(this);
        }
    }
    SetupControl();
}

void CSkillSpendControl::LoadComplete()
{
    // start observing skills so we can update the total value column
    for (size_t i = Skill_Unknown + 1; i < Skill_Count; ++i)
    {
        BreakdownType bt = SkillToBreakdown(static_cast<SkillType>(i));
        BreakdownItem* pBreakdown = FindBreakdown(bt);
        if (pBreakdown != NULL)
        {
            pBreakdown->AttachObserver(this);
        }
    }
}

#pragma warning(push)
#pragma warning(disable: 4407) // warning C4407: cast between different pointer to member representations, compiler may generate incorrect code
BEGIN_MESSAGE_MAP(CSkillSpendControl, CWnd)
    ON_WM_PAINT()
    ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
    ON_WM_SIZE()
    ON_WM_MOUSEMOVE()
    ON_WM_LBUTTONUP()
    ON_WM_RBUTTONUP()
END_MESSAGE_MAP()
#pragma warning(pop)

// CSkillSpendControl message handlers
void CSkillSpendControl::OnSize(UINT nType, int cx, int cy)
{
    CWnd::OnSize(nType, cx, cy);
}

CSize CSkillSpendControl::RequiredSize()
{
    CSize requiredSize(0, 0);
    if (m_pCharacter != NULL)
    {
        size_t level = MAX_CLASS_LEVEL;
        if (m_pCharacter->ActiveBuild() != NULL)
        {
            level = m_pCharacter->ActiveBuild()->Level();
            level = min(level, MAX_CLASS_LEVEL);
        }
        SetupControl();

        CDC screenDC;
        screenDC.Attach(::GetDC(NULL));
        screenDC.SaveDC();
        screenDC.SelectStockObject(DEFAULT_GUI_FONT);

        // calculate the width of the control required
        m_skillColumnSize = screenDC.GetTextExtent("  30  ");
        m_skillRanksSize = screenDC.GetTextExtent("  Ranks  ");
        m_skillTomeSize = screenDC.GetTextExtent("  Tome  ");
        m_skillTotalSize = screenDC.GetTextExtent("  Total  ");

        // buffer around text
        m_skillColumnSize.cy += 2;
        m_skillRanksSize.cy += 2;
        m_skillTomeSize.cy += 2;
        m_skillTotalSize.cy += 2;

        CSize maxSkillSize(0, m_skillColumnSize.cy);
        for (size_t i = Skill_Unknown + 1; i < Skill_Count; ++i)
        {
            CString strSkill = EnumEntryText(static_cast<SkillType>(i), skillTypeMap);
            CSize strTextSize = screenDC.GetTextExtent(strSkill + "  ");
            maxSkillSize.cx = max(maxSkillSize.cx, strTextSize.cx);
        }
        m_skillNameSize = maxSkillSize;
        requiredSize.cx = m_skillNameSize.cx
                + (m_skillColumnSize.cx * level)
                + m_skillRanksSize.cx
                + m_skillTomeSize.cx
                + m_skillTotalSize.cx
                + 2;        // right border
        requiredSize.cy = maxSkillSize.cy * 26;
        screenDC.RestoreDC(-1);
        ::ReleaseDC(NULL, screenDC.Detach());
    }
    return requiredSize;
}

void CSkillSpendControl::SetupControl()
{
    // ensure no selection till mouse move
    m_selectedRow = c_noSelection;
    m_selectedColumn = c_noSelection;
    Invalidate(FALSE);
}

void CSkillSpendControl::OnPaint()
{
    bool bDarkMode = DarkModeEnabled();

    CPaintDC dc(this);
    CRect rctWindow;
    GetWindowRect(rctWindow);
    ScreenToClient(rctWindow);

    if (m_skillNameSize == CSize(0, 0))
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

    // fill background
    memoryDc.FillSolidRect(rctWindow, bDarkMode ? f_backgroundColourDark : f_backgroundColour);
    memoryDc.Draw3dRect(
            rctWindow,
            bDarkMode ? ::GetSysColor(COLOR_BTNSHADOW) : ::GetSysColor(COLOR_BTNHIGHLIGHT),
            bDarkMode ? ::GetSysColor(COLOR_BTNHIGHLIGHT) : ::GetSysColor(COLOR_BTNSHADOW));

    if (m_pCharacter != NULL)
    {
        Build* pBuild = m_pCharacter->ActiveBuild();
        if (pBuild != NULL)
        {
            DrawButtons(&memoryDc);
            size_t top = 0;
            // draw the skill points line
            top = DrawAvailableSkillPoints(&memoryDc, top);
            // draw the level line
            top = DrawClassImages(&memoryDc, top);
            // draw the level line
            top = DrawLevelLine(&memoryDc, top);
            // draw all the skill lines
            for (size_t i = Skill_Unknown + 1; i < Skill_Count; ++i)
            {
                top = DrawSkillLine(&memoryDc, static_cast<SkillType>(i), top);
            }
            // draw the level line again
            top = DrawClassImages(&memoryDc, top);
            // finally draw the not spent skill point line
            top = DrawRemainingSkillPoints(&memoryDc, top);

            DrawSelection(&memoryDc);
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

void CSkillSpendControl::DrawButtons(CDC* pDC)
{
    // buttons are in the top left and same width of the skill name column
    size_t third = m_skillNameSize.cy / 3;
    CRect rctButtonItem(0, third, m_skillNameSize.cx, m_skillNameSize.cy + third);
    rctButtonItem.DeflateRect(5, 0, 5, 0);
    m_rctAutoBuy = rctButtonItem;
    DrawButton(pDC, rctButtonItem, "Auto Buy", m_bAutoBuy);
    rctButtonItem += CPoint(0, m_skillNameSize.cy + third);
    m_rctClearSkills = rctButtonItem;
    DrawButton(pDC, rctButtonItem, "Clear Skills", m_bClearSkills);
}

void CSkillSpendControl::DrawButton(CDC* pDC, CRect& rctItem, const CString& text, bool bSelected)
{
    bool bDarkMode = DarkModeEnabled();
    pDC->Draw3dRect(
            rctItem,
            bDarkMode ? ::GetSysColor(COLOR_BTNSHADOW) : ::GetSysColor(COLOR_BTNHIGHLIGHT),
            bDarkMode ? ::GetSysColor(COLOR_BTNHIGHLIGHT) : ::GetSysColor(COLOR_BTNSHADOW));
    CSize strSize = pDC->GetTextExtent(text);
    if (bSelected)
    {
        CRect rctnterior(rctItem);
        rctnterior.DeflateRect(1, 1, 1, 1);
        pDC->FillSolidRect(rctnterior, f_selectedColour);
        pDC->SetTextColor(f_white);
        pDC->TextOut(rctItem.left + (rctItem.Width() - strSize.cx) / 2, rctItem.top + 1, text);
    }
    else
    {
        pDC->SetTextColor(bDarkMode ? f_white : f_black);
        pDC->TextOut(rctItem.left + (rctItem.Width() - strSize.cx) / 2, rctItem.top + 1, text);
    }
}

size_t CSkillSpendControl::DrawAvailableSkillPoints(CDC* pDC, size_t top)
{
    bool bDarkMode = DarkModeEnabled();
    CRect rctLevelItem(0, top, m_skillColumnSize.cx, top + m_skillColumnSize.cy);
    rctLevelItem += CPoint(m_skillNameSize.cx, 0);

    Build* pBuild = m_pCharacter->ActiveBuild();

    // show only heroic class levels
    CString str;
    size_t maxLevel = min(m_pCharacter->ActiveBuild()->Level(), MAX_CLASS_LEVEL);
    for (size_t iLevel = 0; iLevel < maxLevel; ++iLevel)
    {
        size_t skillPoints = pBuild->LevelData(iLevel).SkillPointsAvailable();
        str.Format("%d", skillPoints);
        CSize strSize = pDC->GetTextExtent(str);
        if (m_selectedColumn == static_cast<int>(iLevel))
        {
            pDC->FillSolidRect(rctLevelItem, f_selectedColour);
            pDC->SetTextColor(f_white);
        }
        else
        {
            pDC->SetTextColor(bDarkMode ? f_white : f_black);
        }
        pDC->TextOut(
                rctLevelItem.left + (rctLevelItem.Width() - strSize.cx) / 2,
                rctLevelItem.top + 1,
                str);
        rctLevelItem += CPoint(m_skillColumnSize.cx, 0);
    }
    return (top + m_skillColumnSize.cy);
}

size_t CSkillSpendControl::DrawClassImages(CDC* pDC, size_t top)
{
    IMAGEINFO imageInfo;
    CImageList& il = Class::SmallClassImageList();
    il.GetImageInfo(0, &imageInfo);
    CRect rctImage(imageInfo.rcImage);
    size_t itemHeight = rctImage.Height();

    Build* pBuild = m_pCharacter->ActiveBuild();

    // show only heroic class levels
    size_t maxLevel = min(pBuild->Level(), MAX_CLASS_LEVEL);
    CRect rctLevelItem(0, top, m_skillColumnSize.cx, top + m_skillColumnSize.cy);
    rctLevelItem += CPoint(m_skillNameSize.cx, 0);
    for (size_t iLevel = 0; iLevel < maxLevel; ++iLevel)
    {
        std::string cn = pBuild->LevelData(iLevel).HasClass()
                ? pBuild->LevelData(iLevel).Class()
                : Class_Unknown;
        const Class& c = FindClass(cn);
        il.Draw(
                pDC,
                c.Index(),
                CPoint(rctLevelItem.left + (rctLevelItem.Width() - rctImage.Width()) / 2,
                        rctLevelItem.top),
                ILD_NORMAL);
        rctLevelItem += CPoint(m_skillColumnSize.cx, 0);
    }

    // finally draw the tome icon
    rctLevelItem += CPoint(m_skillRanksSize.cx, 0); // skip Ranks column
    rctLevelItem.right = rctLevelItem.left + m_skillTomeSize.cx;
    if (m_selectedColumn == c_tomeColumn)
    {
        pDC->FillSolidRect(rctLevelItem, f_selectedColour);
    }
    il.Draw(
            pDC,
            Class::TomeImageIndex(),
            CPoint(rctLevelItem.left + (rctLevelItem.Width() - rctImage.Width()) / 2,
                    rctLevelItem.top),
            ILD_NORMAL);

    return (top + itemHeight);
}

size_t CSkillSpendControl::DrawLevelLine(CDC* pDC, size_t top)
{
    bool bDarkMode = DarkModeEnabled();
    CString str;

    // show only heroic class levels
    size_t maxLevel = min(m_pCharacter->ActiveBuild()->Level(), MAX_CLASS_LEVEL);

    // skill name column comes first
    CRect rctLevel(0, top, m_skillNameSize.cx, top + m_skillNameSize.cy);

    // now do the level column headers
    rctLevel.left = m_skillNameSize.cx + 1;
    rctLevel.right = rctLevel.left + m_skillColumnSize.cx;
    m_rctLevels = rctLevel;
    for (size_t iLevel = 0; iLevel < maxLevel; ++iLevel)
    {
        str.Format("%d", iLevel + 1);
        CSize strSize = pDC->GetTextExtent(str);
        pDC->Draw3dRect(
                rctLevel,
                bDarkMode ? ::GetSysColor(COLOR_BTNSHADOW) : ::GetSysColor(COLOR_BTNHIGHLIGHT),
                bDarkMode ? ::GetSysColor(COLOR_BTNHIGHLIGHT) : ::GetSysColor(COLOR_BTNSHADOW));
        if (m_selectedColumn == static_cast<int>(iLevel)
                || m_highlightedColumn == static_cast<int>(iLevel))
        {
            CRect rctInterior(rctLevel);
            rctInterior.DeflateRect(1, 1, 1, 1);
            pDC->FillSolidRect(rctInterior, f_selectedColour);
            pDC->SetTextColor(f_white);
        }
        else
        {
            pDC->SetTextColor(bDarkMode ? f_white : f_black);
        }
        pDC->TextOut(
                rctLevel.left + (rctLevel.Width() - strSize.cx) / 2,
                rctLevel.top + 1,
                str);
        rctLevel += CPoint(m_skillColumnSize.cx, 0);
    }
    m_rctLevels.left = m_skillNameSize.cx + 1;
    m_rctLevels.right = rctLevel.right;

    // next comes the Ranks column
    rctLevel.right = rctLevel.left + m_skillRanksSize.cx;
    pDC->Draw3dRect(
            rctLevel,
            bDarkMode ? ::GetSysColor(COLOR_BTNSHADOW) : ::GetSysColor(COLOR_BTNHIGHLIGHT),
            bDarkMode ? ::GetSysColor(COLOR_BTNHIGHLIGHT) : ::GetSysColor(COLOR_BTNSHADOW));
    pDC->SetTextColor(bDarkMode ? f_white : f_black);
    pDC->TextOut(rctLevel.left, rctLevel.top + 1, "  Ranks  ");
    rctLevel += CPoint(m_skillRanksSize.cx, 0);

    // then the Tome column
    rctLevel.right = rctLevel.left + m_skillTomeSize.cx;
    pDC->Draw3dRect(
            rctLevel,
            bDarkMode ? ::GetSysColor(COLOR_BTNSHADOW) : ::GetSysColor(COLOR_BTNHIGHLIGHT),
            bDarkMode ? ::GetSysColor(COLOR_BTNHIGHLIGHT) : ::GetSysColor(COLOR_BTNSHADOW));
    if (m_selectedColumn == c_tomeColumn)
    {
        CRect rctInterior(rctLevel);
        rctInterior.DeflateRect(1, 1, 1, 1);
        pDC->FillSolidRect(rctInterior, f_selectedColour);
        pDC->SetTextColor(f_white);
    }
    pDC->SetTextColor(bDarkMode ? f_white : f_black);
    pDC->TextOut(rctLevel.left, rctLevel.top + 1, "  Tome  ");
    m_rctTomeTitle = rctLevel;
    rctLevel += CPoint(m_skillTomeSize.cx, 0);

    // last comes the Total column
    rctLevel.right = rctLevel.left + m_skillTotalSize.cx;
    pDC->Draw3dRect(
            rctLevel,
            bDarkMode ? ::GetSysColor(COLOR_BTNSHADOW) : ::GetSysColor(COLOR_BTNHIGHLIGHT),
            bDarkMode ? ::GetSysColor(COLOR_BTNHIGHLIGHT) : ::GetSysColor(COLOR_BTNSHADOW));
    pDC->TextOut(rctLevel.left, rctLevel.top + 1, "  Total  ");
    m_rctTotal = rctLevel;
    return (top + m_skillTotalSize.cy);
}

size_t CSkillSpendControl::DrawSkillLine(CDC* pDC, SkillType skill, size_t top)
{
    bool bDarkMode = DarkModeEnabled();
    CRect rctSkillItem(0, top, m_skillNameSize.cx, top + m_skillNameSize.cy);
    if (skill == Skill_Unknown + 1)
    {
        // this is top left of the skill names rectangle
        m_rctSkillNames.left = rctSkillItem.left;
        m_rctSkillNames.top = rctSkillItem.top;
    }
    if (skill == Skill_Count - 1)
    {
        // this is bottom right of the skill names rectangle
        m_rctSkillNames.right = rctSkillItem.right;
        m_rctSkillNames.bottom = rctSkillItem.bottom;
    }
    CString strSkill = " " + EnumEntryText(skill, skillTypeMap) + " ";
    pDC->Draw3dRect(
            rctSkillItem,
            bDarkMode ? ::GetSysColor(COLOR_BTNSHADOW) : ::GetSysColor(COLOR_BTNHIGHLIGHT),
            bDarkMode ? ::GetSysColor(COLOR_BTNHIGHLIGHT) : ::GetSysColor(COLOR_BTNSHADOW));
    if (skill == m_highlightSkill)
    {
        CRect rctSkillInterior(rctSkillItem);
        rctSkillInterior.DeflateRect(1, 1, 1, 1);
        pDC->FillSolidRect(rctSkillInterior, f_selectedColour);
        pDC->SetTextColor(f_white);
        pDC->TextOut(rctSkillItem.left, rctSkillItem.top + 1, strSkill);
    }
    else
    {
        pDC->SetTextColor(bDarkMode ? f_white : f_black);
        pDC->TextOut(rctSkillItem.left, rctSkillItem.top + 1, strSkill);
    }
    rctSkillItem += CPoint(m_skillNameSize.cx, 0);

    Build* pBuild = m_pCharacter->ActiveBuild();
    size_t maxLevel = min(m_pCharacter->ActiveBuild()->Level(), MAX_CLASS_LEVEL);

    // the background colour of a skill line alternates between
    // white and gray for non-class skills
    bool bGrey = ((skill % 2) != 0);
    if (!bGrey) // default background colour is grey
    {
        rctSkillItem.right = rctSkillItem.left
                + (m_skillColumnSize.cx * maxLevel)
                + m_skillRanksSize.cx
                + m_skillTomeSize.cx
                + m_skillTotalSize.cx;
        pDC->FillSolidRect(rctSkillItem, bDarkMode ? RGB(128, 128, 128)  : f_white);
    }
    // do we have a highlighted column?
    if (m_highlightedColumn >= 0
            && m_highlightedColumn < static_cast<int>(maxLevel))
    {
        CRect rctHighlightedColumn(rctSkillItem);
        rctHighlightedColumn.left = rctSkillItem.left + (m_skillColumnSize.cx * m_highlightedColumn);
        rctHighlightedColumn.right = rctHighlightedColumn.left + m_skillColumnSize.cx;
        pDC->FillSolidRect(rctHighlightedColumn, f_selectedColour);
    }

    if (skill == Skill_Unknown + 1)
    {
        // this is top left of the skills rectangle
        m_rctSkills.left = rctSkillItem.left;
        m_rctSkills.top = rctSkillItem.top;
    }
    // now draw the number of trained ranks in this skill with backgrounds as follows:
    // green - class skill
    // red - untrainable skill
    CString str;
    CSize strSize;
    rctSkillItem.right = rctSkillItem.left + m_skillColumnSize.cx;
    for (size_t iLevel = 0; iLevel < maxLevel; ++iLevel)
    {
        const LevelTraining& lt = pBuild->LevelData(iLevel);
        std::string cn = pBuild->LevelData(iLevel).HasClass()
                ? pBuild->LevelData(iLevel).Class()
                : Class_Unknown;
        const Class& c = FindClass(cn);

        // get the number of skill points spent in all skills at this level
        const std::list<TrainedSkill> & ts = lt.TrainedSkills();
        std::vector<size_t> skillRanks(Skill_Count, 0);
        for (auto&& it: ts)
        {
            skillRanks[it.Skill()]++;
        }

        double skillValue = pBuild->SkillAtLevel(
                skill,
                iLevel,
                false);     // skill tome not included
        double maxSkill = pBuild->MaxSkillForLevel(
                skill,
                iLevel);
        if (maxSkill == 0.0)
        {
            // need a red background for this cell (untrainable)
            pDC->FillSolidRect(rctSkillItem, f_untrainableSkillColour);
        }

        pDC->SetTextColor(bDarkMode ? f_white : f_black);
        bool bClassSkill = c.IsClassSkill(skill);
        str = "";
        if (bClassSkill)
        {
            // need a green background for this cell
            pDC->SetTextColor(f_black);
            if (static_cast<int>(iLevel) == m_highlightedColumn)
            {
                pDC->FillSolidRect(rctSkillItem, f_classSkillColourHighlighted);
            }
            else
            {
                pDC->FillSolidRect(rctSkillItem, f_classSkillColour);
            }
            if (skillRanks[skill] > 0)
            {
                str.Format("%d", skillRanks[skill]);    // full ranks
            }
        }
        else
        {
            // cross class skill points spent here
            int fullRanks = (skillRanks[skill] / 2);
            if (fullRanks > 0)
            {
                str.Format("%d", fullRanks);
            }
            // its a cross class skill, show in multiples of ½
            if (skillRanks[skill] % 2 != 0)
            {
                str += "½";
            }
        }
        strSize = pDC->GetTextExtent(str);
        if (static_cast<int>(iLevel) == m_highlightedColumn)
        {
            // this is the highlighted item
            pDC->SetTextColor(f_white);
        }
        if (skillValue > maxSkill)
        {
            pDC->SetTextColor(f_skillOverspendColour);
        }
        pDC->TextOut(
                rctSkillItem.left + (rctSkillItem.Width() - strSize.cx) / 2,
                rctSkillItem.top + 1,
                str);
        pDC->SetTextColor(f_black);
        // move across for next level
        rctSkillItem += CPoint(rctSkillItem.Width(), 0);
    }
    if (skill == Skill_Count - 1)
    {
        // this is bottom right of the skills rectangle
        m_rctSkills.right = rctSkillItem.left - 1;
        m_rctSkills.bottom = rctSkillItem.bottom;
    }

    // show the total ranks at max heroic class level
    double skillTotal = pBuild->SkillAtLevel(
            skill,
            maxLevel - 1,       // 0 based
            false);
    rctSkillItem.right = rctSkillItem.left + m_skillRanksSize.cx;
    pDC->FillSolidRect(rctSkillItem, f_totalSkillColour);
    str = "";
    if (skillTotal >= 1)
    {
        str.Format("%d", static_cast<int>(skillTotal));
    }
    if ((skillTotal - static_cast<int>(skillTotal)) != 0)
    {
        str += "½";
    }
    strSize = pDC->GetTextExtent(str);
    pDC->TextOut(
            rctSkillItem.left + (rctSkillItem.Width() - strSize.cx) / 2,
            rctSkillItem.top + 1,
            str);
    // move across for tome
    rctSkillItem += CPoint(rctSkillItem.Width(), 0);

    // show the tome level
    rctSkillItem.right = rctSkillItem.left + m_skillTomeSize.cx;
    size_t tome = pBuild->SkillTomeValue(skill, MAX_GAME_LEVEL);
    str.Format("%+d", tome);
    strSize = pDC->GetTextExtent(str);
    pDC->TextOut(
            rctSkillItem.left + (rctSkillItem.Width() - strSize.cx) / 2,
            rctSkillItem.top + 1,
            str);
    if (skill == Skill_Unknown + 1)
    {
        // this is top left of the tomes rectangle
        m_rctTomes.left = rctSkillItem.left;
        m_rctTomes.top = rctSkillItem.top;
    }
    if (skill == Skill_Count - 1)
    {
        // this is bottom right of the tomes rectangle
        m_rctTomes.right = rctSkillItem.right;
        m_rctTomes.bottom = rctSkillItem.bottom;
    }
    rctSkillItem += CPoint(rctSkillItem.Width(), 0);

    // show the skill buffed total
    // show the total ranks at max heroic class level
    BreakdownItem* pSkillBreakdown = FindBreakdown(SkillToBreakdown(skill));
    skillTotal = 0;
    if (pSkillBreakdown != NULL)
    {
        skillTotal = pSkillBreakdown->Total();
    }
    rctSkillItem.right = rctSkillItem.left + m_skillTotalSize.cx;
    pDC->FillSolidRect(rctSkillItem, f_totalSkillColour);
    double maxSkill = pBuild->MaxSkillForLevel(
            skill,
            m_pCharacter->ActiveBuild()->Level());
    if (maxSkill != 0)
    {
        str.Format("%d", static_cast<int>(skillTotal));
    }
    else
    {
        str = "N/A";
    }
    strSize = pDC->GetTextExtent(str);
    pDC->TextOut(
        rctSkillItem.left + (rctSkillItem.Width() - strSize.cx) / 2,
        rctSkillItem.top + 1,
        str);
    return (top + m_skillNameSize.cy);
}

size_t CSkillSpendControl::DrawRemainingSkillPoints(CDC* pDC, size_t top)
{
    bool bDarkMode = DarkModeEnabled();
    pDC->SetTextColor(bDarkMode ? f_white : f_black);
    CRect rctSkillItem(0, top, m_skillNameSize.cx, top + m_skillNameSize.cy);
    pDC->TextOut(rctSkillItem.left, rctSkillItem.top + 1, " Not Spent");
    CRect rctLevelItem(0, top, m_skillColumnSize.cx, top + m_skillColumnSize.cy);
    rctLevelItem += CPoint(m_skillNameSize.cx, 0);

    Build* pBuild = m_pCharacter->ActiveBuild();

    // show only heroic class levels
    CString str;
    size_t maxLevel = min(m_pCharacter->ActiveBuild()->Level(), MAX_CLASS_LEVEL);
    for (size_t iLevel = 0; iLevel < maxLevel; ++iLevel)
    {
        size_t skillPoints = pBuild->LevelData(iLevel).SkillPointsAvailable();
        size_t spentSkillPoints = pBuild->LevelData(iLevel).SkillPointsSpent();
        int availablePoints = (static_cast<int>(skillPoints) - static_cast<int>(spentSkillPoints));
        str.Format("%d", availablePoints);
        CSize strSize = pDC->GetTextExtent(str);
        pDC->SetTextColor(bDarkMode ? f_white : f_black);
        if (m_selectedColumn == static_cast<int>(iLevel))
        {
            pDC->FillSolidRect(rctLevelItem, f_selectedColour);
            pDC->SetTextColor(f_white);
        }
        if (availablePoints < 0)
        {
            pDC->SetTextColor(RGB(255, 0, 0)); // red if over spent
        }
        pDC->TextOut(
                rctLevelItem.left + (rctLevelItem.Width() - strSize.cx) / 2,
                rctLevelItem.top + 1,
                str);
        rctLevelItem += CPoint(m_skillColumnSize.cx, 0);
    }
    pDC->SetTextColor(bDarkMode ? f_white : f_black);
    return (top + m_skillColumnSize.cy);
}

void CSkillSpendControl::DrawSelection(CDC* pDC)
{
    bool bDarkMode = DarkModeEnabled();
    CPen pen(PS_SOLID, 1, bDarkMode ? f_white : f_black);
    pDC->SelectObject(&pen);
// show skill selection (This goes all the way across)
    if (m_selectedRow != c_noSelection)
    {
        // show horizontal bars for this skill selection
        pDC->MoveTo(m_rctSkills.left, m_rctSkills.top + m_skillColumnSize.cy * m_selectedRow);
        pDC->LineTo(m_rctTotal.right, m_rctSkills.top + m_skillColumnSize.cy * m_selectedRow);

        pDC->MoveTo(m_rctSkills.left, m_rctSkills.top + m_skillColumnSize.cy * (m_selectedRow + 1) - 1);
        pDC->LineTo(m_rctTotal.right, m_rctSkills.top + m_skillColumnSize.cy * (m_selectedRow + 1) - 1);
    }
    if (m_selectedColumn != c_noSelection)
    {
        if (m_bSkillSelected)
        {
            // show vertical bars for this skill selection
            pDC->MoveTo(m_rctSkills.left + m_skillColumnSize.cx * m_selectedColumn, 0);
            pDC->LineTo(m_rctSkills.left + m_skillColumnSize.cx * m_selectedColumn,
                    m_rctSkills.bottom + m_skillColumnSize.cy * 2);

            pDC->MoveTo(m_rctSkills.left + m_skillColumnSize.cx * (m_selectedColumn + 1), 0);
            pDC->LineTo(m_rctSkills.left + m_skillColumnSize.cx * (m_selectedColumn + 1),
                    m_rctSkills.bottom + m_skillColumnSize.cy * 2);
        }
        else
        {
            // its the tome line thats selected
            pDC->MoveTo(m_rctTomes.left, m_rctTomes.top);
            pDC->LineTo(m_rctTomes.left, m_rctTomes.bottom);

            pDC->MoveTo(m_rctTomes.right, m_rctTomes.top);
            pDC->LineTo(m_rctTomes.right, m_rctTomes.bottom);
        }
    }
}

LRESULT CSkillSpendControl::OnMouseLeave(WPARAM, LPARAM)
{
    m_selectedRow = c_noSelection;
    m_selectedColumn = c_noSelection;
    m_bSkillSelected = false;
    m_bAutoBuy = false;
    m_bClearSkills = false;
    m_highlightSkill = Skill_Unknown;
    Invalidate(FALSE);
    return 0;
}

void CSkillSpendControl::OnMouseMove(UINT nFlags, CPoint point)
{
    int oldRow = m_selectedRow;
    int oldColumn = m_selectedColumn;
    SkillType lastSkill = m_highlightSkill;
    // determine which item the mouse is over
    if (m_rctLevels.PtInRect(point))
    {
        // its over a level item
        m_bSkillSelected = false;
        m_selectedRow = c_noSelection;
        m_selectedColumn = (point.x - m_rctLevels.left) / m_skillColumnSize.cx;
        m_highlightSkill = Skill_Unknown;
    }
    else if (m_rctTomeTitle.PtInRect(point))
    {
        // its over the tome label item
        m_bSkillSelected = false;
        m_selectedRow = c_noSelection;
        m_selectedColumn = c_tomeColumn;
        m_highlightSkill = Skill_Unknown;
    }
    else if (m_rctSkills.PtInRect(point))
    {
        // its over a skill item
        m_bSkillSelected = true;
        m_selectedRow = (point.y - m_rctSkills.top) / m_skillColumnSize.cy;
        m_selectedColumn = (point.x - m_rctSkills.left) / m_skillColumnSize.cx;
        m_highlightSkill = static_cast<SkillType>(m_selectedRow + 1);
    }
    else if (m_rctTomes.PtInRect(point))
    {
        // its over a tome item
        m_bSkillSelected = false;
        m_selectedRow = (point.y - m_rctTomes.top) / m_skillTomeSize.cy;
        m_selectedColumn = c_tomeColumn;
        m_highlightSkill = static_cast<SkillType>(m_selectedRow + 1);
    }
    else if (m_rctSkillNames.PtInRect(point))
    {
        m_selectedRow = c_noSelection;
        m_selectedColumn = c_noSelection;
        m_bSkillSelected = false;
        int skill = (point.y - m_rctTomes.top) / m_skillTomeSize.cy;
        m_highlightSkill = static_cast<SkillType>(skill + 1);
    }
    else
    {
        // its not over an item
        m_selectedRow = c_noSelection;
        m_selectedColumn = c_noSelection;
        m_highlightSkill = Skill_Unknown;
    }

    // buttons in top left
    bool bOldBuy = m_bAutoBuy;
    bool bOldClear = m_bClearSkills;
    m_bAutoBuy = m_rctAutoBuy.PtInRect(point);
    m_bClearSkills = m_rctClearSkills.PtInRect(point);

    if (oldRow != m_selectedRow
            || oldColumn != m_selectedColumn
            || lastSkill != m_highlightSkill
            || bOldBuy != m_bAutoBuy
            || bOldClear != m_bClearSkills)
    {
        // need to re-draw
        Invalidate(FALSE);
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

void CSkillSpendControl::OnLButtonUp(UINT nFlags, CPoint point)
{
    UNREFERENCED_PARAMETER(nFlags);
    UNREFERENCED_PARAMETER(point);
    // we know which item was clicked on as we have the selected row/column
    // and whether its over a skill or tome
    if (m_pCharacter != NULL)
    {
        Build * pBuild = m_pCharacter->ActiveBuild();
        if (pBuild != NULL)
        {
            if (m_selectedRow == c_noSelection
                    && m_selectedColumn == c_tomeColumn)
            {
                // user is applying a skill tome to all skills
                for (size_t skill = Skill_Unknown + 1; skill < Skill_Count; ++skill)
                {
                    // upgrade the skill tome if we can
                    size_t tome = pBuild->SkillTomeValue((SkillType)skill, MAX_GAME_LEVEL);
                    if (tome < m_maxSkillTome)
                    {
                        ++tome;
                        pBuild->SetSkillTome((SkillType)skill, tome);
                    }
                }
                Invalidate(FALSE);
            }
            else if (m_selectedRow != c_noSelection
                    || m_selectedColumn != c_noSelection)
            {
                SkillType skill = static_cast<SkillType>(m_selectedRow + 1);
                if (m_bSkillSelected)
                {
                    size_t level = m_selectedColumn;
                    // add a skill rank here if:
                    // its trainable and we have skill points to spend
                    double maxSkill = pBuild->MaxSkillForLevel(
                            skill,
                            level);
                    double currentSkill = pBuild->SkillAtLevel(skill, level, false);
                    if (maxSkill > 0.0
                            && currentSkill < maxSkill)
                    {
                        // must have skill points left to spend
                        const LevelTraining & levelData = pBuild->LevelData(level);
                        int available = levelData.SkillPointsAvailable();
                        int spent = levelData.SkillPointsSpent();
                        if (available > spent)
                        {
                            // yes, the user can train this skill
                            pBuild->SpendSkillPoint(level, skill);
                            Invalidate(FALSE);
                        }
                    }
                }
                else
                {
                    if (skill == Skill_Unknown)
                    {
                        // user has clicked a level item, highlight/de-hilight that item
                        if (m_highlightedColumn != m_selectedColumn)
                        {
                            m_highlightedColumn = m_selectedColumn;
                        }
                        else
                        {
                            m_highlightedColumn = c_noSelection;
                        }
                        Invalidate(FALSE);
                    }
                    else
                    {
                        // upgrade the skill tome if we can
                        size_t tome = pBuild->SkillTomeValue(skill, MAX_GAME_LEVEL);
                        if (tome < m_maxSkillTome)
                        {
                            ++tome;
                            pBuild->SetSkillTome(skill, tome);
                            Invalidate(FALSE);
                        }
                    }
                }
            }
            else if (m_highlightSkill != Skill_Unknown)
            {
                // user has clicked on the skill name, max the skill!
                MaxThisSkill(m_highlightSkill);
            }
            else if (m_rctAutoBuy.PtInRect(point))
            {
                AutoBuySkills();
            }
            else if (m_rctClearSkills.PtInRect(point))
            {
                ClearAllSkills();
            }
            // ensure highlight is correct
            OnMouseMove(nFlags, point);
        }
    }
}

void CSkillSpendControl::OnRButtonUp(UINT nFlags, CPoint point)
{
    UNREFERENCED_PARAMETER(nFlags);
    UNREFERENCED_PARAMETER(point);
    // we know which item was clicked on as we have the selected row/column
    // and whether its over a skill or tome
    if (m_pCharacter != NULL)
    {
        Build * pBuild = m_pCharacter->ActiveBuild();
        if (pBuild != NULL)
        {
            if (m_selectedRow == c_noSelection
                    && m_selectedColumn == c_tomeColumn)
            {
                // user is applying a skill tome to all skills
                for (size_t skill = Skill_Unknown + 1; skill < Skill_Count; ++skill)
                {
                    // upgrade the skill tome if we can
                    size_t tome = pBuild->SkillTomeValue((SkillType)skill, MAX_GAME_LEVEL);
                    if (tome > 0)
                    {
                        --tome;
                        pBuild->SetSkillTome((SkillType)skill, tome);
                    }
                }
                Invalidate(FALSE);
            }
            else if (m_selectedRow != c_noSelection
                    || m_selectedColumn != c_noSelection)
            {
                SkillType skill = static_cast<SkillType>(m_selectedRow + 1);
                if (m_bSkillSelected)
                {
                    size_t level = m_selectedColumn;
                    // revoke a rank if they have trained ranks at this level
                    const LevelTraining & levelData = pBuild->LevelData(level);
                    const std::list<TrainedSkill> & trainedSkills = levelData.TrainedSkills();
                    std::list<TrainedSkill>::const_iterator it = trainedSkills.begin();
                    bool hasTrainedRanks = false;
                    while (it != trainedSkills.end())
                    {
                        if ((*it).Skill() == skill)
                        {
                            // has had a spend on this skill at this level
                            hasTrainedRanks = true;
                        }
                        ++it;
                    }
                    if (hasTrainedRanks)
                    {
                        // yes, the user can revoke this skill
                        pBuild->RevokeSkillPoint(level, skill, false);
                        Invalidate(FALSE);
                    }
                }
                else
                {
                    // if possible decrease the skill tome for this skill
                    size_t tome = pBuild->SkillTomeValue(skill, MAX_GAME_LEVEL);
                    if (tome > 0)
                    {
                        --tome;
                        pBuild->SetSkillTome(skill, tome);
                        Invalidate(FALSE);
                    }
                }
            }
            else if (m_highlightSkill != Skill_Unknown)
            {
                // user has clicked on the skill name, clear the skill!
                ClearThisSkill(m_highlightSkill);
            }
            // ensure highlight is correct
            OnMouseMove(nFlags, point);
        }
    }
}

void CSkillSpendControl::UpdateActiveBuildChanged(Character * pCharacter)
{
    SetCharacter(pCharacter);
}

void CSkillSpendControl::UpdateAbilityTomeChanged(Life*, AbilityType)
{
    Invalidate();   // numbers in columns will have changed
}

void CSkillSpendControl::UpdateRaceChanged(Life*, const std::string&)
{
    SetupControl();
}

void CSkillSpendControl::UpdateBuildLevelChanged(Build *)
{
    SetupControl();
}

void CSkillSpendControl::UpdateClassChanged(
        Build *,
        const std::string&,
        const std::string&,
        size_t)
{
    SetupControl();
}

void CSkillSpendControl::UpdateTotalChanged(BreakdownItem*, BreakdownType)
{
    SetupControl();
}

void CSkillSpendControl::MaxThisSkill(SkillType skill)
{
    if (m_pCharacter != NULL)
    {
        Build * pBuild = m_pCharacter->ActiveBuild();
        if (pBuild != NULL)
        {
            for (size_t level = 0; level < pBuild->Level(); ++level)
            {
                bool spent = true;
                while (spent)
                {
                    // try and spend skill points on this skill at this level
                    double maxSkill = pBuild->MaxSkillForLevel(
                            skill,
                            level);
                    double currentSkill = pBuild->SkillAtLevel(skill, level, false);
                    // also ensure we do not overspend a skill across all levels
                    double maxSkillMaxBuildLevel = pBuild->MaxSkillForLevel(
                            skill,
                            min(pBuild->Level()-1, MAX_CLASS_LEVEL-1)); // 0 based
                    double currentSkillMaxBuildLevel = pBuild->SkillAtLevel(
                            skill,
                            min(pBuild->Level()-1, MAX_CLASS_LEVEL-1),
                            false);
                    if (maxSkill > 0.0
                            && currentSkill < maxSkill
                            && currentSkillMaxBuildLevel < maxSkillMaxBuildLevel)
                    {
                        // special case for last class level and a non-class skill
                        // which we do not want to train to 11.5 ranks
                        if (MAX_CLASS_LEVEL % 2 == 0
                                && level == (MAX_CLASS_LEVEL - 1)
                                && maxSkill == (MAX_CLASS_LEVEL + 3) / 2.0
                                && currentSkill == (MAX_CLASS_LEVEL + 2) / 2.0)
                        {
                            // we don't spend for this specific case
                            spent = false;
                        }
                        else
                        {
                            // must have skill points left to spend
                            const LevelTraining & levelData = pBuild->LevelData(level);
                            int available = levelData.SkillPointsAvailable();
                            int spentAtLevel = levelData.SkillPointsSpent();
                            if (available > spentAtLevel)
                            {
                                // yes, the user can train this skill
                                pBuild->SpendSkillPoint(level, skill);
                            }
                            else
                            {
                                spent = false;
                            }
                        }
                    }
                    else
                    {
                        spent = false;
                    }
                }
            }
            Invalidate(FALSE);
        }
    }
}

void CSkillSpendControl::ClearThisSkill(SkillType skill)
{
    if (m_pCharacter != NULL)
    {
        Build * pBuild = m_pCharacter->ActiveBuild();
        if (pBuild != NULL)
        {
            // clear any trained ranks for this skill at all class levels
            for (size_t level = 0; level < pBuild->Level(); ++level)
            {
                const LevelTraining & levelData = pBuild->LevelData(level);
                bool rankRevoked = true;
                while (rankRevoked)
                {
                    rankRevoked = false;
                    // revoke a rank if they have trained ranks at this level
                    const std::list<TrainedSkill> & trainedSkills = levelData.TrainedSkills();
                    std::list<TrainedSkill>::const_iterator it = trainedSkills.begin();
                    while (it != trainedSkills.end())
                    {
                        if ((*it).Skill() == skill)
                        {
                            // has had a spend on this skill at this level
                            rankRevoked = true;
                        }
                        ++it;
                    }
                    if (rankRevoked)
                    {
                        // yes, the user can revoke this skill
                        pBuild->RevokeSkillPoint(level, skill, true);
                    }
                }
            }
            pBuild->SkillsUpdated();
            Invalidate(FALSE);
        }
    }
}

void CSkillSpendControl::AutoBuySkills()
{
    if (m_pCharacter != NULL)
    {
        Build * pBuild = m_pCharacter->ActiveBuild();
        if (pBuild != NULL)
        {
            CWaitCursor wait;
            // to auto spend skill points we first have to rank all the skills
            // into a spend order. This can be tricky if the character is multi-classed
            // first we will create a weighting scheme for all the skills based on the
            // classes trained.
            std::vector<WeightedSkill> skills;
            for (size_t skill = Skill_Unknown; skill < Skill_Count; ++skill)
            {
                skills.push_back(WeightedSkill((SkillType)skill));
            }
            std::vector<size_t> classLevels;
            classLevels = pBuild->ClassLevels(min(pBuild->Level()-1, MAX_CLASS_LEVEL-1));
            for (size_t ci = 0; ci < classLevels.size(); ++ci)
            {
                if (classLevels[ci] > 0)
                {
                    // we have levels trained in this class
                    const Class & c = ClassFromIndex(ci);
                    const std::list<SkillType> & buySkills = c.AutoBuySkill();
                    std::list<SkillType>::const_iterator it = buySkills.begin();
                    while (it != buySkills.end())
                    {
                        skills[(*it)].AddWeighting(100);
                        ++it;
                    }
                }
            }
            // now also include weighting for any Race based skills
            const Race & race = FindRace(pBuild->Race());
            const std::list<SkillType> & buySkills = race.AutoBuySkill();
            std::list<SkillType>::const_iterator it = buySkills.begin();
            while (it != buySkills.end())
            {
                skills[(*it)].AddWeighting(100);
                ++it;
            }
            // now add the basic weights for all skills. This is the order I would
            // take them if I had spare skill points
            skills[Skill_UMD].AddWeighting(21);
            skills[Skill_DisableDevice].AddWeighting(20);
            skills[Skill_OpenLock].AddWeighting(19);
            skills[Skill_Spot].AddWeighting(18);
            skills[Skill_Search].AddWeighting(17);
            skills[Skill_Balance].AddWeighting(16);
            skills[Skill_Swim].AddWeighting(15);
            skills[Skill_Bluff].AddWeighting(14);
            skills[Skill_Diplomacy].AddWeighting(13);
            skills[Skill_Haggle].AddWeighting(12);
            skills[Skill_Concentration].AddWeighting(11);
            skills[Skill_Heal].AddWeighting(10);
            skills[Skill_Hide].AddWeighting(9);
            skills[Skill_Intimidate].AddWeighting(8);
            skills[Skill_MoveSilently].AddWeighting(7);
            skills[Skill_Perform].AddWeighting(6);
            skills[Skill_Jump].AddWeighting(5);
            skills[Skill_SpellCraft].AddWeighting(4);
            skills[Skill_Repair].AddWeighting(3);
            skills[Skill_Tumble].AddWeighting(2);
            skills[Skill_Listen].AddWeighting(1);
            // now sort into decreasing order and then spend the skill points
            std::sort(skills.begin(), skills.end(), SortSkillDescending);
            // now we have the skills sorted, try and spend on each in order
            for (size_t i = 0; i < skills.size(); ++i)
            {
                MaxThisSkill(skills[i].Skill());
            }
            Invalidate(FALSE);
            pBuild->SkillsUpdated();
        }
    }
}

void CSkillSpendControl::ClearAllSkills()
{
    if (m_pCharacter != NULL)
    {
        Build * pBuild = m_pCharacter->ActiveBuild();
        if (pBuild != NULL)
        {
            // clear any trained ranks for all skills at all class levels
            for (size_t level = 0; level < pBuild->Level(); ++level)
            {
                const LevelTraining & levelData = pBuild->LevelData(level);
                bool rankRevoked = true;
                while (rankRevoked)
                {
                    rankRevoked = false;
                    // revoke all trained ranks for all skills at this level
                    std::list<TrainedSkill> trainedSkills = levelData.TrainedSkills();
                    std::list<TrainedSkill>::const_iterator it = trainedSkills.begin();
                    while (it != trainedSkills.end())
                    {
                        // don't re-evaluate feats etc on each skill point revoke
                        pBuild->RevokeSkillPoint(level, (*it).Skill(), true);
                        ++it;
                    }
                }
            }
            pBuild->SkillsUpdated();
            Invalidate(FALSE);
        }
    }
}
