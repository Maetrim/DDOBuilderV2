// InfoTip.cpp
//

#include "stdafx.h"
#include "InfoTip.h"

#include "Augment.h"
#include "Build.h"
#include "Class.h"
#include "EnhancementTreeItem.h"
#include "Feat.h"
#include "Filigree.h"
#include "Gem.h"
#include "GlobalSupportFunctions.h"
#include "Item.h"
#include "LevelTraining.h"
#include "Patron.h"
#include "SetBonus.h"
#include "SubItem.h"
#include "InfoTipItem.h"
#include "BreakdownItem.h"
#include "OptionalBuff.h"
#include <map>

CInfoTip::CInfoTip() :
    m_origin(CPoint(0, 0)),
    m_bRightAlign(false)
{
}

CInfoTip::~CInfoTip()
{
    ClearOldTipItems();
}

void CInfoTip::ClearOldTipItems()
{
    for (auto&& it : m_tipItems)
    {
        delete it;
    }
    m_tipItems.clear();
}

BEGIN_MESSAGE_MAP(CInfoTip, CWnd)
    //{{AFX_MSG_MAP(CInfoTip)
    ON_WM_PAINT()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CInfoTip::Create(CWnd* pParentWnd) 
{
    // Must have a parent
    ASSERT(pParentWnd != NULL);

    BOOL bSuccess = CreateEx(
            NULL, 
            AfxRegisterWndClass(0), 
            NULL, 
            WS_POPUP, 
            0, 
            0, 
            0, 
            0, 
            pParentWnd->GetSafeHwnd(), 
            NULL, 
            NULL);

    return bSuccess;
}

void CInfoTip::Show()
{
    CDC * pDC = GetDC();
    CSize windowSize;
    GetWindowSize(pDC, &windowSize);
    ReleaseDC(pDC);

    if (!m_bRightAlign)
    {
        // before actually showing the tip, determine whether it fits on the screen
        // if it extends below the bottom or past the right we move the window
        // accordingly to make sure it is visible. Do this for the monitor the
        // origin point for this tooltip is on.
        HMONITOR hMonitor = MonitorFromPoint(m_origin, MONITOR_DEFAULTTONEAREST);
        MONITORINFOEX monitorInfo;
        memset(&monitorInfo, 0, sizeof(MONITORINFOEX));
        monitorInfo.cbSize = sizeof(MONITORINFOEX);
        GetMonitorInfo(hMonitor, &monitorInfo);
        CRect monitorSize(monitorInfo.rcWork);
        if (m_origin.x + windowSize.cx > monitorSize.right)
        {
            // move the tip left to ensure all text visible
            m_origin.x = monitorSize.right - windowSize.cx;
        }
        // alternate position is above
        if (m_origin.y + windowSize.cy > monitorSize.bottom)
        {
            // move the tip above the origin position to ensure content visible
            m_origin.y = m_alternate.y - windowSize.cy; // alternate position
        }
    }
    else
    {
        // if it extends beyond the left we use the alternate position
        // accordingly to make sure it is visible. 
        // we move the tool tip up as required
        HMONITOR hMonitor = MonitorFromPoint(m_origin, MONITOR_DEFAULTTONEAREST);
        MONITORINFOEX monitorInfo;
        memset(&monitorInfo, 0, sizeof(MONITORINFOEX));
        monitorInfo.cbSize = sizeof(MONITORINFOEX);
        GetMonitorInfo(hMonitor, &monitorInfo);
        CRect monitorSize(monitorInfo.rcWork);
        if (m_origin.x - windowSize.cx < monitorSize.left)
        {
            // need to use the alternate position
            m_origin = m_alternate;
        }
        else
        {
            m_origin.x -= (windowSize.cx + 2);
        }
        // alternate position is above
        if (m_origin.y + windowSize.cy > monitorSize.bottom)
        {
            // move the tip up
            m_origin.y = monitorSize.bottom - windowSize.cy;
        }
    }

    SetWindowPos(
            &wndTop,
            m_origin.x,
            m_origin.y,
            windowSize.cx,
            windowSize.cy,
            SWP_NOACTIVATE | SWP_SHOWWINDOW);
}

void CInfoTip::SetOrigin(CPoint origin, CPoint alternate, bool rightAlign)
{
    m_origin = origin;
    m_alternate = alternate;
    m_bRightAlign = rightAlign;
}

void CInfoTip::Hide()
{
    ShowWindow(SW_HIDE);
}

void CInfoTip::OnPaint() 
{
    CPaintDC dc( this ); // device context for painting

    CRect rc;
    CBrush tooltipColourBrush;
    CBrush frameBrush;

    // Get the client rectangle
    GetClientRect(rc);

    // Create the brushes
    frameBrush.CreateSolidBrush(::GetSysColor(COLOR_INFOTEXT));
    tooltipColourBrush.CreateSolidBrush(::GetSysColor(COLOR_INFOBK));

    // Draw the frame
    dc.FillRect(rc, &tooltipColourBrush);
    dc.DrawEdge(rc, BDR_SUNKENOUTER, BF_RECT);
    dc.SetBkMode(TRANSPARENT);

    dc.SaveDC();
    CRect rect(rc);
    rect.top += c_controlSpacing;       // space at top of control
    for (auto&& it : m_tipItems)
    {
        it->Draw(&dc, rect);
        rect.top += it->Size().cy;
    }
    //dc.SelectObject(m_boldFont);
    //// draw the icon
    //m_image.TransparentBlt(
    //        dc.GetSafeHdc(),
    //        CRect(c_controlSpacing, c_controlSpacing, c_controlSpacing + 32, c_controlSpacing + 32),
    //        CRect(0, 0, 32, 32));
    //// render the title in bold
    //CRect rcTitle(c_controlSpacing + 32 + c_controlSpacing, c_controlSpacing, rc.right, rc.bottom);
    //dc.DrawText(m_title, rcTitle, DT_LEFT | DT_EXPANDTABS | DT_NOPREFIX);
    //CSize sizeCost = dc.GetTextExtent(m_cost);
    //CSize sizeRanks = dc.GetTextExtent(m_ranks);
    //dc.TextOut(rc.right - c_controlSpacing - sizeCost.cx, c_controlSpacing, m_cost);
    //COLORREF old = dc.SetTextColor(RGB(0,128, 0));
    //dc.TextOut(rc.right - c_controlSpacing - sizeRanks.cx, c_controlSpacing + sizeCost.cy, m_ranks);
    //dc.SetTextColor(old);

    //dc.SelectObject(m_standardFont);
    //// optional requirements list shown above description, 1 per line
    //int top = c_controlSpacing + max(sizeRanks.cy * 2, 32) + c_controlSpacing;
    //ASSERT(m_requirements.size() == m_bRequirementMet.size());
    //for (size_t ri = 0; ri < m_requirements.size(); ++ri)
    //{
    //    // text drawn in green or red depending on whether the requirement is met or not
    //    dc.SetTextColor(m_bRequirementMet[ri] ? RGB(0, 128, 0) : RGB(255, 0, 0));
    //    CRect rctRequirement(c_controlSpacing, top, rc.right, rc.bottom);
    //    dc.TextOut(c_controlSpacing, top, m_requirements[ri]);
    //    top += dc.GetTextExtent(m_requirements[ri]).cy;
    //}
    //dc.SetTextColor(RGB(0, 0, 0));    // ensure black for the rest
    //// optional Effect Descriptions list shown above description, these can word wrap
    //for (size_t ed = 0; ed < m_effectDescriptions.size(); ++ed)
    //{
    //    CRect rctEd;
    //    // measure
    //    dc.DrawText(m_effectDescriptions[ed], &rctEd, DT_CALCRECT | DT_LEFT | DT_EXPANDTABS | DT_NOPREFIX);
    //    rctEd += CPoint(2, top);
    //    dc.DrawText(m_effectDescriptions[ed], &rctEd, DT_LEFT | DT_EXPANDTABS | DT_NOPREFIX);
    //    CString text;
    //    if (m_effectDescriptions[ed].Find(':') >= 0)
    //    {
    //        text = m_effectDescriptions[ed].Left(m_effectDescriptions[ed].Find(':'));   // get the effect name to draw in bold
    //    }
    //    else
    //    {
    //        // draw all text in bold
    //        text = m_effectDescriptions[ed];
    //    }
    //    rctEd += CPoint(1, 0);
    //    dc.DrawText(text, &rctEd, DT_LEFT | DT_EXPANDTABS | DT_NOPREFIX);
    //    top += rctEd.Height();
    //}
    //CRect rcDescription(c_controlSpacing, top, rc.right, rc.bottom);
    //dc.DrawText(m_description, &rcDescription, DT_LEFT | DT_EXPANDTABS | DT_NOPREFIX);
    //// Clean up GDI
    dc.RestoreDC(-1);
}

