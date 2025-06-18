// InfoTip.cpp
//

#include "stdafx.h"
#include "InfoTip.h"

#include "Attack.h"
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
#include "SpellDC.h"
#include "SubItem.h"
#include "InfoTipItem.h"
#include "BreakdownItem.h"
#include "OptionalBuff.h"
#include <map>

CInfoTip::CInfoTip() :
    m_origin(CPoint(0, 0)),
    m_bRightAlign(false),
    m_bAlternate(false)
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

BOOL CInfoTip::Create(CWnd* pParentWnd, bool bAlternate) 
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
    m_bAlternate = bAlternate;

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
            if (m_origin.y < 0)
            {
                // extends off the top also, bump right and move down so all is visible
                m_origin.y = 0;
                m_origin.x += 60;
            }
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
                    static_cast<size_t>(eit.Amount()[2]),
                    0);         // granted spells have no cooldown
                spell.UpdateSpell(cs, eit.Item().back(), static_cast<int>(eit.Amount()[0])); // 2nd item is class
                AppendSpellItem(build, spell);
                addedSpells[spellName] = 1; // value doesn't matter
            }
        }
        if (eit.IsType(Effect_SpellLikeAbility))
        {
            const TrainedEnhancement * te = build.IsTrained(pItem->InternalName(), selection);
            size_t offset = 0;
            if (te != NULL)
            {
                offset = te->Ranks() * 4 - 4;
            }
            std::string slaName = eit.Item().front();
            if (addedSpells.count(slaName) == 0)
            {
                Spell spell = FindSpellByName(slaName);
                ClassSpell cs(
                    static_cast<int>(eit.Amount()[offset]),
                    static_cast<size_t>(eit.Amount()[offset + 1]),
                    static_cast<size_t>(eit.Amount()[offset + 2]),
                    static_cast<size_t>(eit.Amount()[offset + 3]));
                spell.UpdateSpell(cs, eit.Item().back(), static_cast<int>(eit.Amount()[offset])); // 2nd item is class
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
                    static_cast<size_t>(eit.Amount()[2]),
                    0);         // granted spells have no cooldown
                spell.UpdateSpell(cs, eit.Item().back(), static_cast<int>(eit.Amount()[0])); // 2nd item is class
                AppendSpellItem(build, spell);
                addedSpells[spellName] = 1; // value doesn't matter
            }
        }
        if (eit.IsType(Effect_SpellLikeAbility))
        {
            const TrainedEnhancement * te = build.IsTrained(pItem->InternalName(), pSelection->Name());
            size_t offset = 0;
            if (te != NULL)
            {
                offset = te->Ranks() * 4 - 4;
            }
            std::string slaName = eit.Item().front();
            if (addedSpells.count(slaName) == 0)
            {
                Spell spell = FindSpellByName(slaName);
                ClassSpell cs(
                    static_cast<int>(eit.Amount()[offset]),
                    static_cast<size_t>(eit.Amount()[offset + 1]),
                    static_cast<size_t>(eit.Amount()[offset + 2]),
                    static_cast<size_t>(eit.Amount()[offset + 3]));
                spell.UpdateSpell(cs, eit.Item().back(), static_cast<int>(eit.Amount()[offset])); // 2nd item is class
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
        bool featSwapWarning,
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

    if (m_bAlternate)
    {
        InfoTipItem_Requirements* pRequirements = new InfoTipItem_Requirements;
        pRequirements->AddRequirement("This is an alternate feat selection.", false);
        m_tipItems.push_back(pRequirements);
    }

    if (featSwapWarning)
    {
        InfoTipItem_Requirements* pRequirements = new InfoTipItem_Requirements;
        pRequirements->AddRequirement("Requires a feat swap with Fred the Mindflayer.", false);
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
    bool bSuppressed,
    const Build* pBuild)
{
    ClearOldTipItems();
    AppendSetBonusDescription(item.Type(), numStacks, bSuppressed, pBuild);
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
    if (pItem->HasMaterial())
    {
        InfoTipItem_Requirements* pRequirements = new InfoTipItem_Requirements;
        CString material;
        material.Format("Made from: %s", pItem->Material().c_str());
        pRequirements->AddRequirement(material, true);
        m_tipItems.push_back(pRequirements);
    }

    if (pItem->HasWeapon()
            && pItem->HasDamageDice())
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

        if (pItem->HasShieldBonus())
        {
            entry.Format("\r\nShield Bonus: %d", pItem->ShieldBonus());
            text += entry;
        }

        if (pItem->HasDamageReduction())
        {
            entry.Format("\r\nDamage Reduction(DR): %d", pItem->DamageReduction());
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
                AppendFilledAugment(pItem->MinLevel(), ait, &augment, *pBuild);
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
        AppendSetBonusDescription(sbit, pBuild->SetBonusCount(sbit), bSetSuppressed, pBuild);
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
        const Filigree * pFiligree,
        const Build& build)
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
        size_t numStacks = build.SetBonusCount(sbit);
        AppendSetBonusDescription(sbit, numStacks, false, NULL);
    }
}

void CInfoTip::SetAugment(
    const Augment* pAugment,
    const Build& build)
{
    ClearOldTipItems();
    AppendAugment(pAugment, build);
}

void CInfoTip::AppendFilledAugment(
    int itemLevel,
    const ItemAugment& slot,
    const Augment* pAugment,
    const Build& build)
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
    pDescription->SetText(pAugment->CompoundDescription().c_str());
    m_tipItems.push_back(pDescription);

    for (auto&& sbit : pAugment->SetBonus())
    {
        AppendSetBonusDescription(sbit, build.SetBonusCount(sbit), false, NULL);
    }
}

