// FeatsClassControl.cpp : implementation file
//
#include "stdafx.h"
#include "FeatsClassControl.h"

#include "AutomaticFeatsPane.h"
#include "GlobalSupportFunctions.h"
#include <algorithm>
#include "Class.h"
#include "Feat.h"
#include "Race.h"
#include "Resource.h"
#include "MainFrm.h"

// CFeatsClassControl
namespace
{
    const int c_FeatColumnWidth = 200;      // pixels
    const int c_dudLevel = MAX_BUILDER_LEVEL + 1;
}

IMPLEMENT_DYNAMIC(CFeatsClassControl, CWnd)

CFeatsClassControl::CFeatsClassControl() :
    m_pCharacter(NULL),
    m_bMenuDisplayed(false),
    m_maxRequiredFeats(0),
    m_numClassColumns(0),
    m_bUpdatePending(false),
    m_highlightedLevelLine(c_dudLevel),   // starts invalid
    m_lastNotifiedLevelLine(c_dudLevel),   // starts invalid
    m_showingTip(false),
    m_tipCreated(false),
    m_tooltipItem(HT_None, CRect(0, 0, 0, 0), 0, 0),
    m_featSelectItem(HT_None, CRect(0, 0, 0, 0), 0, 0)
{
}

CFeatsClassControl::~CFeatsClassControl()
{
}