BOOL CInfoTip::GetWindowSize(CDC* pDC, CSize * size)
{
    pDC->SaveDC();
    // measure all the items
    int width = 0;
    int height = c_controlSpacing;  // space at top of control
    for (auto&& it : m_tipItems)
    {
        it->Measure(pDC);
        width = max(width, it->Size().cx);
        height += it->Size().cy;
    }
    height += c_controlSpacing;     // spacing at end of control
    pDC->RestoreDC(-1);
    width = max(10, width);
    *size = CSize(width, height);
    //ASSERT(pDC != NULL);
    //// an enhancement item looks like this:
    //// +---------------------------------------------------+
    //// | +----+ Enhancement name                    Cost x |
    //// | |icon|                                    Ranks n |
    //// | +----+                                            |
    //// | [requirements list if any]                        |
    //// | [EffectDescription1 (if any)                     ]|
    //// | [EffectDescription2                              ]|
    //// | [EffectDescription..n                            ]|
    //// | +------------------------------------------------+|
    //// | |Description                                     ||
    //// | +------------------------------------------------+|
    //// +---------------------------------------------------+
    //CRect rcWnd(0, 0, 0, 0);
    //// border space first
    //rcWnd.InflateRect(c_controlSpacing, c_controlSpacing, c_controlSpacing, c_controlSpacing);
    //pDC->SaveDC();
    //pDC->SelectObject(m_boldFont);
    //// Calculate the area for the tip text
    //CRect rcTitle;
    //pDC->DrawText(m_title, &rcTitle, DT_CALCRECT | DT_LEFT | DT_EXPANDTABS | DT_NOPREFIX);
    //rcWnd.bottom += max(rcTitle.Height() * 2, 32) + c_controlSpacing;      // 2 lines or icon height

    //pDC->SelectObject(m_standardFont);
    //CRect rcRanks;
    //pDC->DrawText(m_ranks, &rcRanks, DT_CALCRECT | DT_LEFT | DT_EXPANDTABS | DT_NOPREFIX);
    //size_t topWidth = 32 + c_controlSpacing + rcTitle.Width() + c_controlSpacing + rcRanks.Width();

    //// optional requirements list shown above description, 1 per line
    //ASSERT(m_requirements.size() == m_bRequirementMet.size());
    //for (size_t ri = 0; ri < m_requirements.size(); ++ri)
    //{
    //    CRect rctRequirement;
    //    pDC->DrawText(m_requirements[ri], &rctRequirement, DT_CALCRECT | DT_LEFT | DT_EXPANDTABS | DT_NOPREFIX);
    //    topWidth = max(topWidth, (size_t)rctRequirement.Width());
    //    rcWnd.InflateRect(
    //            0,
    //            0,
    //            0,
    //            rctRequirement.Height());
    //}
    //// optional Effect Descriptions list shown above description, these can word wrap
    //for (size_t ed = 0; ed < m_effectDescriptions.size(); ++ed)
    //{
    //    CRect rctEd;
    //    pDC->DrawText(m_effectDescriptions[ed], &rctEd, DT_CALCRECT | DT_LEFT | DT_EXPANDTABS | DT_NOPREFIX);
    //    topWidth = max(topWidth, (size_t)rctEd.Width());
    //    rcWnd.InflateRect(
    //            0,
    //            0,
    //            0,
    //            rctEd.Height());
    //}
    //CRect rcDescription;
    //pDC->DrawText(m_description, &rcDescription, DT_CALCRECT | DT_LEFT | DT_EXPANDTABS | DT_NOPREFIX);

    //// now work out the total window size
    //size_t bottomWidth = rcDescription.Width();
    //rcWnd.InflateRect(
    //        0,
    //        0,
    //        max(topWidth, bottomWidth),
    //        rcDescription.Height());
    //// Set the window size
    //if (size != NULL)
    //{
    //    size->cx = rcWnd.Width();
    //    size->cy = rcWnd.Height();
    //}
    //pDC->RestoreDC(-1);

    return TRUE;
}

