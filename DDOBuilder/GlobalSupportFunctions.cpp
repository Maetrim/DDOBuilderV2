// GlobalSupportFunctions.h
//
#include "stdafx.h"
#include "GlobalSupportFunctions.h"

#include "Augment.h"
#include "LogPane.h"
#include "DDOBuilder.h"
#include "MainFrm.h"
#include "EnhancementTree.h"
#include "Feat.h"
#include "SetBonus.h"
#include "Spell.h"

const std::list<Bonus>& BonusTypes()
{
    return theApp.BonusTypes();
}

const std::list<Race> & Races()
{
    return theApp.Races();
}

const std::list<Class> & Classes()
{
    return theApp.Classes();
}

const std::map<std::string, Feat> & StandardFeats()
{
    return theApp.AllFeats();
}

const std::list<Feat> & HeroicPastLifeFeats()
{
    return theApp.HeroicPastLifeFeats();
}

const std::list<Feat> & RacialPastLifeFeats()
{
    return theApp.RacialPastLifeFeats();
}

const std::list<Feat> & IconicPastLifeFeats()
{
    return theApp.IconicPastLifeFeats();
}

const std::list<Feat> & EpicPastLifeFeats()
{
    return theApp.EpicPastLifeFeats();
}

const std::list<Feat> & SpecialFeats()
{
    return theApp.SpecialFeats();
}

const std::list<Feat>& UniversalTreeFeats()
{
    return theApp.UniversalTreeFeats();
}

const std::list<Feat>& DestinyTreeFeats()
{
    return theApp.DestinyTreeFeats();
}

const std::list<Feat> & FavorFeats()
{
    return theApp.FavorFeats();
}

const std::list<EnhancementTree> & EnhancementTrees()
{
    return theApp.EnhancementTrees();
}

const std::list<Filigree>& Filigrees()
{
    return theApp.Filigrees();
}

const std::list<Augment>& Augments()
{
    return theApp.Augments();
}

const std::list<SetBonus>& SetBonuses()
{
    return theApp.SetBonuses();
}

const std::list<Stance>& Stances()
{
    return theApp.Stances();
}

const std::list<Spell>& Spells()
{
    return theApp.Spells();
}

const std::list<Item>& Items()
{
    return theApp.Items();
}

CImageList& ItemsImages()
{
    return theApp.ItemImageList();
}

const std::list<Gem>& SentientGems()
{
    return theApp.SentientGems();
}

const std::list<WeaponGroup>& WeaponGroups()
{
    return theApp.WeaponGroups();
}

const std::list<Buff>& ItemBuffs()
{
    return theApp.ItemBuffs();
}

const std::list<Spell>& ItemClickies()
{
    return theApp.ItemClickies();
}

const Item& FindItem(const std::string& itemName)
{
    static Item badItem;
    const std::list<Item>& items = Items();
    std::list<Item>::const_iterator it = items.begin();
    while (it != items.end())
    {
        if ((*it).Name() == itemName)
        {
            return (*it);
        }
        ++it;
    }
    return badItem;
}

const Bonus& FindBonus(const std::string& bonus)
{
    // find the information about a specific feat from the feat list
    static Bonus bonusNotFound;
    const std::list<Bonus> & allBonus = BonusTypes();
    auto it = allBonus.begin();
    while (it != allBonus.end())
    {
        if ((*it).Name() == bonus)
        {
            // this is the feat were looking for
            return (*it);
        }
        ++it;
    }
    return bonusNotFound;
}

const Feat & FindFeat(const std::string& featName)
{
    // find the information about a specific feat from the feat list
    static Feat featNotFound("Feat not found", "No description", "Unknown");
    const std::map<std::string, Feat>& standardFeats = StandardFeats();
    auto ff = standardFeats.find(featName);
    if (ff != standardFeats.end())
    {
        return ff->second;
    }
    auto heroicPastLifeFeats = HeroicPastLifeFeats();
    for (auto&& it : heroicPastLifeFeats)
    {
        if (it.Name() == featName)
        {
            return it;
        }
    }
    auto racialPastLifeFeats = RacialPastLifeFeats();
    for (auto&& it : racialPastLifeFeats)
    {
        if (it.Name() == featName)
        {
            return it;
        }
    }
    auto iconicPastLifeFeats = IconicPastLifeFeats();
    for (auto&& it : iconicPastLifeFeats)
    {
        if (it.Name() == featName)
        {
            return it;
        }
    }
    auto epicPastLifeFeats = EpicPastLifeFeats();
    for (auto&& it : epicPastLifeFeats)
    {
        if (it.Name() == featName)
        {
            return it;
        }
    }
    auto specialFeats = SpecialFeats();
    for (auto&& it : specialFeats)
    {
        if (it.Name() == featName)
        {
            return it;
        }
    }
    auto universalTreeFeats = UniversalTreeFeats();
    for (auto&& it : universalTreeFeats)
    {
        if (it.Name() == featName)
        {
            return it;
        }
    }
    auto favorFeats = FavorFeats();
    for (auto&& it : favorFeats)
    {
        if (it.Name() == featName)
        {
            return it;
        }
    }
    CString text;
    text.Format("Feat \"%s\" not found", featName.c_str());
    GetLog().AddLogEntry(text);
    return featNotFound;
}

const Race & FindRace(const std::string& raceName)
{
    static Race badRace;

    bool bFound = false;
    const std::list<Race> & races = Races();
    std::list<Race>::const_iterator rit = races.begin();
    while (!bFound && rit != races.end())
    {
        if ((*rit).Name() == raceName)
        {
            bFound = true;
        }
        else
        {
            ++rit;
        }
    }
    if (bFound)
    {
        return (*rit);
    }
    return badRace;
}

const Class & FindClass(const std::string& className)
{
    static Class badClass;

    bool bFound = false;
    const std::list<Class> & classes = Classes();
    std::list<Class>::const_iterator cit = classes.begin();
    while (!bFound && cit != classes.end())
    {
        if ((*cit).Name() == className)
        {
            bFound = true;
        }
        else
        {
            ++cit;
        }
    }
    if (bFound)
    {
        return (*cit);
    }
    return badClass;
}

