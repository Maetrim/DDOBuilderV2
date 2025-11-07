// ForumExportDlg.cpp
//
#include "stdafx.h"
#include "ForumExportDlg.h"
#include "BreakdownItem.h"
#include "BreakdownItemMRR.h"
#include "BreakdownItemSave.h"
#include "BreakdownItemWeaponEffects.h"
#include "MainFrm.h"
#include "SLAControl.h"
#include "Race.h"
#include "Class.h"
#include "GrantedFeatsPane.h"
#include "EnhancementTreeItem.h"
#include "SetBonus.h"
#include "DCPane.h"
#include "SpellsPane.h"
#include "SpellLikeAbilityPage.h"
#include "StancesPane.h"
#include <numeric>

CForumExportDlg::CForumExportDlg(Build* pBuild) :
    CDialogEx(CForumExportDlg::IDD),
    m_pBuild(pBuild),
    m_bPopulatingControl(false),
    m_exportType(0) // default to Forum BB code
{
    // read section order and display states from the registry
    for (size_t i = 0; i < FES_Count; ++i)
    {
        CString shown;
        CString section;
        shown.Format("ShowSection%d", i);
        section.Format("SectionType%d", i);
        m_bShowSection[i] = (AfxGetApp()->GetProfileInt("ForumExport", shown, 1) != 0);
        m_SectionOrder[i] = (ForumExportSections)(AfxGetApp()->GetProfileInt("ForumExport", section, i));
    }
    m_exportType = AfxGetApp()->GetProfileInt("ForumExport", "ExportFormat", m_exportType);
}

CForumExportDlg::~CForumExportDlg()
{
    // save section order and display states to the registry
    for (size_t i = 0; i < FES_Count; ++i)
    {
        CString shown;
        CString section;
        shown.Format("ShowSection%d", i);
        section.Format("SectionType%d", i);
        AfxGetApp()->WriteProfileInt("ForumExport", shown, m_bShowSection[i]);
        AfxGetApp()->WriteProfileInt("ForumExport", section, m_SectionOrder[i]);
    }
}

void CForumExportDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST_SECTIONS, m_listConfigureExport);
    DDX_Control(pDX, IDC_COMBO_FORMAT, m_comboFormat);
    DDX_Control(pDX, IDC_EDIT_EXPORT, m_editExport);
    DDX_Control(pDX, IDC_BUTTON_PROMOTE, m_buttonMoveUp);
    DDX_Control(pDX, IDC_BUTTON_DEMOTE, m_buttonMoveDown);
}

BEGIN_MESSAGE_MAP(CForumExportDlg, CDialogEx)
    ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_SECTIONS, OnItemchangedListConfigureExport)
    ON_BN_CLICKED(IDC_BUTTON_PROMOTE, OnMoveUp)
    ON_BN_CLICKED(IDC_BUTTON_DEMOTE, OnMoveDown)
    ON_CBN_SELCHANGE(IDC_COMBO_FORMAT,&CForumExportDlg::OnSelchangeComboFormat)
END_MESSAGE_MAP()

BOOL CForumExportDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    m_comboFormat.ResetContent();
    m_comboFormat.AddString("Forum BB Code");
    m_comboFormat.AddString("Plain Text");
    m_comboFormat.SetCurSel(m_exportType);

    // need non-proportional font for the edit control
    LOGFONT lf;
    ZeroMemory((PVOID)&lf, sizeof(LOGFONT));
    NONCLIENTMETRICS nm;
    nm.cbSize = sizeof(NONCLIENTMETRICS);
    VERIFY(SystemParametersInfo(SPI_GETNONCLIENTMETRICS, nm.cbSize,&nm, 0));
    lf = nm.lfMenuFont;
    strcpy_s(lf.lfFaceName, LF_FACESIZE, "Consolas");
    m_font.CreateFontIndirect(&lf);
    m_editExport.SetFont(&m_font);

    // Configure the list control with items and states
    m_listConfigureExport.InsertColumn(0, "Export Item", LVCFMT_LEFT, 150);
    m_listConfigureExport.SetExtendedStyle(
            m_listConfigureExport.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES | LVS_EX_LABELTIP);
    PopulateConfigureExportList();
    PopulateExport();

    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}

void CForumExportDlg::PopulateConfigureExportList()
{
    m_bPopulatingControl = true;
    m_listConfigureExport.LockWindowUpdate();
    // get current selection
    int sel = m_listConfigureExport.GetSelectionMark();
    m_listConfigureExport.DeleteAllItems();
    for (size_t i = 0; i < FES_Count; ++i)
    {
        CString item;
        item = EnumEntryText(m_SectionOrder[i], forumExportSectionsMap);
        m_listConfigureExport.InsertItem(i, item, 0);
        m_listConfigureExport.SetItemData(i, m_SectionOrder[i]);
        m_listConfigureExport.SetItemState(
                i,
                INDEXTOSTATEIMAGEMASK((int)m_bShowSection[i] + 1),
                LVIS_STATEIMAGEMASK);
    }
    // restore selection (if any)
    if (sel >= 0)
    {
        m_listConfigureExport.SetItemState(
                sel,
                LVIS_SELECTED | LVIS_FOCUSED,
                LVIS_SELECTED | LVIS_FOCUSED);
        m_listConfigureExport.SetSelectionMark(sel);
        // enable the Move Up/Down buttons appropriately
        m_buttonMoveUp.EnableWindow(sel > 0);
        m_buttonMoveDown.EnableWindow(sel < FES_Count - 1);
    }
    else
    {
        m_buttonMoveUp.EnableWindow(FALSE);
        m_buttonMoveDown.EnableWindow(FALSE);
    }
    m_listConfigureExport.UnlockWindowUpdate();
    m_bPopulatingControl = false;
}

void CForumExportDlg::OnItemchangedListConfigureExport(NMHDR* pNMHDR, LRESULT* pResult)
{
    if (!m_bPopulatingControl)
    {
        NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

        BOOL oldCheckState = ((pNMListView->uOldState& LVIS_STATEIMAGEMASK)>>12) - 1;
        BOOL newCheckState = ((pNMListView->uNewState& LVIS_STATEIMAGEMASK)>>12) - 1;

        if (oldCheckState != -1
              && newCheckState != -1
              && oldCheckState != newCheckState)
        {
            m_bShowSection[pNMListView->iItem] = !m_bShowSection[pNMListView->iItem];
            PopulateExport();
        }

        if ((pNMListView->uChanged& LVIF_STATE) 
              && (pNMListView->uNewState& LVIS_SELECTED))
        {
            // item selection has changed, select it
            int sel = pNMListView->iItem;
            // enable the Move Up/Down buttons appropriately
            m_buttonMoveUp.EnableWindow(sel > 0);
            m_buttonMoveDown.EnableWindow(sel < FES_Count - 1);
        }
    }
   *pResult = 0;
}

void CForumExportDlg::PopulateExport()
{
    std::stringstream forumExport;
    forumExport << "[font=courier]\r\n";
    for (size_t i = 0 ; i < FES_Count; ++i)
    {
        if (m_bShowSection[i])
        {
            switch (m_SectionOrder[i])
            {
            case FES_Header:
                AddCharacterHeader(forumExport);
                break;
            case FES_PastLives:
                AddPastLives(forumExport);
                break;
            case FES_SpecialFeats:
                AddSpecialFeats(forumExport);
                break;
            case FES_Saves:
                AddSaves(forumExport);
                break;
            case FES_EnergyResistances:
                AddEnergyResistances(forumExport);
                break;
            case FES_FeatSelections:
            case FES_FeatSelectionsNoSkills:
                AddFeatSelections(forumExport, m_SectionOrder[i] == FES_FeatSelections);
                break;
            case FES_GrantedFeats:
                AddGrantedFeats(forumExport);
                break;
            case FES_AutomaticFeats:
                AddAutomaticFeats(forumExport);
                break;
            case FES_ConsolidatedFeats:
                AddConsolidatedFeats(forumExport);
                break;
            case FES_Skills:
                AddSkills(forumExport);
                break;
            case FES_ActiveStances:
                AddActiveStances(forumExport);
                break;
            case FES_SelfAndPartyBuffs:
                AddSelfAndPartyBuffs(forumExport);
                break;
            case FES_EnhancementTrees:
                AddEnhancements(forumExport);
                break;
            case FES_DestinyTrees:
                AddEpicDestinyTree(forumExport);
                break;
            case FES_ReaperTrees:
                AddReaperTrees(forumExport);
                break;
            case FES_SpellPowers:
                AddSpellPowers(forumExport);
                break;
            case FES_Spells:
                AddSpells(forumExport);
                break;
            case FES_SLAs:
                AddSLAs(forumExport);
                break;
            case FES_WeaponDamage:
                AddWeaponDamage(forumExport);
                break;
            case FES_TacticalDCs:
                AddTacticalDCs(forumExport);
                break;
            case FES_Gear:
                AddGear(forumExport);
                break;
            case FES_AlternateGearLayouts:
                AddAlternateGear(forumExport);
                break;
            case FES_SimpleGear:
                AddSimpleGear(forumExport);
                break;
            }
        }
    }
    forumExport << "[/font]\r\n";
    // to format correctly to the forums multiple space " " characters cannot
    // be present else they get reduced to a single space losing all layout the formatting
    // to avoid this we do a a replace of all "  " to " ." in the generated
    // string on all odd character positions from the start of each line
    std::string generatedData = forumExport.str();
    FormatExportData(&generatedData);
    ApplyFormatSelection(&generatedData);
    // need to retain the control scroll position
    int pos = m_editExport.GetFirstVisibleLine();
    m_editExport.SetWindowText(generatedData.c_str());
    m_editExport.LineScroll(pos, 0);
}

void CForumExportDlg::OnOK()
{
    // copy to clipboard and close dialog
    CString clipboardText;
    m_editExport.GetWindowText(clipboardText);
    // now place the text on the clipboard
    if (::OpenClipboard(NULL))
    {
        HGLOBAL clipbuffer = ::GlobalAlloc(GMEM_DDESHARE, clipboardText.GetLength()+1);
        ASSERT(clipbuffer != NULL);
        char*buffer = (char*)::GlobalLock(clipbuffer);
        strcpy_s(buffer, clipboardText.GetLength()+1, clipboardText);
        ::GlobalUnlock(clipbuffer);
        ::EmptyClipboard();
        ::SetClipboardData(CF_TEXT, clipbuffer);
        ::CloseClipboard();
    }
    CDialogEx::OnOK();
}