void CInfoTip::SetEnhancementTreeItem(
        const Build& build,
        const EnhancementTreeItem* pItem,
        const std::string& selection,
        size_t spentInTree)
{
    ClearOldTipItems();
    InfoTipItem_Header* pHeader = new InfoTipItem_Header;
    pHeader->LoadIcon("DataFiles\\EnhancementImages\\", pItem->Icon(), true);
    pHeader->SetTitle(pItem->Name().c_str());
    CString str;
    str.Format("Cost %d", pItem->Cost(selection, 0));
    if (pItem->CostVaries(selection))
    {
        for (size_t rank = 1; rank < pItem->Ranks(selection); ++rank)
        {
            CString rankCost;
            rankCost.Format("/%d", pItem->Cost(selection, rank));
            str += rankCost;
        }
    }
    pHeader->SetCost(str);
    str.Format("Ranks %d", pItem->Ranks(selection));
    pHeader->SetRank(str);
    m_tipItems.push_back(pHeader);

    // add the requirements for this enhancement item next
    InfoTipItem_Requirements* pRequirements = new InfoTipItem_Requirements;
    pRequirements->CreateRequirementsStrings(build, pItem, spentInTree);
    m_tipItems.push_back(pRequirements);

    // now add the description under the requirements
    InfoTipItem_MultilineText* pDescription = new InfoTipItem_MultilineText;
    pDescription->SetText(pItem->Description().c_str());
    m_tipItems.push_back(pDescription);

    // check all effects to see if they grant a feat/spell, if they do, list the feat/spell
    // as additional info in the tooltip
    std::map<std::string, int> addedSpells;
    for (auto&& eit : pItem->Effects())
    {
        if (eit.IsType(Effect_GrantFeat))
        {
            Requirements req;
            if (eit.HasRequirementsToBeActive())
            {
                req = eit.RequirementsToBeActive();
            }
            AppendFeatInfo(build, eit.Item().front(), req);
        }
        // note that some enhancements can list the same spell/sla being added
        // to multiple different classes. Make sure the spell entry only appears
        // once in the tool tip
        if (eit.IsType(Effect_GrantSpell))
        {
            std::string spellName = eit.Item().front();
            if (addedSpells.count(spellName) == 0)
            {
                Spell spell = FindSpellByName(spellName);
                ClassSpell cs(
                    static_cast<int>(eit.Amount()[0]),
                    static_cast<size_t>(eit.Amount()[1]),
                    static_cast<size_t>(eit.Amount()[2]));
                spell.UpdateSpell(cs, eit.Item().back()); // 2nd item is class
                AppendSpellItem(build, spell);
                addedSpells[spellName] = 1; // value doesn't matter
            }
        }
        if (eit.IsType(Effect_SpellLikeAbility))
        {
            std::string slaName = eit.Item().front();
            if (addedSpells.count(slaName) == 0)
            {
                Spell spell = FindSpellByName(slaName);
                ClassSpell cs(
                    static_cast<int>(eit.Amount()[0]),
                    static_cast<size_t>(eit.Amount()[1]),
                    static_cast<size_t>(eit.Amount()[2]));
                spell.UpdateSpell(cs, eit.Item().back()); // 2nd item is class
                AppendSpellItem(build, spell);
                addedSpells[slaName] = 1; // value doesn't matter
            }
        }
    }
    // check to see if any DCs are being defined
    for (auto&& dcit : pItem->GetDCs(""))
    {
        AppendDCInfo(build, dcit);
    }
}

void CInfoTip::SetEnhancementSelectionItem(
        const Build& build,
        const EnhancementTreeItem* pItem,
        const EnhancementSelection* pSelection,
        size_t spentInTree,
        size_t /*ranks*/)
{
    ClearOldTipItems();
    InfoTipItem_Header* pHeader = new InfoTipItem_Header;
    pHeader->LoadIcon("DataFiles\\EnhancementImages\\", pSelection->Icon(), true);
    pHeader->SetTitle(pSelection->Name().c_str());
    CString str;
    str.Format("Cost %d", pSelection->Cost(0));
    if (pSelection->CostVaries())
    {
        for (size_t rank = 1; rank < pItem->Ranks(pSelection->Name()); ++rank)
        {
            CString rankCost;
            rankCost.Format("/%d", pSelection->Cost(rank));
            str += rankCost;
        }
    }
    pHeader->SetCost(str);
    str.Format("Ranks %d", pItem->Ranks(pSelection->Name()));
    pHeader->SetRank(str);
    m_tipItems.push_back(pHeader);

    // add the requirements for this enhancement item next
    InfoTipItem_Requirements* pRequirements = new InfoTipItem_Requirements;
    pRequirements->CreateRequirementsStrings(build, pItem, pSelection, spentInTree);
    m_tipItems.push_back(pRequirements);

    // now add the description under the requirements
    InfoTipItem_MultilineText* pDescription = new InfoTipItem_MultilineText;
    pDescription->SetText(pSelection->Description().c_str());
    m_tipItems.push_back(pDescription);

    // check all effects to see if they grant a feat/spell, if they do, list the feat/spell
    // as additional info in the tooltip
    std::map<std::string, int> addedSpells;
    for (auto&& eit : pSelection->Effects())
    {
        if (eit.IsType(Effect_GrantFeat))
        {
            Requirements req;
            if (eit.HasRequirementsToBeActive())
            {
                req = eit.RequirementsToBeActive();
            }
            AppendFeatInfo(build, eit.Item().front(), req);
        }
        if (eit.IsType(Effect_GrantSpell))
        {
            std::string spellName = eit.Item().front();
            if (addedSpells.count(spellName) == 0)
            {
                Spell spell = FindSpellByName(spellName);
                ClassSpell cs(
                    static_cast<int>(eit.Amount()[0]),
                    static_cast<size_t>(eit.Amount()[1]),
                    static_cast<size_t>(eit.Amount()[2]));
                spell.UpdateSpell(cs, eit.Item().back()); // 2nd item is class
                AppendSpellItem(build, spell);
                addedSpells[spellName] = 1; // value doesn't matter
            }
        }
        if (eit.IsType(Effect_SpellLikeAbility))
        {
            std::string slaName = eit.Item().front();
            if (addedSpells.count(slaName) == 0)
            {
                Spell spell = FindSpellByName(slaName);
                ClassSpell cs(
                    static_cast<int>(eit.Amount()[0]),
                    static_cast<size_t>(eit.Amount()[1]),
                    static_cast<size_t>(eit.Amount()[2]));
                spell.UpdateSpell(cs, eit.Item().back()); // 2nd item is class
                AppendSpellItem(build, spell);
                addedSpells[slaName] = 1; // value doesn't matter
            }
        }
    }
    // check to see if any DCs are being defined
    for (auto&& dcit : pItem->GetDCs(pSelection->Name()))
    {
        AppendDCInfo(build, dcit);
    }
}