const Buff& FindBuff(const std::string& buffName)
{
    const std::list<Buff>& buffs = ItemBuffs();
    for (auto&& it : buffs)
    {
        if (it.Type() == buffName)
        {
            // this is the buff we are looking for
            return it;
        }
    }
    return FindBuff("BuffNotFound");
}

const EnhancementTreeItem * FindEnhancement(
        const std::string& internalName,
        std::string * treeName) // can be NULL
{
    const EnhancementTreeItem * item = NULL;
    const std::list<EnhancementTree> & trees = EnhancementTrees();
    bool found = false;
    std::list<EnhancementTree>::const_iterator tit = trees.begin();
    while (!found && tit != trees.end())
    {
        item = (*tit).FindEnhancementItem(internalName);
        found = (item != NULL);
        if (found && treeName != NULL)
        {
            *treeName = (*tit).Name();
        }
        ++tit;
    }
    return item;
}

const EnhancementTree& GetEnhancementTree(const std::string& treeName)
{
    static EnhancementTree emptyTree;
    const std::list<EnhancementTree> & allTrees = EnhancementTrees();
    std::list<EnhancementTree>::const_iterator it = allTrees.begin();
    while (it != allTrees.end())
    {
        // find the requested tree
        if ((*it).Name() == treeName)
        {
            // this is the one we want
            return (*it);
        }
        ++it;
    }
    return emptyTree;
}

size_t ClassIndex(const std::string& className)
{
    size_t ci = 0;
    const std::list<Class> & classes = Classes();
    std::list<Class>::const_iterator cit = classes.begin();
    while (cit != classes.end())
    {
        if ((*cit).Name() == className)
        {
            break;
        }
        ++cit;
        ++ci;
    }
    return ci;
}

const Class & ClassFromIndex(size_t index)
{
    const std::list<Class> & classes = Classes();
    std::list<Class>::const_iterator cit = classes.begin();
    std::advance(cit, index);
    return (*cit);
}

int BaseStatToBonus(double ability)
{
    ability -= 10;      // 10 = no bonus
    int bonus;
    if (ability < 0)
    {
        // round up for penalties
        bonus = (int)ceil((ability - 1) / 2);
    }
    else
    {
        // round down for bonuses
        bonus = (int)floor(ability / 2);
    }
    return bonus;
}

BOOL OnEraseBackground(
        CWnd * pWnd,
        CDC * pDC,
        const int controlsNotToBeErased[])
{
    pDC->SaveDC();

    const int * pId = controlsNotToBeErased;
    while (*pId != 0)
    {
        // Get rectangle of the control.
        CWnd * pControl = pWnd->GetDlgItem(*pId);
        if (pControl && pControl->IsWindowVisible())
        {
            CRect controlClip;
            pControl->GetWindowRect(&controlClip);
            pWnd->ScreenToClient(&controlClip);
            if (pControl->IsKindOf(RUNTIME_CLASS(CComboBox)))
            {
                // combo boxes return the height of the whole control, including the drop rectangle
                // reduce the height to the control size without it shown
                controlClip.bottom = controlClip.top
                        + GetSystemMetrics(SM_CYHSCROLL)
                        + GetSystemMetrics(SM_CYEDGE) * 2;
                // special case for combo boxes with image lists
                CComboBoxEx * pCombo = dynamic_cast<CComboBoxEx*>(pControl);
                if (pCombo != NULL)
                {
                    CImageList * pImage = pCombo->GetImageList();
                    if (pImage != NULL)
                    {
                        IMAGEINFO info;
                        pImage->GetImageInfo(0, &info);
                        // limit to the the height of the selection combo
                        controlClip.bottom = controlClip.top
                                + info.rcImage.bottom
                                - info.rcImage.top
                                + GetSystemMetrics(SM_CYEDGE) * 2;
                    }
                }
            }
            pDC->ExcludeClipRect(&controlClip);
        }

        // next in list
        ++pId;
    }
    CRect rctClient;
    pWnd->GetWindowRect(&rctClient);
    pWnd->ScreenToClient(&rctClient);

    // use the selected theme in use for the background colour selection
    CMFCVisualManager * manager = CMFCVisualManager::GetInstance();
    if (manager != NULL)
    {
        manager->OnFillPopupWindowBackground(pDC, rctClient);
    }
    else
    {
        pDC->FillSolidRect(rctClient, GetSysColor(COLOR_BTNFACE));
    }
    pDC->RestoreDC(-1);

    return TRUE;
}

HTREEITEM GetNextTreeItem(const CTreeCtrl& treeCtrl, HTREEITEM hItem)
{
    // we return the next HTEEITEM for a tree such as:
    // Root (1)
    //      Child1 (2)
    //          xxxx (3)
    //          yyyy (4)
    //      Chiled2 (5)
    // Item (6)
    
    // has this item got any children
    if (treeCtrl.ItemHasChildren(hItem))
    {
        return treeCtrl.GetNextItem(hItem, TVGN_CHILD);
    }
    else if (treeCtrl.GetNextItem(hItem, TVGN_NEXT) != NULL)
    {
        // the next item at this level
        return treeCtrl.GetNextItem(hItem, TVGN_NEXT);
    }
    else
    {
        // return the next item after our parent
        hItem = treeCtrl.GetParentItem(hItem);
        if (hItem == NULL)
        {
            // no parent
            return NULL;
        }
        while (hItem != NULL && treeCtrl.GetNextItem(hItem, TVGN_NEXT) == NULL)
        {
            hItem = treeCtrl.GetParentItem(hItem);
        }
        // next item that follows our parent
        return treeCtrl.GetNextItem(hItem, TVGN_NEXT);
    }
}

CLogPane & GetLog()
{
    CWnd* pWnd = AfxGetApp()->m_pMainWnd;
    CMainFrame * pMainFrame = dynamic_cast<CMainFrame*>(pWnd);
    return pMainFrame->GetLog();
}

