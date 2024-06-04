// BreakdownsPane.cpp
//
#include "stdafx.h"
#include "BreakdownsPane.h"
#include "Class.h"
#include "GlobalSupportFunctions.h"

#include "BreakdownItemAbility.h"
#include "BreakdownItemAC.h"
#include "BreakdownItemBAB.h"
#include "BreakdownItemCasterLevel.h"
#include "BreakdownItemEnergyCasterLevel.h"
#include "BreakdownItemSchoolCasterLevel.h"
#include "BreakdownItemDestinyAps.h"
#include "BreakdownItemDodge.h"
#include "BreakdownItemOffhandDoublestrike.h"
#include "BreakdownItemDuration.h"
#include "BreakdownItemDR.h"
#include "BreakdownItemEnergyAbsorption.h"
#include "BreakdownItemEnergyResistance.h"
#include "BreakdownItemHitpoints.h"
#include "BreakdownItemImmunities.h"
#include "BreakdownItemMaximumKi.h"
#include "BreakdownItemMDB.h"
#include "BreakdownItemMRR.h"
#include "BreakdownItemMRRCap.h"
#include "BreakdownItemPactDice.h"
#include "BreakdownItemPRR.h"
#include "BreakdownItemSave.h"
#include "BreakdownItemSkill.h"
#include "BreakdownItemSpellPoints.h"
#include "BreakdownItemSpellPower.h"
#include "BreakdownItemSpellSchool.h"
#include "BreakdownItemTactical.h"
#include "BreakdownItemTurnUndeadLevel.h"
#include "BreakdownItemTurnUndeadHitDice.h"
#include "BreakdownItemWeaponEffects.h"
#include "BreakdownItemSneakAttackDice.h"
#include "BreakdownItemUniversalSpellPower.h"

namespace
{
    const char * f_treeSizeKey = "BreakdownsPane";
    const char * f_treeSizeEntry = "TreeSize";
}

IMPLEMENT_DYNCREATE(CBreakdownsPane, CFormView)

CBreakdownsPane::CBreakdownsPane() :
    CFormView(CBreakdownsPane::IDD),
    m_pCharacter(NULL),
    m_itemBreakdownTree(true, true),
    m_bDraggingDivider(false),
    m_treeSizePercent(75),      // defaults to 75% on first run of software
    m_pWeaponEffects(NULL),
    m_hWeaponsHeader(NULL),
    m_bUpdateBreakdowns(false),
    m_bHadInitialUpdate(false)
{

}

CBreakdownsPane::~CBreakdownsPane()
{
    for (size_t i = 0; i < m_items.size(); ++i)
    {
        delete m_items[i];
        m_items[i] = NULL;
    }
    m_items.clear();
    delete m_pWeaponEffects;
    m_pWeaponEffects = NULL;
}

void CBreakdownsPane::DoDataExchange(CDataExchange* pDX)
{
    CFormView::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_ITEM_BREAKDOWN, m_itemBreakdownList);
    DDX_Control(pDX, IDC_DIVIDER, m_divider);
    DDX_Control(pDX, IDC_BUTTON_CLIPBOARD, m_buttonClipboard);
}

#pragma warning(push)
#pragma warning(disable: 4407) // warning C4407: cast between different pointer to member representations, compiler may generate incorrect code
BEGIN_MESSAGE_MAP(CBreakdownsPane, CFormView)
    ON_WM_SIZE()
    ON_REGISTERED_MESSAGE(UWM_NEW_DOCUMENT, OnNewDocument)
    ON_REGISTERED_MESSAGE(UWM_LOAD_COMPLETE, OnLoadComplete)
    ON_NOTIFY(TVN_SELCHANGED, 1, OnSelChangedBreakdownTree)
    ON_WM_SETCURSOR()
    ON_BN_CLICKED(IDC_DIVIDER, OnDividerClicked)
    ON_WM_MOUSEMOVE()
    ON_WM_LBUTTONUP()
    ON_NOTIFY(HDN_ENDTRACK, IDC_ITEM_BREAKDOWN, OnEndtrackBreakdownList)
    ON_NOTIFY(HDN_DIVIDERDBLCLICK, IDC_ITEM_BREAKDOWN, OnEndtrackBreakdownList)
    ON_BN_CLICKED(IDC_BUTTON_CLIPBOARD, OnButtonClipboardCopy)
END_MESSAGE_MAP()
#pragma warning(pop)

void CBreakdownsPane::RegisterBuildCallbackEffect(
    EffectType type,
    EffectCallbackItem* pItem)
{
    if (m_mapBuildCallbacks.find(type) == m_mapBuildCallbacks.end())
    {
        // does not currently exist, add it
        std::list<EffectCallbackItem*> items;
        items.push_back(pItem);
        m_mapBuildCallbacks[type] = items;
    }
    else
    {
        // need to update its list of points
        std::list<EffectCallbackItem*> items = m_mapBuildCallbacks[type];
        items.push_back(pItem);
        m_mapBuildCallbacks[type] = items;
    }
}

void CBreakdownsPane::UpdateBreakdown()
{
    OnSelChangedBreakdownTree(NULL, NULL);
}

LRESULT CBreakdownsPane::OnNewDocument(WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(wParam);
    if (m_pCharacter != NULL)
    {
        m_pCharacter->DetachObserver(this);
        m_pCharacter = NULL;
    }
    // wParam is the document pointer
    // lParam is the character pointer
    Character * pCharacter = (Character *)(lParam);
    if (pCharacter != m_pCharacter)
    {
        m_pCharacter = pCharacter;
        // let all the breakdown items know about the document change
        for (auto&& iit: m_items)
        {
            iit->BuildChanged(m_pCharacter);
        }
        if (pCharacter != NULL)
        {
            // need to know about gear changes
            pCharacter->AttachObserver(this);
        }
        else
        {
            if (IsWindow(m_itemBreakdownList.GetSafeHwnd()))
            {
                // just empty the control
                m_itemBreakdownList.DeleteAllItems();
                // no specific item selected, therefore for clipboard copy
                m_buttonClipboard.EnableWindow(FALSE);
            }
        }
        m_bUpdateBreakdowns = true;
    }
    return 0L;
}

void CBreakdownsPane::UpdateActiveLifeChanged(Character*)
{
    Life* pLife = m_pCharacter->ActiveLife();
    Build* pBuild = m_pCharacter->ActiveBuild();
    if (pLife != NULL
        && pBuild != NULL)
    {
        pLife->AttachObserver(this);
        pBuild->AttachObserver(this);
        for (auto&& iit : m_items)
        {
            iit->BuildChanged(m_pCharacter);
        }
        for (auto&& iit : m_items)
        {
            iit->BuildChangeComplete();
        }
    }
}

void CBreakdownsPane::UpdateActiveBuildChanged(Character* pCharacter)
{
    Life* pLife = m_pCharacter->ActiveLife();
    Build* pBuild = m_pCharacter->ActiveBuild();
    if (pLife != NULL
        && pBuild != NULL)
    {
        pLife->AttachObserver(this);
        pBuild->AttachObserver(this);
        for (auto&& iit : m_items)
        {
            iit->BuildChanged(m_pCharacter);
        }
        for (auto&& iit : m_items)
        {
            iit->BuildChangeComplete();
        }
    }
    if (m_pWeaponEffects != NULL)
    {
        m_pWeaponEffects->SetCharacter(pCharacter);
    }
}

void CBreakdownsPane::OnInitialUpdate()
{
    if (!m_bHadInitialUpdate)
    {
        m_bHadInitialUpdate = true;
        CFormView::OnInitialUpdate();

        m_treeSizePercent = AfxGetApp()->GetProfileInt(f_treeSizeKey, f_treeSizeEntry, 75);
        m_itemBreakdownTree.CreateEx(
                WS_EX_CLIENTEDGE,
                WC_TREEVIEW,
                "BreakdownItems",
                WS_VISIBLE | WS_CHILD | WS_TABSTOP,
                CRect(0,0,10,10),
                this,
                1);
        m_itemBreakdownTree.InsertColumn(0, "Item", LVCFMT_LEFT, 250);
        m_itemBreakdownTree.InsertColumn(1, "Value", LVCFMT_LEFT, 75);

        m_itemBreakdownList.InsertColumn(0, "Breakdown source", LVCFMT_LEFT, 220);
        m_itemBreakdownList.InsertColumn(1, "Stacks", LVCFMT_LEFT, 50);
        m_itemBreakdownList.InsertColumn(2, "Value", LVCFMT_LEFT, 50);
        m_itemBreakdownList.InsertColumn(3, "Bonus Type", LVCFMT_LEFT, 50);
        m_itemBreakdownList.SetExtendedStyle(m_itemBreakdownList.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_LABELTIP);
        LoadColumnWidthsByName(&m_itemBreakdownList, "BreakdownList_%s");
        // Image for copy to clipboard button
        m_buttonClipboard.SetImage(IDB_BITMAP_COPYTOCLIPBOARD);
    }
}

LRESULT CBreakdownsPane::OnLoadComplete(WPARAM, LPARAM)
{
    CreateBreakdowns();
    return 0;
}

void CBreakdownsPane::OnSize(UINT nType, int cx, int cy)
{
    CWnd::OnSize(nType, cx, cy);
    if (IsWindow(m_itemBreakdownList.GetSafeHwnd()))
    {
        // +---------------------------------+  ^
        // |                                 |  |
        // |    Breakdowns Tree List         |  | m_treeSizePercent as a percentage
        // |                                 |  |
        // |                                 |  |
        // |                                 |  |
        // |                                 |  |
        // +---------------------------------+  v
        // [-- -------Drag divider--------][c]
        // +---------------------------------+
        // |   Selected Item breakdown List  |
        // |                                 |
        // |                                 |
        // +---------------------------------+
        // control area configured by the m_treeSizePercent variable which is a percentage
        CRect rctDivider;
        CRect rctButtonClipboard;
        m_divider.GetWindowRect(rctDivider);
        m_buttonClipboard.GetWindowRect(rctButtonClipboard);
        // position the divider at its percentage location
        rctDivider -= rctDivider.TopLeft();
        rctDivider.left = c_controlSpacing;
        rctDivider.right = cx - c_controlSpacing - rctButtonClipboard.Width() - c_controlSpacing;
        int verticalSpace = cy - (c_controlSpacing * 4) - rctDivider.Height();
        rctDivider += CPoint (0, c_controlSpacing * 2 + (verticalSpace * m_treeSizePercent) / 100);
        rctButtonClipboard -= rctButtonClipboard.TopLeft();
        rctButtonClipboard += CPoint(cx - rctButtonClipboard.Width() - c_controlSpacing,c_controlSpacing * 2 + (verticalSpace * m_treeSizePercent) / 100);
        // all other items placed relative to the divider
        CRect rctTree(c_controlSpacing, c_controlSpacing, cx - c_controlSpacing, rctDivider.top - c_controlSpacing);
        CRect rctList(c_controlSpacing, rctDivider.bottom + c_controlSpacing, cx - c_controlSpacing, cy - c_controlSpacing);
        m_itemBreakdownTree.MoveWindow(rctTree);
        m_divider.MoveWindow(rctDivider);
        m_buttonClipboard.MoveWindow(rctButtonClipboard);
        m_itemBreakdownList.MoveWindow(rctList);
    }
}

void CBreakdownsPane::CreateBreakdowns()
{
    CreateAbilityBreakdowns();
    CreateSavesBreakdowns();
    CreateSkillBreakdowns();
    CreatePhysicalBreakdowns();
    CreateMagicalBreakdowns();
    CreateSongBreakdowns();
    CreateTurnUndeadBreakdowns();
    CreateEnergyResistancesBreakdowns();
    CreateHirelingBreakdowns();
    CreateWeaponBreakdowns();
}

void CBreakdownsPane::CreateAbilityBreakdowns()
{
    // insert the abilities root tree item
    HTREEITEM hParent = m_itemBreakdownTree.InsertItem(
            "Abilities", 
            0,
            TVI_ROOT);
    m_itemBreakdownTree.SetItemData(hParent, 0);
    for (size_t ai = Ability_Unknown + 1; ai < Ability_Count; ++ai)
    {
        HTREEITEM hItem = m_itemBreakdownTree.InsertItem(
                EnumEntryText((AbilityType)ai, abilityTypeMap),
                hParent,
                TVI_LAST);
        BreakdownItem * pAbility = new BreakdownItemAbility(
                this,
                (AbilityType)ai,
                (BreakdownType)(Breakdown_Strength + ai - Ability_Unknown - 1),
                &m_itemBreakdownTree,
                hItem);
        m_itemBreakdownTree.SetItemData(hItem, (DWORD)(void*)pAbility);
        m_items.push_back(pAbility);
        // we need to monitor changes of ability breakdowns
        pAbility->AttachObserver(this);
    }
}

void CBreakdownsPane::CreateSavesBreakdowns()
{
    // insert the Saves root tree item
    HTREEITEM hParent = m_itemBreakdownTree.InsertItem(
            "Saving Throws", 
            0,
            TVI_ROOT);
    m_itemBreakdownTree.SetItemData(hParent, 0);
    {
        HTREEITEM hItem = m_itemBreakdownTree.InsertItem(
                "Fortitude",
                hParent,
                TVI_LAST);
        BreakdownItemSave * pFS = new BreakdownItemSave(
                this,
                Breakdown_SaveFortitude,
                Save_Fortitude,
                &m_itemBreakdownTree,
                hItem,
                Ability_Constitution,
                NULL);
        m_itemBreakdownTree.SetItemData(hItem, (DWORD)(void*)pFS);
        m_items.push_back(pFS);
        // some sub variations of fortitude for specific sub-save types
        {
            HTREEITEM hSubItem = m_itemBreakdownTree.InsertItem(
                    "vs Poison",
                    hItem,
                    TVI_LAST);
            BreakdownItem * pPS = new BreakdownItemSave(
                    this,
                    Breakdown_SavePoison,
                    Save_Poison,
                    &m_itemBreakdownTree,
                    hSubItem,
                    Ability_Unknown,
                    pFS);
            m_itemBreakdownTree.SetItemData(hSubItem, (DWORD)(void*)pPS);
            m_items.push_back(pPS);
        }
        {
            HTREEITEM hSubItem = m_itemBreakdownTree.InsertItem(
                    "vs Disease",
                    hItem,
                    TVI_LAST);
            BreakdownItem * pPS = new BreakdownItemSave(
                    this,
                    Breakdown_SaveDisease,
                    Save_Disease,
                    &m_itemBreakdownTree,
                    hSubItem,
                    Ability_Unknown,
                    pFS);
            m_itemBreakdownTree.SetItemData(hSubItem, (DWORD)(void*)pPS);
            m_items.push_back(pPS);
        }
    }

    {
        HTREEITEM hItem = m_itemBreakdownTree.InsertItem(
                "Reflex",
                hParent,
                TVI_LAST);
        BreakdownItemSave * pRS = new BreakdownItemSave(
                this,
                Breakdown_SaveReflex,
                Save_Reflex,
                &m_itemBreakdownTree,
                hItem,
                Ability_Dexterity,
                NULL);
        m_itemBreakdownTree.SetItemData(hItem, (DWORD)(void*)pRS);
        m_items.push_back(pRS);
        // some sub variations of reflex for specific sub-save types
        {
            HTREEITEM hSubItem = m_itemBreakdownTree.InsertItem(
                    "vs Traps",
                    hItem,
                    TVI_LAST);
            BreakdownItem * pTS = new BreakdownItemSave(
                    this,
                    Breakdown_SaveTraps,
                    Save_Traps,
                    &m_itemBreakdownTree,
                    hSubItem,
                    Ability_Unknown,
                    pRS);
            m_itemBreakdownTree.SetItemData(hSubItem, (DWORD)(void*)pTS);
            m_items.push_back(pTS);
        }
        {
            HTREEITEM hSubItem = m_itemBreakdownTree.InsertItem(
                    "vs Spell",
                    hItem,
                    TVI_LAST);
            BreakdownItem * pSS = new BreakdownItemSave(
                    this,
                    Breakdown_SaveSpell,
                    Save_Spell,
                    &m_itemBreakdownTree,
                    hSubItem,
                    Ability_Unknown,
                    pRS);
            m_itemBreakdownTree.SetItemData(hSubItem, (DWORD)(void*)pSS);
            m_items.push_back(pSS);
        }
        {
            HTREEITEM hSubItem = m_itemBreakdownTree.InsertItem(
                    "vs Magic",
                    hItem,
                    TVI_LAST);
            BreakdownItem * pSM = new BreakdownItemSave(
                    this,
                    Breakdown_SaveMagic,
                    Save_Magic,
                    &m_itemBreakdownTree,
                    hSubItem,
                    Ability_Unknown,
                    pRS);
            m_itemBreakdownTree.SetItemData(hSubItem, (DWORD)(void*)pSM);
            m_items.push_back(pSM);
        }
    }

    {
        HTREEITEM hItem = m_itemBreakdownTree.InsertItem(
                "Will",
                hParent,
                TVI_LAST);
        BreakdownItemSave * pWS = new BreakdownItemSave(
                this,
                Breakdown_SaveWill,
                Save_Will,
                &m_itemBreakdownTree,
                hItem,
                Ability_Wisdom,
                NULL);
        m_itemBreakdownTree.SetItemData(hItem, (DWORD)(void*)pWS);
        m_items.push_back(pWS);
        // some sub variations of Will for specific sub-save types
        {
            HTREEITEM hSubItem = m_itemBreakdownTree.InsertItem(
                    "vs Enchantment",
                    hItem,
                    TVI_LAST);
            BreakdownItem * pES = new BreakdownItemSave(
                    this,
                    Breakdown_SaveEnchantment,
                    Save_Enchantment,
                    &m_itemBreakdownTree,
                    hSubItem,
                    Ability_Unknown,
                    pWS);
            m_itemBreakdownTree.SetItemData(hSubItem, (DWORD)(void*)pES);
            m_items.push_back(pES);
        }
        {
            HTREEITEM hSubItem = m_itemBreakdownTree.InsertItem(
                    "vs Illusion",
                    hItem,
                    TVI_LAST);
            BreakdownItem * pIS = new BreakdownItemSave(
                    this,
                    Breakdown_SaveIllusion,
                    Save_Illusion,
                    &m_itemBreakdownTree,
                    hSubItem,
                    Ability_Unknown,
                    pWS);
            m_itemBreakdownTree.SetItemData(hSubItem, (DWORD)(void*)pIS);
            m_items.push_back(pIS);
        }
        {
            HTREEITEM hSubItem = m_itemBreakdownTree.InsertItem(
                    "vs Fear",
                    hItem,
                    TVI_LAST);
            BreakdownItem * pFS = new BreakdownItemSave(
                    this,
                    Breakdown_SaveFear,
                    Save_Fear,
                    &m_itemBreakdownTree,
                    hSubItem,
                    Ability_Unknown,
                    pWS);
            m_itemBreakdownTree.SetItemData(hSubItem, (DWORD)(void*)pFS);
            m_items.push_back(pFS);
        }
        {
            HTREEITEM hSubItem = m_itemBreakdownTree.InsertItem(
                    "vs Curse",
                    hItem,
                    TVI_LAST);
            BreakdownItem * pSC = new BreakdownItemSave(
                    this,
                    Breakdown_SaveCurse,
                    Save_Curse,
                    &m_itemBreakdownTree,
                    hSubItem,
                    Ability_Unknown,
                    pWS);
            m_itemBreakdownTree.SetItemData(hSubItem, (DWORD)(void*)pSC);
            m_items.push_back(pSC);
        }
    }
}