void CForumExportDlg::AddCharacterHeader(std::stringstream& forumExport)
{
    // Character Name: [.................................]
    // Classes: [x][class1], [y][class2], [z][class3] and [10]Epic
    // Race: [...............] Alignment: [..............]
    //
    //      Start Tome Final     Incorp: xx%         Displacement: xx%
    // Str: [...] [..] [...]     HP:    [....]       Incorp:
    // Dex: [...] [..] [...]     PRR:   [....]       AC: [...]
    // Con: [...] [..] [...]     MRR:   [....]/[MAX] +Healing Amp: [...]
    // Int: [...] [..] [...]     Dodge: [...]%/[MAX] -Healing Amp: [...]
    // Wis: [...] [..] [...]     Fort:  [...]%       Repair Amp:   [...]
    // Cha: [...] [..] [...]     SR:    [...]        BAB:   [..]
    // DR:  List of DR items
    // immunities:  List of immunities

     // first line is the character name
    forumExport << "Character name: " << m_pBuild->Name() << "\r\n";

    // Classes line
    forumExport << "Classes: " << m_pBuild->ComplexUIDescription() << "\r\n";

    // Race/Alignment line
    forumExport << "Race: ";
    forumExport.fill(' ');
    forumExport.width(21);
    forumExport << std::left << m_pBuild->Race();
    std::string alignment = EnumEntryText(m_pBuild->Alignment(), alignmentTypeMap);
    forumExport << "Alignment: ";
    forumExport << std::left << alignment << "\r\n";
    // blank line
    forumExport << "\r\n";
    forumExport << "     Start Tome Final";
    AddBreakdown(forumExport, "      HP: ", 10, Breakdown_Hitpoints);
    AddBreakdown(forumExport, "      Displacement: ", 4, Breakdown_Displacement);
    forumExport << "%\r\n";

    AddAbilityValues(forumExport, Ability_Strength);
    AddBreakdown(forumExport, "      Unc Rng: ", 5, Breakdown_UnconsciousRange);
    AddBreakdown(forumExport, "      Incorp: ", 10, Breakdown_Incorporeality);
    forumExport << "%\r\n";

    AddAbilityValues(forumExport, Ability_Dexterity);
    AddBreakdown(forumExport, "      PRR: ", 9, Breakdown_PRR);
    AddBreakdown(forumExport, "      AC: ", 15, Breakdown_AC);
    forumExport << "\r\n";

    AddAbilityValues(forumExport, Ability_Constitution);
    AddBreakdown(forumExport, "      MRR: ", 9, Breakdown_MRR);
    AddBreakdown(forumExport, "      +Healing Amp: ", 5, Breakdown_HealingAmplification);
    forumExport << "\r\n";

    AddAbilityValues(forumExport, Ability_Intelligence);
    AddBreakdown(forumExport, "      Dodge: ", 4, Breakdown_Dodge);
    if (m_pBuild->IsStanceActive("Tower Shield"))
    {
        AddBreakdown(forumExport, "/", 1, Breakdown_DodgeCapTowerShield);
    }
    else
    {
        AddBreakdown(forumExport, "/", 1, Breakdown_DodgeCap);
    }
    AddBreakdown(forumExport, "      -Healing Amp: ", 5, Breakdown_NegativeHealingAmplification);
    forumExport << "\r\n";

    AddAbilityValues(forumExport, Ability_Wisdom);
    AddBreakdown(forumExport, "      Fort: ", 7, Breakdown_Fortification);
    AddBreakdown(forumExport, "%      Repair Amp:   ", 5, Breakdown_RepairAmplification);
    forumExport << "\r\n";

    AddAbilityValues(forumExport, Ability_Charisma);
    AddBreakdown(forumExport, "      SR: ", 10, Breakdown_SpellResistance);
    AddBreakdown(forumExport, "      BAB: ", 14, Breakdown_BAB);
    forumExport << "\r\n";
    BreakdownItem* pDR = FindBreakdown(Breakdown_DR);
    forumExport << "DR: " << pDR->Value();
    forumExport << "\r\n";
    BreakdownItem* pImmunities = FindBreakdown(Breakdown_Immunities);
    forumExport << "Immunities: " << pImmunities->Value();
    forumExport << "\r\n\r\n";
}

void CForumExportDlg::AddPastLives(std::stringstream& forumExport)
{
    // add the special feats (past lives and other feats such as inherent)
    std::list<TrainedFeat> feats = m_pBuild->SpecialFeats();
    if (feats.size() > 0)
    {
        if (feats.size() > 1)
        {
            // combine duplicates in the list
            std::list<TrainedFeat>::iterator it = feats.begin();
            while (it != feats.end())
            {
                // look at all following items and combine if required
                std::list<TrainedFeat>::iterator nit = it;
                ++nit;          // start from next element
                while (nit != feats.end())
                {
                    if ((*it) == (*nit))
                    {
                        // need to combine these elements
                        (*it).IncrementCount();
                        nit = feats.erase(nit);
                    }
                    else
                    {
                        // check the next one
                        ++nit;
                    }
                }
                ++it;
            }
        }
        // sort the feats before output
        feats.sort();
        AddFeats(forumExport, "Heroic Past Lives", "HeroicPastLife", feats);
        AddFeats(forumExport, "Racial Past Lives", "RacialPastLife", feats);
        AddFeats(forumExport, "Iconic Past Lives", "IconicPastLife", feats);
        AddFeats(forumExport, "Epic Past Lives", "EpicPastLife", feats);
        forumExport << "\r\n";
    }
}

void CForumExportDlg::AddSpecialFeats(std::stringstream& forumExport)
{
    // add the special feats (past lives and other feats such as inherent)
    std::list<TrainedFeat> feats = m_pBuild->SpecialFeats();
    if (feats.size() > 0)
    {
        if (feats.size() > 1)
        {
            // combine duplicates in the list
            std::list<TrainedFeat>::iterator it = feats.begin();
            while (it != feats.end())
            {
                // look at all following items and combine if required
                std::list<TrainedFeat>::iterator nit = it;
                ++nit;          // start from next element
                while (nit != feats.end())
                {
                    if ((*it) == (*nit))
                    {
                        // need to combine these elements
                        (*it).IncrementCount();
                        nit = feats.erase(nit);
                    }
                    else
                    {
                        // check the next one
                        ++nit;
                    }
                }
                ++it;
            }
        }
        // sort the feats before output
        feats.sort();
        AddFeats(forumExport, "Special Feats", "Special", feats);
        AddFeats(forumExport, "Favor Feats", "Favor", feats);
        forumExport << "\r\n";
    }
}

void CForumExportDlg::AddFeats(
        std::stringstream& forumExport,
        const std::string& heading,
        const std::string& featType,
        const std::list<TrainedFeat>& feats) const
{
    bool first = true;
    std::list<TrainedFeat>::const_iterator it = feats.begin();
    while (it != feats.end())
    {
        if ((*it).Type() == featType)
        {
            if (first)
            {
                forumExport << heading << "\r\n";
                forumExport << "[HR][/HR]\r\n";
                first = false;
            }
            // add this one to the export
            forumExport << (*it).FeatName();
            if ((*it).Count() > 1)
            {
                forumExport << "(" << (*it).Count() << ")";
            }
            forumExport << "\r\n";
        }
        ++it;
    }
    if (!first)
    {
        forumExport << "\r\n";
    }
}

void CForumExportDlg::AddSaves(std::stringstream& forumExport)
{
    forumExport << "Saves:\r\n";
    forumExport << "[TABLE]";
    AddTableEntryBreakdown(forumExport, "Fortitude", "", Breakdown_SaveFortitude);
    AddTableEntryBreakdown(forumExport, "", "vs Poison", Breakdown_SavePoison);
    AddTableEntryBreakdown(forumExport, "", "vs Disease     ", Breakdown_SaveDisease);
    AddTableEntryBreakdown(forumExport, "Will", "", Breakdown_SaveWill);
    AddTableEntryBreakdown(forumExport, "", "vs Enchantment", Breakdown_SaveEnchantment);
    AddTableEntryBreakdown(forumExport, "", "vs Illusion", Breakdown_SaveIllusion);
    AddTableEntryBreakdown(forumExport, "", "vs Fear", Breakdown_SaveFear);
    AddTableEntryBreakdown(forumExport, "", "vs Curse", Breakdown_SaveCurse);
    AddTableEntryBreakdown(forumExport, "Reflex", "", Breakdown_SaveReflex);
    AddTableEntryBreakdown(forumExport, "", "vs Traps", Breakdown_SaveTraps);
    AddTableEntryBreakdown(forumExport, "", "vs Spell", Breakdown_SaveSpell);
    AddTableEntryBreakdown(forumExport, "", "vs Magic", Breakdown_SaveMagic);
    forumExport << "[/TABLE]\r\n";
    forumExport << "Marked with a* is no fail on a 1 if required DC met\r\n";
    forumExport << "\r\n";
}

void CForumExportDlg::AddAbilityValues(std::stringstream& forumExport, AbilityType ability)
{
    size_t baseValue = m_pBuild->BaseAbilityValue(ability);
    BreakdownItem* pBI = FindBreakdown(StatToBreakdown(ability));
    size_t buffedValue = (size_t)pBI->Total();      // whole numbers only
    size_t tomeValue = m_pBuild->TomeAtLevel(ability, m_pBuild->Level()-1);
    // get the stat name and limit to first 3 character
    std::string name = (EnumEntryText(ability, abilityTypeMap));
    name.resize(3);
    forumExport.width(3);
    forumExport << name;
    forumExport << ": ";
    forumExport.width(5);
    forumExport << std::right << baseValue << " ";
    forumExport.width(4);
    forumExport << std::right << tomeValue << " ";
    forumExport.width(5);
    forumExport << std::right << buffedValue;
}

void CForumExportDlg::AddTableEntryBreakdown(
    std::stringstream& forumExport,
    const std::string& header,
    const std::string& sub,
    BreakdownType bt)
{
    BreakdownItem* pBI = FindBreakdown(bt);
    size_t value = (size_t)pBI->CappedTotal();      // whole numbers only
    forumExport << "[TR]";
    forumExport << "[TD]" << header << "[/TD]";
    forumExport << "[TD]" << sub << "[/TD]";
    forumExport << "[TD]";
    forumExport.width(3);
    forumExport.fill(' ');
    forumExport << value;
    BreakdownItemSave* pBIS = dynamic_cast<BreakdownItemSave*>(pBI);
    if (pBIS != NULL)
    {
        if (pBIS->HasNoFailOn1())
        {
            forumExport << "*";
        }
    }
    forumExport << "[/TD][/TR]\r\n";
}

void CForumExportDlg::AddBreakdown(
        std::stringstream& forumExport,
        const std::string& header,
        size_t width,
        BreakdownType bt)
{
    BreakdownItem* pBI = FindBreakdown(bt);
    int value = static_cast<int>(pBI->CappedTotal());      // whole numbers only
    if (bt == Breakdown_MRR)
    {
        BreakdownItem* pBIMRRCap = FindBreakdown(Breakdown_MRRCap);
        if (pBIMRRCap->Total() > 0
              && pBIMRRCap->Total() < pBI->Total())
        {
            // show that the MRR value is capped
            CString text;
            text.Format("%d/%d", value, static_cast<int>(pBIMRRCap->Total()));
            forumExport << header;
            forumExport.width(width);
            forumExport << std::right << (LPCTSTR)text;
        }
        else
        {
            // just show the MRR value
            forumExport << header;
            forumExport.width(width);
            forumExport << std::right << value;
        }
    }
    else
    {
        forumExport << header;
        forumExport.width(width);
        forumExport << std::right << value;
        BreakdownItemSave* pBIS = dynamic_cast<BreakdownItemSave*>(pBI);
        if (pBIS != NULL)
        {
            if (pBIS->HasNoFailOn1())
            {
                forumExport << "*";
            }
        }
    }
}