void CInfoTip::SetFeatItem(
        const Build& build,
        const Feat* pItem,
        bool /*featSwapWarning*/,
        size_t /*level*/,
        bool /*grantedFeat*/)
{
    ClearOldTipItems();
    InfoTipItem_Header* pHeader = new InfoTipItem_Header;
    pHeader->LoadIcon("DataFiles\\FeatImages\\", pItem->Icon(), true);
    pHeader->SetTitle(pItem->Name().c_str());
    if (pItem->MaxTimesAcquire() != 1)
    {
        CString str;
        str.Format("Max Acquire %d", pItem->MaxTimesAcquire());
        pHeader->SetCost(str);
    }
    m_tipItems.push_back(pHeader);

    if (IsInIgnoreList(pItem->Name()))
    {
        InfoTipItem_Requirements* pRequirements = new InfoTipItem_Requirements;
        pRequirements->AddRequirement("This Feat is in your ignore list (Right click to restore it).", false);
        m_tipItems.push_back(pRequirements);
    }

    // add the requirements for this granted feat next
    if (pItem->HasRequirementsToTrain())
    {
        InfoTipItem_Requirements* pRequirements = new InfoTipItem_Requirements;
        pRequirements->CreateRequirementsStrings(build, pItem->RequirementsToTrain());
        m_tipItems.push_back(pRequirements);
    }

    InfoTipItem_MultilineText* pDescription = new InfoTipItem_MultilineText;
    pDescription->SetText(pItem->Description().c_str());
    m_tipItems.push_back(pDescription);

    for (auto&& siit : pItem->SubItems())
    {
        AppendSubItem(siit);
    }

    // check to see if any DCs are being defined
    for (auto&& dcit : pItem->DCs())
    {
        AppendDCInfo(build, dcit);
    }
}

void CInfoTip::AppendSubItem(const SubItem& item)
{
    InfoTipItem_Header* pHeader = new InfoTipItem_Header;
    pHeader->LoadIcon("DataFiles\\FeatImages\\", item.Icon(), true);
    pHeader->SetTitle(item.Name().c_str());
    m_tipItems.push_back(pHeader);

    InfoTipItem_MultilineText* pDescription = new InfoTipItem_MultilineText;
    pDescription->SetText(item.Description().c_str());
    m_tipItems.push_back(pDescription);
}

void CInfoTip::SetStanceItem(const Stance& item)
{
    ClearOldTipItems();
    InfoTipItem_Header* pHeader = new InfoTipItem_Header;
    if (!pHeader->LoadIcon("DataFiles\\FeatImages\\", item.Icon(), false))
    {
        if (!pHeader->LoadIcon("DataFiles\\EnhancementImages\\", item.Icon(), false))
        {
            if (!pHeader->LoadIcon("DataFiles\\UiImages\\", item.Icon(), false))
            {
                pHeader->LoadIcon("DataFiles\\AugmentImages\\", item.Icon(), true);
            }
        }
    }
    pHeader->SetTitle(item.Name().c_str());
    m_tipItems.push_back(pHeader);

    InfoTipItem_MultilineText* pDescription = new InfoTipItem_MultilineText;
    pDescription->SetText(item.Description().c_str());
    m_tipItems.push_back(pDescription);
}

void CInfoTip::SetSetBonusItem(
    const SetBonus& item,
    size_t numStacks,
    bool bSuppressed)
{
    ClearOldTipItems();
    AppendSetBonusDescription(item.Type(), numStacks, bSuppressed);
}