void CBreakdownsPane::CreateSkillBreakdowns()
{
    // insert the skills root tree item
    HTREEITEM hParent = m_itemBreakdownTree.InsertItem(
            "Skills", 
            0,
            TVI_ROOT);
    m_itemBreakdownTree.SetItemData(hParent, 0);
    for (size_t si = Skill_Unknown + 1; si < Skill_Count; ++si)
    {
        // find the correct ability breakdown for this skill
        // so it can be observed
        AbilityType at = StatFromSkill((SkillType)si);
        BreakdownItem * pAbility = NULL;
        switch (at)
        {
        case Ability_Strength:
            pAbility = FindBreakdown(Breakdown_Strength);
            break;
        case Ability_Dexterity:
            pAbility = FindBreakdown(Breakdown_Dexterity);
            break;
        case Ability_Constitution:
            pAbility = FindBreakdown(Breakdown_Constitution);
            break;
        case Ability_Intelligence:
            pAbility = FindBreakdown(Breakdown_Intelligence);
            break;
        case Ability_Wisdom:
            pAbility = FindBreakdown(Breakdown_Wisdom);
            break;
        case Ability_Charisma:
            pAbility = FindBreakdown(Breakdown_Charisma);
            break;
        }
        ASSERT(pAbility != NULL);   // should have found one
        HTREEITEM hItem = m_itemBreakdownTree.InsertItem(
                EnumEntryText((SkillType)si, skillTypeMap),
                hParent,
                TVI_LAST);
        BreakdownItem * pSkill = new BreakdownItemSkill(
                this,
                static_cast<SkillType>(si),
                static_cast<BreakdownType>(Breakdown_SkillBalance + si - Skill_Unknown - 1),
                &m_itemBreakdownTree,
                hItem,
                pAbility);
        m_itemBreakdownTree.SetItemData(hItem, (DWORD)(void*)pSkill);
        m_items.push_back(pSkill);
    }
}