void CForumExportDlg::AddFeatSelections(std::stringstream& forumExport, bool bIncludeSkills)
{
    forumExport << "Class and Feat Selection\r\n";
    forumExport << "[TABLE]";
    forumExport << "[TR][TD]Level[/TD][TD]Class[/TD][TD]Feats[/TD][/TR]\r\n";
    for (size_t level = 0; level < m_pBuild->Level(); ++level)
    {
        forumExport << "[TR][TD]";
        const LevelTraining& levelData = m_pBuild->LevelData(level);
        std::vector<size_t> classLevels = m_pBuild->ClassLevels(level);
        std::string cn = levelData.HasClass() ? levelData.Class() : Class_Unknown;
        const Class& c = FindClass(cn);
        CString className;
        className.Format("%s(%d)",
                cn.c_str(),
                levelData.HasClass() ? classLevels[c.Index()] : 0);
        forumExport << (level + 1) << "[/TD][TD]" << className << "[/TD]";
        std::list<std::string> lines = GetLevelEntries(level, bIncludeSkills);
        if (lines.size() > 0)
        {
            bool bFirst = true;
            for (auto&& lit : lines)
            {
                if (!bFirst)
                {
                    forumExport << "[TR][TD][/TD][TD][/TD]";
                }
                bFirst = false;
                forumExport << "[TD]" << lit << "[/TD]";
                forumExport << "[/TR]\r\n";
            }
        }
        else
        {
            forumExport << "[TD][/TD][/TR]\r\n";
        }
    }
    forumExport << "[/TABLE]\r\n";
}

void CForumExportDlg::AddGrantedFeats(std::stringstream& forumExport)
{
    CWnd* pWnd = AfxGetMainWnd();
    CMainFrame* pMainWnd = dynamic_cast<CMainFrame*>(pWnd);
    const CGrantedFeatsPane* pGrantedFeatsPane = dynamic_cast<const CGrantedFeatsPane*>(pMainWnd->GetPaneView(RUNTIME_CLASS(CGrantedFeatsPane)));
    if (pGrantedFeatsPane != NULL)
    {
        const std::list<Effect>& grantedFeats = pGrantedFeatsPane->GrantedFeats();
        if (grantedFeats.size() > 0)
        {
            forumExport << "Granted Feats\r\n";
            forumExport << "[HR][/HR]\r\n";
            std::list<Effect>::const_iterator it = grantedFeats.begin();
            while (it != grantedFeats.end())
            {
                if (!(*it).HasRequirementsToBeActive()
                        || (*it).RequirementsToBeActive().Met(*m_pBuild, m_pBuild->Level() - 1, true, Inventory_Unknown, Weapon_Unknown, Weapon_Unknown))
                {
                    forumExport << (*it).Item().front();
                    forumExport << "\r\n";
                }
                ++it;
            }
            forumExport << "[HR][/HR]\r\n";
        }
        // blank line after
        forumExport << "\r\n";
    }
}

void CForumExportDlg::AddAutomaticFeats(std::stringstream& forumExport)
{
    // list all the automatic feats gained at each level
    forumExport << "Automatic Feats\r\n";
    forumExport << "[TABLE]";
    forumExport << "[TR][TD]Level[/TD][TD]Class[/TD][TD]Feats[/TD][/TR]\r\n";
    for (size_t level = 0; level < m_pBuild->Level(); ++level)
    {
        forumExport << "[TR][TD]";
        const LevelTraining& levelData = m_pBuild->LevelData(level);
        std::vector<size_t> classLevels = m_pBuild->ClassLevels(level);
        std::string cn = levelData.HasClass() ? levelData.Class() : Class_Unknown;
        const Class& c = FindClass(cn);
        CString className;
        className.Format("%s(%d)",
                cn.c_str(),
                levelData.HasClass() ? classLevels[c.Index()] : 0);
        forumExport << (level + 1) << "[/TD][TD]" << className << "[/TD]";
        // now add the automatic feats
        const std::list<TrainedFeat>& feats = levelData.AutomaticFeats();
        if (feats.size() > 0)
        {
            std::list<TrainedFeat>::const_iterator it = feats.begin();
            bool bFirst = true;
            while (it != feats.end())
            {
                if (!bFirst)
                {
                    forumExport << "[TR][TD][/TD][TD][/TD]";
                }
                forumExport << "[TD]" << (*it).FeatName() << "[/TD]";
                forumExport << "[/TR]\r\n";
                ++it;
                bFirst = false;
            }
        }
        else
        {
            forumExport << "[TD][/TD][/TR]\r\n";
        }
    }
    forumExport << "[/TABLE]\r\n";
}

void CForumExportDlg::AddConsolidatedFeats(std::stringstream& forumExport)
{
    forumExport << "Class and Feat Selection (Consolidated)\r\n";
    forumExport << "[TABLE]";
    forumExport << "[TR][TD]Level[/TD][TD]Class[/TD][TD]Feats[/TD][/TR]\r\n";
    for (size_t level = 0; level < m_pBuild->Level(); ++level)
    {
        forumExport << "[TR][TD]";
        bool bFirst = true;
        const LevelTraining& levelData = m_pBuild->LevelData(level);
        std::vector<size_t> classLevels = m_pBuild->ClassLevels(level);
        std::string cn = levelData.HasClass() ? levelData.Class() : Class_Unknown;
        const Class& c = FindClass(cn);
        CString className;
        className.Format("%s(%d)",
                cn.c_str(),
                levelData.HasClass() ? classLevels[c.Index()] : 0);
        forumExport << (level + 1) << "[/TD][TD]" << className << "[/TD]";
        if (level == 0)
        {
            std::string expectedClass = levelData.HasClass() ? levelData.Class() : Class_Unknown;
            const Race& race = FindRace(m_pBuild->Race());
            bool requiresHeartOfWood = race.HasIconicClass() ? race.IconicClass() != expectedClass : false;
            if (requiresHeartOfWood)
            {
                if (c.HasBaseClass() && c.BaseClass() == race.IconicClass())
                {
                    forumExport << "[TD][COLOR=rgb(184, 49, 47)]Requires a Lesser Reincarnation to switch from Iconic class to Archetype class[/COLOR][/TD]\r\n";
                }
                else
                {
                    forumExport << "[TD][COLOR=rgb(184, 49, 47)]Requires a +1 Heart of Wood to switch out of Iconic Class[/COLOR][/TD]\r\n";
                }
                bFirst = false;
            }
        }
        // now add the trainable feat types and their selections
        std::vector<FeatSlot> trainable = m_pBuild->TrainableFeatTypeAtLevel(level);
        if (trainable.size() > 0)
        {
            for (size_t tft = 0; tft < trainable.size(); ++tft)
            {
                if (!bFirst)
                {
                    forumExport << "[TR][TD][/TD][TD][/TD]";
                }
                CString label = "[TD][COLOR=rgb(65, 168, 95)]";
                label += trainable[tft].FeatType().c_str();
                label += ": [/COLOR]";
                TrainedFeat tf = m_pBuild->GetTrainedFeat(
                    level,
                    trainable[tft].FeatType());
                label += "[COLOR=rgb(184, 49, 47)]";
                if (tf.FeatName().empty())
                {
                    label += "Empty Feat Slot";
                }
                else
                {
                    label += tf.FeatName().c_str();
                    if (tf.HasFeatSwapWarning())
                    {
                        label += "[/COLOR][COLOR=rgb(147, 101, 184)] (Requires Feat Swap with Fred)[/COLOR][COLOR=rgb(184, 49, 47)]";
                    }
                    if (tf.HasAlternateFeatName())
                    {
                        label += "[/COLOR][COLOR=rgb(250, 197, 28)] Alternate: [/COLOR][COLOR=rgb(184, 49, 47)]";
                        label += tf.AlternateFeatName().c_str();
                    }
                }
                label += "[/COLOR][/TD][/TR]";
                forumExport << (LPCTSTR)label;
            }
            bFirst = false;
        }
        // also need to show ability adjustment on every 4th level
        AbilityType ability = m_pBuild->AbilityLevelUp(level + 1);
        if (ability != Ability_Unknown)
        {
            if (!bFirst)
            {
                forumExport << "[TR][TD][/TD][TD][/TD]";
            }
            forumExport << EnumEntryText(ability, abilityTypeMap);
            forumExport << "[TD][COLOR=rgb(250, 197, 28)]";
            forumExport << EnumEntryText(ability, abilityTypeMap);
            forumExport << ": +1 Level up[/COLOR][/TD][/TR]\r\n";
            bFirst = false;
        }
        // now add the automatic feats
        const std::list<TrainedFeat>& feats = levelData.AutomaticFeats();
        std::list<TrainedFeat>::const_iterator it = feats.begin();
        while (it != feats.end())
        {
            if (!bFirst)
            {
                forumExport << "[TR][TD][/TD][TD][/TD]";
            }
            forumExport << "[TD]Automatic: " << (*it).FeatName() << "[/TD]";
            forumExport << "[/TR]\r\n";
            ++it;
            bFirst = false;
        }
        if (bFirst)
        {
            forumExport << "[TD][/TD][/TR]\r\n";
        }
    }
    forumExport << "[/TABLE]\r\n";
}

void CForumExportDlg::AddActiveStances(std::stringstream& forumExport)
{
    CWnd* pWnd = AfxGetMainWnd();
    CMainFrame* pMainWnd = dynamic_cast<CMainFrame*>(pWnd);
    CStancesPane* pStancePane = dynamic_cast<CStancesPane*>(pMainWnd->GetPaneView(RUNTIME_CLASS(CStancesPane)));
    if (pStancePane != NULL)
    {
        forumExport << "Active Stances\r\n";
        forumExport << "[HR][/HR]\r\n";
        const std::list<StanceGroup*>& groups = pStancePane->Groups();
        for (auto&& git: groups)
        {
            size_t count = git->NumButtons();
            for (size_t i = 0; i < count; ++i)
            {
                const CStanceButton* pSB = git->GetStance(i);
                if (pSB->IsSelected())
                {
                    forumExport << git->GroupName() << ": ";
                    forumExport << pSB->Name() << "\r\n";
                }
            }
        }
        forumExport << "[HR][/HR]\r\n";
        forumExport << "\r\n";
    }
}

void CForumExportDlg::AddSelfAndPartyBuffs(std::stringstream& forumExport)
{
    UNREFERENCED_PARAMETER(forumExport);
    forumExport << "Self and Party Buffs\r\n";
    forumExport << "[HR][/HR]\r\n";
    const std::list<std::string>& buffs = m_pBuild->GetLife()->SelfAndPartyBuffs();
    std::list<std::string>::const_iterator it = buffs.begin();
    while (it != buffs.end())
    {
        forumExport << (*it) << "\r\n";
        ++it;
    }
    forumExport << "\r\n";
}