void CInfoTip::SetItem(
        const Item * pItem,
        const Build* pBuild)
{
    ClearOldTipItems();
    InfoTipItem_Header* pHeader = new InfoTipItem_Header;
    pHeader->IconFromImageList(ItemsImages(), pItem->IconIndex());
    pHeader->SetTitle(pItem->Name().c_str());
    CString str;
    str.Format("Minimum Level: %d", pItem->MinLevel());
    pHeader->SetCost(str);
    if (pItem->HasMinorArtifact())
    {
        pHeader->SetRank("Minor Artifact");
    }
    m_tipItems.push_back(pHeader);

    if (pItem->HasRequirementsToUse())
    {
        pItem->RequirementsToUse().AddItemRequirements(m_tipItems);
    }

    if (pItem->HasDropLocation())
    {
        InfoTipItem_Requirements* pRequirements = new InfoTipItem_Requirements;
        CString dropLoc;
        dropLoc.Format("Drop Location: %s", pItem->DropLocation().c_str());
        BreakUpLongLines(dropLoc);
        pRequirements->AddRequirement(dropLoc, false);  // red highlighted line
        m_tipItems.push_back(pRequirements);
    }
    if (pItem->HasRestrictedSlots())
    {
        InfoTipItem_Requirements* pRequirements = new InfoTipItem_Requirements;
        CString restrictedLocs;
        const EquipmentSlot& slots = pItem->RestrictedSlots();
        for (size_t i = Inventory_Arrows; i < Inventory_Count; ++i)
        {
            if (slots.HasSlot((InventorySlotType)i))
            {
                restrictedLocs.Format("Restricted Slot: %s", (LPCTSTR)EnumEntryText((InventorySlotType)i, InventorySlotTypeMap));
                pRequirements->AddRequirement(restrictedLocs, false);  // red highlighted line
                m_tipItems.push_back(pRequirements);
            }
        }
    }

    if (pItem->HasWeapon())
    {
        CString text;
        CString entry;
        int value = pItem->BuffValue(Effect_Weapon_Enchantment);
        entry.Format("Damage %.2f[%s] + %d ", pItem->WeaponDamage(), (LPCTSTR)pItem->DamageDice().DiceAsText(), value);
        // now append its DR breaking types
        bool first = true;
        for (auto&& it: pItem->DRBypass())
        {
            if (!first) entry += ", ";
            first = false;
            entry += EnumEntryText(it, drTypeMap);
        }
        text += entry;

        size_t criticalRange = pItem->RealCriticalThreatRange(); // includes Keen etc
        if (criticalRange > 1)
        {
            entry.Format("\r\nCritical Roll: %d-20 / x%d", 21 - criticalRange, pItem->CriticalMultiplier());
        }
        else
        {
            entry.Format("\r\nCritical Roll: 20 / x%d", pItem->CriticalMultiplier());
        }
        text += entry;

        if (pItem->AttackModifier().size() > 0)
        {
            entry.Format("\r\nAttack Mod: ");
            first = true;
            for (auto&& it : pItem->AttackModifier())
            {
                if (!first) entry += ", ";
                first = false;
                entry += EnumEntryText(it, abilityTypeMap);
            }
            text += entry;
        }

        if (pItem->DamageModifier().size() > 0)
        {
            entry.Format("\r\nDamage Mod: ");
            first = true;
            for (auto&& it : pItem->DamageModifier())
            {
                if (!first) entry += ", ";
                first = false;
                entry += EnumEntryText(it, abilityTypeMap);
            }
            text += entry;
        }

        InfoTipItem_MultilineText* pDescription = new InfoTipItem_MultilineText;
        pDescription->SetText(text);
        m_tipItems.push_back(pDescription);
    }

    if (pItem->HasArmorBonus()
            && pItem->Armor() != Armor_Cloth)
    {
        // add the items armor stats
        CString text;
        if (pItem->Armor() == Armor_Docent)
        {
            text.Format("Composite Plating Bonus: %d", pItem->ArmorBonus());
            if (pItem->HasMithralBody())
            {
                CString mb;
                mb.Format("\r\nMithral Body Bonus: %d", pItem->MithralBody());
                text += mb;
            }
            if (pItem->HasAdamantineBody())
            {
                CString ab;
                ab.Format("\r\nAdamantine Body Bonus: %d", pItem->AdamantineBody());
                text += ab;
            }
        }
        else
        {
            text.Format("Armor Bonus w/Dex: %d\r\n"
                "Armor Bonus: %d",
                pItem->ArmorBonus() + (pItem->HasMaximumDexterityBonus() ? pItem->MaximumDexterityBonus() : 0),
                pItem->ArmorBonus());
        }
        if (pItem->HasMaximumDexterityBonus())
        {
            CString mdb;
            mdb.Format("\r\nMax Dex Bonus: %d", pItem->MaximumDexterityBonus());
            text += mdb;
        }
        if (pItem->HasArmorCheckPenalty() && pItem->ArmorCheckPenalty() > 0)
        {
            CString acp;
            acp.Format("\r\nArmor Check Penalty: %d", pItem->ArmorCheckPenalty());
            text += acp;
        }
        if (pItem->HasArcaneSpellFailure() && pItem->ArcaneSpellFailure() != 0)
        {
            CString asf;
            asf.Format("\r\nSpell Failure: %d%%", pItem->ArcaneSpellFailure());
            text += asf;
        }
        InfoTipItem_MultilineText* pDescription = new InfoTipItem_MultilineText;
        pDescription->SetText(text);
        m_tipItems.push_back(pDescription);
    }

    for (auto&& eit : pItem->Effects())
    {
        if (eit.IsType(Effect_ItemClickie))
        {
            std::string slaName = eit.Item().front();
            Spell spell = FindItemClickieByName(slaName);
            AppendSLA(spell,
                static_cast<int>(eit.Amount()[0]),
                static_cast<int>(eit.Amount()[1]),
                static_cast<int>(eit.Amount()[2]));
        }
        if (eit.IsType(Effect_SpellLikeAbility))
        {
            std::string slaName = eit.Item().front();
            Spell spell = FindItemClickieByName(slaName);
            AppendSLA(spell,
                static_cast<int>(eit.Amount()[0]),
                static_cast<int>(eit.Amount()[1]),
                static_cast<int>(eit.Amount()[2]));
        }
    }
    std::vector<CString> buffDescriptions = pItem->BuffDescriptions(pBuild);
    for (auto&& bdit : buffDescriptions)
    {
        InfoTipItem_BuffDescription* pBuff = new InfoTipItem_BuffDescription(bdit);
        m_tipItems.push_back(pBuff);
    }
    bool bSetSuppressed = false;
    if (pItem->Augments().size() > 0)
    {
        InfoTipItem_Requirements* pRequirements = new InfoTipItem_Requirements;
        for (auto&& ait : pItem->Augments())
        {
            CString text;
            if (ait.HasSelectedAugment())
            {
                CString augmentName(ait.SelectedAugment().c_str());
                // augment name may need to be updated to include the actual bonus value
                const Augment& augment = FindAugmentByName((LPCTSTR)augmentName, pItem);
                AppendFilledAugment(pItem->MinLevel(), ait, &augment);
                if (augment.HasSuppressSetBonus())
                {
                    bSetSuppressed = true;
                }
            }
            else
            {
                text.Format("%s Slot: Empty", ait.Type().c_str());
                pRequirements->AddRequirement(text, true);
            }
        }
        m_tipItems.push_back(pRequirements);
    }
    for (auto&& sbit : pItem->SetBonus())
    {
        AppendSetBonusDescription(sbit, 0, bSetSuppressed);
    }
    if (!pItem->Description().empty())
    {
        InfoTipItem_MultilineText* pDescription = new InfoTipItem_MultilineText;
        pDescription->SetText(pItem->Description().c_str());
        m_tipItems.push_back(pDescription);
    }
}

void CInfoTip::SetSelfBuff(const std::string& name)
{
    ClearOldTipItems();
    const OptionalBuff& opBuff = FindOptionalBuff(name);
    InfoTipItem_Header* pHeader = new InfoTipItem_Header;
    if (!pHeader->LoadIcon("DataFiles\\SpellImages\\", opBuff.Icon(), false))
    {
        if (!pHeader->LoadIcon("DataFiles\\AugmentImages\\", opBuff.Icon(), false))
        {
            pHeader->LoadIcon("DataFiles\\FeatImages\\", opBuff.Icon(), true);
        }
    }
    pHeader->SetTitle(opBuff.Name().c_str());
    m_tipItems.push_back(pHeader);

    InfoTipItem_MultilineText* pDescription = new InfoTipItem_MultilineText;
    pDescription->SetText(opBuff.Description().c_str());
    m_tipItems.push_back(pDescription);
}

void CInfoTip::SetFiligree(
        const Filigree * pFiligree)
{
    ClearOldTipItems();
    InfoTipItem_Header* pHeader = new InfoTipItem_Header;
    if (!pHeader->LoadIcon("DataFiles\\FiligreeImages\\", pFiligree->Icon(), false))
    {
        pHeader->LoadIcon("DataFiles\\AugmentImages\\", pFiligree->Icon(), true);
    }
    pHeader->SetTitle(pFiligree->Name().c_str());
    m_tipItems.push_back(pHeader);

    InfoTipItem_MultilineText* pDescription = new InfoTipItem_MultilineText;
    pDescription->SetText(pFiligree->Description().c_str());
    m_tipItems.push_back(pDescription);

    for (auto&& sbit : pFiligree->SetBonus())
    {
        AppendSetBonusDescription(sbit, 0, false);
    }
}

void CInfoTip::SetAugment(
    const Augment* pAugment)
{
    ClearOldTipItems();
    AppendAugment(pAugment);
}