void CFeatsClassControl::SetCharacter(Character * pCharacter)
{
    m_hitChecks.clear();
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

#pragma warning(push)
#pragma warning(disable: 4407) // warning C4407: cast between different pointer to member representations, compiler may generate incorrect code
BEGIN_MESSAGE_MAP(CFeatsClassControl, CWnd)
    ON_WM_PAINT()
    ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
    ON_WM_SIZE()
    ON_WM_MOUSEMOVE()
    ON_WM_LBUTTONUP()
    ON_REGISTERED_MESSAGE(UWM_UPDATE, UpdateControl)
    ON_CBN_SELENDOK(IDC_COMBO_FEATSELECT, OnFeatSelectOk)
    ON_CBN_SELENDCANCEL(IDC_COMBO_FEATSELECT, OnFeatSelectCancel)
    ON_MESSAGE(WM_MOUSEHOVER, OnHoverComboBox)
END_MESSAGE_MAP()
#pragma warning(pop)

// CFeatsClassControl message handlers
void CFeatsClassControl::OnSize(UINT nType, int cx, int cy)
{
    CWnd::OnSize(nType, cx, cy);
}

CSize CFeatsClassControl::RequiredSize()
{
    CSize requiredSize(0, 0);
    if (m_pCharacter != NULL)
    {
        SetupControl();

        CDC screenDC;
        screenDC.Attach(::GetDC(NULL));
        screenDC.SaveDC();
        screenDC.SelectStockObject(DEFAULT_GUI_FONT);

        // calculate the width of the control required
        CSize levelCol = screenDC.GetTextExtent(" 30 ");
        levelCol.cy = max(20, levelCol.cy); // min height required for icon + border
        // blank item in top left
        CRect rctItem(0, 0, levelCol.cx, levelCol.cy);
        m_levelRect = rctItem;
        int width = m_levelRect.Width();
        for (size_t i = 0; i < m_numClassColumns; ++i)
        {
            width += 36;  // 36 pixels per class rect
        }
        for (size_t i = 0; i < m_maxRequiredFeats; ++i)
        {
            width += c_FeatColumnWidth;
        }
        // additional columns for stats [bab][str][dex][con][int][wis][cha]
        for (size_t stats = Ability_Unknown; stats < Ability_Count; ++stats)
        {
            CString strText;
            if (stats != Ability_Unknown)
            {
                strText.Format(" %3.3s ",
                        (LPCTSTR)EnumEntryText((AbilityType)stats, abilityTypeMap));
            }
            else
            {
                strText = " BAB ";
            }
            strText = " " + strText + " ";
            CSize csText = screenDC.GetTextExtent(strText);
            width += (csText.cx + 1);
        }
        int height = m_levelRect.Height();
        size_t maxLevel = (m_pCharacter->ActiveBuild() != NULL)
                ? m_pCharacter->ActiveBuild()->Level()
                : 0;
        for (size_t level = 0; level < maxLevel; ++level)
        {
            bool bFeatsAvailable = (m_availableFeats[level].size() > 0);
            if (bFeatsAvailable)
            {
                // 36 pixels for height for this item
                height += 36;
            }
            else
            {
                height += 21;
            }
        }
        requiredSize.cx = width;
        requiredSize.cy = height;
        screenDC.RestoreDC(-1);
        ::ReleaseDC(NULL, screenDC.Detach());
    }
    return requiredSize;
}

void CFeatsClassControl::SetupControl()
{
    if (!m_tipCreated)
    {
        m_tooltip.Create(this);
        m_tipCreated = true;
    }
    if (!IsWindow(m_featSelector.GetSafeHwnd()))
    {
        m_featSelector.Create(
                WS_CHILD | WS_VSCROLL | WS_TABSTOP
                | CBS_DROPDOWNLIST | CBS_OWNERDRAWVARIABLE | CBS_HASSTRINGS,
                CRect(77,36,119,250),
                this,
                IDC_COMBO_FEATSELECT);
        CFont* pDefaultGUIFont = CFont::FromHandle(
                (HFONT)GetStockObject(DEFAULT_GUI_FONT));
        m_featSelector.SetFont(pDefaultGUIFont, TRUE);
    }
    m_availableFeats.clear();
    m_maxRequiredFeats = 0;
    m_numClassColumns = 0;
    if (m_pCharacter != NULL)
    {
        Build * pBuild = m_pCharacter->ActiveBuild();
        if (pBuild != NULL)
        {
            // work out how many class select columns we need
            m_numClassColumns++;    // always at least 1 class column
            if (pBuild->Class1() != Class_Unknown)
            {
                m_numClassColumns++;        // need to show class 2 column
                if (pBuild->Class2() != Class_Unknown)
                {
                    m_numClassColumns++;        // need to show class 3 column
                }
            }
            // work out the available feat slots at each level
            size_t maxLevel = (m_pCharacter->ActiveBuild() != NULL)
                    ? m_pCharacter->ActiveBuild()->Level()
                    : 0;
            for (size_t level = 0; level < maxLevel; ++level)
            {
                std::vector<FeatSlot> availableFeatSlots =
                        pBuild->TrainableFeatTypeAtLevel(level);
                m_availableFeats.push_back(availableFeatSlots);
                m_maxRequiredFeats = max(m_maxRequiredFeats, availableFeatSlots.size());
            }
        }
    }
    Invalidate(TRUE);
}

void CFeatsClassControl::OnPaint()
{
    CPaintDC dc(this);
    CRect rctWindow;
    GetWindowRect(rctWindow);
    ScreenToClient(rctWindow);

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

    CBrush selectedBackgroundBrush;
    CBrush normalBackgroundBrush;
    selectedBackgroundBrush.CreateSolidBrush(::GetSysColor(COLOR_HIGHLIGHT));
    normalBackgroundBrush.CreateSolidBrush(::GetSysColor(COLOR_BTNFACE));

    // fill background
    memoryDc.FillRect(rctWindow, &normalBackgroundBrush);
    memoryDc.Draw3dRect(
            rctWindow,
            ::GetSysColor(COLOR_BTNHIGHLIGHT),
            ::GetSysColor(COLOR_BTNSHADOW));

    if (m_pCharacter != NULL)
    {
        // the height of all other items depends on whether there are feat
        // selections available for that level.
        size_t top = DrawTopLine(&memoryDc);
        size_t maxLevel = (m_pCharacter->ActiveBuild() != NULL)
                ? m_pCharacter->ActiveBuild()->Level()
                : 0;
        for (size_t level = 0; level < maxLevel; ++level)
        {
            if (m_availableFeats.size() > level)
            {
                m_levelRects[level] = CRect(0, top, rctWindow.right, top);
                top = DrawLevelLine(&memoryDc, top, level);
                m_levelRects[level].bottom = top - 1;
            }
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

size_t CFeatsClassControl::DrawTopLine(CDC * pDC)
{
    m_hitChecks.clear();

    CBrush selectedBackgroundBrush;
    CBrush normalBackgroundBrush;
    selectedBackgroundBrush.CreateSolidBrush(::GetSysColor(COLOR_HIGHLIGHT));
    normalBackgroundBrush.CreateSolidBrush(::GetSysColor(COLOR_BTNFACE));

    // [][class1^][class2^][class3^][Feat1    ][Feat2    ][Feat3    ][str][dex][con][int][wis][cha][BAB]
    // level column, get its max width
    CSize levelCol = pDC->GetTextExtent(" 30 ");
    levelCol.cy = max(20, levelCol.cy); // min height required for icon + border
    // blank item in top left
    CRect rctItem(0, 0, levelCol.cx, levelCol.cy);
    m_levelRect = rctItem;
    CPoint pos(rctItem.right + 4, 2);
    // class columns (variable)
    for (size_t cc = 0; cc < m_numClassColumns; ++cc)
    {
        rctItem.left = rctItem.right + 1;
        rctItem.right = rctItem.left + 35;
        pDC->Draw3dRect(
                rctItem,
                ::GetSysColor(COLOR_BTNHIGHLIGHT),
                ::GetSysColor(COLOR_BTNSHADOW));
        const std::string& ct = (m_pCharacter->ActiveBuild() != NULL)
                ? m_pCharacter->ActiveBuild()->Class(cc)
                : "Unknown";
        Class::SmallClassImageList().Draw(pDC, ClassIndex(ct), pos, ILD_TRANSPARENT);
        CRect rctDropArrow(pos.x, pos.y, pos.x + 12, pos.y + 12);
        rctDropArrow += CPoint(18, 2);
        pDC->DrawFrameControl(rctDropArrow, DFC_SCROLL, DFCS_SCROLLDOWN);
        HitCheckItem classDropArrow((HitType)(HT_Class1 + cc), rctItem, 0, 0);
        m_hitChecks.push_back(classDropArrow);
        pos.x += rctItem.Width();
        m_classRects[cc] = rctItem; // copy for later use
    }
    // feat columns (variable)
    for (size_t fc = 0; fc < m_maxRequiredFeats; ++fc)
    {
        rctItem.left = rctItem.right + 1;
        rctItem.right = rctItem.left + c_FeatColumnWidth;
        pDC->Draw3dRect(
                rctItem,
                ::GetSysColor(COLOR_BTNHIGHLIGHT),
                ::GetSysColor(COLOR_BTNSHADOW));
        CSize csText = pDC->GetTextExtent("1"); // just need the height of text
        pDC->TextOut(
                rctItem.left,
                rctItem.top + (rctItem.Height() - csText.cy) / 2,
                " Feat Selection");
        pos.x += rctItem.Width();
        m_featRects[fc] = rctItem;      // copy for later use
    }
    // additional columns for stats [str][dex][con][int][wis][cha][bab]
    for (size_t stats = Ability_Unknown; stats < Ability_Count; ++stats)
    {
        CString strText;
        if (stats != Ability_Unknown)
        {
            strText.Format(" %3.3s ",
                    (LPCTSTR)EnumEntryText((AbilityType)stats, abilityTypeMap));
        }
        else
        {
            strText = " BAB ";
        }
        strText = " " + strText + " ";
        CSize csText = pDC->GetTextExtent(strText);
        rctItem.left = rctItem.right + 1;
        rctItem.right = rctItem.left + csText.cx;
        pDC->Draw3dRect(
                rctItem,
                ::GetSysColor(COLOR_BTNHIGHLIGHT),
                ::GetSysColor(COLOR_BTNSHADOW));
        pDC->TextOut(
                rctItem.left,
                rctItem.top + (rctItem.Height() - csText.cy) / 2,
                strText);
        pos.x += rctItem.Width();
        m_statRects[stats] = rctItem;      // copy for later use
    }
    return levelCol.cy + 1;
}

size_t CFeatsClassControl::DrawLevelLine(
        CDC * pDC,
        size_t top,
        size_t level)
{
    // switch the fill colour and text colour if this is the highlighted line
    CBrush fillBrush;
    if (level == m_highlightedLevelLine)
    {
        pDC->SetTextColor(::GetSysColor(COLOR_HIGHLIGHTTEXT));
        fillBrush.CreateSolidBrush(::GetSysColor(COLOR_HIGHLIGHT));
    }
    else
    {
        pDC->SetTextColor(RGB(0, 0, 0));  // black text
        fillBrush.CreateSolidBrush(::GetSysColor(COLOR_BTNFACE));
    }
    CRect rctItem = m_levelRect;
    bool bFeatsAvailable = (m_availableFeats[level].size() > 0);
    rctItem.top = top;
    if (bFeatsAvailable)
    {
        // 36 pixels for height for this item
        rctItem.bottom = top + 35;
    }
    else
    {
        rctItem.bottom = top + m_levelRect.Height();
    }
    pDC->Draw3dRect(
            rctItem,
            ::GetSysColor(COLOR_BTNHIGHLIGHT),
            ::GetSysColor(COLOR_BTNSHADOW));
    {
        CRect rctFill(rctItem);
        rctFill.DeflateRect(1, 1, 1, 1);
        pDC->FillRect(rctFill, &fillBrush);
    }
    CString text;
    text.Format("%d", level + 1);
    CSize csText = pDC->GetTextExtent(text);
    pDC->TextOut(
            rctItem.left + (rctItem.Width() - csText.cx) / 2,
            rctItem.top + (rctItem.Height() - csText.cy) / 2,
            text);
    // draw the selected class icon
    const std::string& classSelection = m_pCharacter->ActiveBuild()->LevelData(level).HasClass()
            ? m_pCharacter->ActiveBuild()->LevelData(level).Class()
            : Class_Unknown;
    for (size_t cc = 0; cc < m_numClassColumns; ++cc)
    {
        rctItem.left = m_classRects[cc].left;
        rctItem.right = m_classRects[cc].right;
        pDC->Draw3dRect(
                rctItem,
                ::GetSysColor(COLOR_BTNHIGHLIGHT),
                ::GetSysColor(COLOR_BTNSHADOW));
        CRect rctFill(rctItem);
        rctFill.DeflateRect(1, 1, 1, 1);
        pDC->FillRect(rctFill, &fillBrush);
        const std::string& ct = m_pCharacter->ActiveBuild()->Class(cc);
        // only drawn if it matches current class selection
        if (ct == classSelection
                || (classSelection == Class_Unknown && cc == 0)
                || (classSelection == Class_Epic && cc == 0)
                || (classSelection == Class_Legendary && cc == 0))
        {
            if (bFeatsAvailable)
            {
                // draw the class using the large icon
                CPoint pos = rctItem.TopLeft();
                pos.x += (rctItem.Width() - 32) / 2;    // centre it
                pos.y += (rctItem.Height() - 32) / 2;    // centre it
                Class::LargeClassImageList().Draw(
                        pDC,
                        ClassIndex(classSelection),
                        pos,
                        ILD_TRANSPARENT);
            }
            else
            {
                // draw class using the small icon
                CPoint pos = rctItem.TopLeft();
                pos.x += (rctItem.Width() - 16) / 2;    // centre it
                pos.y += (rctItem.Height() - 15) / 2;    // centre it
                Class::SmallClassImageList().Draw(
                        pDC,
                        ClassIndex(classSelection),
                        pos,
                        ILD_TRANSPARENT);
            }
        }
        // add the hit check for this item
        HitCheckItem classSelect((HitType)(HT_LevelClass1 + cc), rctItem, level, 0);
        m_hitChecks.push_back(classSelect);
    }
    // draw the feat selections (if any)
    for (size_t fc = 0; fc < m_maxRequiredFeats; ++fc)
    {
        rctItem.left = m_featRects[fc].left;
        rctItem.right = m_featRects[fc].right;
        pDC->Draw3dRect(
                rctItem,
                ::GetSysColor(COLOR_BTNHIGHLIGHT),
                ::GetSysColor(COLOR_BTNSHADOW));
        CRect rctFill(rctItem);
        rctFill.DeflateRect(1, 1, 1, 1);
        pDC->FillRect(rctFill, &fillBrush);
        DrawFeat(pDC, rctItem, fc, level);
    }
    // additional columns for stats [str][dex][con][int][wis][cha][bab]
    for (size_t stats = Ability_Unknown; stats < Ability_Count; ++stats)
    {
        rctItem.left = m_statRects[stats].left;
        rctItem.right = m_statRects[stats].right;
        pDC->Draw3dRect(
                rctItem,
                ::GetSysColor(COLOR_BTNHIGHLIGHT),
                ::GetSysColor(COLOR_BTNSHADOW));
        CRect rctFill(rctItem);
        rctFill.DeflateRect(1, 1, 1, 1);
        pDC->FillRect(rctFill, &fillBrush);
        CString strText;
        size_t value = 0;
        switch (stats)
        {
        case Ability_Unknown:       value = m_pCharacter->ActiveBuild()->BaseAttackBonus(level); break;
        case Ability_Strength:      value = m_pCharacter->ActiveBuild()->AbilityAtLevel(Ability_Strength, level, true); break;
        case Ability_Dexterity:     value = m_pCharacter->ActiveBuild()->AbilityAtLevel(Ability_Dexterity, level, true); break;
        case Ability_Constitution:  value = m_pCharacter->ActiveBuild()->AbilityAtLevel(Ability_Constitution, level, true); break;
        case Ability_Intelligence:  value = m_pCharacter->ActiveBuild()->AbilityAtLevel(Ability_Intelligence, level, true); break;
        case Ability_Wisdom:        value = m_pCharacter->ActiveBuild()->AbilityAtLevel(Ability_Wisdom, level, true); break;
        case Ability_Charisma:      value = m_pCharacter->ActiveBuild()->AbilityAtLevel(Ability_Charisma, level, true); break;
        }
        strText.Format("%d", value);
        csText = pDC->GetTextExtent(strText);
        pDC->TextOut(
                rctItem.left + (rctItem.Width() - csText.cx) / 2,
                rctItem.top + (rctItem.Height() - csText.cy) / 2,
                strText);
    }
    // set top of next item
    top = rctItem.bottom + 1;
    return top;
}

void CFeatsClassControl::DrawFeat(
        CDC * pDC,
        CRect rctItem,
        size_t featIndex,
        size_t level)
{
    // if there an actual feat in this slot?
    std::vector<FeatSlot> tfts = m_availableFeats[level];
    if (tfts.size() > featIndex)
    {
        // add the hit check location for this feat
        HitCheckItem feat(HT_Feat, rctItem, level, featIndex);
        m_hitChecks.push_back(feat);

        // if we have an actual trained feat, draw it
        TrainedFeat tf = m_pCharacter->ActiveBuild()->GetTrainedFeat(
                level,
                tfts[featIndex].FeatType());
        // name is blank if no feat currently trained
        if (tf.FeatName() != "")
        {
            // draw the selected feat
            // first draw its icon
            const Feat& foundFeat = FindFeat(tf.FeatName());
            int imageIndex = foundFeat.FeatImageIndex();
            CPoint pos = rctItem.TopLeft();
            pos.x += 2;
            pos.y += 1;
            Feat::FeatImageList().Draw(
                    pDC,
                    imageIndex,
                    pos,
                    ILD_TRANSPARENT);
            CString displayText = tf.FeatName().c_str();
            displayText += "\r\n";
            displayText += tfts[featIndex].FeatType().c_str();
            // show the feat name
            rctItem.left += 36; // space away from the icon
            CRect rctDummy(rctItem);
            int height = pDC->DrawText(displayText, rctDummy, DT_END_ELLIPSIS | DT_CALCRECT);
            rctItem.top += (rctItem.Height() - height) / 2; // center vertically
            pDC->DrawText(displayText, rctItem, DT_END_ELLIPSIS);
        }
        else
        {
            // draw a feat that needs to be selected.
            // we can re-use the class [?] unknown icon
            CPoint pos = rctItem.TopLeft();
            pos.x += 2;
            pos.y += 1;
            Class::LargeClassImageList().Draw(
                    pDC,
                    ClassIndex(Class_Unknown),
                    pos,
                    ILD_TRANSPARENT);
            CString displayText = "Click to select a feat\r\n";
            displayText += tfts[featIndex].FeatType().c_str();
            // show the feat name
            rctItem.left += 36; // space away from the icon
            CRect rctDummy(rctItem);
            int height = pDC->DrawText(displayText, rctDummy, DT_END_ELLIPSIS | DT_CALCRECT);
            rctItem.top += (rctItem.Height() - height) / 2; // center vertically
            pDC->DrawText(displayText, rctItem, DT_END_ELLIPSIS);
        }
    }
    else
    {
        // no trainable feat, leave the position blank
    }
}

LRESULT CFeatsClassControl::OnMouseLeave(WPARAM, LPARAM)
{
    // if the menu is being displayed, leave the item highlighted in the control
    // gets re-drawn when menu is dismissed/closed
    if (!m_bMenuDisplayed)
    {
        m_highlightedLevelLine = c_dudLevel;
        Invalidate();
    }
    if (m_showingTip)
    {
        HideTip();
        m_tooltipItem = HitCheckItem(HT_None, CRect(0, 0, 0, 0), 0, 0);
    }
    return 0;
}


void CFeatsClassControl::OnMouseMove(UINT nFlags, CPoint point)
{
    // determine which row the mouse is over, and change highlight if required
    size_t overLevel = c_dudLevel;
    size_t maxLevel = (m_pCharacter->ActiveBuild() != NULL)
            ? m_pCharacter->ActiveBuild()->Level()
            : 0;
    for (size_t level = 0; level < maxLevel; ++level)
    {
        if (m_levelRects[level].PtInRect(point))
        {
            overLevel = level;
            break;
        }
    }
    if (overLevel != m_highlightedLevelLine)
    {
        m_highlightedLevelLine = overLevel;
        Invalidate();
        if (m_highlightedLevelLine >= 0 && m_highlightedLevelLine < MAX_GAME_LEVEL)
        {
            m_lastNotifiedLevelLine = m_highlightedLevelLine;
            // let the automatic feats view know about the selection change
            CWnd* pWnd = AfxGetMainWnd();
            CMainFrame* pMainWnd = dynamic_cast<CMainFrame*>(pWnd);
            CAutomaticFeatsPane* pPane = dynamic_cast<CAutomaticFeatsPane*>(
                    pMainWnd->GetPane(RUNTIME_CLASS(CAutomaticFeatsPane)));
            pPane->PostMessage(UWM_UPDATE, m_lastNotifiedLevelLine, 0L); // wParam is level to display for
        }
    }
    // hit check the mouse location and display feat tooltips if required
    if (m_pCharacter->ActiveBuild() != NULL)
    {
        HitCheckItem ht = HitCheck(point);
        if (ht.Type() == HT_Feat)
        {
            // mouse is over a feat, display tooltip if its a trained feat
            if (m_showingTip
                    && ht.Type() == m_tooltipItem.Type()
                    && ht.Level() == m_tooltipItem.Level()
                    && ht.Data() == m_tooltipItem.Data())
            {
                // already showing the right tip, do nothing
            }
            else
            {
                HideTip();
                m_tooltipItem = HitCheckItem(HT_None, CRect(0, 0, 0, 0), 0, 0);
                // get the selected feat name (if there is one)
                std::vector<FeatSlot> tfts = m_availableFeats[ht.Level()];
                if (tfts.size() > ht.Data())
                {
                    // if we have an actual trained feat, draw it
                    TrainedFeat tf = m_pCharacter->ActiveBuild()->GetTrainedFeat(
                            ht.Level(),
                            tfts[ht.Data()].FeatType());
                    std::string featName = tf.FeatName();
                    if (featName != "")
                    {
                        const Feat & feat = FindFeat(tf.FeatName());
                        // determine whether there is a feat swap warning if training at level 1
                        bool warn = false;
                        if (ht.Level() == 0
                                && tfts[ht.Data()].FeatType() != Feat_Automatic
                                && tfts[ht.Data()].FeatType() != Feat_Granted
                                && featName != " No Selection")
                        {
                            warn = !m_pCharacter->ActiveBuild()->IsFeatTrainable(
                                    ht.Level(),
                                    tfts[ht.Data()].FeatType(),
                                    feat,
                                    false,
                                    true);
                        }
                        CPoint tipTopLeft(ht.Rect().left, ht.Rect().bottom);
                        CPoint tipAlternate(ht.Rect().left, ht.Rect().top);
                        ClientToScreen(&tipTopLeft);
                        ClientToScreen(&tipAlternate);
                        m_tooltip.SetOrigin(tipTopLeft, tipAlternate, false);
                        m_tooltip.SetFeatItem(*m_pCharacter->ActiveBuild(), &feat, warn, ht.Level(), false);
                        m_tooltip.Show();
                        m_showingTip = true;
                        m_tooltipItem = ht;
                    }
                }
            }
        }
        else if (ht.Type() == HT_LevelClass1
                || ht.Type() == HT_LevelClass2
                || ht.Type() == HT_LevelClass3)
        {
            // look up the selected class
            const LevelTraining & levelData = m_pCharacter->ActiveBuild()->LevelData(ht.Level());
            std::string expectedClass = levelData.HasClass() ? levelData.Class() : Class_Unknown;
            if (ht.Level() == 0)
            {
                ::Race race = FindRace(m_pCharacter->ActiveBuild()->Race());
                if (race.HasIconicClass())
                {
                    expectedClass = race.IconicClass();
                }
            }
            CPoint tipTopLeft(ht.Rect().left, ht.Rect().bottom);
            CPoint tipAlternate(ht.Rect().left, ht.Rect().top);
            ClientToScreen(&tipTopLeft);
            ClientToScreen(&tipAlternate);
            m_tooltip.SetOrigin(tipTopLeft, tipAlternate, false);
            m_tooltip.SetLevelItem(*m_pCharacter->ActiveBuild(), ht.Level(), &levelData, expectedClass);
            m_tooltip.Show();
            m_showingTip = true;
            m_tooltipItem = ht;
        }
        else
        {
            HideTip();
            m_tooltipItem = HitCheckItem(HT_None, CRect(0, 0, 0, 0), 0, 0);
        }
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

void CFeatsClassControl::OnLButtonUp(UINT nFlags, CPoint point)
{
    UNREFERENCED_PARAMETER(nFlags);
    // so whether the user has clicked on a specific item
    GetCursorPos(&point);
    ScreenToClient(&point);
    HitCheckItem ht = HitCheck(point);
    switch (ht.Type())
    {
    case HT_None:
        // not on anything that can be interacted with
        break;
    case HT_Class1:
        // show the class select drop list menu
        DoClass1();
        break;
    case HT_Class2:
        // show the class select drop list menu
        DoClass2();
        break;
    case HT_Class3:
        // show the class select drop list menu
        DoClass3();
        break;
    case HT_LevelClass1:
        // set the class for this level if its not already this class
        SetClassLevel(m_pCharacter->ActiveBuild()->Class(0), ht.Level());
        break;
    case HT_LevelClass2:
        // set the class for this level if its not already this class
        SetClassLevel(m_pCharacter->ActiveBuild()->Class(1), ht.Level());
        break;
    case HT_LevelClass3:
        // set the class for this level if its not already this class
        SetClassLevel(m_pCharacter->ActiveBuild()->Class(2), ht.Level());
        break;
    case HT_Feat:
        {
            m_featSelectItem = ht;
            std::vector<FeatSlot> tfts = m_availableFeats[ht.Level()];
            TrainedFeat tf = m_pCharacter->ActiveBuild()->GetTrainedFeat(
                    ht.Level(),
                    tfts[ht.Data()].FeatType());
            // get a list of the available feats at current level that the user can train
            std::vector<Feat> availableFeats = m_pCharacter->ActiveBuild()->TrainableFeats(
                    tfts[ht.Data()].FeatType(),
                    ht.Level(),
                    tf.FeatName());
            // include a "No Selection" feat, if > 1 feat in list
            // (i.e. user cannot un-train single feat selections)
            if (availableFeats.size() > 1
                    && tf.FeatName() != "")         // must already have a selection also
            {
                Feat feat(" No Selection",     // space causes to appear at top of list
                        "Clears the current feat selection",
                        "NoSelection");
                availableFeats.push_back(feat);
            }
            std::sort(availableFeats.begin(), availableFeats.end());

            // lock the window to stop flickering while updates are done
            m_featSelector.LockWindowUpdate();
            m_featSelector.SetImageList(NULL);
            m_featSelector.ResetContent();
            m_featSelector.SetImageList(&Feat::FeatImageList());

            // now add the feat names to the combo control
            int sel = CB_ERR;
            std::vector<Feat>::iterator it = availableFeats.begin();
            while (it != availableFeats.end())
            {
                const Feat& foundFeat = FindFeat((*it).Name());
                int featImageIndex = foundFeat.FeatImageIndex();
                size_t pos = m_featSelector.AddString((*it).Name().c_str());
                m_featSelector.SetItemData(pos, featImageIndex);
                if (tf.FeatName() == (*it).Name())
                {
                    sel = pos;
                }
                ++it;
            }
            m_featSelector.SetCurSel(sel);
            m_featSelector.UnlockWindowUpdate();
            // position the drop list combo over the feat selection position
            CRect comboRect(ht.Rect());
            comboRect.bottom = comboRect.top + 960;   // 20 items visible in drop list
            m_featSelector.MoveWindow(comboRect);
            m_featSelector.SetCurSel(sel);
            m_featSelector.SetDroppedWidth(350); // wider to show extra text
            // set control visible to the user
            m_featSelector.ShowWindow(SW_SHOW);
            m_featSelector.SetFocus();
            m_featSelector.ShowDropDown();
        }
        break;
    }
}

HitCheckItem CFeatsClassControl::HitCheck(CPoint mouse) const
{
    static HitCheckItem noHit(HT_None, CRect(0, 0, 0, 0), 0, 0);
    for (size_t i = 0; i < m_hitChecks.size(); ++i)
    {
        if (m_hitChecks[i].PointInRect(mouse))
        {
            return m_hitChecks[i];
        }
    }
    return noHit;
}

void CFeatsClassControl::DoClass1()
{
    if (m_pCharacter != NULL)
    {
        // build the menu of classes that can be selected
        std::string type1 = m_pCharacter->ActiveBuild()->Class1();
        std::string type2 = m_pCharacter->ActiveBuild()->Class2();
        std::string type3 = m_pCharacter->ActiveBuild()->Class3();
        int sel = DoClassContextMenu(
                CPoint(m_classRects[0].left, m_classRects[0].bottom),
                type1,
                type2,
                type3);
        if (sel >= 0)
        {
            const std::list<Class> & classes = Classes();
            std::list<Class>::const_iterator cci = classes.begin();
            std::advance(cci, sel);
            std::string ct = (*cci).Name();
            if (ct != type1)
            {
                m_pCharacter->ActiveBuild()->SetClass1(ct);
                SetupControl();
            }
        }
    }
}

void CFeatsClassControl::DoClass2()
{
    if (m_pCharacter != NULL)
    {
        // build the menu of classes that can be selected
        std::string type1 = m_pCharacter->ActiveBuild()->Class1();
        std::string type2 = m_pCharacter->ActiveBuild()->Class2();
        std::string type3 = m_pCharacter->ActiveBuild()->Class3();
        int sel = DoClassContextMenu(
                CPoint(m_classRects[1].left, m_classRects[1].bottom),
                type2,
                type1,
                type3);
        if (sel >= 0)
        {
            const std::list<Class> & classes = Classes();
            std::list<Class>::const_iterator cci = classes.begin();
            std::advance(cci, sel);
            std::string ct = (*cci).Name();
            if (ct != type2)
            {
                m_pCharacter->ActiveBuild()->SetClass2(ct);
                SetupControl();
            }
        }
    }
}

void CFeatsClassControl::DoClass3()
{
    if (m_pCharacter != NULL)
    {
        // build the menu of classes that can be selected
        std::string type1 = m_pCharacter->ActiveBuild()->Class1();
        std::string type2 = m_pCharacter->ActiveBuild()->Class2();
        std::string type3 = m_pCharacter->ActiveBuild()->Class3();
        int sel = DoClassContextMenu(
                CPoint(m_classRects[2].left, m_classRects[2].bottom),
                type3,
                type1,
                type2);
        if (sel >= 0)
        {
            const std::list<Class> & classes = Classes();
            std::list<Class>::const_iterator cci = classes.begin();
            std::advance(cci, sel);
            std::string ct = (*cci).Name();
            if (ct != type3)
            {
                m_pCharacter->ActiveBuild()->SetClass3(ct);
                SetupControl();
            }
        }
    }
}

int CFeatsClassControl::DoClassContextMenu(
        CPoint menuPos,
        const std::string& original,
        const std::string& exclude1,
        const std::string& exclude2)
{
    CMenu menu;
    menu.CreatePopupMenu();
    const std::list<Class> & classes = Classes();
    std::string baseClass1 = FindClass(exclude1).GetBaseClass();
    std::string baseClass2 = FindClass(exclude2).GetBaseClass();
    std::list<Class>::const_iterator cci = classes.begin();
    size_t ci = 0;
    while (cci != classes.end())
    {
        if (false == (*cci).HasNotHeroic())
        {
            // show all classes in the drop list, with classes disabled
            // if not selectable due to alignment
            CString className = (*cci).Name().c_str();
            std::string baseClassName = (*cci).GetBaseClass();
            bool bAlignmentRestricted = !(*cci).CanTrainClass(m_pCharacter->ActiveBuild()->Alignment());
            // ensure ui allows switch of base types in same selection
            bool bClassArchetypeRestricted = (baseClassName == baseClass1 || baseClassName == baseClass2);
            if ((*cci).Name() != exclude1
                && (*cci).Name() != exclude2
                || (*cci).Name() == Class_Unknown)
            {
                // this class is selectable
                if (className == original.c_str())
                {
                    menu.AppendMenu(
                        MF_STRING | MF_CHECKED,
                        ID_CLASS_PROMPT1 + ci,
                        className);
                }
                else
                {
                    if (bAlignmentRestricted)
                    {
                        // add text to class name to show why it cannot be selected
                        className += " (Wrong Alignment)";
                        menu.AppendMenu(
                            MF_STRING | MF_DISABLED,
                            ID_CLASS_PROMPT1 + ci,
                            className);
                    }
                    else if (bClassArchetypeRestricted && (*cci).Name() != Class_Unknown)
                    {
                        // add text to class name to show why it cannot be selected
                        className += " (Archetype Restricted)";
                        menu.AppendMenu(
                            MF_STRING | MF_DISABLED,
                            ID_CLASS_PROMPT1 + ci,
                            className);
                    }
                    else
                    {
                        menu.AppendMenu(
                            MF_STRING,
                            ID_CLASS_PROMPT1 + ci,
                            className);
                    }
                }
            }
        }
        ++cci;
        ++ci;
    }
    ClientToScreen(&menuPos);
    CWinAppEx * pApp = dynamic_cast<CWinAppEx*>(AfxGetApp());
    int sel = pApp->GetContextMenuManager()->TrackPopupMenu(
            menu.GetSafeHmenu(),
            menuPos.x,
            menuPos.y,
            this);
    if (sel != 0)
    {
        sel -= ID_CLASS_PROMPT1;
    }
    else
    {
        sel = -1;       // cancelled
    }
    return sel;
}

//void CFeatsClassControl::UpdateClassChoiceChanged(Character * charData)
//{
//    if (!m_bUpdatePending)
//    {
//        PostMessage(UWM_UPDATE, 0, 0L);
//        m_bUpdatePending = true;
//    }
//}

//void CFeatsClassControl::UpdateClassChanged(
//        Character * charData,
//        ClassType classFrom,
//        ClassType classTo,
//        size_t level)
//{
//    if (!m_bUpdatePending)
//    {
//        PostMessage(UWM_UPDATE, 0, 0L);
//        m_bUpdatePending = true;
//    }
//}

void CFeatsClassControl::UpdateFeatTrained(
        Build *,
        const std::string&)
{
    if (!m_bUpdatePending)
    {
        PostMessage(UWM_UPDATE, 0, 0L);
        m_bUpdatePending = true;
    }
}

void CFeatsClassControl::UpdateFeatRevoked(
        Build *,
        const std::string&)
{
    if (!m_bUpdatePending)
    {
        PostMessage(UWM_UPDATE, 0, 0L);
        m_bUpdatePending = true;
    }
}

void CFeatsClassControl::UpdateAbilityValueChanged(Build*, AbilityType ability)
{
    UNREFERENCED_PARAMETER(ability);
    Invalidate();       // ability values have changed
}

void CFeatsClassControl::UpdateBuildAutomaticFeatsChanged(Build*, size_t level)
{
    if (level == m_lastNotifiedLevelLine)
    {
        // let the automatic feats view know about the selection change
        CWnd* pWnd = AfxGetMainWnd();
        CMainFrame* pMainWnd = dynamic_cast<CMainFrame*>(pWnd);
        CAutomaticFeatsPane* pPane = dynamic_cast<CAutomaticFeatsPane*>(
                pMainWnd->GetPane(RUNTIME_CLASS(CAutomaticFeatsPane)));
        pPane->PostMessage(UWM_UPDATE, m_lastNotifiedLevelLine, 0L); // wParam is level to display for
    }
}

LRESULT CFeatsClassControl::UpdateControl(WPARAM, LPARAM)
{
    SetupControl();
    Invalidate();
    m_bUpdatePending = false;
    return 0;
}

void CFeatsClassControl::SetClassLevel(const std::string& ct, size_t level)
{
    if (level < MAX_CLASS_LEVEL)
    {
        if (!m_pCharacter->ActiveBuild()->LevelData(level).HasClass()
                || m_pCharacter->ActiveBuild()->LevelData(level).Class() != ct)
        {
            m_pCharacter->ActiveBuild()->SetClass(level, ct);
            // ensure class tooltip updates
            HideTip();
            m_tooltipItem = HitCheckItem(HT_None, CRect(0, 0, 0, 0), 0, 0);
        }
    }
    else
    {
        ::MessageBeep(MB_OK);
    }
}

void CFeatsClassControl::OnFeatSelectOk()
{
    HideTip();
    // feat selection complete
    m_featSelector.ShowWindow(SW_HIDE); // hide control
    int sel = m_featSelector.GetCurSel();
    if (sel != CB_ERR)
    {
        // user has selected a feat, train it!
        std::vector<FeatSlot> tfts = m_availableFeats[m_featSelectItem.Level()];
        CString featName;
        m_featSelector.GetLBText(sel, featName);
        m_pCharacter->ActiveBuild()->TrainFeat(
                (LPCTSTR)featName,
                tfts[m_featSelectItem.Data()].FeatType(),
                m_featSelectItem.Level());
    }
}

void CFeatsClassControl::OnFeatSelectCancel()
{
    // spell selection aborted, just hide the control
    m_featSelector.ShowWindow(SW_HIDE);
    HideTip();
}

LRESULT CFeatsClassControl::OnHoverComboBox(WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    // wParam = selected index
    // lParam = control ID
    if (m_showingTip)
    {
        m_tooltip.Hide();
    }
    if (wParam >= 0)
    {
        // we have a selection, get the feats name
        CString featName;
        m_featSelector.GetLBText(wParam, featName);
        if (!featName.IsEmpty())
        {
            CRect rctWindow;
            m_featSelector.GetWindowRect(&rctWindow);
            rctWindow.right = rctWindow.left + m_featSelector.GetDroppedWidth();
            std::vector<FeatSlot> tfts = m_availableFeats[m_featSelectItem.Level()];
            // tip is shown to the left or the right of the combo box
            const Feat & feat = FindFeat((LPCTSTR)featName);
            CPoint tipTopLeft(rctWindow.left, rctWindow.top);
            CPoint tipAlternate(rctWindow.right, rctWindow.top);
            bool warn = false;
            if (m_featSelectItem.Level() == 0
                    && tfts[m_featSelectItem.Data()].FeatType() != Feat_Automatic
                    && tfts[m_featSelectItem.Data()].FeatType() != Feat_Granted
                    && featName != " No Selection")
            {
                warn = !m_pCharacter->ActiveBuild()->IsFeatTrainable(
                        m_featSelectItem.Level(),
                        tfts[m_featSelectItem.Data()].FeatType(),
                        feat,
                        false,
                        false);
            }
            m_tooltip.SetOrigin(tipTopLeft, tipAlternate, true);
            m_tooltip.SetFeatItem(
                    *m_pCharacter->ActiveBuild(),
                    &feat,
                    warn,
                    m_featSelectItem.Level(),
                    false);
            m_tooltip.Show();
            m_showingTip = true;
        }
    }
    return 0;
}

void CFeatsClassControl::HideTip()
{
    // tip not shown if not over a tip item
    if (m_tipCreated && m_showingTip)
    {
        m_tooltip.Hide();
        m_showingTip = false;
    }
}

void CFeatsClassControl::UpdateActiveBuildChanged(Character * pCharacter)
{
    SetCharacter(pCharacter);
}

void CFeatsClassControl::UpdateAbilityTomeChanged(Life*, AbilityType)
{
    Invalidate();   // numbers in columns will have changed
}

void CFeatsClassControl::UpdateRaceChanged(Life*, const std::string&)
{
    SetupControl();
}

void CFeatsClassControl::UpdateBuildLevelChanged(Build *)
{
    SetupControl();
}

void CFeatsClassControl::UpdateClassChanged(
        Build *,
        const std::string&,
        const std::string&,
        size_t)
{
    SetupControl();
}