void CForumExportDlg::AddSkills(std::stringstream& forumExport)
{
    // Example Output: at level 20
    // Skills
    // -----------------------------------------------------------------------------------------------
    // Skill Points     18  9  9  9 10 10 10 10 10 10 11 11 11 11 11 12 12 12 12 12
    // Skill Name       01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20 Total Tome Buffed
    // ----------------------------------------------------------------------------------------------
    // Disable Device    4  1        3                                              xxx      x xxx
    // Use Magic Device                                                             xxx        xxx
    // -----------------------------------------------------------------------------------------
    // Available Points 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
    size_t maxHeroicLevel = min(m_pBuild->Level(), MAX_CLASS_LEVEL);
    forumExport << "[code]\r\n";
    forumExport << "-----------------------------------";
    for (size_t li = 0; li < maxHeroicLevel; ++li)
    {
        forumExport << "---";
    }
    forumExport << "\r\nSkill Points    ";
    for (size_t level = 0; level < maxHeroicLevel; ++level)
    {
        const LevelTraining& levelData = m_pBuild->LevelData(level);
        forumExport.width(3);
        forumExport << std::right << levelData.SkillPointsAvailable();
    }
    forumExport << "\r\n";
    forumExport << "Skill Name       ";
    for (size_t li = 0; li < maxHeroicLevel; ++li)
    {
        forumExport.fill('0');
        forumExport.width(2);
        forumExport << std::right;
        forumExport << (li + 1);
        forumExport << " ";
    }
    forumExport << " Ranks Tome Buffed\r\n";
    forumExport << "-----------------------------------";
    for (size_t li = 0; li < maxHeroicLevel; ++li)
    {
        forumExport << "---";
    }
    forumExport << "\r\n";
    for (size_t skill = Skill_Unknown + 1; skill < Skill_Count; ++skill)
    {
        CString skillName;
        skillName.Format("%-16s", (LPCTSTR)EnumEntryText((SkillType)skill, skillTypeMap));
        forumExport << skillName;
        // now add the number of skill points spent on this skill at each heroic level
        for (size_t level = 0; level < maxHeroicLevel; ++level)
        {
            const LevelTraining& levelData = m_pBuild->LevelData(level);
            const std::list<TrainedSkill>& ts = levelData.TrainedSkills();
            std::list<TrainedSkill>::const_iterator it = ts.begin();
            std::vector<size_t> skillRanks(Skill_Count, 0);
            while (it != ts.end())
            {
                skillRanks[(*it).Skill()]++;
                ++it;
            }
            CString text;
            if (skillRanks[skill] > 0)
            {
                std::string ct = m_pBuild->LevelData(level).HasClass()
                        ? m_pBuild->LevelData(level).Class()
                        : Class_Unknown;
                const Class& c = FindClass(ct);
                if (c.IsClassSkill((SkillType)skill))
                {
                    text.Format("%d", skillRanks[skill]);
                }
                else
                {
                    // its a cross class skill, show in multiples of ½
                    text = "";
                    int fullRanks = (skillRanks[skill] / 2);
                    if (fullRanks > 0)
                    {
                        text.Format("%d", fullRanks);
                    }
                    if (skillRanks[skill] % 2 != 0)
                    {
                        text += "½";
                    }
                }
                forumExport.fill(' ');
                forumExport.width(3);
                forumExport << std::right << text;
            }
            else
            {
                // no trained ranks at this level, blank field
                forumExport << "   ";
            }
        }
        double total = m_pBuild->SkillAtLevel((SkillType)skill, m_pBuild->Level()-1, false);
        forumExport.precision(1);          // max 1 dp
        forumExport.fill(' ');
        forumExport.width(7);              // xxx.y
        forumExport << std::fixed << std::right << total;
        // show the tome value
        size_t tome = m_pBuild->SkillTomeValue((SkillType)skill, m_pBuild->Level()-1);
        forumExport.width(5);              // .....x
        forumExport << std::fixed << std::right << tome;
        // now add the total for this skill
        BreakdownType bt = SkillToBreakdown((SkillType)skill);
        BreakdownItem* pBI = FindBreakdown(bt);
        forumExport.width(7);              // xxx.y
        forumExport << std::fixed << std::right << pBI->Total();
        forumExport << "\r\n";     // end of this skill line
    }
    forumExport << "-----------------------------------";
    for (size_t li = 0; li < maxHeroicLevel; ++li)
    {
        forumExport << "---";
    }
    forumExport << "\r\n";
    // show how many available skill points there are to spend
    forumExport << "Available Points";
    for (size_t level = 0; level < maxHeroicLevel; ++level)
    {
        const LevelTraining& levelData = m_pBuild->LevelData(level);
        forumExport.fill(' ');
        forumExport.width(3);
        int available = levelData.SkillPointsAvailable();
        int spent = levelData.SkillPointsSpent();
        forumExport << std::right << (available - spent);
    }
    forumExport << "\r\n";
    forumExport << "-----------------------------------";
    for (size_t li = 0; li < maxHeroicLevel; ++li)
    {
        forumExport << "---";
    }
    forumExport << "\r\n";
    forumExport << "[/code]\r\n";
    // blank line after
    forumExport << "\r\n";
}

void CForumExportDlg::AddSkillsAtLevel(size_t level, std::stringstream& forumExport)
{
    // get the ranks spent in each skill for this level
    const LevelTraining& levelData = m_pBuild->LevelData(level);
    std::string cn = levelData.HasClass() ? levelData.Class() : Class_Unknown;
    const Class& c = FindClass(cn);
    const std::list<TrainedSkill>& ts = levelData.TrainedSkills();
    std::list<TrainedSkill>::const_iterator it = ts.begin();
    std::vector<size_t> skillRanks(Skill_Count, 0);
    while (it != ts.end())
    {
        skillRanks[(*it).Skill()]++;
        ++it;
    }
    // add the class skills
    size_t numAdded = 0;
    for (size_t skill = Skill_Unknown + 1; skill < Skill_Count; ++skill)
    {
        if (skillRanks[skill] > 0
              && c.IsClassSkill((SkillType)skill))
        {
            if (numAdded > 0)
            {
                forumExport << ", ";
            }
            else
            {
                forumExport << "                       Class Skills: ";
            }
            forumExport << EnumEntryText((SkillType)skill, skillTypeMap)
                    << "(" << skillRanks[skill] << ")";
            ++numAdded;
        }
    }
    if (numAdded > 0)
    {
        forumExport << "\r\n";
    }
    // add the cross class skills
    numAdded = 0;
    for (size_t skill = Skill_Unknown + 1; skill < Skill_Count; ++skill)
    {
        if (skillRanks[skill] > 0
              && !c.IsClassSkill((SkillType)skill))
        {
            if (numAdded > 0)
            {
                forumExport << ", ";
            }
            else
            {
                forumExport << "                       Cross Class Skills: ";
            }
            forumExport << EnumEntryText((SkillType)skill, skillTypeMap)
                    << "(" << skillRanks[skill] << ")";
            ++numAdded;
        }
    }
    if (numAdded > 0)
    {
        forumExport << "\r\n";
    }
}

void CForumExportDlg::AddEnergyResistances(std::stringstream& forumExport)
{
    // Energy Resistance/Absorbance
    // Energy       Resistance and Absorbance
    // --------------------------------------
    // Acid:             [...]         [...]%
    // Cold:             [...]         [...]%
    // Electric:         [...]         [...]%
    // Fire:             [...]         [...]%
    // Sonic:            [...]         [...]%
    // Light:            [...]         [...]%
    // Negative:         [...]         [...]%
    // Poison:           [...]         [...]%
    // etc
    forumExport << "[TABLE]";
    forumExport << "[TR][TD]Energy[/TD][TD]Resistance[/TD][TD]Absorbance[/TD][/TR]\r\n";
    AddEnergyResistances(forumExport, "Acid:", Breakdown_EnergyResistanceAcid, Breakdown_EnergyAbsorptionAcid);
    AddEnergyResistances(forumExport, "Chaos:", Breakdown_EnergyResistanceChaos, Breakdown_EnergyAbsorptionChaos);
    AddEnergyResistances(forumExport, "Cold:", Breakdown_EnergyResistanceCold, Breakdown_EnergyAbsorptionCold);
    AddEnergyResistances(forumExport, "Electric:", Breakdown_EnergyResistanceElectric, Breakdown_EnergyAbsorptionElectric);
    AddEnergyResistances(forumExport, "Evil:", Breakdown_EnergyResistanceEvil, Breakdown_EnergyAbsorptionEvil);
    AddEnergyResistances(forumExport, "Fire:", Breakdown_EnergyResistanceFire, Breakdown_EnergyAbsorptionFire);
    AddEnergyResistances(forumExport, "Force:", Breakdown_EnergyResistanceForce, Breakdown_EnergyAbsorptionForce);
    AddEnergyResistances(forumExport, "Good:", Breakdown_EnergyResistanceGood, Breakdown_EnergyAbsorptionGood);
    AddEnergyResistances(forumExport, "Lawful:", Breakdown_EnergyResistanceLawful, Breakdown_EnergyAbsorptionLawful);
    AddEnergyResistances(forumExport, "Light:", Breakdown_EnergyResistanceLight, Breakdown_EnergyAbsorptionLight);
    AddEnergyResistances(forumExport, "Negative:", Breakdown_EnergyResistanceNegative, Breakdown_EnergyAbsorptionNegative);
    AddEnergyResistances(forumExport, "Poison:", Breakdown_EnergyResistancePoison, Breakdown_EnergyAbsorptionPoison);
    //AddEnergyResistances(forumExport, "Positive:", Breakdown_EnergyResistancePositive, Breakdown_EnergyAbsorptionPositive);
    //AddEnergyResistances(forumExport, "Repair:", Breakdown_EnergyResistanceRepair, Breakdown_EnergyAbsorptionRepair);
    //AddEnergyResistances(forumExport, "Rust:", Breakdown_EnergyResistanceRust, Breakdown_EnergyAbsorptionRust);
    AddEnergyResistances(forumExport, "Sonic:", Breakdown_EnergyResistanceSonic, Breakdown_EnergyAbsorptionSonic);
    forumExport << "[/TABLE]\r\n";
}

void CForumExportDlg::AddEnergyResistances(
        std::stringstream& forumExport,
        const std::string& name,
        BreakdownType bt1,
        BreakdownType bt2)
{
    forumExport << "[TR][TD]";
    forumExport << name << "[/TD][TD]";
    BreakdownItem* pB1 = FindBreakdown(bt1);
    forumExport << (int)pB1->Total() << "[/TD][TD]";
    BreakdownItem* pB2 = FindBreakdown(bt2);
    forumExport << (int)pB2->Total() << "%[/TD][/TR]\r\n";
}