void CBreakdownsPane::CreatePhysicalBreakdowns()
{
    // insert the physical root tree item
    HTREEITEM hParent = m_itemBreakdownTree.InsertItem(
            "Physical Breakdowns", 
            0,
            TVI_ROOT);
    m_itemBreakdownTree.SetItemData(hParent, 0);

    {
        HTREEITEM hItem = m_itemBreakdownTree.InsertItem(
                "Negative Levels",
                hParent,
                TVI_LAST);
        BreakdownItem * pNegLevel = new BreakdownItemSimple(
                this,
                Breakdown_NegativeLevels,
                Effect_NegativeLevel,
                "Negative Levels",
                &m_itemBreakdownTree,
                hItem);
        m_itemBreakdownTree.SetItemData(hItem, (DWORD)(void*)pNegLevel);
        m_items.push_back(pNegLevel);
    }
    {
        HTREEITEM hItem = m_itemBreakdownTree.InsertItem(
            "Fate Points",
            hParent,
            TVI_LAST);
        BreakdownItem* pU51FP = new BreakdownItemSimple(
            this,
            Breakdown_FatePoints,
            Effect_FatePoint,
            "Fate Points",
            &m_itemBreakdownTree,
            hItem);
        m_itemBreakdownTree.SetItemData(hItem, (DWORD)(void*)pU51FP);
        m_items.push_back(pU51FP);
    }
    {
        HTREEITEM hItem = m_itemBreakdownTree.InsertItem(
                "Destiny APs",
                hParent,
                TVI_LAST);
        BreakdownItem * pDestinyAps = new BreakdownItemDestinyAps(
                this,
                &m_itemBreakdownTree,
                hItem);
        m_itemBreakdownTree.SetItemData(hItem, (DWORD)(void*)pDestinyAps);
        m_items.push_back(pDestinyAps);
    }
    {
        HTREEITEM hItem = m_itemBreakdownTree.InsertItem(
                "Hitpoints",
                hParent,
                TVI_LAST);
        BreakdownItem * pHP = new BreakdownItemHitpoints(
                this,
                &m_itemBreakdownTree,
                hItem,
                FindBreakdown(Breakdown_Constitution));
        m_itemBreakdownTree.SetItemData(hItem, (DWORD)(void*)pHP);
        m_items.push_back(pHP);
        {
            HTREEITEM hStyleBonus = m_itemBreakdownTree.InsertItem(
                    "Style Bonus Feats",
                    hItem,
                    TVI_LAST);
            BreakdownItem * pSB = new BreakdownItemSimple(
                    this,
                    Breakdown_StyleBonusFeats,
                    Effect_HitpointsStyleBonus,
                    "Style Bonus Feats",
                    &m_itemBreakdownTree,
                    hStyleBonus);
            m_itemBreakdownTree.SetItemData(hStyleBonus, (DWORD)(void*)pSB);
            m_items.push_back(pSB);
        }
        {
            HTREEITEM hUnconsciousRange = m_itemBreakdownTree.InsertItem(
                    "Unconscious Range",
                    hItem,
                    TVI_LAST);
            BreakdownItem * pUR = new BreakdownItemSimple(
                    this,
                    Breakdown_UnconsciousRange,
                    Effect_UnconsciousRange,
                    "Unconscious Range",
                    &m_itemBreakdownTree,
                    hUnconsciousRange);
            m_itemBreakdownTree.SetItemData(hUnconsciousRange, (DWORD)(void*)pUR);
            m_items.push_back(pUR);
        }
        {
            HTREEITEM hFalseLife = m_itemBreakdownTree.InsertItem(
                    "False Life",
                    hItem,
                    TVI_LAST);
            BreakdownItem* pFL = new BreakdownItemSimple(
                    this,
                    Breakdown_FalseLife,
                    Effect_FalseLife,
                    "False Life",
                    &m_itemBreakdownTree,
                    hFalseLife);
            m_itemBreakdownTree.SetItemData(hFalseLife, (DWORD)(void*)pFL);
            m_items.push_back(pFL);
        }
        {
            HTREEITEM hReaperHP = m_itemBreakdownTree.InsertItem(
                "Reaper Hitpoints",
                hItem,
                TVI_LAST);
            BreakdownItem* pRHP = new BreakdownItemSimple(
                this,
                Breakdown_ReaperHitpoints,
                Effect_HitpointsReaper,
                "Reaper Hitpoints",
                &m_itemBreakdownTree,
                hReaperHP);
            m_itemBreakdownTree.SetItemData(hReaperHP, (DWORD)(void*)pRHP);
            m_items.push_back(pRHP);
        }
    }
    {
        HTREEITEM hItem = m_itemBreakdownTree.InsertItem(
                "Movement Speed",
                hParent,
                TVI_LAST);
        BreakdownItem * pSpeed = new BreakdownItemSimple(
                this,
                Breakdown_MovementSpeed,
                Effect_MovementSpeed,
                "Movement Speed",
                &m_itemBreakdownTree,
                hItem);
        m_itemBreakdownTree.SetItemData(hItem, (DWORD)(void*)pSpeed);
        m_items.push_back(pSpeed);
    }
    {
        HTREEITEM hItem = m_itemBreakdownTree.InsertItem(
                "Armor Check Penalty",
                hParent,
                TVI_LAST);
        BreakdownItem * pACP = new BreakdownItemSimple(
                this,
                Breakdown_ArmorCheckPenalty,
                Effect_ArmorCheckPenalty,
                "Armor Check penalty",
                &m_itemBreakdownTree,
                hItem);
        m_itemBreakdownTree.SetItemData(hItem, (DWORD)(void*)pACP);
        m_items.push_back(pACP);
    }
    // defensive physical items
    // defensive items are:
    //      AC
    //          Shield AC bonus
    //          Armor AC bonus
    //      MaxDexBonus
    //          MaxDexBonusShields
    //      PRR
    //      MRR
    //      Fortification
    //      Dodge
    //          Dodge Cap
    //      Missile Deflection
    HTREEITEM hDefensiveParent = m_itemBreakdownTree.InsertItem(
            "Defensive Breakdowns", 
            hParent,
            TVI_LAST);
    m_itemBreakdownTree.SetItemData(hDefensiveParent, 0);
    {
        HTREEITEM hItem = m_itemBreakdownTree.InsertItem(
                "AC",
                hDefensiveParent,
                TVI_LAST);
        BreakdownItem * pAC = new BreakdownItemAC(
                this,
                Breakdown_AC,
                &m_itemBreakdownTree,
                hItem);
        m_itemBreakdownTree.SetItemData(hItem, (DWORD)(void*)pAC);
        m_items.push_back(pAC);
        {
            HTREEITEM hNaturalArmor = m_itemBreakdownTree.InsertItem(
                    "Natural Armor",
                    hItem,
                    TVI_LAST);
            BreakdownItem * pNaturalArmor = new BreakdownItemSimple(
                    this,
                    Breakdown_NaturalArmor,
                    Effect_NaturalArmor,
                    "Natural Armor",
                    &m_itemBreakdownTree,
                    hNaturalArmor);
            m_itemBreakdownTree.SetItemData(hNaturalArmor, (DWORD)(void*)pNaturalArmor);
            m_items.push_back(pNaturalArmor);
        }
        {
            HTREEITEM hArmorItem = m_itemBreakdownTree.InsertItem(
                    "Armor % Bonus",
                    hItem,
                    TVI_LAST);
            BreakdownItem * pArmorAC = new BreakdownItemSimple(
                    this,
                    Breakdown_BonusArmorAC,
                    Effect_ArmorACBonus,
                    "Armor % Bonus",
                    &m_itemBreakdownTree,
                    hArmorItem);
            m_itemBreakdownTree.SetItemData(hArmorItem, (DWORD)(void*)pArmorAC);
            m_items.push_back(pArmorAC);
        }
        {
            HTREEITEM hShieldItem = m_itemBreakdownTree.InsertItem(
                    "Shield % Bonus",
                    hItem,
                    TVI_LAST);
            BreakdownItem * pShieldAC = new BreakdownItemSimple(
                    this,
                    Breakdown_BonusShieldAC,
                    Effect_ACBonusShield,
                    "Shield % Bonus",
                    &m_itemBreakdownTree,
                    hShieldItem);
            m_itemBreakdownTree.SetItemData(hShieldItem, (DWORD)(void*)pShieldAC);
            m_items.push_back(pShieldAC);
        }
    }

    {
        HTREEITEM hItem = m_itemBreakdownTree.InsertItem(
                "Max Dex Bonus",
                hDefensiveParent,
                TVI_LAST);
        BreakdownItem * pMDB = new BreakdownItemMDB(
                this,
                Breakdown_MaxDexBonus,
                &m_itemBreakdownTree,
                hItem);
        m_itemBreakdownTree.SetItemData(hItem, (DWORD)(void*)pMDB);
        m_items.push_back(pMDB);
        {
            HTREEITEM hMDBShields = m_itemBreakdownTree.InsertItem(
                    "Tower Shield MDB",
                    hItem,
                    TVI_LAST);
            BreakdownItem * pMDBSheilds = new BreakdownItemSimple(
                    this,
                    Breakdown_MaxDexBonusShields,
                    Effect_MaxDexBonusTowerShield,
                    "Tower Shield MDB",
                    &m_itemBreakdownTree,
                    hMDBShields);
            m_itemBreakdownTree.SetItemData(hMDBShields, (DWORD)(void*)pMDBSheilds);
            m_items.push_back(pMDBSheilds);
        }
    }

    {
        HTREEITEM hItem = m_itemBreakdownTree.InsertItem(
                "DR",
                hDefensiveParent,
                TVI_LAST);
        BreakdownItem * pDR = new BreakdownItemDR(
                this,
                Breakdown_DR,
                &m_itemBreakdownTree,
                hItem);
        m_itemBreakdownTree.SetItemData(hItem, (DWORD)(void*)pDR);
        m_items.push_back(pDR);
    }

    {
        HTREEITEM hItem = m_itemBreakdownTree.InsertItem(
                "PRR",
                hDefensiveParent,
                TVI_LAST);
        BreakdownItem * pPRR = new BreakdownItemPRR(
                this,
                Breakdown_PRR,
                "PRR",
                &m_itemBreakdownTree,
                hItem);
        m_itemBreakdownTree.SetItemData(hItem, (DWORD)(void*)pPRR);
        m_items.push_back(pPRR);
    }

    {
        HTREEITEM hItem = m_itemBreakdownTree.InsertItem(
                "MRR",
                hDefensiveParent,
                TVI_LAST);
        BreakdownItem * pMRR = new BreakdownItemMRR(
                this,
                Breakdown_MRR,
                "MRR",
                &m_itemBreakdownTree,
                hItem);
        m_itemBreakdownTree.SetItemData(hItem, (DWORD)(void*)pMRR);
        m_items.push_back(pMRR);
        {
            HTREEITEM hItemCap = m_itemBreakdownTree.InsertItem(
                    "MRR Cap",
                    hItem,
                    TVI_LAST);
            BreakdownItem * pMRRCap = new BreakdownItemMRRCap(
                    this,
                    Breakdown_MRRCap,
                    "MRR Cap",
                    &m_itemBreakdownTree,
                    hItemCap);
            m_itemBreakdownTree.SetItemData(hItemCap, (DWORD)(void*)pMRRCap);
            m_items.push_back(pMRRCap);
        }
    }

    {
        HTREEITEM hItem = m_itemBreakdownTree.InsertItem(
                "Fortification",
                hDefensiveParent,
                TVI_LAST);
        BreakdownItem * pFort= new BreakdownItemSimple(
                this,
                Breakdown_Fortification,
                Effect_Fortification,
                "Fortification",
                &m_itemBreakdownTree,
                hItem);
        m_itemBreakdownTree.SetItemData(hItem, (DWORD)(void*)pFort);
        m_items.push_back(pFort);
    }

    {
        HTREEITEM hItem = m_itemBreakdownTree.InsertItem(
                "Dodge",
                hDefensiveParent,
                TVI_LAST);
        BreakdownItem * pDodge = new BreakdownItemDodge(
                this,
                Breakdown_Dodge,
                "Dodge",
                &m_itemBreakdownTree,
                hItem);
        m_itemBreakdownTree.SetItemData(hItem, (DWORD)(void*)pDodge);
        m_items.push_back(pDodge);
        {
            HTREEITEM hItemCap = m_itemBreakdownTree.InsertItem(
                    "Dodge Cap (Armor)",
                    hItem,
                    TVI_LAST);
            BreakdownItem * pDC = new BreakdownItemSimple(
                    this,
                    Breakdown_DodgeCap,
                    Effect_DodgeCapBonus,
                    "Dodge Cap (Armor)",
                    &m_itemBreakdownTree,
                    hItemCap);
            m_itemBreakdownTree.SetItemData(hItemCap, (DWORD)(void*)pDC);
            m_items.push_back(pDC);
        }
        {
            HTREEITEM hItemCap2 = m_itemBreakdownTree.InsertItem(
                    "Dodge Cap (Tower Shield)",
                    hItem,
                    TVI_LAST);
            BreakdownItem * pDC2 = new BreakdownItemSimple(
                    this,
                    Breakdown_DodgeCapTowerShield,
                    Effect_MaxDexBonusTowerShield,
                    "Dodge Cap (Tower Shield)",
                    &m_itemBreakdownTree,
                    hItemCap2);
            m_itemBreakdownTree.SetItemData(hItemCap2, (DWORD)(void*)pDC2);
            m_items.push_back(pDC2);
        }
    }

    {
        HTREEITEM hItem = m_itemBreakdownTree.InsertItem(
                "Missile Deflection",
                hDefensiveParent,
                TVI_LAST);
        BreakdownItem * pMD = new BreakdownItemSimple(
                this,
                Breakdown_MissileDeflection,
                Effect_MissileDeflection,
                "Missile Deflection",
                &m_itemBreakdownTree,
                hItem);
        m_itemBreakdownTree.SetItemData(hItem, (DWORD)(void*)pMD);
        m_items.push_back(pMD);
    }

    {
        HTREEITEM hItem = m_itemBreakdownTree.InsertItem(
                "Incorporeality",
                hDefensiveParent,
                TVI_LAST);
        BreakdownItem * pIncorp = new BreakdownItemSimple(
                this,
                Breakdown_Incorporeality,
                Effect_Incorporeality,
                "Incorporeality",
                &m_itemBreakdownTree,
                hItem);
        m_itemBreakdownTree.SetItemData(hItem, (DWORD)(void*)pIncorp);
        m_items.push_back(pIncorp);
    }

    {
        HTREEITEM hItem = m_itemBreakdownTree.InsertItem(
                "Displacement",
                hDefensiveParent,
                TVI_LAST);
        BreakdownItem * pDisp = new BreakdownItemSimple(
                this,
                Breakdown_Displacement,
                Effect_Displacement,
                "Displacement",
                &m_itemBreakdownTree,
                hItem);
        m_itemBreakdownTree.SetItemData(hItem, (DWORD)(void*)pDisp);
        m_items.push_back(pDisp);
    }
    {
        HTREEITEM hHelpless = m_itemBreakdownTree.InsertItem(
                "Helpless Damage Reduction",
                hDefensiveParent,
                TVI_LAST);
        BreakdownItem * pHelpless = new BreakdownItemSimple(
                this,
                Breakdown_HelplessDamageReduction,
                Effect_HelplessDamageReduction,
                "Helpless Damage Reduction",
                &m_itemBreakdownTree,
                hHelpless);
        m_itemBreakdownTree.SetItemData(hHelpless, (DWORD)(void*)pHelpless);
        m_items.push_back(pHelpless);
    }

    {
        HTREEITEM hItem = m_itemBreakdownTree.InsertItem(
                "Healing Amplification (Positive)",
                hParent,
                TVI_LAST);
        BreakdownItem * pHA = new BreakdownItemSimple(
                this,
                Breakdown_HealingAmplification,
                Effect_HealingAmplification,
                "Healing Amplification (Positive)",
                &m_itemBreakdownTree,
                hItem);
        m_itemBreakdownTree.SetItemData(hItem, (DWORD)(void*)pHA);
        m_items.push_back(pHA);
    }

    {
        HTREEITEM hItem = m_itemBreakdownTree.InsertItem(
                "Healing Amplification (Negative)",
                hParent,
                TVI_LAST);
        BreakdownItem * pNHA = new BreakdownItemSimple(
                this,
                Breakdown_NegativeHealingAmplification,
                Effect_NegativeHealingAmplification,
                "Healing Amplification (Negative)",
                &m_itemBreakdownTree,
                hItem);
        m_itemBreakdownTree.SetItemData(hItem, (DWORD)(void*)pNHA);
        m_items.push_back(pNHA);
    }

    {
        HTREEITEM hItem = m_itemBreakdownTree.InsertItem(
                "Repair Amplification",
                hParent,
                TVI_LAST);
        BreakdownItem * pRA = new BreakdownItemSimple(
                this,
                Breakdown_RepairAmplification,
                Effect_RepairAmplification,
                "Repair Amplification",
                &m_itemBreakdownTree,
                hItem);
        m_itemBreakdownTree.SetItemData(hItem, (DWORD)(void*)pRA);
        m_items.push_back(pRA);
    }

    {
        HTREEITEM hItem = m_itemBreakdownTree.InsertItem(
                "Immunities",
                hParent,
                TVI_LAST);
        BreakdownItem * pIm = new BreakdownItemImmunities(
                this,
                Breakdown_Immunities,
                &m_itemBreakdownTree,
                hItem);
        m_itemBreakdownTree.SetItemData(hItem, (DWORD)(void*)pIm);
        m_items.push_back(pIm);
    }

    // offensive breakdowns include:
    // BAB
    // threat generation
    // off hand attack chance
    // doublestrike
    // offhand doublestrike
    // doubleshot
    // Melee Power
    // Ranged Power
    // Dodge Bypass
    // Fortification Bypass
    // Missile Deflection Bypass
    // Strikethrough
    // Helpless Damage
    // Rune arm charge rate
    // rune arm stable charge
    HTREEITEM hOffensiveParent = m_itemBreakdownTree.InsertItem(
            "Offensive Breakdowns", 
            hParent,
            TVI_LAST);
    m_itemBreakdownTree.SetItemData(hOffensiveParent, 0);
    {
        HTREEITEM hItem = m_itemBreakdownTree.InsertItem(
                "Base Attack Bonus",
                hOffensiveParent,
                TVI_LAST);
        BreakdownItem * pBAB = new BreakdownItemBAB(
                this,
                &m_itemBreakdownTree,
                hItem);
        m_itemBreakdownTree.SetItemData(hItem, (DWORD)(void*)pBAB);
        m_items.push_back(pBAB);
        {
            HTREEITEM hOverrideBab = m_itemBreakdownTree.InsertItem(
                "BAB Override",
                hItem,
                TVI_LAST);
            BreakdownItem* pOverrideBab = new BreakdownItemSimple(
                this,
                Breakdown_OverrideBAB,
                Effect_OverrideBAB,
                "BAB Override",
                &m_itemBreakdownTree,
                hOverrideBab);
            m_itemBreakdownTree.SetItemData(hOverrideBab, (DWORD)(void*)pOverrideBab);
            m_items.push_back(pOverrideBab);
        }
    }
    {
        HTREEITEM hItem = m_itemBreakdownTree.InsertItem(
            "Melee Threat Generation",
            hOffensiveParent,
            TVI_LAST);
        BreakdownItem* pThreat = new BreakdownItemSimple(
                this,
            Breakdown_ThreatMelee,
            Effect_ThreatBonusMelee,
            "Melee Threat Generation",
            &m_itemBreakdownTree,
            hItem);
        m_itemBreakdownTree.SetItemData(hItem, (DWORD)(void*)pThreat);
        m_items.push_back(pThreat);
    }
    {
        HTREEITEM hItem = m_itemBreakdownTree.InsertItem(
            "Ranged Threat Generation",
            hOffensiveParent,
            TVI_LAST);
        BreakdownItem* pThreat = new BreakdownItemSimple(
            this,
            Breakdown_ThreatRanged,
            Effect_ThreatBonusRanged,
            "Ranged Threat Generation",
            &m_itemBreakdownTree,
            hItem);
        m_itemBreakdownTree.SetItemData(hItem, (DWORD)(void*)pThreat);
        m_items.push_back(pThreat);
    }
    {
        HTREEITEM hItem = m_itemBreakdownTree.InsertItem(
            "Spell Threat Generation",
            hOffensiveParent,
            TVI_LAST);
        BreakdownItem* pThreat = new BreakdownItemSimple(
            this,
            Breakdown_ThreatSpell,
            Effect_ThreatBonusSpell,
            "Spell Threat Generation",
            &m_itemBreakdownTree,
            hItem);
        m_itemBreakdownTree.SetItemData(hItem, (DWORD)(void*)pThreat);
        m_items.push_back(pThreat);
    }

    {
        HTREEITEM hItem = m_itemBreakdownTree.InsertItem(
            "Off Hand Attack Chance",
            hOffensiveParent,
            TVI_LAST);
        BreakdownItem* pOHA = new BreakdownItemSimple(
            this,
            Breakdown_OffHandAttackBonus,
            Effect_OffHandAttackBonus,
            "Off Hand Attack Chance",
            &m_itemBreakdownTree,
            hItem);
        m_itemBreakdownTree.SetItemData(hItem, (DWORD)(void*)pOHA);
        m_items.push_back(pOHA);
    }

    {
        HTREEITEM hItem = m_itemBreakdownTree.InsertItem(
            "Doublestrike",
            hOffensiveParent,
            TVI_LAST);
        BreakdownItem* pDS = new BreakdownItemSimple(
            this,
            Breakdown_DoubleStrike,
            Effect_DoubleStrike,
            "Doublestrike",
            &m_itemBreakdownTree,
            hItem);
        m_itemBreakdownTree.SetItemData(hItem, (DWORD)(void*)pDS);
        m_items.push_back(pDS);
    }

    {
        HTREEITEM hItem = m_itemBreakdownTree.InsertItem(
            "Offhand Doublestrike",
            hOffensiveParent,
            TVI_LAST);
        BreakdownItem* pODS = new BreakdownItemOffhandDoublestrike(
            this,
            Breakdown_DoublestrikeOffhand,
            "Offhand Doublestrike",
            &m_itemBreakdownTree,
            hItem);
        m_itemBreakdownTree.SetItemData(hItem, (DWORD)(void*)pODS);
        m_items.push_back(pODS);
    }

    {
        HTREEITEM hItem = m_itemBreakdownTree.InsertItem(
            "Bonus Imbue Dice",
            hOffensiveParent,
            TVI_LAST);
        BreakdownItem* pID = new BreakdownItemSimple(
                this,
                Breakdown_ImbueDice,
                Effect_ImbueDice,
                "Bonus Imbue Dice",
                &m_itemBreakdownTree,
                hItem);
        m_itemBreakdownTree.SetItemData(hItem, (DWORD)(void*)pID);
        m_items.push_back(pID);
    }

    {
        HTREEITEM hItem = m_itemBreakdownTree.InsertItem(
                "Sneak Attack Dice",
                hOffensiveParent,
                TVI_LAST);
        BreakdownItem * pSAD = new BreakdownItemSneakAttackDice(
                this,
                Breakdown_SneakAttackDice,
                Effect_SneakAttackDice,
                "Sneak Attack Dice",
                &m_itemBreakdownTree,
                hItem);
        m_itemBreakdownTree.SetItemData(hItem, (DWORD)(void*)pSAD);
        m_items.push_back(pSAD);
    }

    {
        HTREEITEM hItem = m_itemBreakdownTree.InsertItem(
                "Sneak Attack Damage",
                hOffensiveParent,
                TVI_LAST);
        BreakdownItem * pSAD = new BreakdownItemSimple(
                this,
                Breakdown_SneakAttackDamage,
                Effect_SneakAttackDamage,
                "Sneak Attack Damage",
                &m_itemBreakdownTree,
                hItem);
        m_itemBreakdownTree.SetItemData(hItem, (DWORD)(void*)pSAD);
        m_items.push_back(pSAD);
    }

    {
        HTREEITEM hItem = m_itemBreakdownTree.InsertItem(
                "Sneak Attack Attack",
                hOffensiveParent,
                TVI_LAST);
        BreakdownItem * pSAA = new BreakdownItemSimple(
                this,
                Breakdown_SneakAttackAttack,
                Effect_SneakAttackAttack,
                "Sneak Attack Attack",
                &m_itemBreakdownTree,
                hItem);
        m_itemBreakdownTree.SetItemData(hItem, (DWORD)(void*)pSAA);
        m_items.push_back(pSAA);
    }

    {
        HTREEITEM hItem = m_itemBreakdownTree.InsertItem(
                "Sneak Attack Range",
                hOffensiveParent,
                TVI_LAST);
        BreakdownItem * pSAR = new BreakdownItemSimple(
                this,
                Breakdown_SneakAttackRange,
                Effect_SneakAttackRange,
                "Sneak Attack Range",
                &m_itemBreakdownTree,
                hItem);
        m_itemBreakdownTree.SetItemData(hItem, (DWORD)(void*)pSAR);
        m_items.push_back(pSAR);
    }

    {
        HTREEITEM hItem = m_itemBreakdownTree.InsertItem(
            "Doubleshot",
            hOffensiveParent,
            TVI_LAST);
        BreakdownItem* pRA = new BreakdownItemSimple(
            this,
            Breakdown_DoubleShot,
            Effect_DoubleShot,
            "Doubleshot",
            &m_itemBreakdownTree,
            hItem);
        m_itemBreakdownTree.SetItemData(hItem, (DWORD)(void*)pRA);
        m_items.push_back(pRA);
    }
    {
        HTREEITEM hItem = m_itemBreakdownTree.InsertItem(
            "Melee Power",
            hOffensiveParent,
            TVI_LAST);
        BreakdownItem* pMP = new BreakdownItemSimple(
            this,
            Breakdown_MeleePower,
            Effect_MeleePower,
            "Melee Power",
            &m_itemBreakdownTree,
            hItem);
        m_itemBreakdownTree.SetItemData(hItem, (DWORD)(void*)pMP);
        m_items.push_back(pMP);
    }
    {
        HTREEITEM hItem = m_itemBreakdownTree.InsertItem(
            "Ranged Power",
            hOffensiveParent,
            TVI_LAST);
        BreakdownItem* pRP = new BreakdownItemSimple(
                this,
                Breakdown_RangedPower,
                Effect_RangedPower,
                "Ranged Power",
                &m_itemBreakdownTree,
                hItem);
        m_itemBreakdownTree.SetItemData(hItem, (DWORD)(void*)pRP);
        m_items.push_back(pRP);
    }

    {
        HTREEITEM hItem = m_itemBreakdownTree.InsertItem(
            "Secondary Shield Bash",
            hOffensiveParent,
            TVI_LAST);
        BreakdownItem* pSSB = new BreakdownItemSimple(
            this,
            Breakdown_SecondaryShieldBash,
            Effect_SecondaryShieldBash,
            "Secondary Shield Bash",
            &m_itemBreakdownTree,
            hItem);
        m_itemBreakdownTree.SetItemData(hItem, (DWORD)(void*)pSSB);
        m_items.push_back(pSSB);
    }
    {
        HTREEITEM hItem = m_itemBreakdownTree.InsertItem(
                "Dodge Bypass",
                hOffensiveParent,
                TVI_LAST);
        BreakdownItem * pDodgeBypass = new BreakdownItemSimple(
                this,
                Breakdown_DodgeBypass,
                Effect_DodgeBypass,
                "Dodge Bypass",
                &m_itemBreakdownTree,
                hItem);
        m_itemBreakdownTree.SetItemData(hItem, (DWORD)(void*)pDodgeBypass);
        m_items.push_back(pDodgeBypass);
    }
    {
        HTREEITEM hItem = m_itemBreakdownTree.InsertItem(
                "Fortification Bypass",
                hOffensiveParent,
                TVI_LAST);
        BreakdownItem * pFortBypass = new BreakdownItemSimple(
                this,
                Breakdown_FortificationBypass,
                Effect_FortificationBypass,
                "Fortification Bypass",
                &m_itemBreakdownTree,
                hItem);
        m_itemBreakdownTree.SetItemData(hItem, (DWORD)(void*)pFortBypass);
        m_items.push_back(pFortBypass);
    }
    {
        HTREEITEM hMDB = m_itemBreakdownTree.InsertItem(
                "Missile Deflection Bypass",
                hOffensiveParent,
                TVI_LAST);
        BreakdownItem * pMDB = new BreakdownItemSimple(
                this,
                Breakdown_MissileDeflectionBypass,
                Effect_MissileDeflectionBypass,
                "Missile Deflection Bypass",
                &m_itemBreakdownTree,
                hMDB);
        m_itemBreakdownTree.SetItemData(hMDB, (DWORD)(void*)pMDB);
        m_items.push_back(pMDB);
    }
    {
        HTREEITEM hStrike = m_itemBreakdownTree.InsertItem(
            "Strikethrough",
            hOffensiveParent,
            TVI_LAST);
        BreakdownItem* pStrike = new BreakdownItemSimple(
            this,
            Breakdown_Strikethrough,
            Effect_Strikethrough,
            "Strikethrough",
            &m_itemBreakdownTree,
            hStrike);
        m_itemBreakdownTree.SetItemData(hStrike, (DWORD)(void*)pStrike);
        m_items.push_back(pStrike);
    }
    {
        HTREEITEM hAbilityMultiplier = m_itemBreakdownTree.InsertItem(
            "Main Hand Ability Multiplier",
            hOffensiveParent,
            TVI_LAST);
        BreakdownItem* pMainHand = new BreakdownItemSimple(
            this,
            Breakdown_DamageAbilityMultiplier,
            Effect_DamageAbilityMultiplier,
            "Main Hand Ability Multiplier",
            &m_itemBreakdownTree,
            hAbilityMultiplier);
        m_itemBreakdownTree.SetItemData(hAbilityMultiplier, (DWORD)(void*)pMainHand);
        m_items.push_back(pMainHand);
    }
    {
        HTREEITEM hAbilityMultiplier = m_itemBreakdownTree.InsertItem(
            "Off Hand Ability Multiplier",
            hOffensiveParent,
            TVI_LAST);
        BreakdownItem* pOffHand = new BreakdownItemSimple(
            this,
            Breakdown_DamageAbilityMultiplierOffhand,
            Effect_DamageAbilityMultiplierOffhand,
            "Off Hand Ability Multiplier",
            &m_itemBreakdownTree,
            hAbilityMultiplier);
        m_itemBreakdownTree.SetItemData(hAbilityMultiplier, (DWORD)(void*)pOffHand);
        m_items.push_back(pOffHand);
    }
    {
        HTREEITEM hHelpless = m_itemBreakdownTree.InsertItem(
                "Helpless Damage Bonus",
                hOffensiveParent,
                TVI_LAST);
        BreakdownItem * pHelpless = new BreakdownItemSimple(
                this,
                Breakdown_HelplessDamage,
                Effect_HelplessDamage,
                "Helpless Damage Bonus",
                &m_itemBreakdownTree,
                hHelpless);
        m_itemBreakdownTree.SetItemData(hHelpless, (DWORD)(void*)pHelpless);
        m_items.push_back(pHelpless);
    }
    {
        HTREEITEM hRuneArmChargeRate= m_itemBreakdownTree.InsertItem(
                "Rune Arm Charge Rate",
                hOffensiveParent,
                TVI_LAST);
        BreakdownItem* pRACR = new BreakdownItemSimple(
                this,
                Breakdown_RuneArmChargeRate,
                Effect_RuneArmChargeRate,
                "Rune Arm Charge rate",
                &m_itemBreakdownTree,
                hRuneArmChargeRate);
        m_itemBreakdownTree.SetItemData(hRuneArmChargeRate, (DWORD)(void*)pRACR);
        m_items.push_back(pRACR);
    }
    {
        HTREEITEM hRuneArmStableCharge = m_itemBreakdownTree.InsertItem(
            "Rune Arm Stable Charge",
            hOffensiveParent,
            TVI_LAST);
        BreakdownItem* pRACR = new BreakdownItemSimple(
                this,
                Breakdown_RuneArmStableCharge,
                Effect_RuneArmStableCharge,
                "Rune Arm Stable Charge",
                &m_itemBreakdownTree,
                hRuneArmStableCharge);
        m_itemBreakdownTree.SetItemData(hRuneArmStableCharge, (DWORD)(void*)pRACR);
        m_items.push_back(pRACR);
    }

    HTREEITEM hTacticalParent = m_itemBreakdownTree.InsertItem(
            "Tactical Breakdown Bonuses", 
            hParent,
            TVI_LAST);
    m_itemBreakdownTree.SetItemData(hTacticalParent, 0);
    AddTacticalItem(Breakdown_TacticalAssassinate, Tactical_Assassinate, "Assassinate", hTacticalParent);
    AddTacticalItem(Breakdown_TacticalStunning, Tactical_Stun, "Stun", hTacticalParent);
    AddTacticalItem(Breakdown_TacticalSunder, Tactical_Sunder, "Sunder", hTacticalParent);
    AddTacticalItem(Breakdown_TacticalTrip, Tactical_Trip, "Trip", hTacticalParent);
    AddTacticalItem(Breakdown_TacticalGeneral, Tactical_General, "General", hTacticalParent);
    AddTacticalItem(Breakdown_TacticalStunningShield, Tactical_StunningShield, "Stunning Shield", hTacticalParent);
    AddTacticalItem(Breakdown_TacticalWands, Tactical_Wands, "Wands", hTacticalParent);
    AddTacticalItem(Breakdown_TacticalFear, Tactical_Fear, "Fear", hTacticalParent);
    AddTacticalItem(Breakdown_TacticalInnateAttack, Tactical_InnateAttack, "Innate Attack", hTacticalParent);
    AddTacticalItem(Breakdown_TacticalBreathWeapon, Tactical_BreathWeapon, "Breath Weapon", hTacticalParent);
    AddTacticalItem(Breakdown_TacticalPoison, Tactical_Poison, "Poison", hTacticalParent);
    AddTacticalItem(Breakdown_TacticalRuneArm, Tactical_RuneArm, "Rune Arm", hTacticalParent);

    HTREEITEM hKi = m_itemBreakdownTree.InsertItem(
            "Ki Breakdowns", 
            0,
            TVI_ROOT);
    m_itemBreakdownTree.SetItemData(hKi, 0);
    AddKiBreakdowns(hKi);
}