void CInfoTip::AppendFilledAugment(
    int itemLevel,
    const ItemAugment& slot,
    const Augment* pAugment)
{
    size_t minLevel = 0;
    if (pAugment->HasMinLevel())
    {
        minLevel = pAugment->MinLevel();
    }
    InfoTipItem_Header* pHeader = new InfoTipItem_Header;
    if (!pHeader->LoadIcon("DataFiles\\SetBonusImages\\", pAugment->HasIcon() ? pAugment->Icon() : "", false))
    {
        if (!pHeader->LoadIcon("DataFiles\\FiligreeImages\\", pAugment->HasIcon() ? pAugment->Icon() : "", false))
        {
            pHeader->LoadIcon("DataFiles\\AugmentImages\\", pAugment->HasIcon() ? pAugment->Icon() : "", true);
        }
    }
    pHeader->SetTitle(slot.Type().c_str());

    CString augmentName = pAugment->Name().c_str();
    if (pAugment->HasEnterValue())
    {
        CString txt;
        txt.Format(" %+.0f", slot.Value());
        augmentName += txt;
        if (pAugment->HasDualValues())
        {
            txt.Format("/%+.0f", slot.Value2());
            augmentName += txt;
        }
    }
    if (pAugment->HasChooseLevel())
    {
        if (slot.HasSelectedLevelIndex())
        {
            CString txt;
            if (pAugment->HasLevels())
            {
                minLevel = pAugment->Levels()[slot.SelectedLevelIndex()];
                txt.Format(" %+.0f", pAugment->LevelValue()[slot.SelectedLevelIndex()]);
                augmentName += txt;
                if (pAugment->HasDualValues())
                {
                    txt.Format("/%+.0f", pAugment->LevelValue2()[slot.SelectedLevelIndex()]);
                }
            }
            else if (pAugment->HasLevelValue())
            {
                txt.Format(" %+.0f", pAugment->LevelValue()[slot.SelectedLevelIndex()]);
                augmentName += txt;
                if (pAugment->HasDualValues())
                {
                    txt.Format("/%+.0f", pAugment->LevelValue2()[slot.SelectedLevelIndex()]);
                }
            }
            else
            {
                txt.Format("Augment %s has missing Levels field", pAugment->Name().c_str());
            }
        }
        else
        {
            CString txt;
            if (pAugment->HasLevelValue())
            {
                txt.Format(" %+.0f", pAugment->LevelValue()[itemLevel - 1]);
                augmentName += txt;
                if (pAugment->HasDualValues())
                {
                    txt.Format("/%+.0f", pAugment->LevelValue2()[itemLevel - 1]);
                    augmentName += txt;
                }
            }
            else
            {
                txt.Format("Augment %s has missing LevelValue field", pAugment->Name().c_str());
            }
        }
    }
    pHeader->SetTitle2(augmentName);
    if (minLevel > 0)
    {
        CString text;
        text.Format("Augment Level: %d", minLevel);
        pHeader->SetCost(text);
    }
    m_tipItems.push_back(pHeader);

    InfoTipItem_MultilineText* pDescription = new InfoTipItem_MultilineText;
    pDescription->SetText(pAugment->Description().c_str());
    m_tipItems.push_back(pDescription);

    for (auto&& sbit : pAugment->SetBonus())
    {
        AppendSetBonusDescription(sbit, 0, false);
    }
}

void CInfoTip::AppendAugment(
    const Augment* pAugment)
{
    InfoTipItem_Header* pHeader = new InfoTipItem_Header;
    if (!pHeader->LoadIcon("DataFiles\\SetBonusImages\\", pAugment->HasIcon() ? pAugment->Icon() : "", false))
    {
        if (!pHeader->LoadIcon("DataFiles\\FiligreeImages\\", pAugment->HasIcon() ? pAugment->Icon() : "", false))
        {
            pHeader->LoadIcon("DataFiles\\AugmentImages\\", pAugment->HasIcon() ? pAugment->Icon() : "", true);
        }
    }
    pHeader->SetTitle(pAugment->Name().c_str());
    if (pAugment->HasMinLevel())
    {
        CString text;
        text.Format("Augment Level: %d", pAugment->MinLevel());
        pHeader->SetCost(text);
    }
    else if (pAugment->HasChooseLevel())
    {
        if (pAugment->HasLevels())
        {
            std::vector<int> levels = pAugment->Levels();
            CString text("Augment Level: ");
            for (auto&& lit : levels)
            {
                CString lvl;
                lvl.Format("%d, ", lit);
                text += lvl;
            }
            text.TrimRight(" ,");
            pHeader->SetCost(text);
        }
        else
        {
            CString text;
            text.Format("Augment Levels: Any");
            pHeader->SetCost(text);
        }
    }
    m_tipItems.push_back(pHeader);

    InfoTipItem_MultilineText* pDescription = new InfoTipItem_MultilineText;
    pDescription->SetText(pAugment->Description().c_str());
    m_tipItems.push_back(pDescription);

    for (auto&& sbit : pAugment->SetBonus())
    {
        AppendSetBonusDescription(sbit, 0, false);
    }
}

void CInfoTip::SetSentientGem(
    const Gem* pGem)
{
    ClearOldTipItems();
    InfoTipItem_Header* pHeader = new InfoTipItem_Header;
    pHeader->LoadIcon("DataFiles\\SentientGemImages\\", pGem->Icon(), true);
    pHeader->SetTitle(pGem->Name().c_str());
    m_tipItems.push_back(pHeader);

    InfoTipItem_MultilineText* pDescription = new InfoTipItem_MultilineText;
    pDescription->SetText(pGem->Description().c_str());
    m_tipItems.push_back(pDescription);
}

void CInfoTip::SetSpell(
    const Build& build,
    const Spell* pSpell)
{
    ClearOldTipItems();
    AppendSpellItem(build, *pSpell);
}

void CInfoTip::SetFavorItem(
    const Patron& patron,
    int favorTier)
{
    UNREFERENCED_PARAMETER(favorTier);
    ClearOldTipItems();
    const Feat& feat = FindFeat(patron.AssociatedFavorFeat());
    InfoTipItem_Header* pHeader = new InfoTipItem_Header;
    pHeader->LoadIcon("DataFiles\\FeatImages\\", feat.Icon(), true);
    pHeader->SetTitle(feat.Name().c_str());
    CString maxFavor;
    maxFavor.Format("Max: %d", patron.MaxFavor());
    pHeader->SetRank(maxFavor);
    m_tipItems.push_back(pHeader);

    CString line = ExtractLine(favorTier, feat.Description().c_str());

    InfoTipItem_MultilineText* pDescription = new InfoTipItem_MultilineText;
    pDescription->SetText(line);
    m_tipItems.push_back(pDescription);
}

