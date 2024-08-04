// ForumExportDlg.h
//
#include "Resource.h"
#include "XmlLib\SaxContentElement.h"
#include "GlobalSupportFunctions.h"
#include "EnhancementSpendInTree.h"
#include "DestinySpendInTree.h"
#include "ReaperSpendInTree.h"

class Build;
class EquippedGear;
class TrainedSpell;

enum ForumExportSections
{
    FES_Header = 0,
    FES_PastLives,
    FES_SpecialFeats,
    FES_Saves,
    FES_EnergyResistances,
    FES_FeatSelections,
    FES_FeatSelectionsNoSkills,
    FES_GrantedFeats,
    FES_AutomaticFeats,
    FES_ConsolidatedFeats,
    FES_Skills,
    FES_SelfAndPartyBuffs,
    FES_ActiveStances,
    FES_EnhancementTrees,
    FES_DestinyTrees,
    FES_ReaperTrees,
    FES_SpellPowers,
    FES_Spells,
    FES_SLAs,
    FES_WeaponDamage,
    FES_TacticalDCs,
    FES_Gear,
    FES_AlternateGearLayouts,
    FES_SimpleGear,
    FES_Count,
};

const XmlLib::enumMapEntry<ForumExportSections> forumExportSectionsMap[] =
{
    {FES_Header, L"Character Header"},
    {FES_PastLives, L"Past Lives"},
    {FES_SpecialFeats, L"Special Feats"},
    {FES_Saves, L"Saves"},
    {FES_EnergyResistances, L"Energy Resistances"},
    {FES_FeatSelections, L"Level and Feat Selections"},
    {FES_FeatSelectionsNoSkills, L"Level and Feat Selections (No Skills)"},
    {FES_GrantedFeats, L"Granted Feats"},
    {FES_AutomaticFeats, L"Automatic Feats"},
    {FES_ConsolidatedFeats, L"Consolidated Feats"},
    {FES_Skills, L"Skill Point Spend"},
    {FES_SelfAndPartyBuffs, L"Self and Party Buffs"},
    {FES_ActiveStances, L"Active Stances"},
    {FES_EnhancementTrees, L"Enhancement Trees"},
    {FES_DestinyTrees, L"Destiny Trees"},
    {FES_ReaperTrees, L"Reaper Trees"},
    {FES_SpellPowers, L"Spell Powers"},
    {FES_Spells, L"Spells"},
    {FES_SLAs, L"Spell Like Abilities"},
    {FES_WeaponDamage, L"Weapon Damage"},
    {FES_TacticalDCs, L"Tactical DCs"},
    {FES_Gear, L"Gear"},
    {FES_AlternateGearLayouts, L"Alternate Gear Layouts"},
    {FES_SimpleGear, L"Simple Gear (name and augments only)"},
    {FES_Count, L"Enumeration Count"},
    {ForumExportSections(0), NULL}
};

class CForumExportDlg : public CDialogEx
{
    public:
        CForumExportDlg(Build* pBuild);
        ~CForumExportDlg();

        //{{AFX_DATA(CForumExportDlg)
        enum { IDD = IDD_FORUM_EXPORT_DIALOG };
        //}}AFX_DATA

    protected:
        //{{AFX_VIRTUAL(CForumExportDlg)
        virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
        virtual BOOL OnInitDialog();
        virtual void OnOK();
        //}}AFX_VIRTUAL