void CBreakdownsPane::CreateMagicalBreakdowns()
{
    // insert the magical root tree item
    HTREEITEM hParent = m_itemBreakdownTree.InsertItem(
            "Magical Breakdowns", 
            0,
            TVI_ROOT);
    m_itemBreakdownTree.SetItemData(hParent, 0);
    {
        HTREEITEM hItem = m_itemBreakdownTree.InsertItem(
                "Spell Points",
                hParent,
                TVI_LAST);
        BreakdownItem * pSP = new BreakdownItemSpellPoints(
                this,
                Breakdown_Spellpoints,
                Effect_SpellPoints,
                "Spell Points",
                &m_itemBreakdownTree,
                hItem);
        m_itemBreakdownTree.SetItemData(hItem, (DWORD)(void*)pSP);
        m_items.push_back(pSP);
    }
    {
        HTREEITEM hItem = m_itemBreakdownTree.InsertItem(
                "Wildsurge Chance",
                hParent,
                TVI_LAST);
        BreakdownItem * pWS = new BreakdownItemSimple(
                this,
                Breakdown_Wildsurge,
                Effect_WildsurgeChance,
                "Wildsurge Chance",
                &m_itemBreakdownTree,
                hItem);
        m_itemBreakdownTree.SetItemData(hItem, (DWORD)(void*)pWS);
        m_items.push_back(pWS);
    }
    // Warlock Eldritch blast
    {
        HTREEITEM hItem = m_itemBreakdownTree.InsertItem(
                "Eldritch Blast Dice",
                hParent,
                TVI_LAST);
        BreakdownItem * pEB = new BreakdownItemPactDice(
                this,
                Breakdown_EldritchBlastDice,
                Effect_EldritchBlast,
                "Eldritch Blast Dice",
                &m_itemBreakdownTree,
                hItem,
                8);         // d8's
        m_itemBreakdownTree.SetItemData(hItem, (DWORD)(void*)pEB);
        m_items.push_back(pEB);
    }
    {
        HTREEITEM hItem = m_itemBreakdownTree.InsertItem(
                "Eldritch Pact Blast Pact Dice",
                hParent,
                TVI_LAST);
        BreakdownItem * pPD = new BreakdownItemPactDice(
                this,
                Breakdown_EldritchBlastPactDice,
                Effect_PactDice,
                "Eldritch Blast Pact Dice",
                &m_itemBreakdownTree,
                hItem,
                6);         // d6's
        m_itemBreakdownTree.SetItemData(hItem, (DWORD)(void*)pPD);
        m_items.push_back(pPD);
    }
    {
        HTREEITEM hItem = m_itemBreakdownTree.InsertItem(
                "Arcane Spell Failure (Armor)",
                hParent,
                TVI_LAST);
        BreakdownItem * pASF = new BreakdownItemSimple(
                this,
                Breakdown_ArcaneSpellfailure,
                Effect_ArcaneSpellFailure,
                "Arcane Spell Failure (Armor)",
                &m_itemBreakdownTree,
                hItem);
        m_itemBreakdownTree.SetItemData(hItem, (DWORD)(void*)pASF);
        m_items.push_back(pASF);
    }
    {
        HTREEITEM hItem = m_itemBreakdownTree.InsertItem(
                "Arcane Spell Failure (Shields)",
                hParent,
                TVI_LAST);
        BreakdownItem * pASF = new BreakdownItemSimple(
                this,
                Breakdown_ArcaneSpellfailureShields,
                Effect_ArcaneSpellFailureShields,
                "Arcane Spell Failure (Shields)",
                &m_itemBreakdownTree,
                hItem);
        m_itemBreakdownTree.SetItemData(hItem, (DWORD)(void*)pASF);
        m_items.push_back(pASF);
    }
    {
        HTREEITEM hItem = m_itemBreakdownTree.InsertItem(
                "Spell Resistance",
                hParent,
                TVI_LAST);
        BreakdownItem * pSR = new BreakdownItemSimple(
                this,
                Breakdown_SpellResistance,
                Effect_SpellResistance,
                "Spell Resistance",
                &m_itemBreakdownTree,
                hItem);
        m_itemBreakdownTree.SetItemData(hItem, (DWORD)(void*)pSR);
        m_items.push_back(pSR);
    }
    HTREEITEM hClassCasterLevels = m_itemBreakdownTree.InsertItem(
            "Class Caster Levels", 
            hParent,
            TVI_LAST);
    m_itemBreakdownTree.SetItemData(hClassCasterLevels, 0);
    AddClassCasterLevels(hClassCasterLevels);

    HTREEITEM hEnergyCasterLevels = m_itemBreakdownTree.InsertItem(
            "Energy Caster Levels", 
            hParent,
            TVI_LAST);
    m_itemBreakdownTree.SetItemData(hEnergyCasterLevels, 0);
    AddEnergyCasterLevels(hEnergyCasterLevels);

    HTREEITEM hSchoolCasterLevels = m_itemBreakdownTree.InsertItem(
            "School Caster Levels", 
            hParent,
            TVI_LAST);
    m_itemBreakdownTree.SetItemData(hSchoolCasterLevels, 0);
    AddSchoolCasterLevels(hSchoolCasterLevels);

    {
        HTREEITEM hItem = m_itemBreakdownTree.InsertItem(
                "Spell Penetration",
                hParent,
                TVI_LAST);
        BreakdownItem * pSP = new BreakdownItemSimple(
                this,
                Breakdown_SpellPenetration,
                Effect_SpellPenetrationBonus,
                "Spell Penetration",
                &m_itemBreakdownTree,
                hItem);
        m_itemBreakdownTree.SetItemData(hItem, (DWORD)(void*)pSP);
        m_items.push_back(pSP);
    }

    {
        // spell powers
        HTREEITEM hItem = m_itemBreakdownTree.InsertItem(
                "Spell Powers",
                hParent,
                TVI_LAST);
        m_itemBreakdownTree.SetItemData(hItem, 0);
        {
            HTREEITEM hUSPItem = m_itemBreakdownTree.InsertItem(
                    "Universal Spell power",
                    hItem,
                    TVI_LAST);
            BreakdownItem * usp = new BreakdownItemUniversalSpellPower(
                    this,
                    Breakdown_SpellPowerUniversal,
                    "Universal Spell power",
                    &m_itemBreakdownTree,
                    hUSPItem);
            m_itemBreakdownTree.SetItemData(hUSPItem, (DWORD)(void*)usp);
            m_items.push_back(usp);
            {
                HTREEITEM hIIYHItem = m_itemBreakdownTree.InsertItem(
                    "Implement in Your Hands",
                    hUSPItem,
                    TVI_LAST);
                BreakdownItem* pIIYH = new BreakdownItemSimple(
                    this,
                    Breakdown_ImplementInYourHands,
                    Effect_ImplementInYourHands,
                    "Implement in Your Hands",
                    &m_itemBreakdownTree,
                    hIIYHItem);
                m_itemBreakdownTree.SetItemData(hIIYHItem, (DWORD)(void*)pIIYH);
                m_items.push_back(pIIYH);
            }
        }
        AddSpellPower(Breakdown_SpellPowerAcid, SpellPower_Acid, "Acid Spell power", hItem);
        AddSpellPower(Breakdown_SpellPowerLightAlignment, SpellPower_LightAlignment, "Light/Alignment Spell power", hItem);
        AddSpellPower(Breakdown_SpellPowerChaos, SpellPower_Chaos, "Chaos Spell power", hItem);
        AddSpellPower(Breakdown_SpellPowerCold, SpellPower_Cold, "Cold Spell power", hItem);
        AddSpellPower(Breakdown_SpellPowerElectric, SpellPower_Electric, "Electric Spell power", hItem);
        AddSpellPower(Breakdown_SpellPowerEvil, SpellPower_Evil, "Evil Spell power", hItem);
        AddSpellPower(Breakdown_SpellPowerFire, SpellPower_Fire, "Fire Spell power", hItem);
        AddSpellPower(Breakdown_SpellPowerForceUntyped, SpellPower_ForceUntyped, "Force/Untyped Spell power", hItem);
        AddSpellPower(Breakdown_SpellPowerNegative, SpellPower_Negative, "Negative Spell power", hItem);
        AddSpellPower(Breakdown_SpellPowerPhysical, SpellPower_Physical, "Physical Spell power", hItem);
        AddSpellPower(Breakdown_SpellPowerPoison, SpellPower_Poison, "Poison Spell power", hItem);
        AddSpellPower(Breakdown_SpellPowerPositive, SpellPower_Positive, "Positive Spell power", hItem);
        AddSpellPower(Breakdown_SpellPowerRepair, SpellPower_Repair, "Repair Spell power", hItem);
        AddSpellPower(Breakdown_SpellPowerRust, SpellPower_Rust, "Rust Spell power", hItem);
        AddSpellPower(Breakdown_SpellPowerSonic, SpellPower_Sonic, "Sonic Spell power", hItem);
    }

    {
        // spell critical
        HTREEITEM hItem = m_itemBreakdownTree.InsertItem(
                "Spell Critical Chance",
                hParent,
                TVI_LAST);
        m_itemBreakdownTree.SetItemData(hItem, 0);
        {
            HTREEITEM hUSPItem = m_itemBreakdownTree.InsertItem(
                    "Universal Spell Critical Chance",
                    hItem,
                    TVI_LAST);
            BreakdownItem * usl = new BreakdownItemSimple(
                    this,
                    Breakdown_SpellCriticalChanceUniversal,
                    Effect_UniversalSpellLore,
                    "Universal Spell Critical Chance",
                    &m_itemBreakdownTree,
                    hUSPItem);
            m_itemBreakdownTree.SetItemData(hUSPItem, (DWORD)(void*)usl);
            m_items.push_back(usl);
        }
        AddSpellCriticalChance(Breakdown_SpellCriticalChanceAcid, SpellPower_Acid, "Acid Critical Chance", hItem);
        AddSpellCriticalChance(Breakdown_SpellCriticalChanceLightAlignment, SpellPower_LightAlignment, "Light/Alignment Critical Chance", hItem);
        AddSpellCriticalChance(Breakdown_SpellCriticalChanceChaos, SpellPower_Chaos, "Chaos Critical Chance", hItem);
        AddSpellCriticalChance(Breakdown_SpellCriticalChanceCold, SpellPower_Cold, "Cold Critical Chance", hItem);
        AddSpellCriticalChance(Breakdown_SpellCriticalChanceElectric, SpellPower_Electric, "Electric Critical Chance", hItem);
        AddSpellCriticalChance(Breakdown_SpellCriticalChanceEvil, SpellPower_Evil, "Evil Critical Chance", hItem);
        AddSpellCriticalChance(Breakdown_SpellCriticalChanceFire, SpellPower_Fire, "Fire Critical Chance", hItem);
        AddSpellCriticalChance(Breakdown_SpellCriticalChanceForceUntyped, SpellPower_ForceUntyped, "Force/Untyped Critical Chance", hItem);
        AddSpellCriticalChance(Breakdown_SpellCriticalChanceNegative, SpellPower_Negative, "Negative Critical Chance", hItem);
        AddSpellCriticalChance(Breakdown_SpellCriticalChancePoison, SpellPower_Poison, "Poison Critical Chance", hItem);
        AddSpellCriticalChance(Breakdown_SpellCriticalChancePhysical, SpellPower_Physical, "Physical Critical Chance", hItem);
        AddSpellCriticalChance(Breakdown_SpellCriticalChancePositive, SpellPower_Positive, "Positive Critical Chance", hItem);
        AddSpellCriticalChance(Breakdown_SpellCriticalChanceRepair, SpellPower_Repair, "Repair Critical Chance", hItem);
        AddSpellCriticalChance(Breakdown_SpellCriticalChanceRust, SpellPower_Rust, "Rust Critical Chance", hItem);
        AddSpellCriticalChance(Breakdown_SpellCriticalChanceSonic, SpellPower_Sonic, "Sonic Critical Chance", hItem);
    }

    {
        // spell critical multipliers
        HTREEITEM hItem = m_itemBreakdownTree.InsertItem(
                "Spell Critical Multipliers",
                hParent,
                TVI_LAST);
        m_itemBreakdownTree.SetItemData(hItem, 0);
        {
            HTREEITEM hUSPItem = m_itemBreakdownTree.InsertItem(
                "Universal Spell Critical Multiplier",
                hItem,
                TVI_LAST);
            BreakdownItem* usl = new BreakdownItemSimple(
                this,
                Breakdown_SpellCriticalMultiplierUniversal,
                Effect_UniversalSpellCriticalDamage,
                "Universal Spell Critical Multiplier",
                &m_itemBreakdownTree,
                hUSPItem);
            m_itemBreakdownTree.SetItemData(hUSPItem, (DWORD)(void*)usl);
            m_items.push_back(usl);
        }
        AddSpellCriticalMultiplier(Breakdown_SpellCriticalMultiplierAcid, SpellPower_Acid, "Acid Critical Multiplier", hItem);
        AddSpellCriticalMultiplier(Breakdown_SpellCriticalMultiplierLightAlignment, SpellPower_LightAlignment, "Light/Alignment Critical Multiplier", hItem);
        AddSpellCriticalMultiplier(Breakdown_SpellCriticalMultiplierChaos, SpellPower_Chaos, "Chaos Critical Multiplier", hItem);
        AddSpellCriticalMultiplier(Breakdown_SpellCriticalMultiplierCold, SpellPower_Cold, "Cold Critical Multiplier", hItem);
        AddSpellCriticalMultiplier(Breakdown_SpellCriticalMultiplierElectric, SpellPower_Electric, "Electric Critical Multiplier", hItem);
        AddSpellCriticalMultiplier(Breakdown_SpellCriticalMultiplierEvil, SpellPower_Evil, "Evil Critical Multiplier", hItem);
        AddSpellCriticalMultiplier(Breakdown_SpellCriticalMultiplierFire, SpellPower_Fire, "Fire Critical Multiplier", hItem);
        AddSpellCriticalMultiplier(Breakdown_SpellCriticalMultiplierForceUntyped, SpellPower_ForceUntyped, "Force/Untyped Critical Multiplier", hItem);
        AddSpellCriticalMultiplier(Breakdown_SpellCriticalMultiplierNegative, SpellPower_Negative, "Negative Critical Multiplier", hItem);
        AddSpellCriticalMultiplier(Breakdown_SpellCriticalMultiplierPhysical, SpellPower_Physical, "Physical Critical Multiplier", hItem);
        AddSpellCriticalMultiplier(Breakdown_SpellCriticalMultiplierPoison, SpellPower_Poison, "Poison Critical Multiplier", hItem);
        AddSpellCriticalMultiplier(Breakdown_SpellCriticalMultiplierPositive, SpellPower_Positive, "Positive Critical Multiplier", hItem);
        AddSpellCriticalMultiplier(Breakdown_SpellCriticalMultiplierRepair, SpellPower_Repair, "Repair Critical Multiplier", hItem);
        AddSpellCriticalMultiplier(Breakdown_SpellCriticalMultiplierRust, SpellPower_Rust, "Rust Critical Multiplier", hItem);
        AddSpellCriticalMultiplier(Breakdown_SpellCriticalMultiplierSonic, SpellPower_Sonic, "Sonic Critical Multiplier", hItem);
    }

    {
        // spell schools
        HTREEITEM hItem = m_itemBreakdownTree.InsertItem(
                "Spell School DCs",
                hParent,
                TVI_LAST);
        m_itemBreakdownTree.SetItemData(hItem, 0);
        AddSpellSchool(Breakdown_SpellSchoolAbjuration, SpellSchool_Abjuration, "Abjuration DC", hItem);
        AddSpellSchool(Breakdown_SpellSchoolConjuration, SpellSchool_Conjuration, "Conjuration DC", hItem);
        AddSpellSchool(Breakdown_SpellSchoolDivination, SpellSchool_Divination, "Divination DC", hItem);
        AddSpellSchool(Breakdown_SpellSchoolEnchantment, SpellSchool_Enchantment, "Enchantment DC", hItem);
        AddSpellSchool(Breakdown_SpellSchoolEvocation, SpellSchool_Evocation, "Evocation DC", hItem);
        AddSpellSchool(Breakdown_SpellSchoolIllusion, SpellSchool_Illusion, "Illusion DC", hItem);
        AddSpellSchool(Breakdown_SpellSchoolNecromancy, SpellSchool_Necromancy, "Necromancy DC", hItem);
        AddSpellSchool(Breakdown_SpellSchoolTransmutation, SpellSchool_Transmutation, "Transmutation DC", hItem);
        AddSpellSchool(Breakdown_SpellSchoolFear, SpellSchool_Fear, "Fear DC", hItem);
        AddSpellSchool(Breakdown_SpellSchoolGlobalDC, SpellSchool_GlobalDC, "Global DC Bonus", hItem);
        AddSpellSchool(Breakdown_SpellSchoolRuneArm, SpellSchool_RuneArm, "Rune Arm DC Bonus", hItem);
    }
}