void CInfoTip::AppendSpellItem(const Build& build, const Spell& spell)
{
    InfoTipItem_Header* pHeader = new InfoTipItem_Header;
    pHeader->LoadIcon("DataFiles\\SpellImages\\", spell.HasIcon() ? spell.Icon() : "", true);
    pHeader->SetTitle(spell.Name().c_str());
    if (spell.HasSchool())
    {
        CString school;
        school.Format("School: %s", (LPCTSTR)EnumEntryText(spell.School(), spellSchoolTypeMap));
        pHeader->SetRank(school);
    }
    if (spell.HasCost())
    {
        CString cost;
        cost.Format("SP Cost: %d", spell.Cost());
        pHeader->SetCost(cost);
    }
    m_tipItems.push_back(pHeader);

    if (spell.MetamagicCount() > 0)
    {
        InfoTipItem_Metamagics* pMeta = new InfoTipItem_Metamagics;
        pMeta->SetSpell(spell);
        m_tipItems.push_back(pMeta);
    }

    InfoTipItem_MultilineText* pDescription = new InfoTipItem_MultilineText;
    pDescription->SetText(spell.Description().c_str());
    m_tipItems.push_back(pDescription);

    // add any spell DCs
    for (auto&& dcit : spell.DCs())
    {
        AppendDCInfo(build, dcit);
    }
    // add any spell damage items
    for (auto&& sdit : spell.SpellDamageEffects())
    {
        AppendSpellDamageEffect(build, spell, sdit);
    }
}

void CInfoTip::AppendSLA(const Spell& spell, int nCasterLevel, int nCharges, int nRecharge)
{
    if (spell.HasIcon())
    {
        InfoTipItem_Header* pHeader = new InfoTipItem_Header;
        pHeader->LoadIcon("DataFiles\\SpellImages\\", spell.Icon(), true);
        CString name = spell.Name().c_str();
        name.Replace(" SLA", "");
        pHeader->SetTitle(name);
        m_tipItems.push_back(pHeader);
    }

    InfoTipItem_MultilineText* pDescription = new InfoTipItem_MultilineText;
    pDescription->SetText(spell.Description().c_str());
    m_tipItems.push_back(pDescription);

    if (nCasterLevel != 0
        || nCharges != 0
        || nRecharge != 0)
    {
        InfoTipItem_SLAHeader* pSLAHeader = new InfoTipItem_SLAHeader(nCasterLevel, nCharges, nRecharge);
        m_tipItems.push_back(pSLAHeader);
    }
}

void CInfoTip::AppendSpellDamageEffect(
        const Build& build,
        const Spell& spell,
        const SpellDamage& sd)
{
    InfoTipItem_Requirements* pRequirements = new InfoTipItem_Requirements;
    CString sdt = sd.DiceDamageText(); // e.g. Spell Damage: 1d6+2 + (1d6+3 per 2 caster levels)
    pRequirements->AddRequirement(sdt, false);  // red highlighted line
    m_tipItems.push_back(pRequirements);

    CString mcl = spell.ActualMaxCasterLevelText(sd);
    CString cl = spell.ActualCasterLevelText(sd);
    int castLevel = min(spell.ActualMaxCasterLevel(sd), spell.ActualCasterLevel(sd));
    CString tsd = sd.SpellDamageText(build, castLevel);

    CString tipText = mcl;
    tipText += "\r\n";
    tipText += cl;
    tipText += tsd;

    InfoTipItem_MultilineText* pDescription = new InfoTipItem_MultilineText;
    pDescription->SetText(tipText);
    m_tipItems.push_back(pDescription);
}

void CInfoTip::SetLevelItem(
        const Build & build,
        size_t level,
        const LevelTraining * levelData,
        const std::string& expectedClass)
{
    ClearOldTipItems();

    // icon is the class level
    InfoTipItem_Header* pHeader = new InfoTipItem_Header;
    std::string ct = levelData->HasClass() ? levelData->Class() : Class_Unknown;
    const ::Class& c = FindClass(ct);
    size_t classLevels = build.ClassLevels(ct, level);
    CString text;
    text.Format("Level %d: %s (%d)",
            level + 1,
            ct.c_str(),
            classLevels);
    pHeader->LoadIcon("DataFiles\\ClassImages\\", c.LargeIcon(), true);
    pHeader->SetTitle(text);
    m_tipItems.push_back(pHeader);

    InfoTipItem_Requirements* pRequirements = new InfoTipItem_Requirements;
    // we use requirement options to show an issue with a required
    // +1 heart of wood to be used
    if (level == 0
        && ct != expectedClass)
    {
        text.Format("Requires a +1 Heart of Wood to switch from Iconic class of ");
        text += expectedClass.c_str();
        pRequirements->AddRequirement(text, false);
        pRequirements->AddRequirement("", true);    // blank line
    }

    if (classLevels == 1)
    {
        InfoTipItem_MultilineText* pClassDescription = new InfoTipItem_MultilineText;
        pClassDescription->SetText(c.Description().c_str());
        m_tipItems.push_back(pClassDescription);
    }
    // The description field is the list of feats that can/have been trained
    // along with the list of skills and skill points available
    std::vector<FeatSlot> trainable = build.TrainableFeatTypeAtLevel(level);
    if (trainable.size() > 0)
    {
        for (size_t tft = 0; tft < trainable.size(); ++tft)
        {
            text = trainable[tft].FeatType().c_str();
            text += ": ";
            TrainedFeat tf = build.GetTrainedFeat(
                    level,
                    trainable[tft].FeatType());
            if (tf.FeatName().empty())
            {
                text += "Empty Feat Slot";
                pRequirements->AddRequirement(text, false);
            }
            else
            {
                text += tf.FeatName().c_str();
                pRequirements->AddRequirement(text, true);
            }
        }
    }
    else
    {
        pRequirements->AddRequirement("No trainable feats at this level", true);
    }

    // now show how many skill points we have at this level unless its an epic level
    if (levelData->HasClass()
            && levelData->Class() != Class_Epic
            && levelData->Class() != Class_Legendary)
    {
        pRequirements->AddRequirement("", true);    // blank line
        size_t skillPoints = levelData->SkillPointsAvailable();
        size_t spentSkillPoints = levelData->SkillPointsSpent();
        int availablePoints = (static_cast<int>(skillPoints) - static_cast<int>(spentSkillPoints));
        if (availablePoints < 0)
        {
            text = "You have spent too many skill points at this level";
            pRequirements->AddRequirement(text, false);
            pRequirements->AddRequirement("", true);    // blank line
        }

        text.Format("Skill Points Available %d of %d",
                availablePoints,
                levelData->SkillPointsAvailable());
        pRequirements->AddRequirement(text, true);

        // show what skills (if any) points have been spent on
        // and whether they have been overspent on
        const std::list<TrainedSkill> & ts = levelData->TrainedSkills();
        std::list<TrainedSkill>::const_iterator it = ts.begin();
        std::vector<size_t> skillRanks(Skill_Count, 0);
        while (it != ts.end())
        {
            skillRanks[(*it).Skill()]++;
            ++it;
        }
        for (size_t i = 0; i < Skill_Count; ++i)
        {
            if (skillRanks[i] != 0)
            {
                // this skill has been trained, list it
                bool classSkill = c.IsClassSkill((SkillType)i);
                CString skillName = EnumEntryText((SkillType)i, skillTypeMap);
                if (classSkill)
                {
                    text.Format("%s: %d Ranks",
                            (LPCSTR)skillName,
                            skillRanks[i]);
                }
                else
                {
                    text.Format("%s: %.1f Ranks (%d spent)",
                            (LPCSTR)skillName,
                            skillRanks[i] * 0.5,
                            skillRanks[i]);
                }
                // has it been over trained?
                if (build.SkillAtLevel((SkillType)i, level, false)
                        > build.MaxSkillForLevel((SkillType)i, level))
                {
                    text  += " SKILL OVER TRAINED";
                    pRequirements->AddRequirement(text, false);
                }
                else
                {
                    pRequirements->AddRequirement(text, true);
                }
            }
        }
    }
    m_tipItems.push_back(pRequirements);
}