HRESULT LoadImageFile(
        const std::string& folder,
        const std::string& filename,
        CImage * pImage,
        const CSize & expectedImageSize,
        bool bReportIssue)
{
    // load the specified image and check its valid
    char fullPath[MAX_PATH];
    ::GetModuleFileName(
            NULL,
            fullPath,
            MAX_PATH);

    char pathdrive[_MAX_DRIVE];
    char pathfolder[_MAX_PATH];
    _splitpath_s(fullPath,
            pathdrive, _MAX_DRIVE,
            pathfolder, _MAX_PATH,
            NULL, 0,        // filename
            NULL, 0);       // extension

    char path[_MAX_PATH];
    _makepath_s(path, _MAX_PATH, pathdrive, pathfolder, NULL, NULL);

    std::string location = path + folder;
    std::string pathname = location + filename + ".png";
    HRESULT result = pImage->Load(pathname.c_str());
    if (result != S_OK)
    {
        if (bReportIssue)
        {
            std::stringstream ss;
            ss << "Failed to find image file \"" << pathname.c_str() << "\"\n";
            GetLog().AddLogEntry(ss.str().c_str());
            // and load in a default image
            result = LoadImageFile(
                    "DataFiles\\UIImages\\",
                    "NoImage",
                    pImage,
                    expectedImageSize,
                    false);
        }
    }
    if (result == S_OK)
    {
        pImage->SetTransparentColor(c_transparentColour);
        // check the image is the correct size
        if (pImage->GetHeight() != expectedImageSize.cy
                || pImage->GetWidth() != expectedImageSize.cx)
        {
            std::stringstream ss;
            ss << "Image in file \"" << pathname.c_str() << "\" wrong size\n";
            GetLog().AddLogEntry(ss.str().c_str());
        }
    }
    return result;
}

bool ImageFileExists(
        const std::string& folder,
        const std::string& filename)
{
    // load the specified image and check its valid
    char fullPath[MAX_PATH];
    ::GetModuleFileName(
            NULL,
            fullPath,
            MAX_PATH);

    char pathdrive[_MAX_DRIVE];
    char pathfolder[_MAX_PATH];
    _splitpath_s(fullPath,
            pathdrive, _MAX_DRIVE,
            pathfolder, _MAX_PATH,
            NULL, 0,        // filename
            NULL, 0);       // extension

    char path[_MAX_PATH];
    _makepath_s(path, _MAX_PATH, pathdrive, pathfolder, NULL, NULL);

    std::string location = path + folder;
    std::string pathname = location + filename + ".png";

    bool exists = false;
    WIN32_FIND_DATA findFileData;
    HANDLE hFind = FindFirstFile(pathname.c_str(), &findFileData);
    if (hFind != INVALID_HANDLE_VALUE)
    {
        FindClose(hFind);
        exists = true;
    }
    return exists;
}

// CImage convert to GrayScale by changing the CImage color table
void MakeGrayScale(CImage * pImage, COLORREF transparent)
{
    if (pImage->IsIndexed())
    {
        // if the image is indexed, we can change all the colors by manipulating the
        // color table in use by the bitmap
        size_t numColors = pImage->GetMaxColorTableEntries();
        RGBQUAD * pColors = new RGBQUAD[numColors];
        memset(pColors, 0, sizeof(RGBQUAD) * numColors);
        pImage->GetColorTable(0, numColors, pColors);
        // now change each entry to it's gray scale equivalent unless
        // its the background transparent color
        for (size_t ci = 0; ci < numColors; ++ci)
        {
            COLORREF color = RGB(
                    pColors[ci].rgbRed,
                    pColors[ci].rgbGreen,
                    pColors[ci].rgbBlue);
            if (color != transparent)
            {
                // this is not the transparent background color
                // change it to gray scale using the luminosity method
                // luminosity = 0.21 R  + 0.72 G + 0.07 B.
                BYTE luminosity =
                        (BYTE)(0.21 * pColors[ci].rgbRed
                        + 0.72 * pColors[ci].rgbGreen
                        + 0.07 * pColors[ci].rgbBlue);
                pColors[ci].rgbRed = luminosity;
                pColors[ci].rgbGreen = luminosity;
                pColors[ci].rgbBlue = luminosity;
            }
        }
        // now all colors have been made gray scale set them back on the image
        pImage->SetColorTable(0, numColors, pColors);
        delete []pColors;
    }
    else
    {
        // if its not indexed, we have to manually get and set all bitmap pixels
        size_t xSize = pImage->GetWidth();
        size_t ySize = pImage->GetHeight();
        for (size_t x = 0; x < xSize; ++x)
        {
            for (size_t y = 0; y < ySize; ++y)
            {
                COLORREF color = pImage->GetPixel(x, y);
                if (color != transparent)
                {
                    // HACK, binary representations must match
                    RGBQUAD pixel;
                    memcpy(&pixel, &color, sizeof(COLORREF));
                    BYTE luminosity =
                            (BYTE)(0.21 * pixel.rgbRed
                            + 0.72 * pixel.rgbGreen
                            + 0.07 * pixel.rgbBlue);
                    COLORREF grey = RGB(luminosity, luminosity, luminosity);
                    pImage->SetPixel(x, y, grey);
                }
            }
        }
    }
}

size_t TrainedCount(
        const std::list<TrainedFeat> & currentFeats,
        const std::string& featName)
{
    // look through the list of all feats trained and count how many times the
    // given entry appears in the list
    size_t count = 0;
    std::list<TrainedFeat>::const_iterator it = currentFeats.begin();
    while (it != currentFeats.end())
    {
        if ((*it).FeatName() == featName)
        {
            ++count;
        }
        ++it;
    }
    return count;
}

const EnhancementTreeItem * FindEnhancement(
        const std::string& internalName)
{
    const EnhancementTreeItem* item = NULL;
    // have to search all the trees
    const std::list<EnhancementTree>& trees = EnhancementTrees();
    auto tit = trees.begin();
    while (item == NULL && tit != trees.end())
    {
        item = (*tit).FindEnhancementItem(internalName);
        ++tit;
    }
    return item;
}

const EnhancementTreeItem * FindEnhancement(
        const std::string& treeName,
        const std::string& internalName)
{
    const EnhancementTree & tree = EnhancementTree::GetTree(treeName);
    const EnhancementTreeItem* item = tree.FindEnhancementItem(internalName);
    return item;
}