void CForumExportDlg::AddEnhancements(std::stringstream& forumExport)
{
    forumExport << "[COLOR=rgb(184, 49, 47)][SIZE=6]Enhancements: 80 APs";
    if (m_pBuild->BonusRacialActionPoints() > 0)
    {
        forumExport << ", Racial ";
        forumExport.width(1);
        forumExport << m_pBuild->BonusRacialActionPoints();
    }
    if (m_pBuild->BonusUniversalActionPoints() > 0)
    {
        forumExport << ", Universal ";
        forumExport.width(1);
        forumExport << m_pBuild->BonusUniversalActionPoints();
    }
    forumExport << "[/SIZE][/COLOR]\r\n";
    forumExport << "[HR][/HR]\r\n";
    const Enhancement_SelectedTrees& trees = m_pBuild->EnhancementSelectedTrees();
    for (size_t ti = 0; ti < MAX_ENHANCEMENT_TREES; ++ti)
    {
        const std::string& treeName = trees.Tree(ti);
        if (!Enhancement_SelectedTrees::IsNotSelected(treeName))
        {
            // this is a tree we have to list
            SpendInTree* treeSpend = m_pBuild->FindSpendInTree(treeName);
            if (treeSpend != NULL
                  && treeSpend->Spent() > 0)
            {
                // tree can be selected and have no enhancements trained, thus not be present
                AddEnhancementTree(forumExport,*treeSpend);
            }
        }
    }
}

void CForumExportDlg::AddEpicDestinyTree(
        std::stringstream& forumExport)
{
    int permanentDestinyPoints = 0;
    BreakdownItem* pBD = FindBreakdown(Breakdown_DestinyPoints);
    if (pBD != NULL)
    {
        permanentDestinyPoints = static_cast<int>(pBD->Total());
    }
    forumExport << "[COLOR=rgb(184, 49, 47)][SIZE=6]";
    forumExport << "Epic Destinies: " << permanentDestinyPoints << " Destiny Points";
    forumExport << "[/SIZE][/COLOR]\r\n";
    forumExport << "[HR][/HR]\r\n";
    const Destiny_SelectedTrees& trees = m_pBuild->DestinySelectedTrees();
    for (size_t ti = 0; ti < MAX_DESTINY_TREES; ++ti)
    {
        const std::string& treeName = trees.Tree(ti);
        if (!Destiny_SelectedTrees::IsNotSelected(treeName))
        {
            // this is a tree we have to list
            SpendInTree* treeSpend = m_pBuild->FindSpendInTree(treeName);
            if (treeSpend != NULL
                  && treeSpend->Spent() > 0)
            {
                // tree can be selected and have no enhancements trained, thus not be present
                AddEpicDestinyTree(forumExport,*treeSpend);
            }
        }
    }
}

void CForumExportDlg::AddReaperTrees(std::stringstream& forumExport)
{
    forumExport << "[COLOR=rgb(184, 49, 47)][SIZE=6]";
    forumExport << "Reaper trees:";
    forumExport << "[/SIZE][/COLOR]\r\n";
    // add each of the 3 Reaper Trees
    SpendInTree* treeSpend = m_pBuild->FindSpendInTree("Dread Adversary");
    if (treeSpend != NULL)
    {
        // tree can be selected and have no enhancements trained, thus not be present
        AddReaperTree(forumExport,*treeSpend);
    }
    treeSpend = m_pBuild->FindSpendInTree("Dire Thaumaturge");
    if (treeSpend != NULL)
    {
        // tree can be selected and have no enhancements trained, thus not be present
        AddReaperTree(forumExport,*treeSpend);
    }
    treeSpend = m_pBuild->FindSpendInTree("Grim Barricade");
    if (treeSpend != NULL)
    {
        // tree can be selected and have no enhancements trained, thus not be present
        AddReaperTree(forumExport,*treeSpend);
    }
}

void CForumExportDlg::AddEnhancementTree(
        std::stringstream& forumExport,
        const SpendInTree& treeSpend)
{
    // TreeName: <name> - Points spent : xxx
    // <List of enhancements by display name>
    forumExport << "[COLOR=rgb(65, 168, 95)][SIZE=5]";
    forumExport << treeSpend.TreeName() << " - Points spent: " << treeSpend.Spent() << "[/SIZE][/COLOR]\r\n";
    const std::list<TrainedEnhancement>& enhancements = treeSpend.Enhancements();

    // output each enhancement by buy index
    std::list<TrainedEnhancement>::const_iterator it = enhancements.begin();
    while (it != enhancements.end())
    {
        const EnhancementTreeItem* item = FindEnhancement((*it).EnhancementName());
        if (item != NULL)
        {
            // show the tier of the enhancement
            switch (item->YPosition())
            {
            case 0: forumExport << "Core" << item->XPosition()+1 << " "; break;
            case 1: forumExport << "Tier1 "; break;
            case 2: forumExport << "Tier2 "; break;
            case 3: forumExport << "Tier3 "; break;
            case 4: forumExport << "Tier4 "; break;
            case 5: forumExport << "Tier5 "; break;
            case 6: forumExport << "Tier6 "; break;
            }
            // remove "Treename: " from the output for every individual enhancement
            std::string name = item->DisplayName((*it).HasSelection() ? (*it).Selection() : "");
            if (name.find(':') != std::string::npos)
            {
                name = name.substr(name.find(':') + 2);
            }
            forumExport << name;
            if (item->Ranks(name) > 1)
            {
                forumExport << " - " << (*it).Ranks() << " Ranks";
            }
            forumExport << "\r\n";
        }
        else
        {
            forumExport << "Error - Unknown enhancement\r\n";
        }
        ++it;
    }
    forumExport << "[HR][/HR]\r\n";
}

void CForumExportDlg::AddEpicDestinyTree(
        std::stringstream& forumExport,
        const SpendInTree& treeSpend)
{
    // TreeName: <name> - Points spent : xxx
    // <List of enhancements by display name>
    forumExport << "[COLOR=rgb(65, 168, 95)][SIZE=5]";
    forumExport << treeSpend.TreeName() << " - Points spent: " << treeSpend.Spent() << "[/SIZE][/COLOR]\r\n";
    const std::list<TrainedEnhancement>& enhancements = treeSpend.Enhancements();

    std::list<TrainedEnhancement>::const_iterator it = enhancements.begin();
    while (it != enhancements.end())
    {
        const EnhancementTreeItem* item = FindEnhancement((*it).EnhancementName());
        if (item != NULL)
        {
            // show the tier of the enhancement
            switch (item->YPosition())
            {
            case 0: forumExport << "Core" << item->XPosition()+1 << " "; break;
            case 1: forumExport << "Tier1 "; break;
            case 2: forumExport << "Tier2 "; break;
            case 3: forumExport << "Tier3 "; break;
            case 4: forumExport << "Tier4 "; break;
            case 5: forumExport << "Tier5 "; break;
            case 6: forumExport << "Tier6 "; break;
            }
            // remove "Treename: " from the output for every individual enhancement
            std::string name = item->DisplayName((*it).HasSelection() ? (*it).Selection() : "");
            if (name.find(':') != std::string::npos)
            {
                name = name.substr(name.find(':') + 2);
            }
            forumExport << name;
            if (item->Ranks(name) > 1)
            {
                forumExport << " - " << (*it).Ranks() << " Ranks";
            }
            forumExport << "\r\n";
        }
        else
        {
            forumExport << "Error - Unknown enhancement\r\n";
        }
        ++it;
    }
    forumExport << "[HR][/HR]\r\n";
}

void CForumExportDlg::AddReaperTree(
        std::stringstream& forumExport,
        const SpendInTree& treeSpend)
{
    // TreeName: <name> - Points spent : xxx
    // <List of enhancements by display name>
    forumExport << "[COLOR=rgb(65, 168, 95)][SIZE=5]";
    forumExport << treeSpend.TreeName() << " - Points spent: " << treeSpend.Spent() << "[/SIZE][/COLOR]\r\n";
    const std::list<TrainedEnhancement>& enhancements = treeSpend.Enhancements();

    std::list<TrainedEnhancement>::const_iterator it = enhancements.begin();
    while (it != enhancements.end())
    {
        const EnhancementTreeItem* item = FindEnhancement((*it).EnhancementName());
        if (item != NULL)
        {
            // show the tier of the enhancement
            switch (item->YPosition())
            {
            case 0: forumExport << "Core" << item->XPosition()+1 << " "; break;
            case 1: forumExport << "Tier1 "; break;
            case 2: forumExport << "Tier2 "; break;
            case 3: forumExport << "Tier3 "; break;
            case 4: forumExport << "Tier4 "; break;
            case 5: forumExport << "Tier5 "; break;
            case 6: forumExport << "Tier6 "; break;
            }
            // remove "Treename: " from the output for every individual enhancement
            std::string name = item->DisplayName((*it).HasSelection() ? (*it).Selection() : "");
            forumExport << name;
            if (item->Ranks(name) > 1)
            {
                forumExport << " - " << item->Ranks(name) << " Ranks";
            }
            forumExport << "\r\n";
        }
        else
        {
            forumExport << "Error - Unknown enhancement\r\n";
        }
        ++it;
    }
    forumExport << "[HR][/HR]\r\n";
    forumExport << "\r\n";
}