void CBreakdownsPane::CreateSongBreakdowns()
{
    // insert the magical root tree item
    HTREEITEM hParent = m_itemBreakdownTree.InsertItem(
            "Song Breakdowns", 
            0,
            TVI_ROOT);
    m_itemBreakdownTree.SetItemData(hParent, 0);
    {
        HTREEITEM hItem = m_itemBreakdownTree.InsertItem(
                "Song Count",
                hParent,
                TVI_LAST);
        BreakdownItem * pSC = new BreakdownItemSimple(
                this,
                Breakdown_SongCount,
                Effect_SongCount,
                "Song Count",
                &m_itemBreakdownTree,
                hItem);
        m_itemBreakdownTree.SetItemData(hItem, (DWORD)(void*)pSC);
        m_items.push_back(pSC);
    }
    {
        HTREEITEM hItem = m_itemBreakdownTree.InsertItem(
                "Song Duration",
                hParent,
                TVI_LAST);
        BreakdownItem * pSD = new BreakdownItemDuration(
                this,
                Breakdown_SongDuration,
                Effect_SongDuration,
                "Song Duration",
                &m_itemBreakdownTree,
                hItem);
        m_itemBreakdownTree.SetItemData(hItem, (DWORD)(void*)pSD);
        m_items.push_back(pSD);
    }
    // inspire greatness breakdowns
    hParent = m_itemBreakdownTree.InsertItem(
        "Inspire Greatness Effects",
        hParent,
        TVI_LAST);
    {
        HTREEITEM hItem = m_itemBreakdownTree.InsertItem(
            "AC Bonus",
            hParent,
            TVI_LAST);
        BreakdownItem* pSACB = new BreakdownItemSimple(
            this,
            Breakdown_SongACBonus,
            Effect_SongACBonus,
            "AC Bonus",
            &m_itemBreakdownTree,
            hItem);
        m_itemBreakdownTree.SetItemData(hItem, (DWORD)(void*)pSACB);
        m_items.push_back(pSACB);
    }
    {
        HTREEITEM hItem = m_itemBreakdownTree.InsertItem(
            "Attack Bonus",
            hParent,
            TVI_LAST);
        BreakdownItem* pSAB = new BreakdownItemSimple(
            this,
            Breakdown_SongAttackBonus,
            Effect_SongAttackBonus,
            "Attack Bonus",
            &m_itemBreakdownTree,
            hItem);
        m_itemBreakdownTree.SetItemData(hItem, (DWORD)(void*)pSAB);
        m_items.push_back(pSAB);
    }
    {
        HTREEITEM hItem = m_itemBreakdownTree.InsertItem(
            "Damage Bonus",
            hParent,
            TVI_LAST);
        BreakdownItem* pSDB = new BreakdownItemSimple(
            this,
            Breakdown_SongDamageBonus,
            Effect_SongDamageBonus,
            "Damage Bonus",
            &m_itemBreakdownTree,
            hItem);
        m_itemBreakdownTree.SetItemData(hItem, (DWORD)(void*)pSDB);
        m_items.push_back(pSDB);
    }
    {
        HTREEITEM hItem = m_itemBreakdownTree.InsertItem(
            "Doublestrike Bonus",
            hParent,
            TVI_LAST);
        BreakdownItem* pSDS = new BreakdownItemSimple(
            this,
            Breakdown_SongDoublestrike,
            Effect_SongDoublestrike,
            "Doublestrike Bonus",
            &m_itemBreakdownTree,
            hItem);
        m_itemBreakdownTree.SetItemData(hItem, (DWORD)(void*)pSDS);
        m_items.push_back(pSDS);
    }
    {
        HTREEITEM hItem = m_itemBreakdownTree.InsertItem(
            "Doubleshot Bonus",
            hParent,
            TVI_LAST);
        BreakdownItem* pSDS = new BreakdownItemSimple(
            this,
            Breakdown_SongDoubleshot,
            Effect_SongDoubleshot,
            "Doubleshot Bonus",
            &m_itemBreakdownTree,
            hItem);
        m_itemBreakdownTree.SetItemData(hItem, (DWORD)(void*)pSDS);
        m_items.push_back(pSDS);
    }
    {
        HTREEITEM hItem = m_itemBreakdownTree.InsertItem(
            "Universal Spell Power Bonus",
            hParent,
            TVI_LAST);
        BreakdownItem* pSUSP = new BreakdownItemSimple(
            this,
            Breakdown_SongUniversalSpellPower,
            Effect_SongUniversalSpellPower,
            "Universal Spell Power Bonus",
            &m_itemBreakdownTree,
            hItem);
        m_itemBreakdownTree.SetItemData(hItem, (DWORD)(void*)pSUSP);
        m_items.push_back(pSUSP);
    }
    {
        HTREEITEM hItem = m_itemBreakdownTree.InsertItem(
            "Spell Penetration Bonus",
            hParent,
            TVI_LAST);
        BreakdownItem* pSSP = new BreakdownItemSimple(
            this,
            Breakdown_SongSpellPenetration,
            Effect_SongSpellPenetration,
            "Spell Penetration Bonus",
            &m_itemBreakdownTree,
            hItem);
        m_itemBreakdownTree.SetItemData(hItem, (DWORD)(void*)pSSP);
        m_items.push_back(pSSP);
    }
    {
        HTREEITEM hItem = m_itemBreakdownTree.InsertItem(
            "Caster Level Bonus",
            hParent,
            TVI_LAST);
        BreakdownItem* pSCL = new BreakdownItemSimple(
            this,
            Breakdown_SongCasterLevel,
            Effect_SongCasterLevel,
            "Caster Level Bonus",
            &m_itemBreakdownTree,
            hItem);
        m_itemBreakdownTree.SetItemData(hItem, (DWORD)(void*)pSCL);
        m_items.push_back(pSCL);
    }
    {
        HTREEITEM hItem = m_itemBreakdownTree.InsertItem(
            "Skill Bonus",
            hParent,
            TVI_LAST);
        BreakdownItem* pSSB = new BreakdownItemSimple(
            this,
            Breakdown_SongSkillBonus,
            Effect_SongSkillBonus,
            "Skill Bonus",
            &m_itemBreakdownTree,
            hItem);
        m_itemBreakdownTree.SetItemData(hItem, (DWORD)(void*)pSSB);
        m_items.push_back(pSSB);
    }
    {
        HTREEITEM hItem = m_itemBreakdownTree.InsertItem(
            "PRR Bonus",
            hParent,
            TVI_LAST);
        BreakdownItem* pSPRR = new BreakdownItemSimple(
            this,
            Breakdown_SongPRR,
            Effect_SongPRR,
            "PRR Bonus",
            &m_itemBreakdownTree,
            hItem);
        m_itemBreakdownTree.SetItemData(hItem, (DWORD)(void*)pSPRR);
        m_items.push_back(pSPRR);
    }
    {
        HTREEITEM hItem = m_itemBreakdownTree.InsertItem(
            "MRR Bonus",
            hParent,
            TVI_LAST);
        BreakdownItem* pSMRR = new BreakdownItemSimple(
            this,
            Breakdown_SongMRR,
            Effect_SongMRR,
            "MRR Bonus",
            &m_itemBreakdownTree,
            hItem);
        m_itemBreakdownTree.SetItemData(hItem, (DWORD)(void*)pSMRR);
        m_items.push_back(pSMRR);
    }
    {
        HTREEITEM hItem = m_itemBreakdownTree.InsertItem(
            "Healing Amp Bonus",
            hParent,
            TVI_LAST);
        BreakdownItem* pSHA = new BreakdownItemSimple(
            this,
            Breakdown_SongHealingAmp,
            Effect_SongHealingAmp,
            "Healing Amp Bonus",
            &m_itemBreakdownTree,
            hItem);
        m_itemBreakdownTree.SetItemData(hItem, (DWORD)(void*)pSHA);
        m_items.push_back(pSHA);
    }
    {
        HTREEITEM hItem = m_itemBreakdownTree.InsertItem(
            "Negative Healing Amp Bonus",
            hParent,
            TVI_LAST);
        BreakdownItem* pSNHA = new BreakdownItemSimple(
            this,
            Breakdown_SongNegativeHealingAmp,
            Effect_SongNegativeHealingAmp,
            "Negative Healing Amp Bonus",
            &m_itemBreakdownTree,
            hItem);
        m_itemBreakdownTree.SetItemData(hItem, (DWORD)(void*)pSNHA);
        m_items.push_back(pSNHA);
    }
    {
        HTREEITEM hItem = m_itemBreakdownTree.InsertItem(
            "Repair Amp Bonus",
            hParent,
            TVI_LAST);
        BreakdownItem* pSRA = new BreakdownItemSimple(
            this,
            Breakdown_SongRepairAmp,
            Effect_SongRepairAmp,
            "Repair Amp Bonus",
            &m_itemBreakdownTree,
            hItem);
        m_itemBreakdownTree.SetItemData(hItem, (DWORD)(void*)pSRA);
        m_items.push_back(pSRA);
    }
}

void CBreakdownsPane::CreateTurnUndeadBreakdowns()
{
    // insert the Turn Undead tree item
    HTREEITEM hParent = m_itemBreakdownTree.InsertItem(
            "Turn Undead", 
            0,
            TVI_ROOT);
    m_itemBreakdownTree.SetItemData(hParent, 0);
    {
        HTREEITEM hItem = m_itemBreakdownTree.InsertItem(
                "Turn Undead Level",
                hParent,
                TVI_LAST);
        BreakdownItem * pTUL = new BreakdownItemTurnUndeadLevel(
                this,
                Breakdown_TurnUndeadLevel,
                &m_itemBreakdownTree,
                hItem);
        m_itemBreakdownTree.SetItemData(hItem, (DWORD)(void*)pTUL);
        m_items.push_back(pTUL);
    }
    {
        HTREEITEM hItem = m_itemBreakdownTree.InsertItem(
                "Turn Undead Hit Dice",
                hParent,
                TVI_LAST);
        BreakdownItem * pTUHD = new BreakdownItemTurnUndeadHitDice(
                this,
                Breakdown_TurnUndeadHitDice,
                &m_itemBreakdownTree,
                hItem);
        m_itemBreakdownTree.SetItemData(hItem, (DWORD)(void*)pTUHD);
        m_items.push_back(pTUHD);
    }

}