Spell FindClassSpellByName(const std::string& ct, const std::string& name)
{
    const ::Class& c = FindClass(ct);
    Spell spell = c.FindSpell(name);
    return spell;
}

Spell FindSpellByName(const std::string& name)
{
    Spell spell;
    const std::list<Spell>& spells = Spells();
    std::list<Spell>::const_iterator si = spells.begin();
    while (si != spells.end())
    {
        if ((*si).Name() == name)
        {
            spell = (*si);
            break;  // found, no need to check the rest
        }
        ++si;
    }
    if (spell.Name() == "")
    {
        CString errMsg;
        errMsg.Format("Spell %s not found in Spells.xml file", name.c_str());
        GetLog().AddLogEntry(errMsg);
    }
    return spell;
}

Spell FindItemClickieByName(const std::string& ct)
{
    Spell spell;
    const std::list<Spell>& spells = ItemClickies();
    std::list<Spell>::const_iterator si = spells.begin();
    while (si != spells.end())
    {
        if ((*si).Name() == ct)
        {
            spell = (*si);
            break;  // found, no need to check the rest
        }
        ++si;
    }
    if (spell.Name() == "")
    {
        CString errMsg;
        errMsg.Format("Spell %s not found in ItemClickie.xml file", ct.c_str());
        GetLog().AddLogEntry(errMsg);
    }
    return spell;
}

void SelectComboboxEntry(
        size_t itemData,
        CComboBox * pCombo)
{
    // select a combo box entry via its item data
    int sel = CB_ERR;           // assume select no item
    size_t numItems = pCombo->GetCount();
    for (size_t i = 0; i < numItems; ++i)
    {
        DWORD comboItemData = pCombo->GetItemData(i);
        if (itemData == comboItemData)
        {
            // this is the one we need to select
            sel = i;
            break;      // were done
        }
    }
    pCombo->SetCurSel(sel);
}

void SelectComboboxEntry(
        const std::string& strItem,
        CComboBox * pCombo)
{
    pCombo->SelectString(-1, strItem.c_str());
}

size_t GetComboboxSelection(CComboBox * pCombo)
{
    size_t selectedItem = 0;        // translates to enum Type_Unknown
    int sel = pCombo->GetCurSel();
    ASSERT(sel != CB_ERR);  // we should have a valid selection
    if (sel != CB_ERR)
    {
        selectedItem = (size_t)pCombo->GetItemData(sel);
    }
    return selectedItem;
}

std::string DataFolder()
{
    // load all the global data required by the program
    // all data files are in a sub-folder under the path of the executable
    char fullPath[MAX_PATH];
    ::GetModuleFileName(NULL, fullPath, MAX_PATH);

    char drive[_MAX_DRIVE];
    char folder[_MAX_PATH];
    _splitpath_s(fullPath, drive, _MAX_DRIVE, folder, _MAX_PATH, NULL, 0, NULL, 0);

    char path[_MAX_PATH];
    _makepath_s(path, _MAX_PATH, drive, folder, NULL, NULL);

    std::string folderPath(path);
    folderPath += "DataFiles";
    folderPath += '\\';
    return folderPath;
}

MouseHook* GetMouseHook()
{
    CWnd* pWnd = AfxGetApp()->m_pMainWnd;
    CMainFrame* pMainWnd = dynamic_cast<CMainFrame*>(pWnd);
    return pMainWnd->GetMouseHook();
}

BreakdownItem* FindBreakdown(BreakdownType type)
{
    // we have to get the relevant view from the main frame class
    CWnd* pWnd = AfxGetApp()->m_pMainWnd;
    CMainFrame* pMainWnd = dynamic_cast<CMainFrame*>(pWnd);
    return pMainWnd->FindBreakdown(type);
}

BreakdownType StatToBreakdown(AbilityType ability)
{
    BreakdownType bt = Breakdown_Unknown;
    switch (ability)
    {
    case Ability_Strength:
        bt = Breakdown_Strength;
        break;
    case Ability_Dexterity:
        bt = Breakdown_Dexterity;
        break;
    case Ability_Constitution:
        bt = Breakdown_Constitution;
        break;
    case Ability_Intelligence:
        bt = Breakdown_Intelligence;
        break;
    case Ability_Wisdom:
        bt = Breakdown_Wisdom;
        break;
    case Ability_Charisma:
        bt = Breakdown_Charisma;
        break;
    }
    return bt;
}

BreakdownType SkillToBreakdown(SkillType skill)
{
    BreakdownType bt = Breakdown_Unknown;
    switch (skill)
    {
    case Skill_Balance:
        bt = Breakdown_SkillBalance;
        break;
    case Skill_Bluff:
        bt = Breakdown_SkillBluff;
        break;
    case Skill_Concentration:
        bt = Breakdown_SkillConcentration;
        break;
    case Skill_Diplomacy:
        bt = Breakdown_SkillDiplomacy;
        break;
    case Skill_DisableDevice:
        bt = Breakdown_SkillDisableDevice;
        break;
    case Skill_Haggle:
        bt = Breakdown_SkillHaggle;
        break;
    case Skill_Heal:
        bt = Breakdown_SkillHeal;
        break;
    case Skill_Hide:
        bt = Breakdown_SkillHide;
        break;
    case Skill_Intimidate:
        bt = Breakdown_SkillIntimidate;
        break;
    case Skill_Jump:
        bt = Breakdown_SkillJump;
        break;
    case Skill_Listen:
        bt = Breakdown_SkillListen;
        break;
    case Skill_MoveSilently:
        bt = Breakdown_SkillMoveSilently;
        break;
    case Skill_OpenLock:
        bt = Breakdown_SkillOpenLock;
        break;
    case Skill_Perform:
        bt = Breakdown_SkillPerform;
        break;
    case Skill_Repair:
        bt = Breakdown_SkillRepair;
        break;
    case Skill_Search:
        bt = Breakdown_SkillSearch;
        break;
    case Skill_SpellCraft:
        bt = Breakdown_SkillSpellCraft;
        break;
    case Skill_Spot:
        bt = Breakdown_SkillSpot;
        break;
    case Skill_Swim:
        bt = Breakdown_SkillSwim;
        break;
    case Skill_Tumble:
        bt = Breakdown_SkillTumble;
        break;
    case Skill_UMD:
        bt = Breakdown_SkillUMD;
        break;
    }
    return bt;
}