void CForumExportDlg::AddSpellPowers(std::stringstream& forumExport)
{
    forumExport << "[SIZE=3][TABLE]\r\n";
    forumExport << "[TR][TD][COLOR=rgb(65, 168, 95)]Spell Power[/COLOR][/TD][TD][COLOR=rgb(65, 168, 95)]Base[/COLOR][/TD][TD][COLOR=rgb(65, 168, 95)]Critical Chance[/COLOR][/TD][TD][COLOR=rgb(65, 168, 95)]Critical Multiplier[/COLOR][/TD][/COLOR][/TR]\r\n";
    AddSpellPowerToTable(forumExport, "Acid           ", Breakdown_SpellPowerAcid, Breakdown_SpellCriticalChanceAcid, Breakdown_SpellCriticalMultiplierAcid);
    AddSpellPowerToTable(forumExport, "Light/Alignment", Breakdown_SpellPowerLightAlignment, Breakdown_SpellCriticalChanceLightAlignment, Breakdown_SpellCriticalMultiplierLightAlignment);
    AddSpellPowerToTable(forumExport, "Chaos          ", Breakdown_SpellPowerChaos, Breakdown_SpellCriticalChanceChaos, Breakdown_SpellCriticalMultiplierChaos);
    AddSpellPowerToTable(forumExport, "Cold           ", Breakdown_SpellPowerCold, Breakdown_SpellCriticalChanceCold, Breakdown_SpellCriticalMultiplierCold);
    AddSpellPowerToTable(forumExport, "Electric       ", Breakdown_SpellPowerElectric, Breakdown_SpellCriticalChanceElectric, Breakdown_SpellCriticalMultiplierElectric);
    AddSpellPowerToTable(forumExport, "Evil           ", Breakdown_SpellPowerEvil, Breakdown_SpellCriticalChanceEvil, Breakdown_SpellCriticalMultiplierEvil);
    AddSpellPowerToTable(forumExport, "Fire           ", Breakdown_SpellPowerFire, Breakdown_SpellCriticalChanceFire, Breakdown_SpellCriticalMultiplierFire);
    AddSpellPowerToTable(forumExport, "Force/Untyped  ", Breakdown_SpellPowerForce, Breakdown_SpellCriticalChanceForce, Breakdown_SpellCriticalMultiplierForce);
    AddSpellPowerToTable(forumExport, "Negative       ", Breakdown_SpellPowerNegative, Breakdown_SpellCriticalChanceNegative, Breakdown_SpellCriticalMultiplierNegative);
    AddSpellPowerToTable(forumExport, "Physical       ", Breakdown_SpellPowerPhysical, Breakdown_SpellCriticalChancePhysical, Breakdown_SpellCriticalMultiplierPhysical);
    AddSpellPowerToTable(forumExport, "Poison         ", Breakdown_SpellPowerPoison, Breakdown_SpellCriticalChancePoison, Breakdown_SpellCriticalMultiplierPoison);
    AddSpellPowerToTable(forumExport, "Positive       ", Breakdown_SpellPowerPositive, Breakdown_SpellCriticalChancePositive, Breakdown_SpellCriticalMultiplierPositive);
    AddSpellPowerToTable(forumExport, "Repair         ", Breakdown_SpellPowerRepair, Breakdown_SpellCriticalChanceRepair, Breakdown_SpellCriticalMultiplierRepair);
    AddSpellPowerToTable(forumExport, "Rust           ", Breakdown_SpellPowerRust, Breakdown_SpellCriticalChanceRust, Breakdown_SpellCriticalMultiplierRust);
    AddSpellPowerToTable(forumExport, "Sonic          ", Breakdown_SpellPowerSonic, Breakdown_SpellCriticalChanceSonic, Breakdown_SpellCriticalMultiplierSonic);
    AddSpellPowerToTable(forumExport, "Untyped        ", Breakdown_SpellPowerUntyped, Breakdown_SpellCriticalChanceUntyped, Breakdown_SpellCriticalMultiplierUntyped);
    forumExport << "[/TABLE][/SIZE]\r\n";
}

void CForumExportDlg::AddSpellPower(
        std::stringstream& forumExport,
        const std::string& label,
        BreakdownType btPower,
        BreakdownType btCrit,
        BreakdownType btMult)
{
    forumExport << label;
    // spell power
    forumExport << "          ";
    BreakdownItem* pBPower = FindBreakdown(btPower);
    forumExport.width(4);
    forumExport << std::right << (int)pBPower->Total();
    // spell critical chance
    forumExport << "     ";
    BreakdownItem* pBCrit = FindBreakdown(btCrit);
    forumExport.width(3);
    forumExport << std::right << (int)pBCrit->Total();
    forumExport << "%";
    // multiplier
    forumExport << "               ";
    BreakdownItem* pBMult = FindBreakdown(btMult);
    forumExport.width(3);
    forumExport << std::right << (int)pBMult->Total();
    forumExport << "\r\n";
}

void CForumExportDlg::AddSpellPowerToTable(
    std::stringstream& forumExport,
    const std::string& label,
    BreakdownType btPower,
    BreakdownType btCrit,
    BreakdownType btMult)
{
    forumExport << "[TR][TD]" << label << "[/TD]";
    // spell power
    BreakdownItem* pBPower = FindBreakdown(btPower);
    forumExport << "[TD]" << pBPower->Total() << "[/TD]";
    // spell critical chance
    BreakdownItem* pBCrit = FindBreakdown(btCrit);
    forumExport << "[TD]" << (int)pBCrit->Total() << "%[/TD]";
    // multiplier
    BreakdownItem* pBMult = FindBreakdown(btMult);
    forumExport << "[TD]" << (int)pBMult->Total() << "[/TD][/TR]\r\n";
}

void CForumExportDlg::AddSpells(std::stringstream& forumExport)
{
    // check each possible class for selected spells
    std::vector<size_t> classLevels = m_pBuild->ClassLevels(m_pBuild->Level()-1);
    for (size_t ci = 0; ci < classLevels.size(); ++ci)
    {
        if (classLevels[ci] > 0)
        {
            const Class& c = ClassFromIndex(ci);
            std::vector<size_t> spellSlots = c.SpellSlotsAtLevel(classLevels[ci]);
            // must have at least 1 spell slot at any level to display the
            // required spells for this class
            size_t slotCount = std::accumulate(spellSlots.begin(), spellSlots.end(), 0);
            if (slotCount > 0)
            {
                forumExport << c.Name().c_str() << " Spells\r\n";
                forumExport << "[SIZE=3][TABLE]\r\n";
                forumExport << "[TR]"
                    "[TD][COLOR=rgb(65, 168, 95)]Level[/COLOR][/TD]"
                    "[TD][COLOR=rgb(65, 168, 95)]Spell Name[/COLOR][/TD]"
                    "[TD][COLOR=rgb(65, 168, 95)]School[/COLOR][/TD]"
                    "[TD][COLOR=rgb(65, 168, 95)]CL/MCL[/COLOR][/TD]"
                    "[TD][COLOR=rgb(65, 168, 95)]DC[/COLOR][/TD]"
                    "[TD][COLOR=rgb(65, 168, 95)]Average Damage[/COLOR][/TD]"
                    "[TD][COLOR=rgb(65, 168, 95)]Critical Damage[/COLOR][/TD]"
                    "[/TR]\r\n";
                for (size_t spellLevel = 0; spellLevel < spellSlots.size(); ++spellLevel)
                {
                    // now output each fixed spell
                    std::list<TrainedSpell> fixedSpells = m_pBuild->FixedSpells(
                            c.Name(), spellLevel); // 0 based
                    AddSpellList(
                            forumExport,
                            fixedSpells,
                            spellLevel);
                    // now output each selected spell
                    std::list<TrainedSpell> trainedSpells = m_pBuild->TrainedSpells(
                            c.Name(), spellLevel + 1); // 1 based
                    AddSpellList(
                            forumExport,
                            trainedSpells,
                            spellLevel);
                }
                forumExport << "[/TABLE][/SIZE]\r\n";
            }
        }
    }
}

void CForumExportDlg::AddSpellList(
        std::stringstream& forumExport,
        const std::list<TrainedSpell>& spellList,
        size_t spellLevel) const
{
    std::list<TrainedSpell>::const_iterator it = spellList.begin();
    while (it != spellList.end())
    {
        // spell level
        forumExport << "[TR][TD]" << (spellLevel + 1) << "[/TD]";
        // spell name
        forumExport << "[TD]" << (*it).SpellName() << "[/TD]";
        // spell school
        Spell spell = FindSpellByName((*it).SpellName());
        spell.SetClass((*it).Class());
        forumExport << "[TD]";
        bool bFirst = true;
        for (auto&& sit: spell.School())
        {
            CString school;
            school = EnumEntryText(sit, spellSchoolTypeMap);
            if (!bFirst)
            {
                forumExport << ", ";
            }
            bFirst = false;
            forumExport << school;
        }
        forumExport << "[/TD]";
        size_t cl = 0;
        size_t mcl = 0;
        if (spell.SpellDamageEffects().size() > 0)
        {
            cl = spell.ActualCasterLevel(*m_pBuild);
            mcl = spell.ActualMaxCasterLevel(*m_pBuild);
            // CL and MCL
            if (mcl > 0)
            {
                forumExport << "[TD]" << cl << "/" << mcl << "[/TD]";
            }
            else
            {
                forumExport << "[TD]" << cl << "[/TD]";
            }
        }
        else
        {
            forumExport << "[TD]-[/TD]";
        }
        // spell DC
        size_t spellDC = spell.DC(*m_pBuild);
        if (spellDC != 0)
        {
            forumExport << "[TD]" << spellDC << "[/TD]";
        }
        else
        {
            forumExport << "[TD]-[/TD]";
        }
        if (spell.SpellDamageEffects().size() > 0)
        {
            // average damage
            forumExport << "[TD]" << spell.SpellDamageEffects().front().AverageDamageText(cl) << "[/TD]";
            // critical damage
            forumExport << "[TD]" << spell.SpellDamageEffects().front().CriticalDamageText(cl) << "[/TD]";
        }
        else
        {
            forumExport << "[TD]-[/TD]";
            forumExport << "[TD]-[/TD]";
        }
        forumExport << "[/TR]\r\n";
        ++it;
    }
}

void CForumExportDlg::AddSLAs(std::stringstream& forumExport)
{
    bool first = true;
    // find the SLA control view and ask it for the list of SLAs
    CWnd* pWnd = AfxGetMainWnd();
    CMainFrame* pMainWnd = dynamic_cast<CMainFrame*>(pWnd);
    CSpellsPane* pSpellsPane = dynamic_cast<CSpellsPane*>(pMainWnd->GetPaneView(RUNTIME_CLASS(CSpellsPane)));
    if (pSpellsPane != NULL)
    {
        CSpellLikeAbilityPage* pSLAPage = pSpellsPane->GetSLAPage();
        const std::list<SLA>& slas = pSLAPage->Control()->SLAs();
        std::list<SLA>::const_iterator it = slas.begin();
        while (it != slas.end())
        {
            if (first)
            {
                forumExport << "Spell Like / Special Abilities\r\n";
                forumExport << "[HR][/HR]\r\n";
                first = false;
            }
            forumExport.width(44);
            forumExport << std::left << (*it).Name();
            forumExport << "\r\n";
            ++it;
        }
    }
    if (!first)
    {
        forumExport << "[HR][/HR]\r\n";
        forumExport << "\r\n";
    }
}