void CBreakdownsPane::AddKiBreakdowns(HTREEITEM hParent)
{
    {
        HTREEITEM hItem = m_itemBreakdownTree.InsertItem(
                "Maximum Ki",
                hParent,
                TVI_LAST);
        BreakdownItem * pKi = new BreakdownItemMaximumKi(
                this,
                &m_itemBreakdownTree,
                hItem);
        m_itemBreakdownTree.SetItemData(hItem, (DWORD)(void*)pKi);
        m_items.push_back(pKi);
    }
    {
        HTREEITEM hItem = m_itemBreakdownTree.InsertItem(
                "Ki Passive Regeneration",
                hParent,
                TVI_LAST);
        BreakdownItem * pKi = new BreakdownItemSimple(
                this,
                Breakdown_KiPassive,
                Effect_KiPassive,
                "Ki Passive Regeneration",
                &m_itemBreakdownTree,
                hItem);
        m_itemBreakdownTree.SetItemData(hItem, (DWORD)(void*)pKi);
        m_items.push_back(pKi);
    }
    {
        HTREEITEM hItem = m_itemBreakdownTree.InsertItem(
                "Ki on Hit",
                hParent,
                TVI_LAST);
        BreakdownItem * pKi = new BreakdownItemSimple(
                this,
                Breakdown_KiHit,
                Effect_KiHit,
                "Ki on Hit",
                &m_itemBreakdownTree,
                hItem);
        m_itemBreakdownTree.SetItemData(hItem, (DWORD)(void*)pKi);
        m_items.push_back(pKi);
    }
    {
        HTREEITEM hItem = m_itemBreakdownTree.InsertItem(
                "Ki on Critical Hit",
                hParent,
                TVI_LAST);
        BreakdownItem * pKi = new BreakdownItemSimple(
                this,
                Breakdown_KiCritical,
                Effect_KiCritical,
                "Ki on Critical Hit",
                &m_itemBreakdownTree,
                hItem);
        m_itemBreakdownTree.SetItemData(hItem, (DWORD)(void*)pKi);
        m_items.push_back(pKi);
    }
}

void CBreakdownsPane::AddClassCasterLevels(HTREEITEM hParent)
{
    UNREFERENCED_PARAMETER(hParent);
    const std::list<::Class>& classes = Classes();
    for (auto&& cit : classes)
    {
        // we can see if this is a caster class by checking to see if it has spell points
        // at heroic level 20
        if (cit.SpellPointsAtLevel(MAX_CLASS_LEVEL-1) > 0)
        {
            std::stringstream ss;
            ss << cit.Name() << " Caster Level";
            HTREEITEM hItem = m_itemBreakdownTree.InsertItem(
                    ss.str().c_str(),
                    hParent,
                    TVI_LAST);
            BreakdownItem * pClassCasterLevel = new BreakdownItemClassCasterLevel(
                    this,
                    cit.Name(),
                    static_cast<BreakdownType>(Breakdown_CasterLevel_First + cit.Index()),
                    Effect_CasterLevel,
                    &m_itemBreakdownTree,
                    hItem);
            m_itemBreakdownTree.SetItemData(hItem, (DWORD)(void*)pClassCasterLevel);
            m_items.push_back(pClassCasterLevel);
            // also add the max caster level option as a child of the item just added
            std::stringstream maxss;
            maxss << "Max " << cit.Name() << " Caster Level";
            HTREEITEM hMaxItem = m_itemBreakdownTree.InsertItem(
                    maxss.str().c_str(),
                    hItem,
                    TVI_LAST);
            BreakdownItem* pMaxClassCasterLevel = new BreakdownItemClassCasterLevel(
                    this,
                    cit.Name(),
                    static_cast<BreakdownType>(Breakdown_MaxCasterLevel_First + cit.Index()),
                    Effect_MaxCasterLevel,
                    &m_itemBreakdownTree,
                    hMaxItem);
            m_itemBreakdownTree.SetItemData(hMaxItem, (DWORD)(void*)pMaxClassCasterLevel);
            m_items.push_back(pMaxClassCasterLevel);
        }
    }
}

void CBreakdownsPane::AddEnergyCasterLevels(HTREEITEM hParent)
{
    AddEnergyCasterLevels(hParent, Breakdown_CasterLevel_Spell_Acid, Breakdown_MaxCasterLevel_Spell_Acid, Energy_Acid);
    AddEnergyCasterLevels(hParent, Breakdown_CasterLevel_Spell_Cold, Breakdown_MaxCasterLevel_Spell_Cold, Energy_Cold);
    AddEnergyCasterLevels(hParent, Breakdown_CasterLevel_Spell_Electric, Breakdown_MaxCasterLevel_Spell_Electric, Energy_Electric);
    AddEnergyCasterLevels(hParent, Breakdown_CasterLevel_Spell_Evil, Breakdown_MaxCasterLevel_Spell_Evil, Energy_Evil);
    AddEnergyCasterLevels(hParent, Breakdown_CasterLevel_Spell_Fire, Breakdown_MaxCasterLevel_Spell_Fire, Energy_Fire);
    AddEnergyCasterLevels(hParent, Breakdown_CasterLevel_Spell_Force, Breakdown_MaxCasterLevel_Spell_Force, Energy_Force);
    AddEnergyCasterLevels(hParent, Breakdown_CasterLevel_Spell_Negative, Breakdown_MaxCasterLevel_Spell_Negative, Energy_Negative);
    AddEnergyCasterLevels(hParent, Breakdown_CasterLevel_Spell_Poison, Breakdown_MaxCasterLevel_Spell_Poison, Energy_Poison);
    AddEnergyCasterLevels(hParent, Breakdown_CasterLevel_Spell_Positive, Breakdown_MaxCasterLevel_Spell_Positive, Energy_Positive);
    AddEnergyCasterLevels(hParent, Breakdown_CasterLevel_Spell_Sonic, Breakdown_MaxCasterLevel_Spell_Sonic, Energy_Sonic);
    AddEnergyCasterLevels(hParent, Breakdown_CasterLevel_Ceruleite, Breakdown_MaxCasterLevel_Ceruleite, Energy_Ceruleite);
    AddEnergyCasterLevels(hParent, Breakdown_CasterLevel_Crimsonite, Breakdown_MaxCasterLevel_Crimsonite, Energy_Crimsonite);
    AddEnergyCasterLevels(hParent, Breakdown_CasterLevel_Gildleaf, Breakdown_MaxCasterLevel_Gildleaf, Energy_Gildleaf);
}

void CBreakdownsPane::AddEnergyCasterLevels(
        HTREEITEM hParent,
        BreakdownType ecl,
        BreakdownType emcl,
        EnergyType et)
{
    CString nameCasterLevel;
    CString nameMaxCasterLevel;
    CString energyName = EnumEntryText(et, energyTypeMap);
    nameCasterLevel.Format("%s Spell Caster Level", (LPCTSTR)energyName);
    nameMaxCasterLevel.Format("%s Max Spell Caster Level", (LPCTSTR)energyName);

    HTREEITEM hItem = m_itemBreakdownTree.InsertItem(
            nameCasterLevel,
            hParent,
            TVI_LAST);
    BreakdownItem * pEnergy = new BreakdownItemEnergyCasterLevel(
            this,
            Effect_CasterLevel,
            et,
            ecl,
            &m_itemBreakdownTree,
            hItem);
    m_itemBreakdownTree.SetItemData(hItem, (DWORD)(void*)pEnergy);
    m_items.push_back(pEnergy);
    // Max caster level shown as child of main element
    HTREEITEM hMaxItem = m_itemBreakdownTree.InsertItem(
            nameMaxCasterLevel,
            hItem,
            TVI_LAST);
    BreakdownItem * pMaxEnergy = new BreakdownItemEnergyCasterLevel(
            this,
            Effect_MaxCasterLevel,
            et,
            emcl,
            &m_itemBreakdownTree,
            hMaxItem);
    m_itemBreakdownTree.SetItemData(hMaxItem, (DWORD)(void*)pMaxEnergy);
    m_items.push_back(pMaxEnergy);
}

void CBreakdownsPane::AddSchoolCasterLevels(HTREEITEM hParent)
{
    AddSchoolCasterLevels(hParent, Breakdown_CasterLevel_School_Abjuration, Breakdown_MaxCasterLevel_School_Abjuration, SpellSchool_Abjuration);
    AddSchoolCasterLevels(hParent, Breakdown_CasterLevel_School_Conjuration, Breakdown_MaxCasterLevel_School_Conjuration, SpellSchool_Conjuration);
    AddSchoolCasterLevels(hParent, Breakdown_CasterLevel_School_Divination, Breakdown_MaxCasterLevel_School_Divination, SpellSchool_Divination);
    AddSchoolCasterLevels(hParent, Breakdown_CasterLevel_School_Enchantment, Breakdown_MaxCasterLevel_School_Enchantment, SpellSchool_Enchantment);
    AddSchoolCasterLevels(hParent, Breakdown_CasterLevel_School_Evocation, Breakdown_MaxCasterLevel_School_Evocation, SpellSchool_Evocation);
    AddSchoolCasterLevels(hParent, Breakdown_CasterLevel_School_Illusion, Breakdown_MaxCasterLevel_School_Illusion, SpellSchool_Illusion);
    AddSchoolCasterLevels(hParent, Breakdown_CasterLevel_School_Necromancy, Breakdown_MaxCasterLevel_School_Necromancy, SpellSchool_Necromancy);
    AddSchoolCasterLevels(hParent, Breakdown_CasterLevel_School_Transmutation, Breakdown_MaxCasterLevel_School_Transmutation, SpellSchool_Transmutation);
}

void CBreakdownsPane::AddSchoolCasterLevels(
        HTREEITEM hParent,
        BreakdownType scl,
        BreakdownType smcl,
        SpellSchoolType st)
{
    CString nameCasterLevel;
    CString nameMaxCasterLevel;
    CString schoolName = EnumEntryText(st, spellSchoolTypeMap);
    nameCasterLevel.Format("%s Caster Level", (LPCTSTR)schoolName);
    nameMaxCasterLevel.Format("%s Max Caster Level", (LPCTSTR)schoolName);

    HTREEITEM hItem = m_itemBreakdownTree.InsertItem(
            nameCasterLevel,
            hParent,
            TVI_LAST);
    BreakdownItem * pSchool = new BreakdownItemSchoolCasterLevel(
            this,
            Effect_CasterLevel,
            st,
            scl,
            &m_itemBreakdownTree,
            hItem);
    m_itemBreakdownTree.SetItemData(hItem, (DWORD)(void*)pSchool);
    m_items.push_back(pSchool);
    // Max caster level shown as child of main element
    HTREEITEM hMaxItem = m_itemBreakdownTree.InsertItem(
            nameMaxCasterLevel,
            hItem,
            TVI_LAST);
    BreakdownItem * pMaxSchool = new BreakdownItemSchoolCasterLevel(
            this,
            Effect_MaxCasterLevel,
            st,
            smcl,
            &m_itemBreakdownTree,
            hMaxItem);
    m_itemBreakdownTree.SetItemData(hMaxItem, (DWORD)(void*)pMaxSchool);
    m_items.push_back(pMaxSchool);
}

void CBreakdownsPane::CreateEnergyResistancesBreakdowns()
{
    // insert the Resistances root tree item
    HTREEITEM hParent = m_itemBreakdownTree.InsertItem(
            "Energy Resistances Breakdowns", 
            0,
            TVI_ROOT);
    m_itemBreakdownTree.SetItemData(hParent, 0);
    {
        // Resistances
        HTREEITEM hItem = m_itemBreakdownTree.InsertItem(
                "Resistances",
                hParent,
                TVI_LAST);
        m_itemBreakdownTree.SetItemData(hItem, 0);
        AddEnergyResistance(Breakdown_EnergyResistanceAcid, Energy_Acid, "Acid Resistance", hItem);
        AddEnergyResistance(Breakdown_EnergyResistanceChaos, Energy_Chaos, "Chaos Resistance", hItem);
        AddEnergyResistance(Breakdown_EnergyResistanceCold, Energy_Cold, "Cold Resistance", hItem);
        AddEnergyResistance(Breakdown_EnergyResistanceElectric, Energy_Electric, "Electric Resistance", hItem);
        AddEnergyResistance(Breakdown_EnergyResistanceEvil, Energy_Evil, "Evil Resistance", hItem);
        AddEnergyResistance(Breakdown_EnergyResistanceFire, Energy_Fire, "Fire Resistance", hItem);
        AddEnergyResistance(Breakdown_EnergyResistanceForce, Energy_Force, "Force Resistance", hItem);
        AddEnergyResistance(Breakdown_EnergyResistanceGood, Energy_Good, "Good Resistance", hItem);
        AddEnergyResistance(Breakdown_EnergyResistanceLawful, Energy_Lawful, "Lawful Resistance", hItem);
        AddEnergyResistance(Breakdown_EnergyResistanceLight, Energy_Light, "Light Resistance", hItem);
        AddEnergyResistance(Breakdown_EnergyResistanceNegative, Energy_Negative, "Negative Resistance", hItem);
        AddEnergyResistance(Breakdown_EnergyResistancePoison, Energy_Poison, "Poison Resistance", hItem);
        //AddEnergyResistance(Breakdown_EnergyResistancePositive, Energy_Positive, "Positive Resistance", hItem);
        //AddEnergyResistance(Breakdown_EnergyResistanceRepair, Energy_Repair, "Repair Resistance", hItem);
        //AddEnergyResistance(Breakdown_EnergyResistanceRust, Energy_Rust, "Rust Resistance", hItem);
        AddEnergyResistance(Breakdown_EnergyResistanceSonic, Energy_Sonic, "Sonic Resistance", hItem);
    }
    {
        // Absorptions
        HTREEITEM hItem = m_itemBreakdownTree.InsertItem(
                "Absorption",
                hParent,
                TVI_LAST);
        m_itemBreakdownTree.SetItemData(hItem, 0);
        AddEnergyAbsorption(Breakdown_EnergyAbsorptionAcid, Energy_Acid, "Acid Absorption", hItem);
        AddEnergyAbsorption(Breakdown_EnergyAbsorptionChaos, Energy_Chaos, "Chaos Absorption", hItem);
        AddEnergyAbsorption(Breakdown_EnergyAbsorptionCold, Energy_Cold, "Cold Absorption", hItem);
        AddEnergyAbsorption(Breakdown_EnergyAbsorptionElectric, Energy_Electric, "Electric Absorption", hItem);
        AddEnergyAbsorption(Breakdown_EnergyAbsorptionEvil, Energy_Evil, "Evil Absorption", hItem);
        AddEnergyAbsorption(Breakdown_EnergyAbsorptionFire, Energy_Fire, "Fire Absorption", hItem);
        AddEnergyAbsorption(Breakdown_EnergyAbsorptionForce, Energy_Force, "Force Absorption", hItem);
        AddEnergyAbsorption(Breakdown_EnergyAbsorptionGood, Energy_Good, "Good Absorption", hItem);
        AddEnergyAbsorption(Breakdown_EnergyAbsorptionLawful, Energy_Lawful, "Lawful Absorption", hItem);
        AddEnergyAbsorption(Breakdown_EnergyAbsorptionLight, Energy_Light, "Light Absorption", hItem);
        AddEnergyAbsorption(Breakdown_EnergyAbsorptionNegative, Energy_Negative, "Negative Absorption", hItem);
        AddEnergyAbsorption(Breakdown_EnergyAbsorptionPoison, Energy_Poison, "Poison Absorption", hItem);
        //AddEnergyAbsorption(Breakdown_EnergyAbsorptionPositive, Energy_Positive, "Positive Absorption", hItem);
        //AddEnergyAbsorption(Breakdown_EnergyAbsorptionRepair, Energy_Repair, "Repair Absorption", hItem);
        //AddEnergyAbsorption(Breakdown_EnergyAbsorptionRust, Energy_Rust, "Rust Absorption", hItem);
        AddEnergyAbsorption(Breakdown_EnergyAbsorptionSonic, Energy_Sonic, "Sonic Absorption", hItem);
    }
}