BreakdownItem* CasterLevelBreakdown(const std::string& ct)
{
    BreakdownItem* pItem = NULL;
    const std::list<::Class>& classes = Classes();
    for (auto&& cit : classes)
    {
        if (cit.Name() == ct)
        {
            pItem = FindBreakdown(static_cast<BreakdownType>(Breakdown_CasterLevel_First + cit.Index()));
        }
    }
    return pItem;
}


AbilityType StatFromSkill(SkillType skill)
{
    // return which ability provides the bonus to the skill type
    AbilityType at = Ability_Unknown;
    switch (skill)
    {
    case Skill_Bluff:
    case Skill_Diplomacy:
    case Skill_Haggle:
    case Skill_Intimidate:
    case Skill_Perform:
    case Skill_UMD:
        at = Ability_Charisma;
        break;

    case Skill_Concentration:
        at = Ability_Constitution;
        break;

    case Skill_Balance:
    case Skill_Hide:
    case Skill_MoveSilently:
    case Skill_OpenLock:
    case Skill_Tumble:
        at = Ability_Dexterity;
        break;

    case Skill_DisableDevice:
    case Skill_Repair:
    case Skill_Search:
    case Skill_SpellCraft:
        at = Ability_Intelligence;
        break;

    case Skill_Heal:
    case Skill_Listen:
    case Skill_Spot:
        at = Ability_Wisdom;
        break;

    case Skill_Jump:
    case Skill_Swim:
        at = Ability_Strength;
        break;

    default:
        break;
    }
    ASSERT(at != Ability_Unknown);  // should have been found
    return at;
}

int ArmorCheckPenalty_Multiplier(SkillType skill)
{
    int multiplier = 0; // default
    switch (skill)
    {
    case Skill_Balance:
    case Skill_Hide:
    case Skill_Jump:
    case Skill_MoveSilently:
    case Skill_Tumble:
        multiplier = 1;
        break;
    case Skill_Swim:
        // is subject to double the standard Armor check penalty 
        multiplier = 2;
        break;
    }
    return multiplier;
}

bool IsVowel(char ch)
{
    bool bVowel = false;
    ch = static_cast<char>(tolower(ch));
    switch (ch)
    {
        case 'a':
        case 'e':
        case 'i':
        case 'o':
        case 'u':
            bVowel = true;
            break;
    }
    return bVowel;
}

bool IsOffHandWeapon(WeaponType wt)
{
    bool bOffHandItem = false;
    switch (wt)
    {
        case Weapon_ShieldBuckler:
        case Weapon_ShieldSmall:
        case Weapon_ShieldLarge:
        case Weapon_ShieldTower:
        case Weapon_Orb:
        case Weapon_RuneArm:
            bOffHandItem = true;
            break;
    }
    return bOffHandItem;
}

BreakdownType TacticalToBreakdown(TacticalType tactical)
{
    BreakdownType bt = Breakdown_Unknown;
    switch (tactical)
    {
        case Tactical_Assassinate:
            bt = Breakdown_TacticalAssassinate;
            break;
        case Tactical_Trip:
            bt = Breakdown_TacticalTrip;
            break;
        case Tactical_Stun:
            bt = Breakdown_TacticalStunning;
            break;
        case Tactical_Sunder:
            bt = Breakdown_TacticalSunder;
            break;
        case Tactical_StunningShield:
            bt = Breakdown_TacticalStunningShield;
            break;
        case Tactical_General:
            bt = Breakdown_TacticalGeneral;
            break;
        case Tactical_Wands:
            bt = Breakdown_TacticalWands;
            break;
        case Tactical_Fear:
            bt = Breakdown_TacticalFear;
            break;
        case Tactical_InnateAttack:
            bt = Breakdown_TacticalInnateAttack;
            break;
        case Tactical_BreathWeapon:
            bt = Breakdown_TacticalBreathWeapon;
            break;
        case Tactical_Poison:
            bt = Breakdown_TacticalPoison;
            break;
        case Tactical_RuneArm:
            bt = Breakdown_TacticalRuneArm;
            break;
        case Tactical_QuiveringPalm:
            bt = Breakdown_TacticalQuiveringPalm;
            break;
    }
    return bt;
}

BreakdownType SchoolToBreakdown(SpellSchoolType school)
{
    BreakdownType bt = Breakdown_Unknown;
    switch (school)
    {
        case SpellSchool_Abjuration:
            bt = Breakdown_SpellSchoolAbjuration;
            break;
        case SpellSchool_Conjuration:
            bt = Breakdown_SpellSchoolConjuration;
            break;
        case SpellSchool_Divination:
            bt = Breakdown_SpellSchoolDivination;
            break;
        case SpellSchool_Enchantment:
            bt = Breakdown_SpellSchoolEnchantment;
            break;
        case SpellSchool_Evocation:
            bt = Breakdown_SpellSchoolEvocation;
            break;
        case SpellSchool_Illusion:
            bt = Breakdown_SpellSchoolIllusion;
            break;
        case SpellSchool_Necromancy:
            bt = Breakdown_SpellSchoolNecromancy;
            break;
        case SpellSchool_Transmutation:
            bt = Breakdown_SpellSchoolTransmutation;
            break;
        case SpellSchool_GlobalDC:
            bt = Breakdown_SpellSchoolGlobalDC;
            break;
    }
    return bt;
}

BreakdownType CasterLevelSchoolToBreakdown(SpellSchoolType school)
{
    BreakdownType bt = Breakdown_Unknown;
    switch (school)
    {
        case SpellSchool_Abjuration:
            bt = Breakdown_CasterLevel_School_Abjuration;
            break;
        case SpellSchool_Conjuration:
            bt = Breakdown_CasterLevel_School_Conjuration;
            break;
        case SpellSchool_Divination:
            bt = Breakdown_CasterLevel_School_Divination;
            break;
        case SpellSchool_Enchantment:
            bt = Breakdown_CasterLevel_School_Enchantment;
            break;
        case SpellSchool_Evocation:
            bt = Breakdown_CasterLevel_School_Evocation;
            break;
        case SpellSchool_Illusion:
            bt = Breakdown_CasterLevel_School_Illusion;
            break;
        case SpellSchool_Necromancy:
            bt = Breakdown_CasterLevel_School_Necromancy;
            break;
        case SpellSchool_Transmutation:
            bt = Breakdown_CasterLevel_School_Transmutation;
            break;
    }
    return bt;
}