void CForumExportDlg::AddWeaponDamage(std::stringstream& forumExport)
{
    forumExport << "Weapon Damage\r\n";
    forumExport << "[HR][/HR]\r\n";
    BreakdownItem* pBI = FindBreakdown(Breakdown_MeleePower);
    forumExport << "Melee Power:  " << pBI->Total() << "\r\n";
    AddBreakdown(forumExport, "Doublestrike: ", 1, Breakdown_DoubleStrike);
    forumExport << "%\r\n";
    AddBreakdown(forumExport, "Strikethrough: ", 1, Breakdown_Strikethrough);
    forumExport << "%\r\n";
    AddBreakdown(forumExport, "Mainhand damage ability multiplier: ", 1, Breakdown_DamageAbilityMultiplier);
    forumExport << "\r\n";
    AddBreakdown(forumExport, "Offhand damage ability multiplier: ", 1, Breakdown_DamageAbilityMultiplierOffhand);
    forumExport << "\r\n";
    AddBreakdown(forumExport, "Off-Hand attack Chance: ", 1, Breakdown_OffHandAttackBonus);
    forumExport << "%\r\n";
    AddBreakdown(forumExport, "Fortification Bypass: ", 1, Breakdown_FortificationBypass);
    forumExport << "%\r\n";
    AddBreakdown(forumExport, "Dodge Bypass: ", 1, Breakdown_DodgeBypass);
    forumExport << "%\r\n";
    AddBreakdown(forumExport, "Helpless Damage bonus: ", 1, Breakdown_HelplessDamage);
    forumExport << "%\r\n";

    pBI = FindBreakdown(Breakdown_RangedPower);
    forumExport << "Ranged Power: " << pBI->Total();
    forumExport << "\r\n";
    AddBreakdown(forumExport, "Doubleshot Chance: ", 1, Breakdown_DoubleShot);
    forumExport << "%\r\n";
    forumExport << "\r\n";

    AddBreakdown(forumExport, "Sneak Attack Attack bonus: ", 1, Breakdown_SneakAttackAttack);
    forumExport << "\r\n";
    AddBreakdown(forumExport, "Sneak Attack Damage: ", 1, Breakdown_SneakAttackDice);
    forumExport << "d6+";
    AddBreakdown(forumExport, "", 1, Breakdown_SneakAttackDamage);
    forumExport << "\r\n";
    forumExport << "\r\n";

    EquippedGear gear = m_pBuild->ActiveGearSet();
    if (gear.HasItemInSlot(Inventory_Weapon1))
    {
        pBI = FindBreakdown(Breakdown_WeaponEffectHolder);
        if (pBI != NULL)
        {
            BreakdownItemWeaponEffects* pBIWE = dynamic_cast<BreakdownItemWeaponEffects*>(pBI);
            if (pBIWE != NULL)
            {
                pBIWE->AddForumExportData(forumExport);
            }
        }
    }
    forumExport << "\r\n";
}

void CForumExportDlg::AddTacticalDCs(std::stringstream& forumExport)
{
    CWnd* pWnd = AfxGetMainWnd();
    CMainFrame* pMainWnd = dynamic_cast<CMainFrame*>(pWnd);
    CDCPane* pDCPane = dynamic_cast<CDCPane*>(pMainWnd->GetPaneView(RUNTIME_CLASS(CDCPane)));
    if (pDCPane != NULL)
    {
        const std::vector<CDCButton*>& dcs = pDCPane->DCs();
        forumExport << "[SIZE=3][TABLE]";
        forumExport << "[TR][TD]Tactical DC[/TD][TD]Value[/TD][TD]Evaluation[/TD][/TR]\r\n";
        for (size_t i = 0; i < dcs.size(); ++i)
        {
            // this is an active stance
            const DC& dc = dcs[i]->GetDCItem();
            forumExport << "[TR][TD]" << dc.Name() << "[/TD][TD]";
            std::string dcText = dc.DCBreakdown(m_pBuild);
            dcText = ReplaceAll(dcText, " : ", "[/TD][TD]");
            forumExport << dcText;
            forumExport << "[/TD][/TR]\r\n";
        }
        forumExport << "[/TABLE][/SIZE]\r\n";
    }
}

void CForumExportDlg::AddGear(std::stringstream& forumExport)
{
    EquippedGear gear = m_pBuild->ActiveGearSet();
    ExportGear(gear, forumExport, false);
}

void CForumExportDlg::AddSimpleGear(std::stringstream& forumExport)
{
    EquippedGear gear = m_pBuild->ActiveGearSet();
    ExportGear(gear, forumExport, true);
}

void CForumExportDlg::ExportGear(
        const EquippedGear& gear,
        std::stringstream& forumExport,
        bool bSimple)
{
    forumExport << "[COLOR=rgb(184, 49, 47)][SIZE=6]";
    forumExport << "Equipped Gear Set: " << gear.Name() << "[/SIZE][/COLOR]\r\n";
    forumExport << "[SIZE=3][TABLE]";
    for (size_t gi = Inventory_Arrows; gi < Inventory_Count; ++gi)
    {
        if (gear.IsSlotRestricted((InventorySlotType)gi, m_pBuild))
        {
            forumExport << "[TR][TD]";
            forumExport << EnumEntryText((InventorySlotType)gi, InventorySlotTypeMap);
            forumExport << "[/TD][TD][COLOR=rgb(184, 49, 47)]Restricted by another item in this gear set[/COLOR][/TD][TD][/TD]\r\n";
        }
        else if (gear.HasItemInSlot((InventorySlotType)gi))
        {
            Item item = gear.ItemInSlot((InventorySlotType)gi);
            forumExport << "[TR][TD][COLOR=rgb(65,168,95)]";
            forumExport << EnumEntryText((InventorySlotType)gi, InventorySlotTypeMap);
            forumExport << "[/COLOR][/TD][TD][COLOR=rgb(250, 197, 28)]";
            forumExport << item.Name() << "[/COLOR][/TD][TD]";
            if (item.HasDropLocation())
            {
                forumExport << "Drops in: " << item.DropLocation();
            }
            forumExport << "[/TD]";
            forumExport << "[/TR]\r\n";
            if (!bSimple)
            {
                // show effect descriptions up to the first encountered ":" character
                std::vector<CString> eds = item.BuffDescriptions(m_pBuild);
                std::vector<CString>::const_iterator it = eds.begin();
                while (it != eds.end())
                {
                    std::string processedDescription = (*it);
                    processedDescription = processedDescription.substr(0, processedDescription.find(':'));
                    forumExport << "[TR][TD][/TD][TD]";
                    forumExport << processedDescription;
                    forumExport << "[/TD][TD][/TD][/TR]\r\n";
                    ++it;
                }
            }
            // show any augment slots also
            bool bSetBonusSuppressed = false;
            const std::vector<ItemAugment>& augments = item.Augments();
            for (size_t i = 0; i < augments.size(); ++i)
            {
                if (augments[i].HasSelectedAugment())
                {
                    forumExport << "[TR][TD][/TD][TD]";
                    forumExport << augments[i].Type();
                    forumExport << ": ";
                    // may be a configurable augment
                    if (augments[i].HasValue())
                    {
                        CString text;
                        text.Format("%+.0f ", augments[i].Value());
                        forumExport << (LPCTSTR)text;
                    }
                    forumExport << augments[i].SelectedAugment();
                    const Augment& augment = FindAugmentByName(augments[i].SelectedAugment(), &item);
                    if (augment.HasLevels())
                    {
                        int value = static_cast<int>(augment.LevelValue()[augments[i].SelectedLevelIndex()]);
                        forumExport << " +" << value;
                    }
                    bSetBonusSuppressed |= augment.HasSuppressSetBonus();
                    forumExport << "[/TD][TD][/TD][/TR]\r\n";
                }
                else
                {
                    std::string type = augments[i].Type();
                    size_t mpos = type.find("Mythic");
                    size_t rpos = type.find("Reaper");
                    if (mpos != std::string::npos
                          && rpos != std::string::npos)
                    {
                        forumExport << "[TR][TD][/TD][TD]";
                        forumExport << augments[i].Type();
                        forumExport << ": ";
                        forumExport << "[COLOR=rgb(250, 197, 28)]Empty augment slot[/COLOR]";
                        forumExport << "[/TD][TD][/TD][/TR]\r\n";
                    }
                }
            }
            // show any set bonuses (update name if suppressed)
            const std::list<std::string>& sets = item.SetBonus();
            std::list<std::string>::const_iterator sit = sets.begin();
            while (sit != sets.end())
            {
                const SetBonus& set = FindSetBonus((*sit));
                std::string processedDescription = set.Type();
                forumExport << "[TR][TD][/TD][TD][COLOR=rgb(65,168,95)]";
                if (bSetBonusSuppressed)
                {
                    forumExport << "[S]";
                }
                forumExport << processedDescription;
                if (bSetBonusSuppressed)
                {
                    forumExport << "[/S] (Suppressed)";
                }
                forumExport << "[/COLOR][/TD][TD][/TD][/TR]\r\n";
                ++sit;
            }
            // show artifact filigrees if this is the artifact
            if (item.HasMinorArtifact())
            {
                for (size_t fi = 0; fi < MAX_ARTIFACT_FILIGREE; ++fi)
                {
                    std::string filigree = gear.GetArtifactFiligree(fi);
                    if (filigree != "")
                    {
                        forumExport << "[TR][TD][/TD][TD]";
                        forumExport << "Filigree " << (fi + 1) << ": ";
                        forumExport << filigree;
                        if (gear.IsRareArtifactFiligree(fi))
                        {
                            forumExport << "(Rare)";
                        }
                        forumExport << "[/TD][TD][/TD][/TR]\r\n";
                    }
                }
            }
            if (gi == Inventory_Weapon1)
            {
                // now add the Filigree upgrades
                bool bHadGem = false;
                for (size_t fi = 0; fi < MAX_FILIGREE; ++fi)
                {
                    std::string filigree = gear.GetFiligree(fi);
                    if (filigree != "")
                    {
                        // add any sentient weapon Filigree to the list also
                        if (!bHadGem&& gear.HasPersonality())
                        {
                            forumExport << "[TR][TD][/TD][TD]";
                            forumExport << "Sentient Weapon Personality: ";
                            forumExport << gear.Personality();
                            forumExport << "[/TD][TD][/TD][/TR]\r\n";
                            bHadGem = true;
                        }
                        forumExport << "[TR][TD][/TD][TD]";
                        forumExport << "Filigree " << (fi + 1) << ": ";
                        forumExport << filigree;
                        if (gear.IsRareFiligree(fi))
                        {
                            forumExport << "(Rare)";
                        }
                        forumExport << "[/TD][TD][/TD][/TR]\r\n";
                    }
                }
            }
        }
    }
    forumExport << "[/TABLE][/SIZE]\r\n";
}

void CForumExportDlg::AddAlternateGear(std::stringstream& forumExport)
{
    // export all other gear layouts except the current one
    const std::list<EquippedGear>& setups = m_pBuild->GearSetups();
    std::list<EquippedGear>::const_iterator it = setups.begin();
    while (it != setups.end())
    {
        if ((*it).Name() != m_pBuild->ActiveGear())
        {
            ExportGear((*it), forumExport, true);
        }
        ++it;
    }
}

void CForumExportDlg::OnMoveUp()
{
    int sel = m_listConfigureExport.GetSelectionMark();
    if (sel > 0)
    {
        bool bOld = m_bShowSection[sel-1];
        m_bShowSection[sel-1] = m_bShowSection[sel];
        m_bShowSection[sel] = bOld;
        // move the option up also
        ForumExportSections old = m_SectionOrder[sel-1];
        m_SectionOrder[sel-1] = m_SectionOrder[sel];
        m_SectionOrder[sel] = old;
        m_listConfigureExport.SetItemState(
                sel,
                LVIS_SELECTED | LVIS_FOCUSED,
                LVIS_SELECTED | LVIS_FOCUSED);
        m_listConfigureExport.SetSelectionMark(sel-1);
        // update export and control list
        PopulateConfigureExportList();
        PopulateExport();
    }
}