void CBreakdownsPane::CreateWeaponBreakdowns()
{
    // we need to know which is this element as we dynamically add/remove
    // items from the list when the equipped items change.
    m_hWeaponsHeader = m_itemBreakdownTree.InsertItem(
            "Weapons", 
            0,
            TVI_ROOT);
    m_itemBreakdownTree.SetItemData(m_hWeaponsHeader, NULL);
    // this is not actually added to the list control, but does need access
    // to the tree list and the "Weapons" tree item
    m_pWeaponEffects = new BreakdownItemWeaponEffects(
            this,
            &m_itemBreakdownTree,
            m_hWeaponsHeader);
}

void CBreakdownsPane::CreateHirelingBreakdowns()
{
    // insert the hirelings root tree item
    HTREEITEM hParent = m_itemBreakdownTree.InsertItem(
            "Hireling Breakdowns", 
            0,
            TVI_ROOT);
    m_itemBreakdownTree.SetItemData(hParent, 0);
    {
        HTREEITEM hItem = m_itemBreakdownTree.InsertItem(
                "Hireling Abilities",
                hParent,
                TVI_LAST);
        BreakdownItem * pHAB = new BreakdownItemSimple(
                this,
                Breakdown_HirelingAbilityBonus,
                Effect_HirelingAbilityBonus,
                "Hireling Abilities",
                &m_itemBreakdownTree,
                hItem);
        m_itemBreakdownTree.SetItemData(hItem, (DWORD)(void*)pHAB);
        m_items.push_back(pHAB);
    }
    {
        HTREEITEM hItem = m_itemBreakdownTree.InsertItem(
                "Hireling Hitpoints",
                hParent,
                TVI_LAST);
        BreakdownItem * pHP = new BreakdownItemSimple(
                this,
                Breakdown_HirelingHitpoints,
                Effect_HirelingHitpoints,
                "Hireling Hitpoints",
                &m_itemBreakdownTree,
                hItem);
        m_itemBreakdownTree.SetItemData(hItem, (DWORD)(void*)pHP);
        m_items.push_back(pHP);
    }
    {
        HTREEITEM hItem = m_itemBreakdownTree.InsertItem(
                "Hireling Fortification",
                hParent,
                TVI_LAST);
        BreakdownItem * pFort = new BreakdownItemSimple(
                this,
                Breakdown_HirelingFortification,
                Effect_HirelingFortification,
                "Hireling Fortification",
                &m_itemBreakdownTree,
                hItem);
        m_itemBreakdownTree.SetItemData(hItem, (DWORD)(void*)pFort);
        m_items.push_back(pFort);
    }
    {
        HTREEITEM hItem = m_itemBreakdownTree.InsertItem(
                "Hireling PRR",
                hParent,
                TVI_LAST);
        BreakdownItem * pPRR = new BreakdownItemSimple(
                this,
                Breakdown_HirelingPRR,
                Effect_HirelingPRR,
                "Hireling PRR",
                &m_itemBreakdownTree,
                hItem);
        m_itemBreakdownTree.SetItemData(hItem, (DWORD)(void*)pPRR);
        m_items.push_back(pPRR);
    }
    {
        HTREEITEM hItem = m_itemBreakdownTree.InsertItem(
                "Hireling MRR",
                hParent,
                TVI_LAST);
        BreakdownItem * pMRR = new BreakdownItemSimple(
                this,
                Breakdown_HirelingMRR,
                Effect_HirelingMRR,
                "Hireling MRR",
                &m_itemBreakdownTree,
                hItem);
        m_itemBreakdownTree.SetItemData(hItem, (DWORD)(void*)pMRR);
        m_items.push_back(pMRR);
    }
    {
        HTREEITEM hItem = m_itemBreakdownTree.InsertItem(
                "Hireling Dodge",
                hParent,
                TVI_LAST);
        BreakdownItem * pDodge = new BreakdownItemSimple(
                this,
                Breakdown_HirelingDodge,
                Effect_HirelingDodge,
                "Hireling Dodge",
                &m_itemBreakdownTree,
                hItem);
        m_itemBreakdownTree.SetItemData(hItem, (DWORD)(void*)pDodge);
        m_items.push_back(pDodge);
    }
    {
        HTREEITEM hItem = m_itemBreakdownTree.InsertItem(
                "Hireling Melee Power",
                hParent,
                TVI_LAST);
        BreakdownItem * pMelee = new BreakdownItemSimple(
                this,
                Breakdown_HirelingMeleePower,
                Effect_HirelingMeleePower,
                "Hireling Melee Power",
                &m_itemBreakdownTree,
                hItem);
        m_itemBreakdownTree.SetItemData(hItem, (DWORD)(void*)pMelee);
        m_items.push_back(pMelee);
    }
    {
        HTREEITEM hItem = m_itemBreakdownTree.InsertItem(
                "Hireling Ranged Power",
                hParent,
                TVI_LAST);
        BreakdownItem * pRanged = new BreakdownItemSimple(
                this,
                Breakdown_HirelingRangedPower,
                Effect_HirelingRangedPower,
                "Hireling Ranged Power",
                &m_itemBreakdownTree,
                hItem);
        m_itemBreakdownTree.SetItemData(hItem, (DWORD)(void*)pRanged);
        m_items.push_back(pRanged);
    }
    {
        HTREEITEM hItem = m_itemBreakdownTree.InsertItem(
                "Hireling Spell Power",
                hParent,
                TVI_LAST);
        BreakdownItem * pSpell = new BreakdownItemSimple(
                this,
                Breakdown_HirelingSpellPower,
                Effect_HirelingSpellPower,
                "Hireling Spell Power",
                &m_itemBreakdownTree,
                hItem);
        m_itemBreakdownTree.SetItemData(hItem, (DWORD)(void*)pSpell);
        m_items.push_back(pSpell);
    }
    {
        HTREEITEM hItem = m_itemBreakdownTree.InsertItem(
            "Hireling Concealment",
            hParent,
            TVI_LAST);
        BreakdownItem* pConcealment = new BreakdownItemSimple(
            this,
            Breakdown_HirelingConcealment,
            Effect_HirelingConcealment,
            "Hireling Concealment",
            &m_itemBreakdownTree,
            hItem);
        m_itemBreakdownTree.SetItemData(hItem, (DWORD)(void*)pConcealment);
        m_items.push_back(pConcealment);
    }
}

void CBreakdownsPane::AddTacticalItem(
        BreakdownType bt,
        TacticalType tt,
        const std::string& name,
        HTREEITEM hParent)
{
    HTREEITEM hItem = m_itemBreakdownTree.InsertItem(
            name.c_str(),
            hParent,
            TVI_LAST);
    BreakdownItem * pTT = new BreakdownItemTactical(
            this,
            bt,
            tt,
            &m_itemBreakdownTree,
            hItem);
    m_itemBreakdownTree.SetItemData(hItem, (DWORD)(void*)pTT);
    m_items.push_back(pTT);
}

void CBreakdownsPane::AddSpellPower(
        BreakdownType bt,
        SpellPowerType type,
        const std::string& name,
        HTREEITEM hParent)
{
    HTREEITEM hItem = m_itemBreakdownTree.InsertItem(
            name.c_str(),
            hParent,
            TVI_LAST);
    BreakdownItem * pSPItem = new BreakdownItemSpellPower(
            this,
            bt,
            Effect_SpellPower,
            type,
            name.c_str(),
            &m_itemBreakdownTree,
            hItem);
    m_itemBreakdownTree.SetItemData(hItem, (DWORD)(void*)pSPItem);
    m_items.push_back(pSPItem);
}

void CBreakdownsPane::AddSpellCriticalChance(
        BreakdownType bt,
        SpellPowerType type,
        const std::string& name,
        HTREEITEM hParent)
{
    HTREEITEM hItem = m_itemBreakdownTree.InsertItem(
            name.c_str(),
            hParent,
            TVI_LAST);
    BreakdownItem * pSPItem = new BreakdownItemSpellPower(
            this,
            bt,
            Effect_SpellLore,
            type,
            name.c_str(),
            &m_itemBreakdownTree,
            hItem);
    m_itemBreakdownTree.SetItemData(hItem, (DWORD)(void*)pSPItem);
    m_items.push_back(pSPItem);
}

void CBreakdownsPane::AddSpellCriticalMultiplier(
        BreakdownType bt,
        SpellPowerType type,
        const std::string& name,
        HTREEITEM hParent)
{
    HTREEITEM hItem = m_itemBreakdownTree.InsertItem(
            name.c_str(),
            hParent,
            TVI_LAST);
    BreakdownItem * pSPItem = new BreakdownItemSpellPower(
            this,
            bt,
            Effect_SpellCriticalDamage,
            type,
            name.c_str(),
            &m_itemBreakdownTree,
            hItem);
    m_itemBreakdownTree.SetItemData(hItem, (DWORD)(void*)pSPItem);
    m_items.push_back(pSPItem);
}

void CBreakdownsPane::AddSpellSchool(
        BreakdownType bt,
        SpellSchoolType type,
        const std::string& name,
        HTREEITEM hParent)
{
    HTREEITEM hItem = m_itemBreakdownTree.InsertItem(
            name.c_str(),
            hParent,
            TVI_LAST);
    BreakdownItem * pSSItem = new BreakdownItemSpellSchool(
            this,
            bt,
            Effect_SpellDC,
            type,
            name.c_str(),
            &m_itemBreakdownTree,
            hItem);
    m_itemBreakdownTree.SetItemData(hItem, (DWORD)(void*)pSSItem);
    m_items.push_back(pSSItem);
}

void CBreakdownsPane::AddEnergyResistance(
        BreakdownType bt,
        EnergyType type,
        const std::string& name,
        HTREEITEM hParent)
{
    HTREEITEM hItem = m_itemBreakdownTree.InsertItem(
            name.c_str(),
            hParent,
            TVI_LAST);
    BreakdownItem * pERItem = new BreakdownItemEnergyResistance(
            this,
            bt,
            Effect_EnergyResistance,
            type,
            name.c_str(),
            &m_itemBreakdownTree,
            hItem);
    m_itemBreakdownTree.SetItemData(hItem, (DWORD)(void*)pERItem);
    m_items.push_back(pERItem);
}

void CBreakdownsPane::AddEnergyAbsorption(
        BreakdownType bt,
        EnergyType type,
        const std::string& name,
        HTREEITEM hParent)
{
    HTREEITEM hItem = m_itemBreakdownTree.InsertItem(
            name.c_str(),
            hParent,
            TVI_LAST);
    BreakdownItem * pERItem = new BreakdownItemEnergyAbsorption(
            this,
            bt,
            Effect_EnergyAbsorbance,
            type,
            name.c_str(),
            &m_itemBreakdownTree,
            hItem);
    m_itemBreakdownTree.SetItemData(hItem, (DWORD)(void*)pERItem);
    m_items.push_back(pERItem);
}

BreakdownItem * CBreakdownsPane::FindBreakdown(BreakdownType type) const
{
    BreakdownItem * pItem = NULL;
    for (size_t i = 0; i < m_items.size(); ++i)
    {
        if (m_items[i]->Type() == type)
        {
            pItem = m_items[i];
            break;
        }
    }
    if (type == Breakdown_WeaponEffectHolder)
    {
        pItem = m_pWeaponEffects;
    }
    return pItem;
}

void CBreakdownsPane::OnSelChangedBreakdownTree(NMHDR*, LRESULT*)
{
    HTREEITEM hItem = m_itemBreakdownTree.GetSelectedItem();
    if (hItem != NULL)
    {
        DWORD itemData = m_itemBreakdownTree.GetItemData(hItem);
        if (itemData != 0)      // headings don't have items to display
        {
            BreakdownItem * pItem = static_cast<BreakdownItem *>((void*)itemData);
            pItem->PopulateBreakdownControl(&m_itemBreakdownList);
            m_buttonClipboard.EnableWindow(TRUE);
        }
        else
        {
            // just empty the control
            m_itemBreakdownList.DeleteAllItems();
            // no specific item selected, therefore for clipboard copy
            m_buttonClipboard.EnableWindow(FALSE);
        }
    }
}

BOOL CBreakdownsPane::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
    CPoint point;
    BOOL retVal = FALSE;

    // is the mouse over the divider control?
    GetCursorPos(&point);
    CWnd *pWnd2 = WindowFromPoint(point);
    if (pWnd2 != NULL)
    {
        if (&m_divider == pWnd2)
        {
            // it is, show a drag up/down cursor
            SetCursor(LoadCursor(NULL, IDC_SIZENS));
            retVal = TRUE;
        }
    }
    else if (m_bDraggingDivider)
    {
        // also set the cursor if a drag action is in progress
        SetCursor(LoadCursor(NULL, IDC_SIZENS));
        retVal = TRUE;
    }

    if (retVal == FALSE)
    {
        // call base class if we didn't handle special case
        retVal = CFormView::OnSetCursor(pWnd, nHitTest, message);
    }
    return retVal;
}

void CBreakdownsPane::OnDividerClicked()
{
    CRect rect;
    m_bDraggingDivider = true;
    m_divider.GetWindowRect(&rect);
    ScreenToClient(&rect);
    m_treeSizePercent = rect.top - c_controlSpacing;
    SetCapture();               // need all mouse messages while drag in progress
    SetCursor(LoadCursor(NULL, IDC_SIZENS));
}

void CBreakdownsPane::OnMouseMove(UINT nFlags, CPoint point)
{
    CFormView::OnMouseMove(nFlags, point);

    if (m_bDraggingDivider)
    {
        CalculatePercent(point);
        SetCursor(LoadCursor(NULL, IDC_SIZENS));
        m_divider.Invalidate();
    }
}

void CBreakdownsPane::OnLButtonUp(UINT nFlags, CPoint point)
{
    CFormView::OnLButtonUp(nFlags, point);
    if (m_bDraggingDivider)
    {
        ReleaseCapture();
        m_bDraggingDivider = false;
        CalculatePercent(point);
        // save the new value to registry to persist it
        AfxGetApp()->WriteProfileInt(f_treeSizeKey, f_treeSizeEntry, m_treeSizePercent);
    }
}

void CBreakdownsPane::CalculatePercent(CPoint point)
{
    CRect rect;
    CRect rctDivider;
    GetClientRect(&rect);
    m_divider.GetWindowRect(&rctDivider);
    int verticalSpace = rect.Height() - (c_controlSpacing * 4) - rctDivider.Height();
    m_treeSizePercent = (point.y - (c_controlSpacing * 2)) * 100 / verticalSpace;

    // ensure other window is always visible in some way
    if (m_treeSizePercent < 10)
    {
        m_treeSizePercent = 10;
    }
    if (m_treeSizePercent > 90)
    {
        m_treeSizePercent = 90;
    }

    // now force an on size event
    CRect rctWnd;
    GetClientRect(&rctWnd);
    OnSize(SIZE_RESTORED, rctWnd.Width(), rctWnd.Height());
}

void CBreakdownsPane::OnEndtrackBreakdownList(NMHDR* pNMHDR, LRESULT* pResult)
{
    // just save the column widths to registry so restored next time we run
    UNREFERENCED_PARAMETER(pNMHDR);
    UNREFERENCED_PARAMETER(pResult);
    SaveColumnWidthsByName(&m_itemBreakdownList, "BreakdownList_%s");
}