BreakdownType MaxCasterLevelSchoolToBreakdown(SpellSchoolType school)
{
    BreakdownType bt = Breakdown_Unknown;
    switch (school)
    {
        case SpellSchool_Abjuration:
            bt = Breakdown_MaxCasterLevel_School_Abjuration;
            break;
        case SpellSchool_Conjuration:
            bt = Breakdown_MaxCasterLevel_School_Conjuration;
            break;
        case SpellSchool_Divination:
            bt = Breakdown_MaxCasterLevel_School_Divination;
            break;
        case SpellSchool_Enchantment:
            bt = Breakdown_MaxCasterLevel_School_Enchantment;
            break;
        case SpellSchool_Evocation:
            bt = Breakdown_MaxCasterLevel_School_Evocation;
            break;
        case SpellSchool_Illusion:
            bt = Breakdown_MaxCasterLevel_School_Illusion;
            break;
        case SpellSchool_Necromancy:
            bt = Breakdown_MaxCasterLevel_School_Necromancy;
            break;
        case SpellSchool_Transmutation:
            bt = Breakdown_MaxCasterLevel_School_Transmutation;
            break;
    }
    return bt;
}

BreakdownType MaxDamageTypeToBreakdown(DamageType dt)
{
    BreakdownType bt = Breakdown_Unknown;
    switch (dt)
    {
    case Damage_Acid: bt = Breakdown_MaxCasterLevel_Spell_Acid; break;
    case Damage_Cold: bt = Breakdown_MaxCasterLevel_Spell_Cold; break;
    case Damage_Electric: bt = Breakdown_MaxCasterLevel_Spell_Electric; break;
    case Damage_Evil: bt = Breakdown_MaxCasterLevel_Spell_Evil; break;
    case Damage_Fire: bt = Breakdown_MaxCasterLevel_Spell_Fire; break;
    case Damage_Force: bt = Breakdown_MaxCasterLevel_Spell_Force; break;
    case Damage_Negative:bt = Breakdown_MaxCasterLevel_Spell_Negative; break;
    case Damage_Positive: bt = Breakdown_MaxCasterLevel_Spell_Positive; break;
    case Damage_Sonic: bt = Breakdown_MaxCasterLevel_Spell_Sonic; break;
    case Damage_Ceruleite: bt = Breakdown_MaxCasterLevel_Ceruleite; break;
    case Damage_Crimsonite: bt = Breakdown_MaxCasterLevel_Crimsonite; break;
    case Damage_Gildleaf: bt = Breakdown_MaxCasterLevel_Gildleaf; break;
    }
    return bt;
}

BreakdownType SpellPowerToBreakdown(SpellPowerType sp)
{
    BreakdownType bt = Breakdown_Unknown;
    switch (sp)
    {
    case SpellPower_Acid:           bt = Breakdown_SpellPowerAcid; break;
    case SpellPower_Cold:           bt = Breakdown_SpellPowerCold; break;
    case SpellPower_Electric:       bt = Breakdown_SpellPowerElectric; break;
    case SpellPower_Evil:           bt = Breakdown_SpellPowerEvil; break;
    case SpellPower_Fire:           bt = Breakdown_SpellPowerFire; break;
    case SpellPower_ForceUntyped:   bt = Breakdown_SpellPowerForceUntyped; break;
    case SpellPower_LightAlignment: bt = Breakdown_SpellPowerLightAlignment; break;
    case SpellPower_Negative:       bt = Breakdown_SpellPowerNegative; break;
    case SpellPower_Physical:       bt = Breakdown_SpellPowerPhysical; break;
    case SpellPower_Poison:         bt = Breakdown_SpellPowerPoison; break;
    case SpellPower_Positive:       bt = Breakdown_SpellPowerPositive; break;
    case SpellPower_Repair:         bt = Breakdown_SpellPowerRepair; break;
    case SpellPower_Rust:           bt = Breakdown_SpellPowerRust; break;
    case SpellPower_Sonic:          bt = Breakdown_SpellPowerSonic; break;
    }
    return bt;
}

BreakdownType SpellPowerToCriticalChanceBreakdown(SpellPowerType sp)
{
    BreakdownType bt = Breakdown_Unknown;
    switch (sp)
    {
    case SpellPower_Acid:           bt = Breakdown_SpellCriticalChanceAcid; break;
    case SpellPower_Cold:           bt = Breakdown_SpellCriticalChanceCold; break;
    case SpellPower_Electric:       bt = Breakdown_SpellCriticalChanceElectric; break;
    case SpellPower_Evil:           bt = Breakdown_SpellCriticalChanceEvil; break;
    case SpellPower_Fire:           bt = Breakdown_SpellCriticalChanceFire; break;
    case SpellPower_ForceUntyped:   bt = Breakdown_SpellCriticalChanceForceUntyped; break;
    case SpellPower_LightAlignment: bt = Breakdown_SpellCriticalChanceLightAlignment; break;
    case SpellPower_Negative:       bt = Breakdown_SpellCriticalChanceNegative; break;
    case SpellPower_Physical:       bt = Breakdown_SpellCriticalChancePhysical; break;
    case SpellPower_Poison:         bt = Breakdown_SpellCriticalChancePoison; break;
    case SpellPower_Positive:       bt = Breakdown_SpellCriticalChancePositive; break;
    case SpellPower_Repair:         bt = Breakdown_SpellCriticalChanceRepair; break;
    case SpellPower_Rust:           bt = Breakdown_SpellCriticalChanceRust; break;
    case SpellPower_Sonic:          bt = Breakdown_SpellCriticalChanceSonic; break;
    }
    return bt;
}