void CInfoTip::AppendAugment(
    const Augment* pAugment,
    const Build& build)
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
    pDescription->SetText(pAugment->CompoundDescription().c_str());
    m_tipItems.push_back(pDescription);

    for (auto&& sbit : pAugment->SetBonus())
    {
        AppendSetBonusDescription(sbit, build.SetBonusCount(sbit), false, NULL);
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

void CInfoTip::SetAttack(const Attack& attack)
{
    ClearOldTipItems();
    InfoTipItem_Header* pHeader = new InfoTipItem_Header;
    if (!pHeader->LoadIcon("DataFiles\\FeatImages\\", attack.Icon(), false))
    {
        pHeader->LoadIcon("DataFiles\\EnhancementImages\\", attack.Icon(), true);
    }
    pHeader->SetTitle(attack.Name().c_str());
    if (attack.HasCooldown())
    {
        CString text;
        text.Format("Cooldown: %.2f Seconds", attack.Cooldown()[attack.Stacks()-1]);
        text.Replace(".00", "");
        pHeader->SetCost(text);
        if (attack.HasThisAttack())
        {
            if (attack.ThisAttack().HasBonusDamagePercent())
            {
                text.Format("Deals +%.0f%% Damage", attack.ThisAttack().BonusDamagePercent()[attack.Stacks()-1]);
                pHeader->SetRank(text);
            }
        }
    }
    m_tipItems.push_back(pHeader);
}

void CInfoTip::AppendSpellItem(const Build& build, const Spell& spell)
{
    InfoTipItem_Header* pHeader = new InfoTipItem_Header;
    pHeader->LoadIcon("DataFiles\\SpellImages\\", spell.HasIcon() ? spell.Icon() : "", true);
    pHeader->SetTitle(spell.Name().c_str());
    if (spell.School().size() > 0
        || spell.HasPrimer())
    {
        bool bFirst = true;
        CString schools("School: ");
        for (auto&& sit: spell.School())
        {
            CString school;
            school = EnumEntryText(sit, spellSchoolTypeMap);
            if (!bFirst)
            {
                schools += ", ";
            }
            bFirst = false;
            schools += school;
        }
        if (spell.HasPrimer())
        {
            CString primer;
            primer = EnumEntryText(spell.Primer(), primerTypeMap);
            if (!bFirst)
            {
                schools += ", ";
            }
            bFirst = false;
            schools += primer;
        }
        pHeader->SetRank(schools);
    }
    if (spell.HasCost())
    {
        CString cost;
        if (spell.HasCooldown())
        {
            cost.Format("SP Cost: %d, Cooldown: %.1f s", spell.TotalCost(build), spell.Cooldown());
            cost.Replace(".0", "");
            cost.Replace("SP Cost: 0, ", "");
        }
        else
        {
            cost.Format("SP Cost: %d", spell.TotalCost(build));
        }
        pHeader->SetCost(cost);
    }
    m_tipItems.push_back(pHeader);

    if (IsInIgnoreList(spell.Name()))
    {
        InfoTipItem_Requirements* pRequirements = new InfoTipItem_Requirements;
        pRequirements->AddRequirement("This Spell is in your ignore list (Right click to restore it).", false);
        m_tipItems.push_back(pRequirements);
    }
    if (spell.MetamagicCount() > 0)
    {
        InfoTipItem_Metamagics* pMeta = new InfoTipItem_Metamagics;
        pMeta->SetSpell(spell);
        m_tipItems.push_back(pMeta);
    }

    if (spell.HasMaxCasterLevel()
        && (spell.ActualCasterLevel(build) != 0 || spell.ActualMaxCasterLevel(build) != 0))
    {
        CString casterLevel;
        casterLevel.Format("Caster Level: %d (Max %d)", spell.ActualCasterLevel(build), spell.ActualMaxCasterLevel(build));
        InfoTipItem_Requirements* pRequirements = new InfoTipItem_Requirements;
        pRequirements->AddRequirement(casterLevel, false);  // red highlighted line
        m_tipItems.push_back(pRequirements);
    }
    else if (spell.ActualCasterLevel(build) != 0)
    {
        CString casterLevel;
        casterLevel.Format("Caster Level: %d", spell.ActualCasterLevel(build));
        InfoTipItem_Requirements* pRequirements = new InfoTipItem_Requirements;
        pRequirements->AddRequirement(casterLevel, false);  // red highlighted line
        m_tipItems.push_back(pRequirements);
    }

    InfoTipItem_MultilineText* pDescription = new InfoTipItem_MultilineText;
    pDescription->SetText(spell.Description().c_str());
    m_tipItems.push_back(pDescription);

    // add any spell DCs
    for (auto&& dcit : spell.DCs())
    {
        AppendDCInfo(build, spell, dcit);
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

    CString mcl = spell.ActualMaxCasterLevelText(build);
    CString cl = spell.ActualCasterLevelText(build);
    int castLevel = min(spell.ActualMaxCasterLevel(build), spell.ActualCasterLevel(build));
    CString tsd = sd.SpellDamageText(build, castLevel);

    CString tipText;
    if (castLevel > 0)
    {
        tipText = mcl;
        tipText += "\r\n";
        tipText += cl;
    }
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
        if (c.HasBaseClass() && c.BaseClass() == expectedClass)
        {
            text.Format("Requires a Lesser Reincarnation to switch from Iconic class of ");
            text += expectedClass.c_str();
            text += " to Archetype class";
        }
        else
        {
            text.Format("Requires a +1 Heart of Wood to switch from Iconic class of ");
            text += expectedClass.c_str();
        }
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
    bool bSuppressed,
    const Build* pBuild)
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
    if (setBonus.HasAdditionalDescription())
    {
        InfoTipItem_Requirements* pRequirements = new InfoTipItem_Requirements;
        bool met = pBuild != NULL ? pBuild->IsStanceActive(setBonus.Type()) : false;
        pRequirements->AddRequirement(setBonus.AdditionalDescription().c_str(), met);
        m_tipItems.push_back(pRequirements);
    }

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

void CInfoTip::AppendDCInfo(
    const Build& build,
    const Spell& spell,
    const SpellDC& dc)
{
    CString text;
    text.Format(
        "%s DC: %s",
        dc.DCType().c_str(),
        dc.SpellDCBreakdown(&build, spell).c_str());
    InfoTipItem_DC* pDC = new InfoTipItem_DC;
    pDC->SetText(text);
    m_tipItems.push_back(pDC);
}