        //{{AFX_MSG(CForumExportDlg)
        afx_msg void OnItemchangedListConfigureExport(NMHDR* pNMHDR, LRESULT* pResult);
        afx_msg void OnMoveUp();
        afx_msg void OnMoveDown();
        afx_msg void OnSelchangeComboFormat();
        //}}AFX_MSG
        DECLARE_MESSAGE_MAP()
    private:
        void PopulateConfigureExportList();
        void PopulateExport();
        void AddCharacterHeader(std::stringstream& forumExport);
        void AddPastLives(std::stringstream& forumExport);
        void AddSpecialFeats(std::stringstream& forumExport);
        void AddFeats(
                std::stringstream& forumExport,
                const std::string& heading,
                const std::string& featType,
                const std::list<TrainedFeat>& feats) const;
        void AddAbilityValues(std::stringstream& forumExport, AbilityType ability);
        void AddTableEntryBreakdown(
                std::stringstream& forumExport,
                const std::string& header,
                const std::string& sub,
                BreakdownType bt);
        void AddBreakdown(
                std::stringstream& forumExport,
                const std::string& header,
                size_t width,
                BreakdownType bt);
        void AddSaves(std::stringstream& forumExport);
        void AddFeatSelections(std::stringstream& forumExport, bool bIncludeSkills);
        void AddGrantedFeats(std::stringstream& forumExport);
        void AddAutomaticFeats(std::stringstream& forumExport);
        void AddConsolidatedFeats(std::stringstream& forumExport);
        void AddSelfAndPartyBuffs(std::stringstream& forumExport);
        void AddActiveStances(std::stringstream& forumExport);
        void AddSkills(std::stringstream& forumExport);
        void AddSkillsAtLevel(size_t level, std::stringstream& forumExport);
        void AddEnergyResistances(std::stringstream& forumExport);
        void AddEnergyResistances(std::stringstream& forumExport, const std::string& name, BreakdownType bt1, BreakdownType bt2);
        void AddEnhancements(std::stringstream& forumExport);
        void AddEnhancementTree(std::stringstream& forumExport);
        void AddEpicDestinyTree(std::stringstream& forumExport);
        void AddReaperTrees(std::stringstream& forumExport);
        void AddSpellPowers(std::stringstream& forumExport);
        void AddSpellPower(
                std::stringstream& forumExport,
                const std::string& label,
                BreakdownType btPower,
                BreakdownType btCrit,
                BreakdownType btMult);
        void AddSpellPowerToTable(
            std::stringstream& forumExport,
            const std::string& label,
            BreakdownType btPower,
            BreakdownType btCrit,
            BreakdownType btMult);
        void AddSpells(std::stringstream& forumExport);
        void AddSpellList(std::stringstream& forumExport, const std::list<TrainedSpell>& spellList, size_t spellLevel) const;
        void AddSLAs(std::stringstream& forumExport);
        void AddWeaponDamage(std::stringstream& forumExport);
        void AddTacticalDCs(std::stringstream& forumExport);
        void AddGear(std::stringstream& forumExport);
        void AddAlternateGear(std::stringstream& forumExport);
        void AddSimpleGear(std::stringstream& forumExport);

        void AddEnhancementTree(std::stringstream& forumExport, const SpendInTree& treeSpend);
        void AddEpicDestinyTree(std::stringstream& forumExport, const SpendInTree& treeSpend);
        void AddReaperTree(std::stringstream& forumExport, const SpendInTree& treeSpend);

        void ApplyFormatSelection(std::string* pContent);
        void ConvertToPlainText(std::string& content);
        std::string ConvertTable(std::string tableText);

        struct TableData
        {
            std::vector<std::string> data;
        };

        void ExtractTableRowData(std::string rowText, TableData* pData);
        void AddHeader(std::string& tableText, const std::vector<size_t>& columnWidths);
        void AddRow(std::string& tableText, const TableData& rowData, const std::vector<size_t>& columnWidths);

        void ExportGear(const EquippedGear& gear, std::stringstream& forumExport, bool bSimple);
        std::list<std::string> GetLevelEntries(size_t level, bool bIncludeSkills);

        Build* m_pBuild;
        CListCtrl m_listConfigureExport;
        CComboBox m_comboFormat;
        CEdit m_editExport;
        CButton m_buttonMoveUp;
        CButton m_buttonMoveDown;
        CFont m_font;
        bool m_bShowSection[FES_Count];
        ForumExportSections m_SectionOrder[FES_Count];
        bool m_bPopulatingControl;
        int m_exportType;
};