BreakdownType SpellPowerToCriticalMultiplierBreakdown(SpellPowerType sp)
{
    BreakdownType bt = Breakdown_Unknown;
    switch (sp)
    {
    case SpellPower_Acid:           bt = Breakdown_SpellCriticalMultiplierAcid; break;
    case SpellPower_Cold:           bt = Breakdown_SpellCriticalMultiplierCold; break;
    case SpellPower_Electric:       bt = Breakdown_SpellCriticalMultiplierElectric; break;
    case SpellPower_Evil:           bt = Breakdown_SpellCriticalMultiplierEvil; break;
    case SpellPower_Fire:           bt = Breakdown_SpellCriticalMultiplierFire; break;
    case SpellPower_ForceUntyped:   bt = Breakdown_SpellCriticalMultiplierForceUntyped; break;
    case SpellPower_LightAlignment: bt = Breakdown_SpellCriticalMultiplierLightAlignment; break;
    case SpellPower_Negative:       bt = Breakdown_SpellCriticalMultiplierNegative; break;
    case SpellPower_Physical:       bt = Breakdown_SpellCriticalMultiplierPhysical; break;
    case SpellPower_Poison:         bt = Breakdown_SpellCriticalMultiplierPoison; break;
    case SpellPower_Positive:       bt = Breakdown_SpellCriticalMultiplierPositive; break;
    case SpellPower_Repair:         bt = Breakdown_SpellCriticalMultiplierRepair; break;
    case SpellPower_Rust:           bt = Breakdown_SpellCriticalMultiplierRust; break;
    case SpellPower_Sonic:          bt = Breakdown_SpellCriticalMultiplierSonic; break;
    }
    return bt;
}

BreakdownType DamageTypeToBreakdown(DamageType dt)
{
    BreakdownType bt = Breakdown_Unknown;
    switch (dt)
    {
    case Damage_Acid: bt = Breakdown_CasterLevel_Spell_Acid; break;
    case Damage_Cold: bt = Breakdown_CasterLevel_Spell_Cold; break;
    case Damage_Electric: bt = Breakdown_CasterLevel_Spell_Electric; break;
    case Damage_Evil: bt = Breakdown_CasterLevel_Spell_Evil; break;
    case Damage_Fire: bt = Breakdown_CasterLevel_Spell_Fire; break;
    case Damage_Force: bt = Breakdown_CasterLevel_Spell_Force; break;
    case Damage_Negative:bt = Breakdown_CasterLevel_Spell_Negative; break;
    case Damage_Positive: bt = Breakdown_CasterLevel_Spell_Positive; break;
    case Damage_Sonic: bt = Breakdown_CasterLevel_Spell_Sonic; break;
    case Damage_Ceruleite: bt = Breakdown_CasterLevel_Ceruleite; break;
    case Damage_Crimsonite: bt = Breakdown_CasterLevel_Crimsonite; break;
    case Damage_Gildleaf: bt = Breakdown_CasterLevel_Gildleaf; break;
    }
    return bt;
}

const Augment& FindAugmentByName(const std::string& name, const Item* pItem)
{
    static Augment badAugment;
    // items can have custom augments, check those first for any augments slots
    // this item may have
    bool bFound = false;
    if (pItem != NULL)
    {
        const std::vector<ItemAugment>& itemAugments = pItem->Augments();
        for (size_t i = 0; !bFound && i < itemAugments.size(); ++i)
        {
            const std::list<Augment>& augments = itemAugments[i].ItemSpecificAugments();
            for (auto&& it : augments)
            {
                if (it.Name() == name)
                {
                    bFound = true;
                    return it;
                }
            }
        }
    }
    // if its not an item specific augment, look through the general augment list
    const std::list<Augment> & augments = Augments();
    std::list<Augment>::const_iterator it = augments.begin();
    while (it != augments.end())
    {
        if ((*it).Name() == name)
        {
            return (*it);
        }
        ++it;
    }
    return badAugment;
}

const Filigree& FindFiligreeByName(const std::string& name)
{
    static Filigree badFiligree;
    const std::list<Filigree>& filigrees = Filigrees();
    std::list<Filigree>::const_iterator it = filigrees.begin();
    while (it != filigrees.end())
    {
        if ((*it).Name() == name)
        {
            return (*it);
        }
        ++it;
    }
    return badFiligree;
}

const Gem& FindSentientGemByName(const std::string& name)
{
    static Gem badGem;
    const std::list<Gem>& gems = SentientGems();
    std::list<Gem>::const_iterator it = gems.begin();
    while (it != gems.end())
    {
        if ((*it).Name() == name)
        {
            return (*it);
        }
        ++it;
    }
    return badGem;
}


std::list<Augment> CompatibleAugments(const std::string& name)
{
    std::list<Augment> compatibleAugments;
    const std::list<Augment> & augments = Augments();
    std::list<Augment>::const_iterator it = augments.begin();
    while (it != augments.end())
    {
        if ((*it).IsCompatibleWithSlot(name)
                || ((*it).Name() == " No Augment"))
        {
            compatibleAugments.push_back((*it));
        }
        ++it;
    }
    return compatibleAugments;
}

const SetBonus& FindSetBonus(const std::string& name)
{
    static SetBonus badSetBonus;
    const std::list<SetBonus> & sets = SetBonuses();
    std::list<SetBonus>::const_iterator it = sets.begin();
    while (it != sets.end())
    {
        if ((*it).Type() == name)
        {
            return (*it);
        }
        ++it;
    }
    return badSetBonus;
}