void CBreakdownsPane::OnButtonClipboardCopy()
{
    // build up column data so we can align the clipboard text output
    std::vector<CString> columns[CO_count];
    // title is the selected breakdown and its total
    HTREEITEM hItem = m_itemBreakdownTree.GetSelectedItem();
    if (hItem != NULL)
    {
        DWORD itemData = m_itemBreakdownTree.GetItemData(hItem);
        if (itemData != 0)      // headings don't have items to display
        {
            BreakdownItem * pItem = static_cast<BreakdownItem *>((void*)itemData);
            // first item is the breakdown item and total
            columns[CO_Source].push_back(pItem->Title());
            columns[CO_Value].push_back(pItem->Value());
            columns[CO_Stacks].push_back("");
            columns[CO_BonusType].push_back("");
            // now add headings in the list control
            CHeaderCtrl* pHeaderCtrl = m_itemBreakdownList.GetHeaderCtrl();
            int nColumnCount = pHeaderCtrl->GetItemCount();
            ASSERT(nColumnCount == CO_count);
            for (int column = 0; column < nColumnCount; ++column)
            {
                HDITEM item;
                char itemText[100];
                item.mask = HDI_TEXT;
                item.pszText = itemText;
                item.cchTextMax = 100;
                pHeaderCtrl->GetItem(column, &item);
                columns[column].push_back(item.pszText);
            }
            // now add the content
            size_t count = (size_t)m_itemBreakdownList.GetItemCount();
            for (size_t i = 0; i < count; ++i)
            {
                CString itemText;
                for (int column = 0; column < nColumnCount; ++column)
                {
                    itemText = m_itemBreakdownList.GetItemText(i, column);
                    columns[column].push_back(itemText);
                }
            }
            // now pad all items in each column to the same size
            int maxWidth[CO_count] = {0, 0, 0, 0};
            for (size_t i = 0; i < columns[0].size(); ++i)
            {
                for (int column = 0; column < nColumnCount; ++column)
                {
                    maxWidth[column] = max(maxWidth[column], columns[column][i].GetLength());
                }
            }
            for (size_t i = 0; i < columns[0].size(); ++i)
            {
                CString padded;
                for (int column = 0; column < nColumnCount; ++column)
                {
                    padded.Format("%*s", maxWidth[column] + 2, (LPCTSTR)columns[column][i]); // 2 extra spaces
                    columns[column][i] = padded;
                }
            }
            // now generate the total clipboard text, ensure mono-spaced
            CString clipboardText;
            clipboardText += "[font=courier][code]\r\n";
            for (size_t i = 0; i < columns[0].size(); ++i)
            {
                for (int column = 0; column < nColumnCount; ++column)
                {
                    clipboardText += columns[column][i];
                }
                clipboardText += "\r\n";
            }
            clipboardText += "[/code][/font]\r\n";
            //FormatExportData(&clipboardText); // [code] tags makes this not required
            // now place the text on the clipboard
            if (OpenClipboard())
            {
                HGLOBAL clipbuffer = GlobalAlloc(GMEM_DDESHARE, clipboardText.GetLength()+1);
                ASSERT(clipbuffer != NULL);
                char *buffer = (char*)GlobalLock(clipbuffer);
                strcpy_s(buffer, clipboardText.GetLength()+1, clipboardText);
                GlobalUnlock(clipbuffer);
                EmptyClipboard();
                SetClipboardData(CF_TEXT, clipbuffer);
                CloseClipboard();
            }
        }
    }
}

void CBreakdownsPane::UpdateGearChanged(
    Build* pBuild,
    InventorySlotType slot)
{
    if (slot == Inventory_Weapon1
            || slot == Inventory_Weapon2)
    {
        // m_pWeaponEffects holds the actual breakdowns, we just tell it
        // to re-create them based on the selected items
        m_pWeaponEffects->WeaponsChanged(pBuild->ActiveGearSet());
    }
    // this needs to be passed through to all breakdowns
    for (auto&& it : m_items)
    {
        it->GearChanged(pBuild, slot);
    }
    m_pWeaponEffects->GearChanged(pBuild, slot);
}

void CBreakdownsPane::UpdateTotalChanged(
    BreakdownItem *,
    BreakdownType bt)
{
    AbilityType at = Ability_Unknown;
    switch (bt)
    {
        case Breakdown_Strength:        at = Ability_Strength; break;
        case Breakdown_Intelligence:    at = Ability_Intelligence; break;
        case Breakdown_Wisdom:          at = Ability_Wisdom; break;
        case Breakdown_Dexterity:       at = Ability_Dexterity; break;
        case Breakdown_Constitution:    at = Ability_Constitution; break;
        case Breakdown_Charisma:        at = Ability_Charisma; break;
    }
    // need to notify all breakdowns
    for (auto&& bcit: m_mapBuildCallbacks)
    {
        std::list<EffectCallbackItem*> callbacks = bcit.second;
        for (auto&& cit : callbacks)
        {
            cit->AbilityTotalChanged(m_pCharacter->ActiveBuild(), at);
        }
    }
}

// To avoid observer callback bloat on all effects, only the breakdowns pane
// observes for feat and enhancement effects. This pane maintains a map of
// Effect_<type> to and array of <BreakdownItem*>. It can then use a quick
// look up for the effect type and only apply the update action on the affected
// objects, instead of having them all look at the effect and then decide
// its not for them.
void CBreakdownsPane::UpdateBuildLevelChanged(Build* pBuild)
{
    // this needs to be passed through to all breakdowns
    for (auto&& bcit: m_mapBuildCallbacks)
    {
        std::list<EffectCallbackItem*> callbacks = bcit.second;
        for (auto&& cit : callbacks)
        {
            cit->BuildLevelChanged(pBuild);
        }
    }
}

void CBreakdownsPane::UpdateClassChanged(
        Build* pBuild,
        const std::string& classFrom,
        const std::string& classTo,
        size_t level)
{
    // this needs to be passed through to all breakdowns
    for (auto&& bcit: m_mapBuildCallbacks)
    {
        std::list<EffectCallbackItem*> callbacks = bcit.second;
        for (auto&& cit : callbacks)
        {
            cit->ClassChanged(pBuild, classFrom, classTo, level);
        }
    }
}

void CBreakdownsPane::UpdateFeatEffectApplied(
        Build* pBuild,
        const Effect& effect)
{
    // effects can have multiple types
    const std::list<EffectType> & types = effect.Type();
    Effect copy = effect;   // weapon effect when notified need to have single Type() elements
    for (auto&& tit : types)
    {
        copy.SetType(tit);  // single Type()
        copy.Set_Weapon1();
        copy.Set_Weapon2();
        if (m_mapBuildCallbacks.find(tit) != m_mapBuildCallbacks.end())
        {
            // we have a list of callbacks for this effect type
            std::list<EffectCallbackItem*> callbacks = m_mapBuildCallbacks[tit];
            for (auto&& cit : callbacks)
            {
                cit->FeatEffectApplied(pBuild, copy);
            }
        }
    }
}

void CBreakdownsPane::UpdateFeatEffectRevoked(
        Build* pBuild,
        const Effect& effect)
{
    // effects can have multiple types
    const std::list<EffectType>& types = effect.Type();
    Effect copy = effect;   // weapon effect when notified need to have single Type() elements
    for (auto&& tit : types)
    {
        copy.SetType(tit);  // single Type()
        copy.Set_Weapon1();
        copy.Set_Weapon2();
        if (m_mapBuildCallbacks.find(tit) != m_mapBuildCallbacks.end())
        {
            // we have a list of callbacks for this effect type
            std::list<EffectCallbackItem*> callbacks = m_mapBuildCallbacks[tit];
            for (auto&& cit : callbacks)
            {
                cit->FeatEffectRevoked(pBuild, copy);
            }
        }
    }
}

void CBreakdownsPane::UpdateAbilityValueChanged(Build*, AbilityType ability)
{
    // need to notify all breakdowns
    for (auto&& bcit: m_mapBuildCallbacks)
    {
        std::list<EffectCallbackItem*> callbacks = bcit.second;
        for (auto&& cit : callbacks)
        {
            cit->AbilityValueChanged(m_pCharacter->ActiveBuild(), ability);
        }
    }
}

void CBreakdownsPane::UpdateItemEffectApplied(
        Build* pBuild,
        const Effect& effect)
{
    // effects can have multiple types
    const std::list<EffectType>& types = effect.Type();
    Effect copy = effect;   // weapon effect when notified need to have single Type() elements
    for (auto&& tit : types)
    {
        copy.SetType(tit);  // single Type()
        copy.Set_Weapon1();
        copy.Set_Weapon2();
        if (m_mapBuildCallbacks.find(tit) != m_mapBuildCallbacks.end())
        {
            // we have a list of callbacks for this effect type
            std::list<EffectCallbackItem*> callbacks = m_mapBuildCallbacks[tit];
            for (auto&& cit : callbacks)
            {
                cit->ItemEffectApplied(pBuild, copy);
            }
        }
    }
}

void CBreakdownsPane::UpdateItemEffectRevoked(
        Build* pBuild,
        const Effect& effect)
{
    // effects can have multiple types
    const std::list<EffectType>& types = effect.Type();
    Effect copy = effect;   // weapon effect when notified need to have single Type() elements
    for (auto&& tit : types)
    {
        copy.SetType(tit);  // single Type()
        copy.Set_Weapon1();
        copy.Set_Weapon2();
        if (m_mapBuildCallbacks.find(tit) != m_mapBuildCallbacks.end())
        {
            // we have a list of callbacks for this effect type
            std::list<EffectCallbackItem*> callbacks = m_mapBuildCallbacks[tit];
            for (auto&& cit : callbacks)
            {
                cit->ItemEffectRevoked(pBuild, copy);
            }
        }
    }
}

void CBreakdownsPane::UpdateItemWeaponEffectApplied(
    Build* pBuild,
    const Effect& effect,
    WeaponType wt,
    InventorySlotType ist)
{
    // effects can have multiple types
    const std::list<EffectType>& types = effect.Type();
    Effect copy = effect;   // weapon effect when notified need to have single Type() elements
    for (auto&& tit : types)
    {
        copy.SetType(tit);  // single Type()
        switch (ist)
        {
        case Inventory_Weapon1: copy.Set_Weapon1(); break;
        case Inventory_Weapon2: copy.Set_Weapon2(); break;
        default: copy.Set_Weapon1(); copy.Set_Weapon2(); break;
        }
        if (m_mapBuildCallbacks.find(tit) != m_mapBuildCallbacks.end())
        {
            // we have a list of callbacks for this effect type
            std::list<EffectCallbackItem*> callbacks = m_mapBuildCallbacks[tit];
            for (auto&& cit : callbacks)
            {
                cit->ItemEffectApplied(pBuild, copy, wt);
            }
        }
    }
}

void CBreakdownsPane::UpdateItemWeaponEffectRevoked(
    Build* pBuild,
    const Effect& effect,
    WeaponType wt,
    InventorySlotType ist)
{
    // effects can have multiple types
    const std::list<EffectType>& types = effect.Type();
    Effect copy = effect;   // weapon effect when notified need to have single Type() elements
    for (auto&& tit : types)
    {
        copy.SetType(tit);  // single Type()
        switch (ist)
        {
        case Inventory_Weapon1: copy.Set_Weapon1(); break;
        case Inventory_Weapon2: copy.Set_Weapon2(); break;
        default: copy.Set_Weapon1(); copy.Set_Weapon2(); break;
        }
        if (m_mapBuildCallbacks.find(tit) != m_mapBuildCallbacks.end())
        {
            // we have a list of callbacks for this effect type
            std::list<EffectCallbackItem*> callbacks = m_mapBuildCallbacks[tit];
            for (auto&& cit : callbacks)
            {
                cit->ItemEffectRevoked(pBuild, copy, wt);
            }
        }
    }
}

void CBreakdownsPane::UpdateEnhancementTrained(Build* pBuild, const EnhancementItemParams& item)
{
    // this needs to be passed through to all breakdowns
    for (auto&& it : m_items)
    {
        it->EnhancementTrained(pBuild, item);
    }
    m_pWeaponEffects->EnhancementTrained(pBuild, item);
}

void CBreakdownsPane::UpdateEnhancementRevoked(Build* pBuild, const EnhancementItemParams& item)
{
    // this needs to be passed through to all breakdowns
    for (auto&& it : m_items)
    {
        it->EnhancementRevoked(pBuild, item);
    }
    m_pWeaponEffects->EnhancementRevoked(pBuild, item);
}

void CBreakdownsPane::UpdateEnhancementEffectApplied(
    Build* pBuild,
    const Effect& effect)
{
    // effects can have multiple types
    const std::list<EffectType>& types = effect.Type();
    Effect copy = effect;   // weapon effect when notified need to have single Type() elements
    for (auto&& tit : types)
    {
        copy.SetType(tit);  // single Type()
        copy.Set_Weapon1();
        copy.Set_Weapon2();
        if (m_mapBuildCallbacks.find(tit) != m_mapBuildCallbacks.end())
        {
            // we have a list of callbacks for this effect type
            std::list<EffectCallbackItem*> callbacks = m_mapBuildCallbacks[tit];
            for (auto&& cit : callbacks)
            {
                cit->EnhancementEffectApplied(pBuild, copy);
            }
        }
    }
}

void CBreakdownsPane::UpdateEnhancementEffectRevoked(
    Build* pBuild,
    const Effect& effect)
{
    // effects can have multiple types
    const std::list<EffectType>& types = effect.Type();
    Effect copy = effect;   // weapon effect when notified need to have single Type() elements
    for (auto&& tit : types)
    {
        copy.SetType(tit);  // single Type()
        copy.Set_Weapon1();
        copy.Set_Weapon2();
        if (m_mapBuildCallbacks.find(tit) != m_mapBuildCallbacks.end())
        {
            // we have a list of callbacks for this effect type
            std::list<EffectCallbackItem*> callbacks = m_mapBuildCallbacks[tit];
            for (auto&& cit : callbacks)
            {
                cit->EnhancementEffectRevoked(pBuild, copy);
            }
        }
    }
}

void CBreakdownsPane::UpdateFeatTrained(Build* pBuild, const std::string& featName)
{
    // this needs to be passed through to all breakdowns
    for (auto&& bcit: m_mapBuildCallbacks)
    {
        std::list<EffectCallbackItem*> callbacks = bcit.second;
        for (auto&& cit : callbacks)
        {
            cit->FeatTrained(pBuild, featName);
        }
    }
}

void CBreakdownsPane::UpdateFeatRevoked(Build* pBuild, const std::string& featName)
{
    // this needs to be passed through to all breakdowns
    for (auto&& bcit: m_mapBuildCallbacks)
    {
        std::list<EffectCallbackItem*> callbacks = bcit.second;
        for (auto&& cit : callbacks)
        {
            cit->FeatRevoked(pBuild, featName);
        }
    }
}

void CBreakdownsPane::UpdateSkillSpendChanged(Build* pBuild, size_t level, SkillType skill)
{
    // this needs to be passed through to all breakdowns
    for (auto&& bcit: m_mapBuildCallbacks)
    {
        std::list<EffectCallbackItem*> callbacks = bcit.second;
        for (auto&& cit : callbacks)
        {
            cit->SkillSpendChanged(pBuild, level, skill);
        }
    }
}

// LifeObserver overrides
void CBreakdownsPane::UpdateRaceChanged(Life* pLife, const std::string& race)
{
    // this needs to be passed through to all breakdowns
    for (auto&& bcit: m_mapBuildCallbacks)
    {
        std::list<EffectCallbackItem*> callbacks = bcit.second;
        for (auto&& cit : callbacks)
        {
            cit->RaceChanged(pLife, race);
        }
    }
}

void CBreakdownsPane::UpdateAbilityTomeChanged(Life* pLife, AbilityType ability)
{
    // this needs to be passed through to all breakdowns
    for (auto&& bcit: m_mapBuildCallbacks)
    {
        std::list<EffectCallbackItem*> callbacks = bcit.second;
        for (auto&& cit : callbacks)
        {
            cit->AbilityTomeChanged(pLife, ability);
        }
    }
}

void CBreakdownsPane::UpdateSkillTomeChanged(Life* pLife, SkillType skill)
{
    // this needs to be passed through to all breakdowns
    for (auto&& bcit : m_mapBuildCallbacks)
    {
        std::list<EffectCallbackItem*> callbacks = bcit.second;
        for (auto&& cit : callbacks)
        {
            cit->SkillTomeChanged(pLife, skill);
        }
    }
}

void CBreakdownsPane::UpdateStanceActivated(Build* pBuild, const std::string& stanceName)
{
    // this needs to be passed through to all breakdowns
    for (auto&& bcit: m_mapBuildCallbacks)
    {
        std::list<EffectCallbackItem*> callbacks = bcit.second;
        for (auto&& cit : callbacks)
        {
            cit->StanceActivated(pBuild, stanceName);
        }
    }
}

void CBreakdownsPane::UpdateStanceDeactivated(Build* pBuild, const std::string& stanceName)
{
    // this needs to be passed through to all breakdowns
    for (auto&& bcit: m_mapBuildCallbacks)
    {
        std::list<EffectCallbackItem*> callbacks = bcit.second;
        for (auto&& cit : callbacks)
        {
            cit->StanceDeactivated(pBuild, stanceName);
        }
    }
}

void CBreakdownsPane::UpdateSliderChanged(Build* pBuild, const std::string& sliderName, int newValue)
{
    // this needs to be passed through to all breakdowns
    for (auto&& it : m_items)
    {
        it->SliderChanged(pBuild, sliderName, newValue);
    }
    m_pWeaponEffects->SliderChanged(pBuild, sliderName, newValue);
}