//void CInfoTip::SetSelfBuff(const std::string& name)
//{
//    OptionalBuff buff = FindOptionalBuff(name);
//    m_effectDescriptions.clear();
//    m_image.Destroy();
//    if (S_OK != LoadImageFile(IT_spell, buff.HasIcon() ? buff.Icon() : "", &m_image, false))
//    {
//        // see if its a enhancement icon we need to use
//        if (S_OK != LoadImageFile(IT_enhancement, buff.HasIcon() ? buff.Icon() : "", &m_image, false))
//        {
//             LoadImageFile(IT_feat, buff.HasIcon() ? buff.Icon() : "", &m_image);
//        }
//    }
//    m_image.SetTransparentColor(c_transparentColour);
//    m_title = buff.Name().c_str();
//    m_description = buff.Description().c_str();
//    // actual carriage return are actual \n in text, convert to correct character
//    GenerateLineBreaks(&m_title);
//    GenerateLineBreaks(&m_description);
//    m_requirements.clear();
//    m_bRequirementMet.clear();
//    m_cost = "";
//}

void CInfoTip::SetDCItem(
        const Build& build,
        const DC* pDC)
{
    ClearOldTipItems();
    InfoTipItem_Header* pHeader = new InfoTipItem_Header;
    if (!pHeader->LoadIcon("DataFiles\\FeatImages\\", pDC->Icon(), false))
    {
        if (!pHeader->LoadIcon("DataFiles\\EnhancementImages\\", pDC->Icon(), false))
        {
            if (!pHeader->LoadIcon("DataFiles\\UiImages\\", pDC->Icon(), false))
            {
                pHeader->LoadIcon("DataFiles\\AugmentImages\\", pDC->Icon(), true);
            }
        }
    }
    pHeader->SetTitle(pDC->Name().c_str());
    CString text;
    if (pDC->HasOther())
    {
        text.Format(
                "DC: %s + %d",
                pDC->Other().c_str(),
                pDC->CalculateDC(&build));
    }
    else
    {
        text.Format("DC: %d", pDC->CalculateDC(&build));
    }
    pHeader->SetRank(text);
    m_tipItems.push_back(pHeader);

    InfoTipItem_MultilineText* pDescription = new InfoTipItem_MultilineText;
    pDescription->SetText(pDC->Description().c_str());
    m_tipItems.push_back(pDescription);
    InfoTipItem_Requirements* pRequirements = new InfoTipItem_Requirements;
    pRequirements->AddRequirement(pDC->DCBreakdown(&build).c_str(), true);
    m_tipItems.push_back(pRequirements);
}

void CInfoTip::AppendSetBonusDescription(
    const std::string& setBonusName,
    size_t numStacks,
    bool bSuppressed)
{
    const SetBonus& setBonus = FindSetBonus(setBonusName);
    InfoTipItem_Header* pHeader = new InfoTipItem_Header;
    if (!pHeader->LoadIcon("DataFiles\\SetBonusImages\\", setBonus.Icon(), false))
    {
        if (!pHeader->LoadIcon("DataFiles\\FiligreeImages\\", setBonus.Icon(), false))
        {
            pHeader->LoadIcon("DataFiles\\AugmentImages\\", setBonus.Icon(), true);
        }
    }
    pHeader->SetTitle(setBonusName.c_str());
    if (bSuppressed)
    {
        pHeader->SetTitle2("This set bonus is suppressed by a slotted augment");
    }
    if (numStacks > 0)
    {
        CString str;
        str.Format("Ranks %d", numStacks);
        pHeader->SetCost(str);
    }
    m_tipItems.push_back(pHeader);

    InfoTipItem_Requirements* pRequirements = new InfoTipItem_Requirements;
    pRequirements->CreateSetBonusStrings(setBonus, numStacks);
    m_tipItems.push_back(pRequirements);
}


void CInfoTip::AppendFeatInfo(
        const Build& build,
        const std::string& featName,
        const Requirements& req)
{
    const Feat& feat = FindFeat(featName);
    InfoTipItem_Header* pHeader = new InfoTipItem_Header;
    pHeader->LoadIcon("DataFiles\\FeatImages\\", feat.Icon(), true);
    pHeader->SetTitle(feat.Name().c_str());
    m_tipItems.push_back(pHeader);

    if (IsInIgnoreList(feat.Name()))
    {
        InfoTipItem_Requirements* pRequirements = new InfoTipItem_Requirements;
        pRequirements->AddRequirement("This Feat is in your ignore list (Right click to restore it).", false);
        m_tipItems.push_back(pRequirements);
    }

    // add the requirements for this granted feat next
    InfoTipItem_Requirements* pRequirements = new InfoTipItem_Requirements;
    pRequirements->CreateRequirementsStrings(build, req);
    m_tipItems.push_back(pRequirements);

    InfoTipItem_MultilineText* pDescription = new InfoTipItem_MultilineText;
    pDescription->SetText(feat.Description().c_str());
    m_tipItems.push_back(pDescription);

    for (auto&& siit : feat.SubItems())
    {
        AppendSubItem(siit);
    }

    // check to see if any DCs are being defined
    for (auto&& dcit : feat.DCs())
    {
        AppendDCInfo(build, dcit);
    }
}

void CInfoTip::AppendDCInfo(
        const Build& build,
        const DC& dc)
{
    CString text;
    text.Format(
            "%s DC: %s",
            dc.DCType().c_str(),
            dc.DCBreakdown(&build).c_str());
    InfoTipItem_DC* pDC = new InfoTipItem_DC;
    pDC->SetText(text);
    m_tipItems.push_back(pDC);
}