bool AddMenuItem(
        HMENU hTargetMenu, 
        const CString& itemText, 
        UINT itemID,
        bool bEnabled)
{
    bool bSuccess = false;

    ASSERT(itemText.GetLength() > 0);
    ASSERT(hTargetMenu != NULL);

    // first, does the menu item have
    // any required submenus to be found/created?
    if (itemText.Find('\\') >= 0)
    {
        // yes, we need to do a recursive call
        // on a submenu handle and with that sub
        // menu name removed from itemText

        // 1:get the popup menu name
        CString popupMenuName = itemText.Left(itemText.Find('\\'));

        // 2:get the rest of the menu item name
        // minus the delimiting '\' character
        CString remainingText = itemText.Right(itemText.GetLength() - popupMenuName.GetLength() - 1);

        // 3:See whether the popup menu already exists
        int itemCount = ::GetMenuItemCount(hTargetMenu);
        bool bFoundSubMenu = false;
        MENUITEMINFO menuItemInfo;

        memset(&menuItemInfo, 0, sizeof(MENUITEMINFO));
        menuItemInfo.cbSize = sizeof(MENUITEMINFO);
        menuItemInfo.fMask = MIIM_TYPE | MIIM_STATE | MIIM_ID | MIIM_SUBMENU;
        for (int itemIndex = 0; itemIndex < itemCount && !bFoundSubMenu; ++itemIndex)
        {
            ::GetMenuItemInfo(
                    hTargetMenu, 
                    itemIndex, 
                    TRUE, 
                    &menuItemInfo);
            if (menuItemInfo.hSubMenu != 0)
            {
                // this menu item is a popup menu (non popups give 0)
                TCHAR    buffer[MAX_PATH];
                ::GetMenuString(
                        hTargetMenu, 
                        itemIndex, 
                        buffer, 
                        MAX_PATH, 
                        MF_BYPOSITION);
                if (popupMenuName == buffer)
                {
                    // this is the popup menu we have to add to
                    bFoundSubMenu = true;
                }
            }
        }
        if (remainingText != "")
        {
            // 4: If exists, do recursive call,
            // else create do recursive call
            // and then insert it
            if (bFoundSubMenu)
            {
                bSuccess = AddMenuItem(
                        menuItemInfo.hSubMenu, 
                        remainingText, 
                        itemID,
                        bEnabled);
            }
            else
            {
                // we need to create a new sub menu and insert it
                HMENU hPopupMenu = ::CreatePopupMenu();
                if (hPopupMenu != NULL)
                {
                    bSuccess = AddMenuItem(
                            hPopupMenu, 
                            remainingText, 
                            itemID,
                            bEnabled);
                    if (bSuccess)
                    {
                        if (::AppendMenu(
                                hTargetMenu, 
                                MF_POPUP, 
                                (UINT)hPopupMenu, 
                                popupMenuName) > 0)
                        {
                            bSuccess = true;
                            // hPopupMenu now owned by hTargetMenu,
                            // we do not need to destroy it
                        }
                        else
                        {
                            // failed to insert the popup menu
                            bSuccess = false;
                            // stop a resource leak
                            ::DestroyMenu(hPopupMenu);
                        }
                    }
                }
            }
        }
    }
    else
    {
        // no sub menus required, add this item to this HMENU
        // item ID of 0 means we are adding a separator
        if (itemID != 0)
        {
            // its a normal menu command
            if (::AppendMenu(
                    hTargetMenu,
                    bEnabled? MF_BYCOMMAND : MF_BYCOMMAND | MF_GRAYED,
                    itemID, 
                    itemText) > 0)
            {
                // we successfully added the item to the menu
                bSuccess = true;
            }
        }
        else
        {
            // we are inserting a separator
            if (::AppendMenu(
                    hTargetMenu, 
                    MF_SEPARATOR, 
                    itemID, 
                    itemText) > 0)
            {
                // we successfully added the separator to the menu
                bSuccess = true;
            }
        }
    }

    return bSuccess;
}

int FindItemIndexByItemData(
    CListCtrl* pControl,
    DWORD itemData)
{
    // look through each items item data in the control
    // and return the index of the item that matches what we are looking for
    int index = CB_ERR;     // assume fail
    size_t count = pControl->GetItemCount();
    for (size_t ii = 0; ii < count; ++ii)
    {
        DWORD item = pControl->GetItemData(ii);
        if (item == itemData)
        {
            // found it
            index = ii;
            break;      // no need to keep looking
        }
    }
    return index;
}

bool LimitToRuneArm(Build* pBuild)
{
    bool bLimitToRuneArm = false;
    if (pBuild != NULL
        && (pBuild->IsFeatTrained("Artificer Rune Arm Use")
            || pBuild->IsGrantedFeat("Artificer Rune Arm Use")))
    {
        Item item = pBuild->ActiveGearSet().MainHand();
        switch (item.Weapon())
        {
        case Weapon_GreatCrossbow:
        case Weapon_HeavyCrossbow:
        case Weapon_LightCrossbow:
        case Weapon_RepeatingHeavyCrossbow:
        case Weapon_RepeatingLightCrossbow:
            bLimitToRuneArm = true;
            break;
        }
    }
    return bLimitToRuneArm;
}

bool CanEquipTo2ndWeapon(
    Build* pBuild,
    const Item& item)
{
    bool bAllowRuneArm = false;
    if (pBuild != NULL
        && (pBuild->IsFeatTrained("Artificer Rune Arm Use")
            || pBuild->IsGrantedFeat("Artificer Rune Arm Use")))
    {
        bAllowRuneArm = true;
    }
    // return true if this item allows an item to be equipped in off hand slot
    bool canEquip = true;   // assume we can
    switch (item.Weapon())
    {
        // only need weapon types that preclude off hand in this list
        // TBD: Verify the list of weapons here
    case Weapon_Falchion:
    case Weapon_GreatAxe:
    case Weapon_GreatClub:
    case Weapon_GreatSword:
    case Weapon_Handwraps:
    case Weapon_Longbow:
    case Weapon_Maul:
    case Weapon_Quarterstaff:
    case Weapon_Shortbow:
        canEquip = false;
        break;
    case Weapon_GreatCrossbow:
    case Weapon_HeavyCrossbow:
    case Weapon_LightCrossbow:
    case Weapon_RepeatingHeavyCrossbow:
    case Weapon_RepeatingLightCrossbow:
        canEquip = bAllowRuneArm;
        break;
    }
    return canEquip;
}

void BreakUpLongLines(CString& line)
{
    if (line.Find("\n") < 0)
    {
        size_t length = line.GetLength();
        size_t pos = 75;        // assume 7 characters in
        while (pos < length)
        {
            // look for the first space character from pos onwards
            if (line.GetAt(pos) == ' ')
            {
                // change the space to a "\n"
                line.SetAt(pos, '\n');
                pos += 75;          // move a standard line length from there
            }
            else
            {
                ++pos;
            }
        }
        line.Replace("\n", "\r\n");
    }
}