void CForumExportDlg::OnMoveDown()
{
    int sel = m_listConfigureExport.GetSelectionMark();
    if (sel < FES_Count - 1)
    {
        bool bOld = m_bShowSection[sel+1];
        m_bShowSection[sel+1] = m_bShowSection[sel];
        m_bShowSection[sel] = bOld;
        // move the option up also
        ForumExportSections old = m_SectionOrder[sel+1];
        m_SectionOrder[sel+1] = m_SectionOrder[sel];
        m_SectionOrder[sel] = old;
        m_listConfigureExport.SetItemState(
                sel,
                LVIS_SELECTED | LVIS_FOCUSED,
                LVIS_SELECTED | LVIS_FOCUSED);
        m_listConfigureExport.SetSelectionMark(sel+1);
        // update export and control list
        PopulateConfigureExportList();
        PopulateExport();
    }
}

std::list<std::string> CForumExportDlg::GetLevelEntries(
    size_t level,
    bool bIncludeSkills)
{
    std::list<std::string> lines;
    if (level == 0)
    {
        const LevelTraining& levelData = m_pBuild->LevelData(level);
        std::string expectedClass = levelData.HasClass() ? levelData.Class() : Class_Unknown;
        const Class& c = FindClass(expectedClass);
        const Race& race = FindRace(m_pBuild->Race());
        bool requiresHeartOfWood = race.HasIconicClass() ? race.IconicClass() != expectedClass : false;
        if (requiresHeartOfWood)
        {
            if (c.HasBaseClass() && c.BaseClass() == race.IconicClass())
            {
                lines.push_back("[COLOR=rgb(184, 49, 47)]Requires a Lesser Reincarnation to switch from Iconic class to Archetype class[/COLOR]");
            }
            else
            {
                lines.push_back("[COLOR=rgb(184, 49, 47)]Requires a +1 Heart of Wood to switch out of Iconic Class[/COLOR]");
            }
        }
    }
    // now add the trainable feat types and their selections
    std::vector<FeatSlot> trainable = m_pBuild->TrainableFeatTypeAtLevel(level);
    if (trainable.size() > 0)
    {
        for (size_t tft = 0; tft < trainable.size(); ++tft)
        {
            CString label = "[COLOR=rgb(65, 168, 95)]";
            label += trainable[tft].FeatType().c_str();
            label += ": [/COLOR]";
            TrainedFeat tf = m_pBuild->GetTrainedFeat(
                level,
                trainable[tft].FeatType());
            label += "[COLOR=rgb(184, 49, 47)]";
            if (tf.FeatName().empty())
            {
                label += "Empty Feat Slot";
            }
            else
            {
                label += tf.FeatName().c_str();
                if (tf.HasFeatSwapWarning())
                {
                    label += "[/COLOR][COLOR=rgb(147, 101, 184)] (Requires Feat Swap with Fred)[/COLOR][COLOR=rgb(184, 49, 47)]";
                }
                if (tf.HasAlternateFeatName())
                {
                    label += "[/COLOR][COLOR=rgb(250, 197, 28)] Alternate: [/COLOR][COLOR=rgb(184, 49, 47)]";
                    label += tf.AlternateFeatName().c_str();
                }
            }
            label += "[/COLOR]";
            lines.push_back((LPCTSTR)label);
        }
    }
    // also need to show ability adjustment on every 4th level
    AbilityType ability = m_pBuild->AbilityLevelUp(level + 1);
    if (ability != Ability_Unknown)
    {
        std::stringstream ss;
        ss << "[COLOR=rgb(250, 197, 28)]";
        ss << EnumEntryText(ability, abilityTypeMap);
        ss << ": +1 Level up[/COLOR]";
        lines.push_back(ss.str());
    }
    if (bIncludeSkills)
    {
        // get the ranks spent in each skill for this level
        const LevelTraining& levelData = m_pBuild->LevelData(level);
        std::string cn = levelData.HasClass() ? levelData.Class() : Class_Unknown;
        const Class& c = FindClass(cn);
        const std::list<TrainedSkill>& ts = levelData.TrainedSkills();
        std::list<TrainedSkill>::const_iterator it = ts.begin();
        std::vector<size_t> skillRanks(Skill_Count, 0);
        while (it != ts.end())
        {
            skillRanks[(*it).Skill()]++;
            ++it;
        }
        // add the class skills
        size_t numAdded = 0;
        std::stringstream sscs;
        for (size_t skill = Skill_Unknown + 1; skill < Skill_Count; ++skill)
        {
            if (skillRanks[skill] > 0
              && c.IsClassSkill((SkillType)skill))
            {
                if (numAdded > 0)
                {
                    sscs << ", ";
                }
                else
                {
                    sscs << "Class Skills: ";
                }
                sscs << EnumEntryText((SkillType)skill, skillTypeMap)
                    << "(" << skillRanks[skill] << ")";
                ++numAdded;
            }
        }
        if (numAdded > 0)
        {
            lines.push_back(sscs.str());
        }
        // add the cross class skills
        numAdded = 0;
        std::stringstream ssccs;
        for (size_t skill = Skill_Unknown + 1; skill < Skill_Count; ++skill)
        {
            if (skillRanks[skill] > 0
              && !c.IsClassSkill((SkillType)skill))
            {
                if (numAdded > 0)
                {
                    ssccs << ", ";
                }
                else
                {
                    ssccs << "Cross Class Skills: ";
                }
                ssccs << EnumEntryText((SkillType)skill, skillTypeMap)
                    << "(" << skillRanks[skill] << ")";
                ++numAdded;
            }
        }
        if (numAdded > 0)
        {
            lines.push_back(ssccs.str());
        }
    }
    return lines;
}

void CForumExportDlg::OnSelchangeComboFormat()
{
    int sel = m_comboFormat.GetCurSel();
    if (sel != CB_ERR)
    {
        m_exportType = sel;
        // save the new default
        AfxGetApp()->WriteProfileInt("ForumExport", "ExportFormat", m_exportType);
        PopulateExport();
    }
}

void CForumExportDlg::ApplyFormatSelection(std::string* pContent)
{
    switch (m_exportType)
    {
        case 0: break;      // BB code export format, which it is already in
        case 1: {
                    std::string copy(*pContent);
                    ConvertToPlainText(copy);
                   *pContent = copy;
                }
                break;
    }
}

void CForumExportDlg::ConvertToPlainText(std::string& content)
{
    // first lose all the standard formatting we do not need
    content = ReplaceAll(content, "[font=courier]\r\n", "");
    content = ReplaceAll(content, "[/font]\r\n", "");
    content = ReplaceAll(content, "[code]\r\n", "");
    content = ReplaceAll(content, "[/code]\r\n", "");
    content = ReplaceAll(content, "[COLOR=rgb(184, 49, 47)]", "");
    content = ReplaceAll(content, "[COLOR=rgb(65, 168, 95)]", "");
    content = ReplaceAll(content, "[COLOR=rgb(250, 197, 28)]", "");
    content = ReplaceAll(content, "[COLOR=rgb(65,168,95)]", "");
    content = ReplaceAll(content, "[/COLOR]", "");
    content = ReplaceAll(content, "[SIZE=6]", "");
    content = ReplaceAll(content, "[SIZE=5]", "");
    content = ReplaceAll(content, "[SIZE=4]", "");
    content = ReplaceAll(content, "[SIZE=3]", "");
    content = ReplaceAll(content, "[/SIZE]", "");
    // now we need to convert any tables present
    std::string tableText = ExtractBlock(content, "[TABLE]", "[/TABLE]");
    while (tableText != "")
    {
        std::string newText = ConvertTable(tableText);
        content = ReplaceAll(content, tableText, newText);
        // intialise for next loop
        tableText = ExtractBlock(content, "[TABLE]", "[/TABLE]");
    }
}

std::string CForumExportDlg::ConvertTable(std::string tableText)
{
    // clear the table format codes
    tableText = ReplaceAll(tableText, "[TABLE]\r\n", "");
    tableText = ReplaceAll(tableText, "[TABLE]", "");
    tableText = ReplaceAll(tableText, "[/TABLE]", "");
    std::string rowText = ExtractBlock(tableText, "[TR]", "[/TR]");
    std::vector<TableData> rowData;
    while (rowText != "")
    {
        TableData data;
        ExtractTableRowData(rowText,&data);
        rowData.push_back(data);
        tableText = ReplaceAll(tableText, rowText, "");
        // intialise for next loop
        rowText = ExtractBlock(tableText, "[TR]", "[/TR]");
    }
    // make sure all rowData entries are the same size
    size_t numColumns = 0;
    for (auto&& it: rowData)
    {
        numColumns = max(numColumns, it.data.size());
    }
    for (auto&& it : rowData)
    {
        while (it.data.size() < numColumns)
        {
            it.data.push_back("");  // add an empty column
        }
    }
    // now generate the table text
    std::string table;
    if (rowData.size() > 0)
    {
        // measure the max width of each column
        std::vector<size_t> columnWidths;
        columnWidths.resize(rowData[0].data.size(), 0);
        for (auto&& it : rowData)
        {
            size_t columnIndex = 0;
            for (auto&& cit: it.data)
            {
                columnWidths[columnIndex] = max(columnWidths[columnIndex], cit.size());
                ++columnIndex;
            }
        }
        for (size_t i = 0; i < rowData.size(); ++i)
        {
            if (rowData[i].data[0].size() > 0)
            {
                AddHeader(table, columnWidths);
            }
            AddRow(table, rowData[i], columnWidths);
        }
        AddHeader(table, columnWidths);
    }
    return table;
}

void CForumExportDlg::ExtractTableRowData(std::string rowText, TableData* pData)
{
    rowText = ReplaceAll(rowText, "[TR]", "");
    rowText = ReplaceAll(rowText, "[/TR]", "");
    std::string entry = ExtractBlock(rowText, "[TD]", "[/TD]");
    while (entry != "")
    {
        std::string content(entry);
        content = ReplaceAll(content, "[TD]", "");
        content = ReplaceAll(content, "[/TD]", "");
        pData->data.push_back(content);
        rowText = ReplaceFirst(rowText, entry, "");
        // intialise for next loop
        entry = ExtractBlock(rowText, "[TD]", "[/TD]");
    }
}

void CForumExportDlg::AddHeader(
    std::string& tableText,
    const std::vector<size_t>& columnWidths)
{
    for (auto&& it: columnWidths)
    {
        tableText += "+";
        for (size_t i = 0; i < it; ++i)
        {
            tableText += "-";
        }
    }
    tableText += "+\r\n";
}

void CForumExportDlg::AddRow(
    std::string& tableText,
    const TableData& rowData,
    const std::vector<size_t>& columnWidths)
{
    size_t columnIndex = 0;
    for (auto&& it : columnWidths)
    {
        tableText += "|";
        tableText += rowData.data[columnIndex];
        size_t length = rowData.data[columnIndex].size();
        for (size_t i = length; i < it; ++i)
        {
            tableText += " ";
        }
        ++columnIndex;
    }
    tableText += "|\r\n";
}
