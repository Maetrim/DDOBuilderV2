#include "stdafx.h"
#include "WikiItemFileProcessor.h"

#include "GlobalSupportFunctions.h"
#include "LogPane.h"
#include "MainFrm.h"
#include <deque>
#include "XmlLib\SaxReader.h"
#include "Race.h"
#include "Bonus.h"
#include "SetBonus.h"
#include "ItemAugment.h"
#include "Class.h"
#include "SpellSchoolTypes.h"
#include "EnergyTypes.h"

namespace
{
    const XmlLib::SaxString f_saxElementName = L"Items"; // root element name to look for
}

WikiItemFileProcessor::WikiItemFileProcessor() :
    SaxContentElement(f_saxElementName)
{
    // get the path location for wget
    char fullPath[MAX_PATH];
    ::GetModuleFileName(NULL, fullPath, MAX_PATH);

    char drive[_MAX_DRIVE];
    char folder[_MAX_PATH];
    _splitpath_s(fullPath, drive, _MAX_DRIVE, folder, _MAX_PATH, NULL, 0, NULL, 0);

    char path[_MAX_PATH];
    _makepath_s(path, _MAX_PATH, drive, folder, NULL, NULL);

    m_sourcePath = path;
    m_sourcePath += "www.ddowiki.com\\page";
    m_destinationPath = path;
    m_destinationPath += "DataFiles\\Items";
}

WikiItemFileProcessor::~WikiItemFileProcessor()
{
}

void WikiItemFileProcessor::Start()
{
    CString text;
    text.Format("Processing files in \"%s\"", m_sourcePath.c_str());
    GetLog().AddLogEntry(text);

    int fileCount = 0;
    std::string fileFilter = m_sourcePath;
    fileFilter += "\\*.*";

    // read all the item files found in the Items sub-directory
    // first enumerate each file and load it
    WIN32_FIND_DATA findFileData;
    HANDLE hFind = FindFirstFile(fileFilter.c_str(), &findFileData);
    if (hFind != INVALID_HANDLE_VALUE)
    {
        std::string fullFilename = m_sourcePath;
        fullFilename += "\\";
        fullFilename += findFileData.cFileName;
        ProcessFile(fileCount++, fullFilename);
        while (FindNextFile(hFind, &findFileData))
        {
            fullFilename = m_sourcePath;
            fullFilename += "\\";
            fullFilename += findFileData.cFileName;
            ProcessFile(fileCount++, fullFilename);
            // allow UI to function
            MSG msg;
            while (::PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
            {
                AfxGetThread()->PumpMessage();
            }
        }
        FindClose(hFind);
    }

    text.Format("Processing complete, %d files done", fileCount);
    GetLog().AddLogEntry(text);
}

void WikiItemFileProcessor::ProcessFile(int fileIndex, const std::string& filename)
{
    UNREFERENCED_PARAMETER(fileIndex);
    CFile file;
    if (file.Open(filename.c_str(), CFile::modeRead))
    {
        // load the file into a data buffer
        ULONGLONG filesize = file.GetLength();
        char* pBuffer = new char[static_cast<unsigned int>(filesize + 1)];
        memset(pBuffer, 0, static_cast<size_t>(filesize + 1));
        file.Read(pBuffer, static_cast<unsigned int>(filesize));
        file.Close();
        std::string fileContent = pBuffer;
        // clean up
        delete[]pBuffer;
        bool bMinorArtefact = fileContent.find("Minor artifacts") != std::string::npos;
        // Extract the item fields from the file first
        std::map<std::string, std::string> itemFields;
        ExtractFields(itemFields, fileContent);
        ExtractEnchantmentsText(itemFields, fileContent);
        CreateItem(itemFields, bMinorArtefact);
    }
    else
    {
        CString text;
        text.Format("Error - Filed to open file \"%s\"", filename.c_str());
        GetLog().AddLogEntry(text);
    }
}

void WikiItemFileProcessor::ExtractFields(
    std::map<std::string, std::string>& itemFields,
    const std::string& fileData)
{
    static std::string fieldsToFind[] =
    {
        // "Field name", "search item", "start marker", "end marker"
        "Accept Sentience",         ">Accepts Sentience?",          "\">",      "</td>",
        "Arcane Spell Failure",     ">Arcane Spell Failure",        "\">",      "<",
        "Armor Bonus",              ">Armor Bonus",                 "\">",      "<",
        "Armor Bonus AB",           "Adamantine Body:",             "<b>",      "</b>",
        "Armor Bonus MB",           "Mithral Body:",                "<b>",      "</b>",
        "Armor Bonus CP",           "Composite Plating:",           "<b>",      "</b>",
        "Armor Check Penalty",      ">Armor Check Penalty",         "\">",      "<",
        "Armor Type",               "Armor Type",                   "\">",      "<",
        "Attack Mod",               ">Attack Mod",                  "\">",      "<",
        "Critical Roll",            ">Critical Roll",               "\">",      "<",
        "Critical Roll",            ">Critical threat range",       "\">",      "</td>",
        "Damage",                   ">Damage<",                     "\">",      "<",
        "Damage",                   ">Damage and Type",             "\">",      "</td>",
        "Damage Mod",               ">Damage Mod",                  "\">",      "<",
        "Damage Reduction",         ">Damage Reduction",            "\">",      "<",
        "Description",              ">Description\n",               "<i>",      "</i>",
        "Drop Location",            ">Location",                    "\">",      "</td>",
        "Item Type",                ">Item Type",                   "\">",      "<",
        "Name",                     "firstHeading",                 ">",        "<",
        "Material",                 ">Material",                    "title=\"", "\"",
        "Maximum Dexterity Bonus",  ">Maximum Dexterity Bonus\n",   "\">",      "<",
        "Maximum Dexterity Bonus",  ">Max Dex Bonus\n",             "\">",      "<",
        "Minimum Level",            ">Minimum level\n",             "\">",      "<",
        "Minimum Level",            ">Minimum Level\n",             "\">",      "<",
        "Proficiency",              ">Proficiency\n",               "title=\"", "\"",
        "Proficiency",              ">Proficiency Class",           "\">",      "<",
        "Proficiency",              ">Feat Requirement",            "\">",      "<",
        "Race Absolutely Required", ">Race Absolutely Required\n",  "\">",      "</td>",
        "Required Trait",           ">Required Trait\n",            "\">",      "</td>",
        "Shield Bonus",             ">Shield Bonus",                "\">",      "<",
        "Shield Type",              ">Shield Type",                 "\">",      "<",
        "Slot",                     ">Slot",                        "\">",      "<",
        "Weapon Type",              ">Weapon Type\n",               "\">",      "</td>"
    };
    size_t count = sizeof(fieldsToFind) / sizeof(std::string);
    if (count % 4 != 0)
    {
        throw "Must be an multiple of 4";
    }
    for (size_t i = 0; i < count; i += 4)
    {
        std::string field = ExtractSingleField(fieldsToFind[i+1], fieldsToFind[i+2], fieldsToFind[i+3], fileData);
        if (!field.empty())
        {
            itemFields[fieldsToFind[i]] = field;
        }
    }
}

std::string WikiItemFileProcessor::ExtractSingleField(
        const std::string& field,
        const std::string& startMarker,
        const std::string& endMarker,
        const std::string& fileData)
{
    std::string fieldEntry;

    size_t pos = fileData.find(field, 0);
    if (pos != std::string::npos)
    {
        size_t start = fileData.find(startMarker, pos);
        size_t end = fileData.find(endMarker, start + startMarker.size());
        if (start != std::string::npos
            && end != std::string::npos)
        {
            // we have a field, extract it
            fieldEntry = fileData.substr(start + startMarker.size(), end - start - startMarker.size());
            ReplaceRequiredCharacters(fieldEntry);
            RemoveLinks(fieldEntry);
        }
    }

    return fieldEntry;
}

void WikiItemFileProcessor::ReplaceRequiredCharacters(std::string& field)
{
    static std::string badFields[] =
    {
        // what to find, what to replace with (in pairs)
        "Item:",    "",
        "\r",       "",
        "\n",       "" ,
        "&#39;",    "'",
        "&#32;",    " ",
        "&#160;",    " ",
        "&#8594;",  "->",
        "</a>",     " ",
        "</span>",  "",
        "<b>",      "",
        "</b>",     "",
        "</th>",    "",
        "<ul>",     "",
        "</ul>",    "",
        "<dl>",     "",
        "</dl>",    "",
        "<dd>",     "",
        "</dd>",    "",
        "</div>",   "",
        "<i>",      "",
        "</i>",     "",
        "<p>",      "",
        "</p>",     "",
        "<u>",      "",
        "</u>",     "",
        "<li>",     "",
        "<small>",  "",
        "</small>", "",
        "<tr>",     "",
        "</tr>",    "",
        "</li>",    "\n",
        " ,",       ",",
        "<br />",   "\r\n"              // must be after earlier "\r" and "\n"
    };
    size_t count = sizeof(badFields) / sizeof(std::string);
    if (count % 2 != 0)
    {
        throw "Must be an even number";
    }
    for (size_t i = 0; i < count; i += 2)
    {
        size_t pos = field.find(badFields[i]);
        while (pos != std::string::npos)
        {
            field.replace(pos, badFields[i].size(), badFields[i + 1]);
            pos = field.find(badFields[i]);
        }
    }
}

void WikiItemFileProcessor::RemoveLinks(std::string& field)
{
    static std::string badFields[] =
    {
        // what to find, end tag
        "<a ",                  ">",
        "<span",                ">",
        "<td",                  ">",
        "<th",                  ">",
        "<tr",                  ">",
        "<hr",                  ">",
        "<li",                  ">",
        "<img",                 ">",
        "<div",                 ">",
        "Mythic Armor Boost",   ")",
        "Mythic Armor Boost",   "\n",
        "Mythic Weapon Boost",  ")",
        "Mythic Wrist Boost",   ")",
        "Mythic Trinket Boost", ")",
        "Mythic Ring Boost",    ")",
        "Mythic Neck Boost",    ")",
        "Mythic Goggle Boost",  ")",
        "Mythic Belt Boost",    ")",
        "Mythic Shield Boost",  ")",
        "Mythic Head Boost",    ")",
        "Mythic Boot Boost",    ")",
        "Mythic Hands Boost",   ")",
        "Mythic Cloak Boost",   ")",
        "StarterStarter",       "jewels.",
        "Blue AugmentsNamed",   "+5 to +20",
        "Red AugmentsNamed",    "+5 to +20",
        "Colorless AugmentsNamed", "+5 to +20",
        "Green AugmentsNamed",  "+5 to +20",
        "Orange AugmentsNamed", "+5 to +20",
        "Purple AugmentsNamed", "+5 to +20",
        "Yellow AugmentsNamed", "+5 to +20",
        "2 Pieces",             "\n",
        "3 Pieces",             "\n",
        "4 Pieces",             "\n",
        "5 Pieces",             "\n",
        "6 Pieces",             "\n",
        "7 Pieces",             "\n",
        "8 Pieces",             "\n",
        "9 Pieces",             "\n"
    };
    size_t count = sizeof(badFields) / sizeof(std::string);
    if (count % 2 != 0)
    {
        throw "Must be an even number";
    }
    for (size_t i = 0; i < count; i += 2)
    {
        size_t pos = field.find(badFields[i]);
        while (pos != std::string::npos)
        {
            size_t end = field.find(badFields[i+1], pos);
            if (end != std::string::npos)
            {
                field.replace(pos, end - pos + badFields[i + 1].size(), "");
                pos = field.find(badFields[i]);
            }
            else
            {
                break;
            }
        }
    }
    // clear any trailing '\n characters
    while (field.size() > 0 && field.back() == '\n')
    {
        field.pop_back();
    }
    // also replace all "\n\n" with "\n"
    size_t pos = field.find("\n\n");
    while (pos != std::string::npos)
    {
        field.replace(pos, 2, "\n");
        pos = field.find("\n\n");
    }
    // also replace all "  " with " "
    pos = field.find("  ");
    while (pos != std::string::npos)
    {
        field.replace(pos, 2, " ");
        pos = field.find("  ");
    }
}

void WikiItemFileProcessor::ExtractEnchantmentsText(
        std::map<std::string, std::string>& itemFields,
        const std::string& fileData)
{
    std::string enchantmentsText;
    enchantmentsText = ExtractSingleField(">Enchantments", ">Enchantments", "</td>", fileData);
    if (!enchantmentsText.empty())
    {
        itemFields["Enchantments"] = enchantmentsText;
    }
}

XmlLib::SaxContentElementInterface* WikiItemFileProcessor::StartElement(
    const XmlLib::SaxString& name,
    const XmlLib::SaxAttributes& attributes)
{
    XmlLib::SaxContentElementInterface* subHandler =
        SaxContentElement::StartElement(name, attributes);
    if (subHandler == NULL)
    {
        Item item;
        if (m_item.SaxElementIsSelf(name, attributes))
        {
            subHandler = &m_item;
        }
    }

    return subHandler;
}

void WikiItemFileProcessor::EndElement()
{
    SaxContentElement::EndElement();
}

void WikiItemFileProcessor::CreateItem(const std::map<std::string, std::string>& itemFields, bool bMinorArtifact)
{
    // it's not an item if it doesn't have a name
    if (itemFields.find("Name") != itemFields.end())
    {
        std::string name = itemFields.at("Name");
        CString text;
        text.Format("Item \"%s\"", name.c_str());

        m_item = Item();
        // this item has a name, lets make its filename and load the
        // existing item if it exists
        std::string itemFilename(m_destinationPath);
        itemFilename += "\\";
        itemFilename += name;
        itemFilename += ".item";

        // set up a reader with this as the expected root node
        XmlLib::SaxReader reader(this, f_saxElementName);
        // read in the xml from a file (fully qualified path)
        reader.Open(itemFilename);    // reads if file is present
        // some item files have to be manually set up. In such cases the file has
        // been marked to never be auto updated by this procedure. Skip these files
        if (!m_item.HasNoAutoUpdate())
        {
            GetLog().AddLogEntry(text);
            // clear any existing effects from load action to avoid carry over after recreated
            m_item.m_Buffs.clear();
            m_item.m_SetBonus.clear();
            m_item.m_Augments.clear();
            m_item.m_SlotUpgrades.clear();
            m_item.m_AttackModifier.clear();
            m_item.m_DamageModifier.clear();
            m_item.m_DRBypass.clear();
            m_item.Clear_IsGreensteel();
            m_item.Clear_MinorArtifact();
            m_item.Clear_RequirementsToUse();
            m_item.m_RequirementsToUse = Requirements();

            // now update the item
            m_item.Set_Name(name);
            int minLevel = 1;
            if (itemFields.find("Minimum Level") != itemFields.end())
            {
                std::string levelText = itemFields.at("Minimum Level");
                if (levelText == "None")
                {
                    minLevel = 1;
                }
                else
                {
                    minLevel = atoi(levelText.c_str());
                }
            }
            m_item.Set_MinLevel(minLevel);
            // set the item slot type.
            if (false == SetItemSlot(itemFields))
            {
                // no equipment slot for this item, its not real
                ::DeleteFile(itemFilename.c_str());
                return;
            }

            if (itemFields.find("Drop Location") != itemFields.end())
            {
                m_item.Set_DropLocation(itemFields.at("Drop Location"));
            }
            if (itemFields.find("Description") != itemFields.end())
            {
                text = itemFields.at("Description").c_str();
                BreakUpLongLines(text);
                m_item.Set_Description((LPCTSTR)text);
            }
            if (itemFields.find("Accept Sentience") != itemFields.end())
            {
                std::string as = itemFields.at("Accept Sentience");
                if (as.find("Yes") != std::string::npos)
                {
                    m_item.Set_IsAcceptsSentience();
                }
            }
            if (bMinorArtifact)
            {
                m_item.Set_MinorArtifact();
            }
            if (itemFields.find("Race Absolutely Required") != itemFields.end())
            {
                // note, must be done after SetItemSlot
                // as that can set some requirements also
                std::string race = itemFields.at("Race Absolutely Required");
                if (race.find("None") == std::string::npos
                        && race.find("Warforged") == std::string::npos)
                {
                    m_item.AddRaceRequirement(race);
                }
            }

            AddArmorFields(itemFields);
            AddAttackMods(itemFields);
            AddDamageMods(itemFields);
            AddDamageDice(itemFields);
            AddCriticalThreatAndMultiplier(itemFields);
            AddItemEffects(itemFields);

            try
            {
                XmlLib::SaxWriter writer;
                writer.Open(itemFilename);
                writer.StartDocument(f_saxElementName);
                m_item.Write(&writer);
                writer.EndDocument();
            }
            catch (const std::exception& e)
            {
                std::string errorMessage = e.what();
                CString err;
                err.Format("...Failed to save item. Error \"%s\"", errorMessage.c_str());
                text += err;
            }
        }
        else
        {
            text += " ----- Not auto updated";
            GetLog().AddLogEntry(text);
        }
#ifdef _DEBUG
        // debug build also dumps the itemFields extracted to a file of the same name
        // but a different file extension
        std::string debugFilename(m_destinationPath);
        debugFilename += "\\";
        debugFilename += name;
        debugFilename += ".effects.txt";
        CFile debugFile;
        if (debugFile.Open(debugFilename.c_str(), CFile::modeCreate | CFile::modeWrite))
        {
            for (auto&& it : itemFields)
            {
                text.Format("Field \"%s\"=\"%s\"\r\n", it.first.c_str(), it.second.c_str());
                debugFile.Write((LPCTSTR)text, text.GetLength());
            }
            debugFile.Close();
        }
#endif
    }
}

bool WikiItemFileProcessor::SetItemSlot(const std::map<std::string, std::string>& itemFields)
{
    bool bRealItem = false;
    EquipmentSlot slot(L"EquipmentSlot");
    if (itemFields.find("Slot") != itemFields.end())
    {
        std::string slotType = itemFields.at("Slot");
        if (slotType == "Back")
        {
            slot.SetValue_Cloak(true);
            bRealItem = true;
        }
        else if (slotType == "Trinket")
        {
            slot.SetValue_Trinket(true);
            bRealItem = true;
        }
        else if (slotType == "Wrist")
        {
            slot.SetValue_Bracers(true);
            bRealItem = true;
        }
        else if (slotType == "Head")
        {
            slot.SetValue_Helmet(true);
            bRealItem = true;
        }
        else if (slotType == "Finger")
        {
            slot.SetValue_Ring(true);
            bRealItem = true;
        }
        else if (slotType == "Waist")
        {
            slot.SetValue_Belt(true);
            bRealItem = true;
        }
        else if (slotType == "Eye")
        {
            slot.SetValue_Goggles(true);
            bRealItem = true;
        }
        else if (slotType == "Neck")
        {
            slot.SetValue_Necklace(true);
            bRealItem = true;
        }
        else if (slotType == "Hand")
        {
            slot.SetValue_Gloves(true);
            bRealItem = true;
        }
        else if (slotType == "Feet")
        {
            slot.SetValue_Boots(true);
            bRealItem = true;
        }
        else if (slotType == "Cosmetic Cloak")
        {
            slot.SetValue_CosmeticCloak(true);
            bRealItem = true;
            m_item.Set_MinLevel(0);
        }
        else if (slotType == "Cosmetic Headwear")
        {
            slot.SetValue_CosmeticHelm(true);
            bRealItem = true;
            m_item.Set_MinLevel(0);
        }
        else
        {
            CString text;
            text.Format("...Slot type \"%s\" not recognised", slotType.c_str());
            GetLog().AddLogEntry(text);
        }
    }
    else if (itemFields.find("Weapon Type") != itemFields.end())
    {
        std::string weaponType = itemFields.at("Weapon Type");
        slot.SetValue_Weapon1(true);
        // remove the damage type from the string
        size_t pos = weaponType.find(" / ");
        if (pos != std::string::npos)
        {
            weaponType = weaponType.substr(0, pos);
        }
        WeaponType wt = TextToEnumEntry(weaponType, weaponTypeMap, false);
        switch (wt)
        {
            case Weapon_Dart:
            case Weapon_Falchion:
            case Weapon_GreatAxe:
            case Weapon_GreatClub:
            case Weapon_GreatCrossbow:
            case Weapon_GreatSword:
            case Weapon_Handwraps:
            case Weapon_HeavyCrossbow:
            case Weapon_LightCrossbow:
            case Weapon_Longbow:
            case Weapon_Maul:
            case Weapon_Quarterstaff:
            case Weapon_RepeatingHeavyCrossbow:
            case Weapon_RepeatingLightCrossbow:
            case Weapon_Shortbow:
            case Weapon_Shuriken:
            case Weapon_ThrowingAxe:
            case Weapon_ThrowingDagger:
            case Weapon_ThrowingHammer:
            case Weapon_Unknown:
                break;
            default:
                // all other weapon types can go in the offhand
                slot.SetValue_Weapon2(true);
                break;
        }
        m_item.Set_Weapon(wt);
        bRealItem = true;
    }
    else if (itemFields.find("Armor Type") != itemFields.end())
    {
        bool bAddNonConstructReq = false;
        std::string armorType = itemFields.at("Armor Type");
        if (armorType == "Clothing"
            || armorType == "Outfit"
            || armorType == "Rags"
            || armorType == "Starter Rags"
            || armorType == "Robe")
        {
            slot.SetValue_Armor(true);
            m_item.Set_Armor(Armor_Cloth);
            bAddNonConstructReq = true;
            bRealItem = true;
        }
        else if (armorType == "Light Armor"
            || armorType == "Leather Armor"
            || armorType == "Studded Leather"
            || armorType == "Leather"
            || armorType == "Light"
            || armorType == "Light Armor/Collar"
            || armorType == "Chainmail"
            || armorType == "Padded"
            || armorType == "Chain Shirt")
        {
            slot.SetValue_Armor(true);
            m_item.Set_Armor(Armor_Light);
            bAddNonConstructReq = true;
            bRealItem = true;
        }
        else if (armorType == "Medium Armor"
            || armorType == "Medium"
            || armorType == "Hide"
            || armorType == "Scalemail"
            || armorType == "Breastplate"
            || armorType == "Brigandine"
            || armorType == "Breastplate / Scalemail")
        {
            slot.SetValue_Armor(true);
            m_item.Set_Armor(Armor_Medium);
            bAddNonConstructReq = true;
            bRealItem = true;
        }
        else if (armorType == "Heavy Armor"
            || armorType == "Heavy"
            || armorType == "Plate"
            || armorType == "Splint Mail"
            || armorType == "Banded Mail"
            || armorType == "Half Plate"
            || armorType == "Platemail"
            || armorType == "Fullplate"
            || armorType == "Full Plate")
        {
            slot.SetValue_Armor(true);
            m_item.Set_Armor(Armor_Heavy);
            bAddNonConstructReq = true;
            bRealItem = true;
        }
        else if (armorType == "Docent")
        {
            slot.SetValue_Armor(true);
            m_item.Set_Armor(Armor_Docent);
            RequiresOneOf roo;
            const std::list<Race>& races = Races();
            for (auto&& rit : races)
            {
                if (rit.HasIsConstruct())
                {
                    Requirement raceRequirement(Requirement_Race, rit.Name(), 1);
                    roo.AddRequirement(raceRequirement);
                }
            }
            Requirements req;
            req.AddRequiresOneOf(roo);
            m_item.Set_RequirementsToUse(req);
            bRealItem = true;
        }
        else if (armorType == "Cosmetic Armor")
        {
            slot.SetValue_CosmeticArmor(true);
            bRealItem = true;
            m_item.Set_MinLevel(0);
        }
        else
        {
            CString text;
            text.Format("...Armor type \"%s\" not recognised", armorType.c_str());
            GetLog().AddLogEntry(text);
        }
        if (bAddNonConstructReq)
        {
            RequiresNoneOf rno;
            const std::list<Race>& races = Races();
            for (auto&& rit : races)
            {
                if (rit.HasIsConstruct())
                {
                    Requirement raceRequirement(Requirement_Race, rit.Name(), 1);
                    rno.AddRequirement(raceRequirement);
                }
            }
            Requirements req;
            req.AddRequiresNoneOf(rno);
            m_item.Set_RequirementsToUse(req);
        }
    }
    else if (itemFields.find("Shield Type") != itemFields.end())
    {
        std::string shieldType = itemFields.at("Shield Type");
        slot.SetValue_Weapon2(true);
        bRealItem = true;
        if (shieldType == "Buckler")
        {
            m_item.Set_Weapon(Weapon_ShieldBuckler);
        }
        else if (shieldType == "Small Shield")
        {
            m_item.Set_Weapon(Weapon_ShieldSmall);
        }
        else if (shieldType == "Large Shield")
        {
            m_item.Set_Weapon(Weapon_ShieldLarge);
        }
        else if (shieldType == "Tower Shield")
        {
            m_item.Set_Weapon(Weapon_ShieldTower);
        }
        else if (shieldType == "Orb")
        {
            m_item.Set_Weapon(Weapon_Orb);
        }
        else
        {
            CString text;
            text.Format("...Shield type \"%s\" not recognised", shieldType.c_str());
            GetLog().AddLogEntry(text);
        }
    }
    else if (itemFields.find("Required Trait") != itemFields.end())
    {
        std::string trait = itemFields.at("Required Trait");
        if (trait.find("Artificer Rune Arm") != std::string::npos)
        {
            m_item.Set_Weapon(Weapon_RuneArm);
            slot.SetValue_Weapon2(true);
        }
        bRealItem = true;
    }
    else if (itemFields.find("Item Type") != itemFields.end())
    {
        std::string it = itemFields.at("Item Type");
        if (it.find("Quiver") != std::string::npos)
        {
            slot.SetValue_Quiver(true);
            bRealItem = true;
        }
    }
    m_item.Set_Slots(slot);
    return bRealItem;
}

void WikiItemFileProcessor::AddArmorFields(const std::map<std::string, std::string>& itemFields)
{
    if (itemFields.find("Armor Bonus") != itemFields.end())
    {
        std::string field = itemFields.at("Armor Bonus");
        size_t armor = atoi(field.c_str());
        m_item.Set_ArmorBonus(armor);
    }
    if (itemFields.find("Armor Bonus CP") != itemFields.end())
    {
        std::string field = itemFields.at("Armor Bonus CP");
        size_t armor = atoi(field.c_str());
        m_item.Set_ArmorBonus(armor);
    }
    if (itemFields.find("Armor Bonus MB") != itemFields.end())
    {
        std::string field = itemFields.at("Armor Bonus MB");
        size_t armor = atoi(field.c_str());
        m_item.Set_MithralBody(armor);
    }
    if (itemFields.find("Armor Bonus AB") != itemFields.end())
    {
        std::string field = itemFields.at("Armor Bonus AB");
        size_t armor = atoi(field.c_str());
        m_item.Set_AdamantineBody(armor);
    }
    if (itemFields.find("Maximum Dexterity Bonus") != itemFields.end())
    {
        std::string field = itemFields.at("Maximum Dexterity Bonus");
        size_t mdb = atoi(field.c_str());
        m_item.Set_MaximumDexterityBonus(mdb);
    }
    if (itemFields.find("Armor Check Penalty") != itemFields.end())
    {
        std::string field = itemFields.at("Armor Check Penalty");
        size_t acp = atoi(field.c_str());
        m_item.Set_ArmorCheckPenalty(acp);
    }
    if (itemFields.find("Arcane Spell Failure") != itemFields.end())
    {
        std::string field = itemFields.at("Arcane Spell Failure");
        size_t asf = atoi(field.c_str());
        m_item.Set_ArcaneSpellFailure(asf);
    }
}

void WikiItemFileProcessor::AddAttackMods(const std::map<std::string, std::string>& itemFields)
{
    if (itemFields.find("Attack Mod") != itemFields.end())
    {
        std::list<AbilityType> types;
        std::string attackMods = itemFields.at("Attack Mod");
        size_t pos;
        pos = attackMods.find("STR"); if (pos != std::string::npos) types.push_back(Ability_Strength);
        pos = attackMods.find("INT"); if (pos != std::string::npos) types.push_back(Ability_Intelligence);
        pos = attackMods.find("WIS"); if (pos != std::string::npos) types.push_back(Ability_Wisdom);
        pos = attackMods.find("DEX"); if (pos != std::string::npos) types.push_back(Ability_Dexterity);
        pos = attackMods.find("CON"); if (pos != std::string::npos) types.push_back(Ability_Constitution);
        pos = attackMods.find("CHA"); if (pos != std::string::npos) types.push_back(Ability_Charisma);
        if (types.size() > 0)
        {
            m_item.Set_AttackModifier(types);
        }
    }
}

void WikiItemFileProcessor::AddDamageMods(const std::map<std::string, std::string>& itemFields)
{
    if (itemFields.find("Damage Mod") != itemFields.end())
    {
        std::list<AbilityType> types;
        std::string damageMods = itemFields.at("Damage Mod");
        size_t pos;
        pos = damageMods.find("STR"); if (pos != std::string::npos) types.push_back(Ability_Strength);
        pos = damageMods.find("INT"); if (pos != std::string::npos) types.push_back(Ability_Intelligence);
        pos = damageMods.find("WIS"); if (pos != std::string::npos) types.push_back(Ability_Wisdom);
        pos = damageMods.find("DEX"); if (pos != std::string::npos) types.push_back(Ability_Dexterity);
        pos = damageMods.find("CON"); if (pos != std::string::npos) types.push_back(Ability_Constitution);
        pos = damageMods.find("CHA"); if (pos != std::string::npos) types.push_back(Ability_Charisma);
        if (types.size() > 0)
        {
            m_item.Set_DamageModifier(types);
        }
    }
}

void WikiItemFileProcessor::AddDamageDice(const std::map<std::string, std::string>& itemFields)
{
    if (itemFields.find("Damage") != itemFields.end())
    {
        // w[xdy+z]+n <damage type> <damage type> <damage type>
        std::string damageDice = itemFields.at("Damage");
        // parse the string and work out all the damage types
        std::list<DamageReductionType> typesPresent;
        for (size_t i = DR_Unknown + 1; i < DR_Count; ++i)
        {
            CString type = EnumEntryText((DamageReductionType)i, drTypeMap);
            size_t pos = damageDice.find((LPCTSTR)type);
            if (pos != std::string::npos)
            {
                typesPresent.push_back((DamageReductionType)i);
                damageDice.replace(pos, type.GetLength(), "");
            }
        }
        // get the w, x, y, z and n field values
        double w = atof(damageDice.c_str());
        if (w == 0) w = 1.0;
        size_t pos = damageDice.find('[');
        damageDice.replace(0, pos+1, "");
        int x = atoi(damageDice.c_str());
        pos = damageDice.find('d');
        damageDice.replace(0, pos+1, "");
        int y = atoi(damageDice.c_str());
        damageDice.replace(0, y >= 10 ? 2 : 1, "");
        int z = atoi(damageDice.c_str());
        //pos = damageDice.find(']');
        //damageDice.replace(0, pos+1, "");
        //pos = damageDice.find(' ');
        //while (pos != std::string::npos)
        //{
        //    damageDice.replace(pos, 1, "");    // remove any spaces
        //    pos = damageDice.find(' ');
        //}
        //int n = atoi(damageDice.c_str());
        BaseDice dice;
        dice.Set(x, y, z);
        m_item.Set_WeaponDamage(w);
        m_item.Set_DamageDice(dice);
        m_item.Set_DRBypass(typesPresent);
    }
}

void WikiItemFileProcessor::AddCriticalThreatAndMultiplier(const std::map<std::string, std::string>& itemFields)
{
    if (itemFields.find("Critical Roll") != itemFields.end())
    {
        std::string criticalRoll = itemFields.at("Critical Roll");
        // e.g. 19-20 / x3
        int lowCrit = atoi(criticalRoll.c_str());
        m_item.Set_CriticalThreatRange(20 - lowCrit + 1); // 20 = 1, 19-20 = 2, etc
        size_t pos = criticalRoll.find('x');
        criticalRoll.replace(0, pos + 1, "");
        int multiplier = atoi(criticalRoll.c_str());
        m_item.Set_CriticalMultiplier(multiplier);
    }
}

void WikiItemFileProcessor::AddItemEffects(const std::map<std::string, std::string>& itemFields)
{
    if (itemFields.find("Enchantments") != itemFields.end())
    {
        std::string enchantments = itemFields.at("Enchantments");
        // break the enchantments up into single lines
        std::list<std::string> enchantmentLines;
        size_t pos = enchantments.find('\n');
        if (pos != std::string::npos)
        {
            size_t last = 0;
            while (pos != std::string::npos)
            {
                std::string line = enchantments.substr(last, pos - last);
                enchantmentLines.push_back(line);
                last = pos + 1;     // skip '\n'
                pos = enchantments.find('\n', last);
            }
            std::string line = enchantments.substr(last, 5000); // rest is last line
            enchantmentLines.push_back(line);
        }
        else
        {
            // its a single line, just add it
            enchantmentLines.push_back(enchantments);
        }
        for (auto&& it : enchantmentLines)
        {
            bool bProcessed = AddSetBonuses(it);
            if (!bProcessed) bProcessed = AddAugmentSlots(it);
            if (!bProcessed && m_item.SetBonus().size() > 0) break; // rest of items are bad set bonus/augment items
            if (!bProcessed && m_item.Augments().size() > 0) break; // rest of items are bad set bonus/augment items
            if (!bProcessed) bProcessed = ProcessEnchantmentLine(it);
            if (!bProcessed)
            {
                CString text;
                text.Format("...Line \"%s\" not processed.", it.c_str());
                GetLog().AddLogEntry(text);
            }
        }
    }
}

bool WikiItemFileProcessor::ProcessEnchantmentLine(const std::string& line)
{
    bool bRecognised = false;
    bRecognised |= AddCommonEffect(line, "ArmorEnchantment", "enhancement bonus to Armor Class", "Enhancement", "", 60);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "ArmorBonus", "Armor Bonus", "Armor", "", 10);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "CommandCommand", "Command Command", "", "", 10);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Command ", "Command ", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "ExtraLayOnHands", "Extra Lay on Hands", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "ExtraSmites", "Extra Smites ", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Protection", "Protection", "Deflection", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "SpellcastingImplement", "Spellcasting Implement ", "Implement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "WeaponEnchantment", "enhancement bonus to attack and damage rolls", "Enhancement", "", 60);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "WeaponEnchantmentBad", "-1 Enhancement Bonus", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "GhostTouch", "Ghost Touch Ghost Touch", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "GhostTouch", "Passive: This weapon has Ghost Touch", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Blurry", "Blurry Blurry", "", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Dusk", "Dusk Dusk", "", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Shield", "Shield", "", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "MithralArmor", "Mithral Mithral : Mithral armor", "", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "WeakenUndead", "Weaken Undead Weaken Undead", "", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Unnatural", "Unnatural Unnatural", "", "");

    if (!bRecognised) bRecognised |= AddCommonEffect(line, "MinorActionBoostEnhancement", "Minor Action Boost Enhancement", "Equipment", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "LesserActionBoostEnhancement", "Lesser Action Boost Enhancement", "Equipment", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "ActionBoostEnhancement", "Action Boost Enhancement", "Equipment", "");

    // rune arm charging
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "MaximumChargeTierV", "Maximum Charge Tier V", "Equipment", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "MaximumChargeTierV", "Maximum Charge Tier : V", "Equipment", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "MaximumChargeTierV", "Maximum Charge Tier 5", "Equipment", "");

    if (!bRecognised) bRecognised |= AddCommonEffect(line, "MaximumChargeTierIV", "Maximum Charge Tier IV", "Equipment", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "MaximumChargeTierIV", "Maximum Charge Tier : IV", "Equipment", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "MaximumChargeTierIV", "Maximum Charge Tier 4", "Equipment", "");

    if (!bRecognised) bRecognised |= AddCommonEffect(line, "MaximumChargeTierIII", "Maximum Charge Tier III", "Equipment", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "MaximumChargeTierIII", "Maximum Charge Tier : III", "Equipment", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "MaximumChargeTierIII", "Maximum Charge Tier 3", "Equipment", "");
    
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "MaximumChargeTierII", "Maximum Charge Tier II", "Equipment", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "MaximumChargeTierII", "Maximum Charge Tier : II", "Equipment", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "MaximumChargeTierII", "Maximum Charge Tier 2", "Equipment", "");
    
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "MaximumChargeTierI", "Maximum Charge Tier I", "Equipment", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "MaximumChargeTierI", "Maximum Charge Tier : I", "Equipment", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "MaximumChargeTierI", "Maximum Charge Tier 1", "Equipment", "");

    if (!bRecognised) bRecognised |= AddCommonEffect(line, "StrengthOfPurpose", "Strength of Purpose : You gain a ", "Enhancement", "", 50);

    if (!bRecognised) bRecognised |= AddCommonEffect(line, "AcidVulnerability", "Acid Vulnerability", "Enhancement", "", 50);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "ElectricVulnerability", "Electric Vulnerability", "Enhancement", "", 50);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "FireVulnerability", "Fire Vulnerability", "Enhancement", "", 50);

    if (!bRecognised) bRecognised |= AddCommonEffect(line, "LesserAcidResistance", "Lesser Acid Resistance", "Enhancement", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "ImprovedAcidResistance", "Improved Acid Resistance", "Enhancement", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "GreaterAcidResistance", "Greater Acid Resistance", "Enhancement", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "SuperiorAcidResistance", "Superior Acid Resistance", "Enhancement", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "SovereignAcidResistance", "Sovereign Acid Resistance", "Enhancement", "", 5);

    if (!bRecognised) bRecognised |= AddCommonEffect(line, "LesserColdResistance", "Lesser Cold Resistance", "Enhancement", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "ImprovedColdResistance", "Improved Cold Resistance", "Enhancement", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "GreaterColdResistance", "Greater Cold Resistance", "Enhancement", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "SuperiorColdResistance", "Superior Cold Resistance", "Enhancement", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "SovereignColdResistance", "Sovereign Cold Resistance", "Enhancement", "", 5);

    if (!bRecognised) bRecognised |= AddCommonEffect(line, "LesserLightningResistance", "Lesser Lightning Resistance", "Enhancement", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "ImprovedLightningResistance", "Improved Lightning Resistance", "Enhancement", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "GreaterLightningResistance", "Greater Lightning Resistance", "Enhancement", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "SuperiorLightningResistance", "Superior Lightning Resistance", "Enhancement", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "SovereignLightningResistance", "Sovereign Lightning Resistance", "Enhancement", "", 5);

    if (!bRecognised) bRecognised |= AddCommonEffect(line, "LesserFireResistance", "Lesser Fire Resistance", "Enhancement", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "ImprovedFireResistance", "Improved Fire Resistance", "Enhancement", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "GreaterFireResistance", "Greater Fire Resistance", "Enhancement", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "SuperiorFireResistance", "Superior Fire Resistance", "Enhancement", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "SovereignFireResistance", "Sovereign Fire Resistance", "Enhancement", "", 5);

    if (!bRecognised) bRecognised |= AddCommonEffect(line, "LesserFalseLife", "Lesser False Life", "Enhancement", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "ImprovedFalseLife", "Improved False Life", "Enhancement", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "GreaterFalseLife", "Greater False Life", "Enhancement", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "SuperiorFalseLife", "Superior False Life", "Enhancement", "", 5);

    if (!bRecognised) bRecognised |= AddCommonEffect(line, "LesserIncorporealBane", "Lesser Incorporeal Bane", "Enhancement", "All", 5);

    if (!bRecognised) bRecognised |= AddCommonEffect(line, "ElementalManipulation", "Elemental Manipulation", "Equipment", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "PowerOfTheDarkRestoration", "Power of the Dark Restoration", "Equipment", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "PowerOfTheCreepingDust", "Power of Creeping Dust", "Equipment", "", 5);

    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Crown of Summer", "Crown of Summer Crown of Summer", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Crown of Summer:", "Crown of Summer:", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Winged Allure", "Winged Allure", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Construct Fortification (10%)", "Construct Fortification (10%)", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "On Critical", "On Critical Hit: 1d4 Strength, Dexterity, or Constitution Damage.", "", "", 5);

    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Greater Nimbleness", "Greater Nimbleness", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Superior Nimbleness", "Superior Nimbleness", "", "", 5);

    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Dampened", "Dampened Dampened", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Suppressed Power", "Suppressed Power Suppressed Power", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Hidden Effect Sap", "Hidden Effect: On critical hits, this weapon saps the target", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Hidden Effect Demonic Drain", "Hidden effect: Demonic Drain", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Hidden Effect Cursed Defiance", "Hidden effect: Cursed Defiance", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Hidden Effect Radiance Guard", "Hidden effect: Radiance Guard", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Hidden Effect Will Save -3", "Hidden effect: Will Save -3", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Hidden Effect Madstone Reaction", "Hidden effect: Madstone Reaction", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Hidden Effect All Threat", "Hidden Effect: Increases all threat", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Hidden Effect All Threat", "Hidden Effect: Increases spellcasting", "", "", 5);

    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Knee Cracker", "Knee Cracker Knee Cracker", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Thunderbane", "Thunderbane Thunderbane", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Forged Lightning", "Forged Lightning Forged Lightning", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Polycurse", "Polycurse Polycurse", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "AirGuard", "Air Guard Air Guard", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Bodyfeeder", "Bodyfeeder Bodyfeeder", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "DisintegrationGuard", "Disintegration Guard Disintegration Guard", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "EarthgrabGuard", "Earthgrab Guard Earthgrab Guard", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Earthen Guard", "Earthen Guard Earthen Guard", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Improved Earthen Guard", "Improved Earthen Guard", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Legendary Earthgrab Guard", "Legendary Earthgrab Guard", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "MedusaFury", "Medusa Fury", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "NightmareGuard", "Nightmare Guard Nightmare Guard", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Transmuted Platinum", "Transmuted Platinum", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Arcane Augmentation IX", "Arcane Augmentation IX", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Arcane Augmentation I", "Arcane Augmentation I", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Fire Shield (Cold)", "Fire Shield (Cold)", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Petrification Immunity", "Petrification Immunity", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Demonic Curse", "Demonic Curse Demonic Curse", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Demonic Shield", "Demonic Shield Demonic Shield", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Improved Demonic Shield", "Improved Demonic Shield Improved Demonic Shield", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Demonic Retribution", "Demonic Retribution Demonic Retribution", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Improved Quelling Strikes", "Improved Quelling Strikes", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Crushing Wave", "Crushing Wave Crushing Wave", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Legendary Telekinetic", "Legendary Telekinetic Legendary Telekinetic", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Legendary Sirocco", "Legendary Sirocco Legendary Sirocco", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Improved Destruction", "Improved Destruction Improved Destruction", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Transform Kinetic Energy", "Transform Kinetic Energy", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Minor Lesser Dragonmark Enhancement", "Minor Lesser Dragonmark Enhancement", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Dragonshard Focus: Sentinel", "Dragonshard Focus: Sentinel", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Embodiment of Law", "Embodiment of Law Embodiment of Law", "", "", 5);

    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Paragon Evil Guard", "Paragon Evil Guard Paragon", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Paragon Good Guard", "Paragon Good Guard Paragon", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Paragon Light Guard", "Paragon Light Guard Paragon", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Paragon Negative Guard", "Paragon Negative Guard Paragon", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Epic Paragon Evil Guard", "Epic Paragon Evil Guard Epic Paragon", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Epic Paragon Good Guard", "Epic Paragon Good Guard Epic Paragon", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Epic Paragon Light Guard", "Epic Paragon Light Guard Epic Paragon", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Epic Paragon Negative Guard", "Epic Paragon Negative Guard Epic Paragon", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Paragon Cerulean Wave", "Paragon Cerulean Wave", "", "", 5);

    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Bloody Thorns", "Bloody Thorns", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Greater Thorn Guard", "Greater Thorn Guard", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Linguistics 10%", "Linguistics 10%", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Linguistics 15%", "Linguistics 15%", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Manslayer", "Manslayer Manslayer", "", "", 5);

    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Deathblock IX", "Deathblock IX", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Deathblock VIII", "Deathblock VIII", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Deathblock VII", "Deathblock VII", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Deathblock VI", "Deathblock VI", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Deathblock IV", "Deathblock IV", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Deathblock II", "Deathblock II", "", "", 5);

    if (!bRecognised) bRecognised |= AddCommonEffect(line, "DemonFever", "Demon Fever", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Vengeful", "Vengeful Vengeful", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "BlindnessImmunity", "Blindness Immunity", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Magi", "Magi Magi", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Archmagi", "Archmagi Archmagi", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "ImprovedCursespewing", "Improved Cursespewing", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "SpikeStudded", "Spike-Studded", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Spiked", "Spiked Spiked", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "SpikeGuard", "Spike Guard", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "GlassJawStrike", "Glass Jaw Strike", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Disintegration", "Disintegration Disintegration", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "AngelicGrace", "Angelic Grace", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Telekinetic", "Telekinetic Telekinetic", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "SlicingWinds", "Slicing Winds", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "MortalStrike", "Mortal Strike", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "RadiantGlory", "Radiant Glory", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Tidal", "Tidal Tidal", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "FellingTheOak", "Felling the Oak", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "ImmunityToSlippery", "Immunity to Slippery", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "ProofAgainstPoison", "Proof Against Poison", "Enhancement", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "ProofAgainstPoison", "Insightful Proof Against Poison", "Insightful", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "ProofAgainstPoison", "Quality Proof Against Poison", "Quality", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "ProofAgainstDisease", "Proof Against Disease", "Enhancement", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "ProofAgainstDisease", "Insightful Proof Against Disease", "Insightful", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "ProofAgainstDisease", "Quality Proof Against Disease", "Quality", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "PowerStore", "Power Store", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Righteous", "Righteous Righteous", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Persuasion", "Persuasion Persuasion", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "MagicalEfficiency", "Magical Efficiency", "Enhancement", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "RoughHide", "Rough Hide", "Primal", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "DreamVision", "Dream Vision", "Competence", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Anathema", "Anathema Anathema", "", "", 25);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "UndeadGuard", "Undead Guard", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Ice Guard II", "Ice Guard II", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Lesser Arcane Augmentation I", "Lesser Arcane Augmentation I", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Arcane Casting Dexterity", "Arcane Casting Dexterity", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Lesser Arcane Casting Dexterity", "Lesser Arcane Casting Dexterity", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Antimagic Spike", "Antimagic Spike", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Telekinesis Guard", "Telekinesis Guard", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Invisibility Guard", "Invisibility Guard", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Haste Guard", "Haste Guard", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Legendary Nightmare Guard", "Legendary Nightmare Guard", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Blackscale Ferocity", "Blackscale Ferocity", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Boon of Undeath", "Boon of Undeath", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Profane Well Rounded", "Profane Well Rounded", "Profane", "All", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Vengeful Fury", "Vengeful Fury", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Heroism", "Heroism Heroism", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Metalline", "Metalline Metalline", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Aligned", "Aligned Aligned", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "EnergyAbsorption", "Negative Energy Absorption", "Enhancement", "Negative", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Tendon Slice", "Tendon Slice", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Demonic Might", "Demonic Might", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "RagingFocus", "Raging Focus Raging Focus", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Disruption", "Disruption Disruption", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Raging Strength", "Raging Strength", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Fusible", "Fusible Fusible", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Malleable", "Malleable Malleable", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Tira's Splendor", "Tira's Splendor", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Overfocus", "Overfocus Overfocus", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Sticky Goo Guard", "Sticky Goo Guard", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Ice Barrier", "Ice Barrier Ice Barrier", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Mind Drain", "Mind Drain Mind Drain", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Silver Threaded", "Silver Threaded", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Treason", "Treason", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Battle-Scarred", "Battle-Scarred", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Godly Wrath", "Godly Wrath", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Lesser Undead Guard", "Lesser Undead Guard", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Unraveling Enchantments", "Unraveling Enchantments", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Extra Smite", "Extra Smite", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Legendary Demon Fever", "Legendary Demon Fever", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Overwhelming Shockwave", "Overwhelming Shockwave", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Legendary Stone Prison", "Legendary Stone Prison", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Legendary Air Guard", "Legendary Air Guard", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Lesser Regeneration", "Lesser Regeneration", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Regeneration", "Regeneration Regeneration", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Improved Regeneration", "Improved Regeneration", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Greater Regeneration", "Greater Regeneration", "", "", 5);

    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Freezing Ice Guard", "Freezing Ice Guard Freezing Ice Guard", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Minor Freezing Ice", "Minor Freezing Ice", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Freezing Ice", "Freezing Ice", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Epic Freezing Ice", "Epic Freezing Ice", "", "", 5);

    if (!bRecognised) bRecognised |= AddCommonEffect(line, "DR 5/Adamantine", "DR Damage Reduction 5/Adamantine", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "DR 15/Adamantine", "DR Damage Reduction 15/Adamantine", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "DR 30/Adamantine", "DR Damage Reduction 30/Adamantine", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "DR 1/-", "DR Damage Reduction 1/-", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "DR 2/-", "DR Damage Reduction 2/-", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "DR 3/-", "DR Damage Reduction 3/-", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "DR 5/-", "DR Damage Reduction 5/-", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "DR 15/-", "DR Damage Reduction 15/-", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "DR 3/Bludgeoning", "DR Damage Reduction 3/Bludgeoning", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "DR 30/Chaotic", "DR Damage Reduction 30/Chaotic", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "DR 5/Lawful", "DR Damage Reduction 5/Lawful", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "DR 10/Lawful", "DR Damage Reduction 10/Lawful", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "DR 15/Lawful", "DR Damage Reduction 15/Lawful", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "DR 15/Epic", "DR Damage Reduction 15/Epic", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "DR 5/Evil", "DR Damage Reduction 5/Evil", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "DR 10/Evil", "DR Damage Reduction 10/Evil", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "DR 15/Evil", "DR Damage Reduction 15/Evil", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "DR 50/Evil", "DR Damage Reduction 50/Evil", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "DR 5/Good", "DR Damage Reduction 5/Good", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "DR 10/Good", "DR Damage Reduction 10/Good", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "DR 15/Good", "DR Damage Reduction 15/Good", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "DR 30/Good", "DR Damage Reduction 30/Good", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "DR 30/Lawful", "DR Damage Reduction 30/Lawful", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "DR 3/Lawful", "DR Damage Reduction 3/Lawful", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Invulnerability", "Invulnerability Invulnerability", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Twilight", "Twilight Twilight", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Greater Twilight", "Greater Twilight", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "High Spirits", "High Spirits", "", "", 5);

    if (!bRecognised)
    {
        const std::list<Bonus>& bonuses = BonusTypes();
        for (auto&& bit : bonuses)
        {
            bRecognised = ProcessRomanNumeralTypes(line, bit.Name());
            if (!bRecognised) bRecognised = ProcessBonusType(line, bit.Name());
            if (bRecognised) break;     // and were done
        }
    }

    // curses
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "CurseOfClumsiness", "Curse of Clumsiness", "Penalty", "Dexterity");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "CurseOfDullness", "Curse of Dullness", "Penalty", "Intelligence");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "CurseOfFoolishness", "Curse of Foolishness", "Penalty", "Wisdom");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "CurseOfRepulsiveness", "Curse of Repulsiveness", "Penalty", "Charisma");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "CurseOfWeakness", "Curse of Weakness", "Penalty", "Strength");

    if (!bRecognised) bRecognised |= AddCommonEffect(line, "TaintOfEvil", "Taint of Evil", "", "");

    if (!bRecognised) bRecognised |= AddCommonEffect(line, "StealthStrike", "Stealth Strike", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "ImmunityToFear", "Immunity to Fear", "", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "ProfaneLifeforce", "Profane Lifeforce", "Profane", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "TrueSeeing", "True Seeing", "", "");

    // efficient metamagics
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "EfficientEmboldenII", "Efficient Metamagic - Embolden II", "", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "EfficientEmpowerHealingII", "Efficient Metamagic - Empower Healing II", "", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "EfficientEmpowerII", "Efficient Metamagic - Empower II", "", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "EfficientEnlargeII", "Efficient Metamagic - Enlarge II", "", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "EfficientExtendII", "Efficient Metamagic - Extend II", "", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "EfficientIntensifyII", "Efficient Metamagic - Intensify II", "", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "EfficientMaximizeII", "Efficient Metamagic - Maximize II", "", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "ImprovedHeighten", "Improved Metamagic: Heighten", "", "");

    if (!bRecognised) bRecognised |= AddCommonEffect(line, "EfficientExtendI", "Efficient Metamagic - Extend I", "", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "EfficientEmpowerHealingI", "Efficient Metamagic - Empower Healing I", "", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "EfficientMaximizeI", "Efficient Metamagic - Maximize I", "", "");

    if (!bRecognised) bRecognised |= AddCommonEffect(line, "GreaterAutoRepair", "Greater Auto-Repair ", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "MasterworkArmor", "Masterwork : This armor", "Enhancement", "", 20);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "MasterworkArmor", "Masterwork:: This armor", "Enhancement", "", 20);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "MasterworkWeapon", "Masterwork : This weapon", "Enhancement", "", 20);

    if (!bRecognised) bRecognised |= ProcessKeen(line);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "BluntedAmmunition", "Blunted Ammunition", "Enhancement", "", 5);

    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Brilliance", "Brilliance Brilliance", "Enhancement", "", 20);

    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Legendary Elemental Energy", "Legendary Elemental Energy", "", "", 5);

    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Roaring", "Roaring Roaring", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Lesser Turning", "Lesser Turning Lesser Turning", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Turning", "Turning Turning", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Eternal Faith", "Eternal Faith Eternal Faith", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Shocking Blow", "Shocking Blow Shocking Blow", "", "", 5);

    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Fire Guard X", "Fire Guard X", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Fire Guard VIII", "Fire Guard VIII", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Fire Guard II", "Fire Guard II", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Fire Guard I", "Fire Guard I", "", "", 5);

    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Dripping with Magma", "Dripping with Magma", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Incineration Guard", "Incineration Guard", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Chaotic Curse", "Chaotic Curse", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Hardened Spikes", "Hardened Spikes Hardened Spikes", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Cerulean Wave", "Cerulean Wave Cerulean Wave", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "From The Shadows", "From The Shadows", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Banishing Fists", "Banishing Fists", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Minor Spell Penetration VII", "Minor Spell Penetration VII", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Arcane Lore", "Arcane Lore", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Improved Shattermantle", "Improved Shattermantle", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Thunderclap", "Thunderclap Thunderclap", "", "", 5);

    // weapon damage effects
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "AcidArrowSovereign", "Sovereign Acid Arrow", "", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "AcidArrowSuperior", "Superior Acid Arrow", "", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "AcidArrowGreater", "Greater Acid Arrow", "", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "AcidArrowImproved", "Improved Acid Arrow", "", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "AcidArrow", "Acid Arrow", "", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Acidic", "Acid Acid", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Acidic 2", "Acidic 2", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Acidic 3", "Acidic 3", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Acidic 4", "Acidic 4", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Acidic 5", "Acidic 5", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Acidic 6", "Acidic 6", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "AdamantineLined", "Adamantine Lined ", "Equipment", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Flaming", "Flaming Flaming", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Legendary Paralysis", "Legendary Paralysis", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Paralyzing", "Paralyzing", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Returning", "Returning Returning", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Shocking", "Shock Shock", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Shocking Burst", "Shocking Burst Shocking Burst", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Voltaic", "Voltaic Voltaic", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Electrifying", "Electrifying Electrifying", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Silver", "Silver Silver", "Equipment", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Enhanced Ghostly", "Enhanced Ghostly", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Ghostly", "Ghostly Ghostly", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Adamantine", "Adamantine Adamantine", "Equipment", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Byeshk", "Byeshk Byeshk", "Equipment", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "ColdIron", "Cold Iron", "Equipment", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Crystal", "Crystal Crystal", "Equipment", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Vorpal", " Vorpal Vorpal", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Improved Vorpal", "Improved Vorpal", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Greater Vorpal", "Greater Vorpal", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Superior Vorpal", "Superior Vorpal", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Sovereign Vorpal", "Sovereign Vorpal", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "LesserHalflingBane", "Lesser Halfling Bane", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Axiomatic", "Axiomatic Axiomatic", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Axiomatic 2", "Axiomatic 2", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Axiomatic 3", "Axiomatic 3", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Axiomatic 4", "Axiomatic 4", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Axiomatic 5", "Axiomatic 5", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Axiomatic 6", "Axiomatic 6", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Axiomatic Burst", "Axiomatic Burst Axiomatic Burst", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Axiomatic Burst 2", "Axiomatic Burst 2", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Axiomatic Burst 4", "Axiomatic Burst 4", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Axiomatic Burst 6", "Axiomatic Burst 6", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "EvilOutsiderBane", "Evil Outsider Bane Evil Outsider Bane", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "EvilOutsiderBane 2", "Evil Outsider Bane 2", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "EvilOutsiderBane 3", "Evil Outsider Bane 3", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "EvilOutsiderBane 4", "Evil Outsider Bane 4", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "EvilOutsiderBane 5", "Evil Outsider Bane 5", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "EvilOutsiderBane 6", "Evil Outsider Bane 6", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "AcidTouch", "Acid Touch", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Cold Touch", "Cold Touch", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "ShockingTouch", "Shocking Touch", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Frost", "Frost Frost", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Frostbite", "Frostbite Frostbite", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "FireTouch", "Fire Touch", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Fiery", "Fiery Fiery", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Fiery 2", "Fiery 2", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Fiery 3", "Fiery 3", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Fiery 4", "Fiery 4", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Fiery 5", "Fiery 5", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Fiery 6", "Fiery 6", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Holy", "Holy Holy", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Holy 1", "Holy 1", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Holy 2", "Holy 2", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Holy 3", "Holy 3", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Holy 4", "Holy 4", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Holy 5", "Holy 5", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Holy 6", "Holy 6", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "UndeadBane", "Undead Bane Undead Bane", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "UndeadBane 2", "Undead Bane 2", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "UndeadBane 3", "Undead Bane 3", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "UndeadBane 4", "Undead Bane 4", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "UndeadBane 5", "Undead Bane 5", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "UndeadBane 6", "Undead Bane 6", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "LesserUndeadBane", "Lesser Undead Bane", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "GreaterUndeadBane", "Greater Undead Bane", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "GreaterElfBane", "Greater Elf Bane", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "ElfBane", "Elf Bane", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "ChimerasVitality", "Chimera's Vitality", "Vitality", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "PirateVitality", "Pirate Vitality", "Vitality", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Vitality", "Vitality", "Vitality", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "ShockVIII", "Shock VIII", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "ShockVII", "Shock VII", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "ShockVI", "Shock VI", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "ShockV", "Shock V", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "ShockIV", "Shock IV", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "ShockIV", "Shock IV", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "ShockIII", "Shock III", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "ShockII", "Shock II", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "ShockI", "Shock I", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "ShriekingVIII", "Shrieking VIII", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "ShriekingVII", "Shrieking VII", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "ShriekingVI", "Shrieking VI", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "ShriekingV", "Shrieking V", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "ShriekingIV", "Shrieking IV", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "ShriekingIII", "Shrieking III", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "ShriekingII", "Shrieking II", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "ShriekingI", "Shrieking I", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Shrieking", "Shrieking Shrieking", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Screaming", "Screaming Screaming", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Cacophony", "Cacophony Cacophony", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "RangedPower", "Ranged Power", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "MeleePower", "Melee Power", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "MeleeAlacrity", "Melee Alacrity", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "ShockingBlastVI", "Shocking Blast 6", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "ShockingBlastV", "Shocking Blast 5", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "ShockingBlastIV", "Shocking Blast 4", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "ShockingBlastIII", "Shocking Blast 3", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "ShockingBlastII", "Shocking Blast 2", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Coruscating", "Coruscating Coruscating", "", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Coruscating2", "Coruscating 2", "", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Coruscating3", "Coruscating 3", "", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Coruscating4", "Coruscating 4", "", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Coruscating5", "Coruscating 5", "", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Coruscating6", "Coruscating 6", "", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Incandescence", "Incandescence Incandescence", "", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Impactful6", "Impactful 6", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Impactful5", "Impactful 5", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Impactful4", "Impactful 4", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Impactful3", "Impactful 3", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Impactful2", "Impactful 2", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Impactful1", "Impactful 1", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Ghostbane6", "Ghostbane 6", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Ghostbane5", "Ghostbane 5", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Ghostbane4", "Ghostbane 4", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Ghostbane3", "Ghostbane 3", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Ghostbane2", "Ghostbane 2", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Ghostbane1", "Ghostbane 1", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Ghostbane VIII", "Ghostbane VIII", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Ghostbane VI", "Ghostbane VI", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Ghostbane V", "Ghostbane V", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Ghostbane IV", "Ghostbane IV", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Ghostbane II", "Ghostbane II", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Ghostbane I", "Ghostbane I", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "ConstructBane1", "Construct Bane 1", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "ConstructBane2", "Construct Bane 2", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "ConstructBane3", "Construct Bane 3", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "ConstructBane4", "Construct Bane 4", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "ConstructBane5", "Construct Bane 5", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "ConstructBane6", "Construct Bane 6", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Electrifying1", "Electrifying 1", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Electrifying2", "Electrifying 2", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Electrifying3", "Electrifying 3", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Electrifying4", "Electrifying 4", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Electrifying5", "Electrifying 5", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Electrifying6", "Electrifying 6", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Steam", "Steam Steam", "", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Anarchic Blast 3", "Anarchic Blast 3", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Anarchic Blast 4", "Anarchic Blast 4", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Anarchic Blast 6", "Anarchic Blast 6", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Anarchic", "Anarchic Anarchic", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Anarchic 2", "Anarchic 2", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Anarchic 3", "Anarchic 3", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Anarchic 4", "Anarchic 4", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Anarchic 5", "Anarchic 5", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Anarchic 6", "Anarchic 6", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "FlamingBlastVI", "Flaming Blast 6", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "FlamingBlastV", "Flaming Blast 5", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "FlamingBlastIV", "Flaming Blast 4", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "FlamingBlastIII", "Flaming Blast 3", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "FlamingBlastII", "Flaming Blast 2", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Flaming Burst", "Flaming Burst Flaming Burst", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Flameblade", "Flameblade Flameblade", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "GreaterConstructBane", "Greater Construct Bane", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Poison", "Poison Poison", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Poison Blast 2", "Poison Blast 2", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Poison Blast 3", "Poison Blast 3", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Poison Blast 4", "Poison Blast 4", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Poison Blast 5", "Poison Blast 5", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Poison Blast 6", "Poison Blast 6", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Giant Bane 2", "Giant Bane 2", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Giant Bane 3", "Giant Bane 3", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Giant Bane 4", "Giant Bane 4", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Giant Bane 5", "Giant Bane 5", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Giant Bane 6", "Giant Bane 6", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Reverberating 2", "Reverberating 2", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Reverberating 3", "Reverberating 3", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Reverberating 4", "Reverberating 4", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Reverberating 5", "Reverberating 5", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Reverberating 6", "Reverberating 6", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Jolting 9", "Jolting 9", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Jolting 10", "Jolting 10", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Jolting 11", "Jolting 11", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Jolting 12", "Jolting 12", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Jolting 13", "Jolting 13", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Jolting 14", "Jolting 14", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Jolting 15", "Jolting 15", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Lesser Monstrous Humanoid Bane", "Lesser Monstrous Humanoid Bane", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Humanoid Bane 2", "Humanoid Bane 2", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Humanoid Bane 3", "Humanoid Bane 3", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Humanoid Bane 4", "Humanoid Bane 4", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Humanoid Bane 5", "Humanoid Bane 5", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Humanoid Bane 6", "Humanoid Bane 6", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Icy Burst", "Icy Burst Icy Burst", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Icy Blast 2", "Icy Blast 2", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Icy Blast 4", "Icy Blast 4", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Icy Blast 6", "Icy Blast 6", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Holy Burst 6", "Holy Burst 6", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Holy Burst 4", "Holy Burst 4", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Holy Burst", "Holy Burst Holy Burst", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Ooze Bane", "Ooze Bane Ooze Bane", "", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Ooze Bane 2", "Ooze Bane 2", "", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Ooze Bane 5", "Ooze Bane 5", "", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Overwhelming Incineration", "Overwhelming Incineration", "", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Legendary Blinding Embers", "Legendary Blinding Embers", "", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Tidal Burst", "Tidal Burst Tidal Burst", "", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Tidal 2", "Tidal 2", "", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Tidal 4", "Tidal 4", "", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Tidal 6", "Tidal 6", "", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Unnatural Bane 4", "Unnatural Bane 4", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Unnatural Bane 5", "Unnatural Bane 5", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Supreme Good", "Supreme Good Supreme Good", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Animal Bane", "Animal Bane Animal Bane", "", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Animal Bane 2", "Animal Bane 2", "", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Animal Bane 3", "Animal Bane 3", "", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Animal Bane 4", "Animal Bane 4", "", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Animal Bane 5", "Animal Bane 5", "", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Animal Bane 6", "Animal Bane 6", "", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Force Blast 2", "Force Blast 2", "", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Force Blast 3", "Force Blast 3", "", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Force Blast 4", "Force Blast 4", "", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Force Blast 6", "Force Blast 6", "", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Unholy Burst 6", "Unholy Burst 6", "", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Unholy 2", "Unholy 2", "", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Unholy 3", "Unholy 3", "", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Unholy 4", "Unholy 4", "", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Unholy 5", "Unholy 5", "", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Unholy 6", "Unholy 6", "", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Evil Blast 4", "Evil Blast 4", "", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Evil Blast 6", "Evil Blast 6", "", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Freezing 4", "Freezing 4", "", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Freezing 6", "Freezing 6", "", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Freezing 7", "Freezing 7", "", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Freezing 8", "Freezing 8", "", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Freezing 9", "Freezing 9", "", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Freezing 10", "Freezing 10", "", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Freezing 12", "Freezing 12", "", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Freezing 13", "Freezing 13", "", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Freezing 15", "Freezing 15", "", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Fracturing 2", "Fracturing 2", "", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Fracturing 3", "Fracturing 3", "", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Fracturing 4", "Fracturing 4", "", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Fracturing 6", "Fracturing 6", "", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Eldritch VI", "Eldritch VI", "", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Eldritch IV", "Eldritch IV", "", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Eldritch III", "Eldritch III", "", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Eldritch II", "Eldritch II", "", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Plant Bane 2", "Plant Bane 2", "", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Plant Bane 6", "Plant Bane 6", "", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Good Blast 3", "Good Blast 3", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Good Blast 4", "Good Blast 4", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Good Blast 6", "Good Blast 6", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Heartseeker VII", "Heartseeker VII", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Heartseeker VI", "Heartseeker VI", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Heartseeker V", "Heartseeker V", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Heartseeker IV", "Heartseeker IV", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Heartseeker III", "Heartseeker III", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Heartseeker I", "Heartseeker I", "Enhancement", "");

    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Feybane 2", "Feybane 2", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Feybane 6", "Feybane 6", "Enhancement", "");

    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Stabbing 6", "Stabbing 6", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Stabbing 15", "Stabbing 15", "Enhancement", "");

    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Fetters of Unreality", "Fetters of Unreality", "", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Vulnerable", "Vulnerable", "", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Crippling Flames", "Crippling Flames", "", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Lightning Strike", "Lightning Strike Lightning Strike", "", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Fiery Detonation", "Fiery Detonation", "", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Spell Turmoil", "Spell Turmoil", "", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Taint of Chaos", "Taint of Chaos", "", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Corrosive Salt", "Corrosive Salt", "", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Anthem", "Anthem Anthem", "", "All");

    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Everbright", "Everbright", "", "All");

    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Ribcracker VIII", "Ribcracker VIII", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Ribcracker VI", "Ribcracker VI", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Ribcracker V", "Ribcracker V", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Ribcracker IV", "Ribcracker IV", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Ribcracker III", "Ribcracker III", "Enhancement", "All");

    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Gashing 5", "Gashing 5", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Gashing 6", "Gashing 6", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Gashing 7", "Gashing 7", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Gashing 8", "Gashing 8", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Gashing 9", "Gashing 9", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Gashing 10", "Gashing 10", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Gashing 11", "Gashing 11", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Gashing 12", "Gashing 12", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Gashing 13", "Gashing 13", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Gashing 14", "Gashing 14", "Enhancement", "All");

    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Slashing 2", "Slashing 2", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Slashing 3", "Slashing 3", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Slashing 4", "Slashing 4", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Slashing 5", "Slashing 5", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Slashing 6", "Slashing 6", "Enhancement", "All");

    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Smashing 4", "Smashing 4", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Smashing 5", "Smashing 5", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Smashing 6", "Smashing 6", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Smashing 7", "Smashing 7", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Smashing 8", "Smashing 8", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Smashing 9", "Smashing 9", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Smashing 10", "Smashing 10", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Smashing 11", "Smashing 11", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Smashing 12", "Smashing 12", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Smashing 13", "Smashing 13", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Smashing 14", "Smashing 14", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Smashing 15", "Smashing 15", "Enhancement", "All");

    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Blazing", "Blazing Blazing", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Conflagrating", "Conflagrating Conflagrating", "Enhancement", "All");

    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Blazing 5", "Blazing 5", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Blazing 6", "Blazing 6", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Blazing 7", "Blazing 7", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Blazing 8", "Blazing 8", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Blazing 9", "Blazing 9", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Blazing 10", "Blazing 10", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Blazing 11", "Blazing 11", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Blazing 12", "Blazing 12", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Blazing 13", "Blazing 13", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Blazing 14", "Blazing 14", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Blazing 15", "Blazing 15", "Enhancement", "All");

    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Piercing 2", "Piercing 2", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Piercing 3", "Piercing 3", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Piercing 4", "Piercing 4", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Piercing 5", "Piercing 5", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Piercing 6", "Piercing 6", "Enhancement", "All");

    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Poisonous 2", "Poisonous 2", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Poisonous 3", "Poisonous 3", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Poisonous 4", "Poisonous 4", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Poisonous 5", "Poisonous 5", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Poisonous 6", "Poisonous 6", "Enhancement", "All");

    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Poison VIII", "Poison VIII", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Poison VII", "Poison VII", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Poison VI", "Poison VI", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Poison V", "Poison V", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Poison IVII", "Poison IV", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Poison III", "Poison III", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Poison II", "Poison II", "Enhancement", "All");

    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Chilling 2", "Chilling 2", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Chilling 3", "Chilling 3", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Chilling 4", "Chilling 4", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Chilling 5", "Chilling 5", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Chilling 6", "Chilling 6", "Enhancement", "All");

    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Monstrous Humanoid Bane 2", "Monstrous Humanoid Bane 2", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Monstrous Humanoid Bane 3", "Monstrous Humanoid Bane 3", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Monstrous Humanoid Bane 4", "Monstrous Humanoid Bane 4", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Monstrous Humanoid Bane 5", "Monstrous Humanoid Bane 5", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Monstrous Humanoid Bane 6", "Monstrous Humanoid Bane 6", "Enhancement", "All");

    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Dragon Bane", "Dragon Bane Dragon Bane", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Dragon Bane 2", "Dragon Bane 2", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Dragon Bane 3", "Dragon Bane 3", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Dragon Bane 4", "Dragon Bane 4", "Enhancement", "All");

    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Vermin Bane", "Vermin Bane Vermin Bane", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Vermin Bane 2", "Vermin Bane 2", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Vermin Bane 3", "Vermin Bane 3", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Vermin Bane 4", "Vermin Bane 4", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Vermin Bane 5", "Vermin Bane 5", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Vermin Bane 6", "Vermin Bane 6", "Enhancement", "All");

    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Weakening 1", "Weakening 1", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Weakening 2", "Weakening 2", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Weakening 3", "Weakening 3", "Enhancement", "All");

    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Critical Befouling 1", "Critical Befouling 1", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Critical Befouling 2", "Critical Befouling 2", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Critical Befouling 3", "Critical Befouling 3", "Enhancement", "All");

    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Bludgeoning 2", "Bludgeoning 2", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Bludgeoning 3", "Bludgeoning 3", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Bludgeoning 4", "Bludgeoning 4", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Bludgeoning 5", "Bludgeoning 5", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Bludgeoning 6", "Bludgeoning 6", "Enhancement", "All");

    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Slicing", "Slicing Slicing", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Wailing", "Wailing Wailing", "Enhancement", "All");

    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Crushing4", "Crushing 4", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Crushing5", "Crushing 5", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Crushing6", "Crushing 6", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Crushing7", "Crushing 7", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Crushing8", "Crushing 8", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Crushing9", "Crushing 9", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Crushing10", "Crushing 10", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Crushing11", "Crushing 11", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Crushing12", "Crushing 12", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Crushing13", "Crushing 13", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Crushing14", "Crushing 14", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Crushing15", "Crushing 15", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Bleed", "Bleed Bleed", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Bleeding 2", "Bleeding 2", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Bleeding 3", "Bleeding 3", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Bleeding 4", "Bleeding 4", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Bleeding 5", "Bleeding 5", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Bleeding 6", "Bleeding 6", "Enhancement", "All");

    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Wounding", "Wounding Wounding", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Wounding 1", "Wounding 1", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Wounding 2", "Wounding 2", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Wounding 3", "Wounding 3", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Critical Wounding 2", "Criticial Wounding 2", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Critical Wounding 3", "Criticial Wounding 3", "Enhancement", "All");

    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Maiming", "Maiming Maiming", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Maiming2", "Maiming 2", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Maiming3", "Maiming 3", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Maiming4", "Maiming 4", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Maiming5", "Maiming 5", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Maiming6", "Maiming 6", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "GreaterMaiming", "Greater Maiming", "Enhancement", "All");

    if (!bRecognised) bRecognised |= AddCommonEffect(line, "SonicBlast3", "Sonic Blast 3", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "SonicBlast4", "Sonic Blast 4", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "SonicBlast5", "Sonic Blast 5", "Enhancement", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "SonicBlast6", "Sonic Blast 6", "Enhancement", "All");

    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Force", "Force Force", "Enhancement", "All");

    if (!bRecognised) bRecognised |= AddCommonEffect(line, "TrueChaos", "True Chaos", "", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Giant Slayer", "Giant Slayer", "", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Roar", "Roar Roar", "", "All");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Finesse", "Finesse Finesse", "", "All");

    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Enhanced Ki", "Enhanced Ki", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Holding On", "Holding On Holding On", "Enhancement", "");

    if (!bRecognised) bRecognised |= AddCommonEffect(line, "UnderwaterAction", "Underwater Action", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "FeatherFalling", "Feather Falling", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "LightBringer", "Light Bringer", "", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "RadiantBlast", "Radiant Blast", "", "");

    if (!bRecognised) bRecognised |= AddCommonEffect(line, "AcidV", "Acid V", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "AcidIV", "Acid IV", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "AcidIII", "Acid III", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "AcidII", "Acid II", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "AcidI", "Acid I", "Enhancement", "");

    if (!bRecognised) bRecognised |= AddCommonEffect(line, "FlamingVI", "Flaming VI", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "FlamingV", "Flaming V", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "FlamingIV", "Flaming IV", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "FlamingIII", "Flaming III", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "FlamingII", "Flaming II", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "FlamingI", "Flaming I", "Enhancement", "");

    if (!bRecognised) bRecognised |= AddCommonEffect(line, "FrostVIII", "Frost VIII", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "FrostVII", "Frost VII", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "FrostVI", "Frost VI", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "FrostV", "Frost V", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "FrostIV", "Frost IV", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "FrostIII", "Frost III", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "FrostII", "Frost II", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "FrostI", "Frost I", "Enhancement", "");

    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Bloodletter VII", "Bloodletter VII", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Bloodletter VI", "Bloodletter VI", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Bloodletter V", "Bloodletter V", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Bloodletter IV", "Bloodletter IV", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Bloodletter III", "Bloodletter III", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Bloodletter II", "Bloodletter II", "Enhancement", "");

    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Human Bane", "Human Bane Human Bane", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Human Bane 2", "Human Bane 2", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Human Bane 3", "Human Bane 3", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Human Bane 4", "Human Bane 4", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Human Bane 6", "Human Bane 6", "Enhancement", "");

    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Solar VI", "Solar VI", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Solar V", "Solar V", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Solar IV", "Solar IV", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Solar III", "Solar III", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Solar II", "Solar II", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Solar I", "Solar I", "Enhancement", "");

    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Entropic 2", "Entropic 2", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Entropic 3", "Entropic 3", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Entropic 4", "Entropic 4", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Entropic 6", "Entropic 6", "Enhancement", "");

    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Shrieking Bolt", "Shrieking Bolt", "Enhancement", "");

    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Cloudburst", "Cloudburst Cloudburst", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Improved Paralyzing", "Improved Paralyzing", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Slay Living", "Slay Living Slay Living", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Calamitous Blows", "Calamitous Blows", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Bloodrage Defense", "Bloodrage Defense", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Rune-fueled Warding", "Rune-fueled Warding", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Rune-fueled Warding", "Rune-fueled Warding", "Enhancement", "");

    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Lesser Dwarf Bane", "Lesser Dwarf Bane", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Path of the Guarding Stone", "Path of the Guarding Stone", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Stone Prison", "Stone Prison", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Fracturing", "Fracturing Fracturing", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Bonesplitter", "Bonesplitter Bonesplitter", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Rockslide", "Rockslide Rockslide", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Shockwave", "Shockwave Shockwave", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Anchoring", "Anchoring Anchoring", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Grounded", "Grounded Grounded", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Quenched", "Quenched Quenched", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Smoke Screen", "Smoke Screen Smoke Screen", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Wondrous Craftsmanship", "Wondrous Craftsmanship", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Blunt Trauma", "Blunt Trauma", "Enhancement", "");

    if (!bRecognised) bRecognised |= AddCommonEffect(line, "ProficiencyBastardSword", "Feat : Proficiency: Bastard Sword", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "ProficiencyGreatClub", "Proficiency: Greatclub", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "ProficiencyKhopesh", "Proficiency : Khopesh", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "ProficiencyLongbow", "Feat : Proficiency: Longbow", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "ProficiencyShortbow", "Feat : Proficiency: Shortbow", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "FeatQuickDraw", "Feat : Quick Draw", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "FeatMobility", "Feat : Mobility", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "FeatBowStrength", "Feat : Bow Strength", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Diehard", "Diehard Diehard", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "WeaponFocusFalchion", "Weapon Focus: Falchion", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "FeatPreciseShot", "Feat : Precise Shot", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "FeatWeaponFocusThrown", "Feat : Weapon Focus: Thrown", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "FeatWindThroughTheTrees", "Feat : Wind through the Trees", "Enhancement", "");

    // class requirements are in the Enhancements list
    if (!bRecognised) bRecognised |= AddClassRequirements(line);

    if (!bRecognised) bRecognised |= AddCommonEffect(line, "SuperiorReinforcedFists", "Superior Reinforced Fists", "", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "GreaterReinforcedFists", "Greater Reinforced Fists", "", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "ReinforcedFists", "Reinforced Fists", "", "");

    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Raging Resilience", "Raging Resilience", "", "");

    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Bewildering", "Bewildering", "", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Web Immunity", "Web Immunity", "", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "GoodLuck", "Good Luck", "Luck", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Deathblock", "Deathblock Deathblock", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Striding", "Striding", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "TemperanceOfBelief", "Temperance of Belief", "", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "GreaterSunburst", "Greater Sunburst", "", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Destruction", "Destruction Destruction", "", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "RelentlessFury", "Relentless Fury", "", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "SubtleTarget", "Subtle Target", "", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "CannithCombatInfusion", "Cannith Combat Infusion", "", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "StaggeringBlow", "Staggering Blow", "", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "BaneOfTheDepths", "Bane of the Depths", "", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "BlessWard", "Bless Ward", "", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "PowerOfTheFrozenDepths", "Power of the Frozen Depths", "Equipment", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "MagmaSurge", "Magma Surge Magma Surge", "", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "AlchemicalAirAttunement", "Alchemical Air Attunement", "", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "AlchemicalEarthAttunement", "Alchemical Earth Attunement", "", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "AlchemicalFireAttunement", "Alchemical Fire Attunement", "", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "AlchemicalWaterAttunement", "Alchemical Water Attunement", "", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Elasticity", "Elasticity Elasticity", "", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "RangedAlacrity", "Ranged Alacrity", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Entropic", "Entropic Entropic", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Hemorrhaging", "Hemorrhaging Hemorrhaging", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Crippling", "Crippling Crippling", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Life-Devouring", "Life-Devouring", "", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Life Shield", "Life Shield", "", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Weakening", "Weakening Weakening", "", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Puncturing", "Puncturing Puncturing", "", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Enfeebling", "Enfeebling Enfeebling", "", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "LesserDisplacement", "Lesser Displacement", "", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Riposte", "Riposte", "Insightful", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Greater Dispelling", "Greater Dispelling", "", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Intercession Ward", "Intercession Ward", "", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Legendary Slicing Winds", "Legendary Slicing Winds", "", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Twining Hemlock", "Twining Hemlock", "", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Maker's Touch", "Maker's Touch", "", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Banishing Weapons", "Banishing Weapons", "", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Antipodal", "Antipodal", "", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Life Stealing", "Life Stealing", "", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Vampirism", "Vampirism Vampirism", "", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Vampirism 1", "Vampirism 1", "", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Vampirism 2", "Vampirism 2", "", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Vampirism 3", "Vampirism 3", "", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Nightmares", "Nightmares Nightmares", "", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Identity Crisis", "Identity Crisis", "", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Utter Disintegration", "Utter Disintegration", "", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Nightshade Venom", "Nightshade Venom", "", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Nightsinger", "Nightsinger Nightsinger", "", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Whirlwind Ward", "Whirlwind Ward", "", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Trace of Madness", "Trace of Madness", "", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Metal Fatigue", "Metal Fatigue", "", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Ram's Might", "Ram's Might Ram's Might", "", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Inflict Blight", "Inflict Blight", "", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Mother Night's Embrace", "Mother Night's Embrace", "", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Marksmanship", "Marksmanship Marksmanship", "", "");

    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Echoes of Angdrelve 4", "Echoes of Angdrelve 4", "", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Echoes of Angdrelve 6", "Echoes of Angdrelve 6", "", "");

    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Penalty Lawful", "Penalty: If equipped by a Lawful user, they receive 1 Negative Level", "", "");

    if (!bRecognised) bRecognised |= AddCommonEffect(line, "GreaterIncineration", "Greater Incineration", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Incineration", "Incineration Incineration", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Weaken Construct", "Weaken Construct", "Enhancement", "");

    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Sonic Guard", "Sonic Guard Sonic Guard", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Sonic Guard +2", "Sonic Guard +2", "Enhancement", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Sonic Guard +3", "Sonic Guard +3", "Enhancement", "");

    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Heightened Awareness", "Heightened Awareness", "Insightful", "");
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Hallowed", "Hallowed", "Equipment", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Sacred", "Sacred", "Equipment", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Improved Deception", "Improved Deception Improved Deception", "Enhancement", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Improved Deception", "Improved Deception", "Enhancement", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Alchemical Conservation", "Alchemical Conservation", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Greater Elemental Energy", "Greater Elemental Energy", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Lifesealed", "Lifesealed", "Enhancement", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Freedom of Movement", "Freedom of Movement", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Limb Chopper", "Limb Chopper", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Flametouched Iron", "Flametouched Iron", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Bone Paws", "Bone Paws", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Elemental Resistance", "Elemental Resistance", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Royalty's Frigid Response", "Royalty's Frigid Response", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Vorpal Lethargy", "On Vorpal: Applies a stack of Lethargy", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Evil Characters", "Evil characters wielding this weapon", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Axeblock X", "Axeblock X", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Axeblock VI", "Axeblock VI", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Axeblock V", "Axeblock V", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Axeblock III", "Axeblock III", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Axeblock II", "Axeblock II", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Spearblock X", "Spearblock X", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Spearblock VIII", "Spearblock VIII", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Spearblock VI", "Spearblock VI", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Spearblock V", "Spearblock V", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Spearblock III", "Spearblock III", "", "", 5);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Spearblock II", "Spearblock II", "", "", 5);
    if (!bRecognised) bRecognised |= ProcessSpeed(line);
    if (!bRecognised) bRecognised |= AddFavoredWeapon(line);

    // lines to just be ignored
    if (!bRecognised) bRecognised |= line.find("Starter Starter : Starter equipment cannot be traded, sold or crafted.") != std::string::npos;
    if (!bRecognised) bRecognised |= line.find("This item cannot be fed to sentient jewels .") != std::string::npos;
    if (!bRecognised) bRecognised |= line.find("On Vorpal Hit : If your target") != std::string::npos;
    if (!bRecognised) bRecognised |= line.find(" Silver weapons have 2 points lower hardness") != std::string::npos;
    if (!bRecognised) bRecognised |= line.find("Visibility 1: Allows") != std::string::npos;
    if (!bRecognised) bRecognised |= line.find("Visibility 2: Allows") != std::string::npos;
    if (!bRecognised) bRecognised |= line.find("Drops on leaving adventure") != std::string::npos;
    if (!bRecognised) bRecognised |= line.find("These additional uses will only take effect after you rest") != std::string::npos;
    if (!bRecognised) bRecognised |= line.find("Note: provides 1d3 hp of healing per hit.") != std::string::npos;
    if (!bRecognised) bRecognised |= line.find("Note: internal cooldown of 1 second.") != std::string::npos;
    if (!bRecognised) bRecognised |= line.find("On Vorpal Hit : If an undead struck") != std::string::npos;
    if (!bRecognised) bRecognised |= line.find("Chaotic characters wielding this weapon will suffer one negative level") != std::string::npos;
    if (!bRecognised) bRecognised |= line.find("<abbr title=\"Minimum level") != std::string::npos;
    if (!bRecognised) bRecognised |= line.find("Note: Wielder's second weapon does not benefit from Finesse.") != std::string::npos;
    if (!bRecognised) bRecognised |= line.find("This War Paint applies a") != std::string::npos;
    if (!bRecognised) bRecognised |= line.find("Bear, Dire Bear") != std::string::npos;
    if (!bRecognised) bRecognised |= line.find("On Critical Hit : 1d4 Strength, Dexterity, or Constitution Damage.") != std::string::npos;
    if (!bRecognised) bRecognised |= line.find("None") != std::string::npos;
    if (!bRecognised) bRecognised |= line.find("Required Trait:: Lawful") != std::string::npos;
    if (!bRecognised) bRecognised |= line.find("Immunity to Banishment") != std::string::npos;
    if (!bRecognised) bRecognised |= line.find("(+2 Total Enhancement Value.)") != std::string::npos;
    if (!bRecognised) bRecognised |= line.find("Note: Real slowdown is only 25%") != std::string::npos;
    if (!bRecognised) bRecognised |= line.find("Lawful characters wielding this weapon will suffer one negative level.") != std::string::npos;
    if (!bRecognised) bRecognised |= line.find("Good characters wielding this weapon will suffer one negative level.") != std::string::npos;
    if (!bRecognised) bRecognised |= line.find("Spell failure chances for armors and shields made from crystal are also decreased by 10%.") != std::string::npos;
    if (!bRecognised) bRecognised |= line.find("Passive: Attacks from this weapon bypass the miss chance of incorporeal") != std::string::npos;
    if (!bRecognised) bRecognised |= line.find("Greater: Insightful Fortification +25%") != std::string::npos;

    return bRecognised;
}

bool WikiItemFileProcessor::AddAugmentSlots(const std::string& line)
{
    bool bRecognised = false;
    bRecognised |= AddAugmentSlot(line, "Green Augment Slot");
    bRecognised |= AddAugmentSlot(line, "Blue Augment Slot");
    bRecognised |= AddAugmentSlot(line, "Yellow Augment Slot");
    bRecognised |= AddAugmentSlot(line, "Colorless Augment Slot");
    bRecognised |= AddAugmentSlot(line, "Red Augment Slot");
    bRecognised |= AddAugmentSlot(line, "Purple Augment Slot");
    bRecognised |= AddAugmentSlot(line, "Orange Augment Slot");
    return bRecognised;
}

bool WikiItemFileProcessor::AddAugmentSlot(const std::string& line, const std::string& searchText)
{
    bool bRecognised = false;
    size_t pos = line.find(searchText);
    if (pos != std::string::npos
            && pos <= 2)
    {
        pos = line.find(' ');   // find augment slot colour delimiter
        std::string color;
        color = line.substr(0, pos);
        ItemAugment ia;
        ia.Set_Type(color);
        m_item.AddItemAugment(ia);
        bRecognised = true;
    }
    return bRecognised;
}

bool WikiItemFileProcessor::AddSetBonuses(const std::string& line)
{
    bool bFound = false;
    const std::list<SetBonus>& setBonuses = SetBonuses();
    for (auto&& sbit : setBonuses)
    {
        size_t pos = line.find(sbit.Type());
        if (pos != std::string::npos
            && pos < 5)
        {
            // this set is present
            m_item.AddSetBonus(sbit.Type());
            bFound = true;
            break;
        }
    }
    return bFound;
}


bool WikiItemFileProcessor::AddClassRequirements(const std::string& line)
{
    bool bRecognised = false;
    size_t pos = line.find("Required Class", 0);
    if (pos != std::string::npos)
    {
        const std::list<Class>& classes = Classes();
        for (auto&& cit : classes)
        {
            pos = line.find(cit.Name());
            if (pos != std::string::npos)
            {
                // only interested in the first class we find
                m_item.AddClassRequirement(cit.Name());
                bRecognised = true;
                break;
            }
        }
    }
    return bRecognised;
}

bool WikiItemFileProcessor::ProcessRomanNumeralTypes(const std::string& line, const std::string& bonus)
{
    // add any effects that are covered by Roman Numerals
    static std::string romanNumerals[] =
    {   // deliberately in reverse order
        "XX",
        "XIX",
        "XIIX",
        "XVIII",
        "XVII",
        "XVI",
        "XV",
        "XIV",
        "XIII",
        "XII",
        "XI",
        "X",
        "IX",
        "VIII",
        "VII",
        "VI",
        "V",
        "IV",
        "III",
        "II",
        "I"
    };
    size_t count = sizeof(romanNumerals) / sizeof(std::string);
    bool bProcessed = false;
    for (size_t ni = 0; ni < count && !bProcessed; ++ni)
    {
        bProcessed = ProcessRomanNumeralType(line, romanNumerals[ni], bonus, count - ni);
    }
    return bProcessed;
}

bool WikiItemFileProcessor::ProcessRomanNumeralType(const std::string& line, const std::string& numeral, const std::string& bonus, size_t index)
{
    bool bProcessed = false;
    if (!bProcessed) bProcessed = ProcessWizardryRomanNumeral(line, numeral, bonus, index * 25);
    if (!bProcessed) bProcessed = ProcessSpellFocusRomanNumeral(line, numeral, bonus, index * 25);
    if (!bProcessed) bProcessed = ProcessParryingRomanNumeral(line, numeral, bonus, index * 10);
    if (!bProcessed) bProcessed = ProcessPowerNumeral(line, numeral, bonus, index * 10);
    if (!bProcessed) bProcessed = ProcessSpellPenetrationNumeral(line, numeral, bonus, index);
    return bProcessed;
}

bool WikiItemFileProcessor::ProcessBonusType(const std::string& line, const std::string& bonus)
{
    bool bProcessed = false;

    if (!bProcessed) bProcessed = ProcessAbility(line, bonus);   // look for all ability bonuses of each type
    if (!bProcessed) bProcessed = ProcessFortification(line, bonus);
    if (!bProcessed) bProcessed = ProcessPhysicalSheltering(line, bonus);
    if (!bProcessed) bProcessed = ProcessMagicalSheltering(line, bonus);
    if (!bProcessed) bProcessed = ProcessSheltering(line, bonus);
    if (!bProcessed) bProcessed = ProcessIncite(line, bonus);
    if (!bProcessed) bProcessed = ProcessCombatMastery(line, bonus);
    if (!bProcessed) bProcessed = ProcessArmorMastery(line, bonus);
    if (!bProcessed) bProcessed = ProcessSpellResistance(line, bonus);
    if (!bProcessed) bProcessed = ProcessSpellPower(line, bonus);
    if (!bProcessed) bProcessed = ProcessSpellLore(line, bonus);
    if (!bProcessed) bProcessed = ProcessWizardryNumeric(line, bonus);
    if (!bProcessed) bProcessed = ProcessSpellFocusNumeric(line, bonus);
    if (!bProcessed) bProcessed = ProcessSeeker(line, bonus);
    if (!bProcessed) bProcessed = ProcessDodge(line, bonus);
    if (!bProcessed) bProcessed = ProcessAttackBonus(line, bonus);
    if (!bProcessed) bProcessed = ProcessParrying(line, bonus);
    if (!bProcessed) bProcessed = ProcessWillSave(line, bonus);
    if (!bProcessed) bProcessed = ProcessFortitudeSave(line, bonus);
    if (!bProcessed) bProcessed = ProcessReflexSave(line, bonus);
    if (!bProcessed) bProcessed = ProcessSpellSave(line, bonus);
    if (!bProcessed) bProcessed = ProcessCurseSave(line, bonus);
    if (!bProcessed) bProcessed = ProcessResistance(line, bonus);
    if (!bProcessed) bProcessed = ProcessAbsorption(line, bonus);
    if (!bProcessed) bProcessed = ProcessDoubleshot(line, bonus);
    if (!bProcessed) bProcessed = ProcessDoublestrike(line, bonus);
    if (!bProcessed) bProcessed = ProcessSpellPenetration(line, bonus);
    if (!bProcessed) bProcessed = ProcessArmorPiercing(line, bonus);
    if (!bProcessed) bProcessed = ProcessAccuracy(line, bonus);
    if (!bProcessed) bProcessed = ProcessDeadly(line, bonus);
    if (!bProcessed) bProcessed = ProcessEnergyResistance(line, bonus);
    if (!bProcessed) bProcessed = ProcessEnchantmentSave(line, bonus);
    if (!bProcessed) bProcessed = ProcessNaturalArmor(line, bonus);
    if (!bProcessed) bProcessed = ProcessFalseLife(line, bonus);
    if (!bProcessed) bProcessed = ProcessDiversion(line, bonus);
    if (!bProcessed) bProcessed = ProcessDistantDiversion(line, bonus);
    if (!bProcessed) bProcessed = ProcessMysticDiversion(line, bonus);
    if (!bProcessed) bProcessed = ProcessHealingAmplification(line, bonus);
    if (!bProcessed) bProcessed = ProcessRepairAmplification(line, bonus);
    if (!bProcessed) bProcessed = ProcessNegativeHealingAmplification(line, bonus);
    if (!bProcessed) bProcessed = ProcessDeception(line, bonus);
    if (!bProcessed) bProcessed = ProcessSneakAttack(line, bonus);
    if (!bProcessed) bProcessed = ProcessCombatAction(line, bonus, "Assassinate");
    if (!bProcessed) bProcessed = ProcessCombatAction(line, bonus, "Stunning");
    if (!bProcessed) bProcessed = ProcessCombatAction(line, bonus, "Shatter");
    if (!bProcessed) bProcessed = ProcessCombatAction(line, bonus, "Vertigo");
    if (!bProcessed) bProcessed = ProcessSkill(line, bonus);
    if (!bProcessed) bProcessed = ProcessAlluringSkills(line, bonus);
    if (!bProcessed) bProcessed = ProcessAstuteSkills(line, bonus);

    return bProcessed;
}

bool WikiItemFileProcessor::ProcessAbility(const std::string& line, const std::string& bonus)
{
    bool bProcessed = false;
    for (size_t i = Ability_Unknown + 1; i < Ability_Count && !bProcessed; ++i)
    {
        CString ability = EnumEntryText((AbilityType)i, abilityTypeMap);
        CString searchText;
        if (bonus != "Enhancement")
        {
            searchText.Format("%s %s", bonus.c_str(), (LPCSTR)ability);
            bProcessed = AddCommonEffect(line, "AbilityBonus", (LPCSTR)searchText, bonus, (LPCSTR)ability);
        }
        else
        {
            searchText.Format("%s", (LPCSTR)ability);
            bProcessed = AddCommonEffect(line, "AbilityBonus", (LPCSTR)searchText, bonus, (LPCSTR)ability);
        }
    }
    return bProcessed;
}

bool WikiItemFileProcessor::ProcessFortification(const std::string& line, const std::string& bonus)
{
    // Fortification
    bool bProcessed = false;
    CString searchText;
    if (bonus != "Enhancement")
    {
        searchText.Format("%s Fortification", bonus.c_str());
        bProcessed |= AddCommonEffect(line, "Fortification", (LPCSTR)searchText, bonus, "");
    }
    else
    {
        searchText = ("Fortification ");
        bProcessed = AddCommonEffect(line, "Fortification", (LPCSTR)searchText, bonus, "");
    }
    return bProcessed;
}

bool WikiItemFileProcessor::ProcessPhysicalSheltering(const std::string& line, const std::string& bonus)
{
    // sheltering
    bool bProcessed = false;
    CString searchText;
    if (bonus != "Enhancement")
    {
        searchText.Format("%s Physical Sheltering", bonus.c_str());
        bProcessed |= AddCommonEffect(line, "PhysicalSheltering", (LPCSTR)searchText, bonus, "");
    }
    else
    {
        searchText = ("Physical Sheltering");
        bProcessed = AddCommonEffect(line, "PhysicalSheltering", (LPCSTR)searchText, bonus, "");
    }
    return bProcessed;
}
bool WikiItemFileProcessor::ProcessMagicalSheltering(const std::string& line, const std::string& bonus)
{
    // sheltering
    bool bProcessed = false;
    CString searchText;
    if (bonus != "Enhancement")
    {
        searchText.Format("%s Magical Sheltering", bonus.c_str());
        bProcessed |= AddCommonEffect(line, "MagicalSheltering", (LPCSTR)searchText, bonus, "");
    }
    else
    {
        searchText = ("Magical Sheltering");
        bProcessed = AddCommonEffect(line, "MagicalSheltering", (LPCSTR)searchText, bonus, "");
    }
    return bProcessed;
}

bool WikiItemFileProcessor::ProcessSheltering(const std::string& line, const std::string& bonus)
{
    // sheltering
    bool bProcessed = false;
    CString searchText;
    if (bonus != "Enhancement")
    {
        searchText.Format("%s Sheltering", bonus.c_str());
        bProcessed |= AddCommonEffect(line, "Sheltering", (LPCSTR)searchText, bonus, "");
    }
    else
    {
        searchText = ("Sheltering");
        bProcessed = AddCommonEffect(line, "Sheltering", (LPCSTR)searchText, bonus, "");
    }
    return bProcessed;
}

bool WikiItemFileProcessor::ProcessCombatMastery(const std::string& line, const std::string& bonus)
{
    // combat mastery
    bool bProcessed = false;
    CString searchText;
    if (bonus != "Enhancement")
    {
        searchText.Format("%s Combat Mastery", bonus.c_str());
        bProcessed = AddCommonEffect(line, "CombatMastery", (LPCSTR)searchText, bonus, "");
    }
    else
    {
        searchText = "Combat Mastery";
        bProcessed = AddCommonEffect(line, "CombatMastery", (LPCSTR)searchText, bonus, "");
    }
    return bProcessed;
}

bool WikiItemFileProcessor::ProcessArmorMastery(const std::string& line, const std::string& bonus)
{
    // combat mastery
    bool bProcessed = false;
    CString searchText;
    if (bonus != "Enhancement")
    {
        searchText.Format("%s Armor Mastery", bonus.c_str());
        bProcessed = AddCommonEffect(line, "ArmorMastery", (LPCSTR)searchText, bonus, "");
    }
    else
    {
        searchText = "Armor Mastery";
        bProcessed = AddCommonEffect(line, "ArmorMastery", (LPCSTR)searchText, bonus, "");
    }
    return bProcessed;
}

bool WikiItemFileProcessor::ProcessSpellResistance(const std::string& line, const std::string& bonus)
{
    // Spell Resistance
    bool bProcessed = false;
    CString searchText;
    if (bonus != "Enhancement")
    {
        searchText.Format("%s Spell Resistance", bonus.c_str());
        bProcessed |= AddCommonEffect(line, "SpellResistance", (LPCSTR)searchText, bonus, "");
    }
    else
    {
        bProcessed |= AddCommonEffect(line, "SpellResistance", "Spell Resistance", bonus, "");
    }
    return bProcessed;
}

bool WikiItemFileProcessor::ProcessSpellPower(const std::string& line, const std::string& bonus)
{
    bool bProcessed = false;
    // spell power effects
    static std::string spellPowers[] =
    {
        "Combustion",
        "Corrosion",
        "Devotion",
        "Glaciation",
        "Impulse",
        "Magnetism",
        "Nullification",
        "Potency",
        "Radiance",
        "Reconstruction",
        "Resonance",
        "Universal"
    };
    size_t spellPowerCount = sizeof(spellPowers) / sizeof(std::string);
    CString searchText;
    for (size_t spi = 0; spi < spellPowerCount && !bProcessed; ++spi)
    {
        if (bonus != "Equipment")
        {
            searchText.Format("%s %s", bonus.c_str(), spellPowers[spi].c_str());
            bProcessed = AddCommonEffect(line, spellPowers[spi], (LPCSTR)searchText, bonus, spellPowers[spi]);
        }
        else
        {
            searchText.Format("%s", spellPowers[spi].c_str());
            bProcessed = AddCommonEffect(line, spellPowers[spi], (LPCSTR)searchText, bonus, spellPowers[spi]);
        }
    }
    return bProcessed;
}

bool WikiItemFileProcessor::ProcessSpellLore(const std::string& line, const std::string& bonus)
{
    bool bProcessed = false;
    static std::string lores[] =
    {
        "AcidLore", "Acid Lore",
        "BlightedLore", "Blighted Lore",
        "CreepingDustLore", "Creeping Dust Lore",
        "DarkRestorationLore", "Dark Restoration Lore",
        "FireLore", "Fire Lore",
        "FrozenDepthsLore", "Frozen Depths Lore",
        "IceLore", "Ice Lore",
        "KineticLore", "Kinetic Lore",
        "LightningLore", "Lightning Lore",
        "PowerOfTheSacredGround", "Power of the Sacred Ground",
        "PowerOfTheBlight", "Power of the Blight",
        "SacredGroundLore", "Sacred Ground Lore",
        "UniversalSpellLore", "Universal Spell Lore",
        "SonicLore", "Sonic Lore",
        "SpellLore", "Spell Lore",
        "VoidLore", "Void Lore",
        "PowerOfTheFrozenStorm", "Power of the Frozen Storm"
    };
    size_t count = sizeof(lores) / sizeof(std::string);
    if (count % 2 != 0) throw "Must be an even number";

    CString searchText;
    for (size_t sli = 0; sli < count && !bProcessed; sli += 2)
    {
        if (bonus != "Equipment")
        {
            searchText.Format("%s %s", bonus.c_str(), lores[sli + 1].c_str());
            bProcessed = AddCommonEffect(line, lores[sli], (LPCSTR)searchText, bonus, "");
        }
        else
        {
            bProcessed = AddCommonEffect(line, lores[sli], lores[sli + 1], bonus, "");
        }
    }
    return bProcessed;
}

bool WikiItemFileProcessor::ProcessSkill(const std::string& line, const std::string& bonus)
{
    bool bProcessed = false;
    // skill effects
    CString searchText;
    for (size_t si = Skill_Unknown + 1; si < Skill_Count && !bProcessed; ++si)
    {
        CString skill = EnumEntryText((SkillType)si, skillTypeMap);
        if (bonus != "Competence")
        {
            searchText.Format("%s %s", bonus.c_str(), (LPCSTR)skill);
            bProcessed = AddCommonEffect(line, "SkillBonus", (LPCSTR)searchText, bonus, (LPCTSTR)skill);
        }
        else
        {
            searchText = skill;
            bProcessed = AddCommonEffect(line, "SkillBonus", (LPCSTR)searchText, bonus, (LPCTSTR)skill);
        }
        if (!bProcessed)
        {
            searchText.Format("Enhanced %s", (LPCSTR)skill);
            bProcessed = AddCommonEffect(line, "SkillBonus", (LPCSTR)searchText, "Enhancement", (LPCTSTR)skill);
        }
    }
    return bProcessed;
}

bool WikiItemFileProcessor::ProcessAlluringSkills(const std::string& line, const std::string& bonus)
{
    bool bProcessed = false;
    CString searchText;
    if (bonus != "Enhancement")
    {
        searchText.Format("%s Alluring Skills Bonus", bonus.c_str());
        bProcessed |= AddCommonEffect(line, "Alluring Skills Bonus", (LPCSTR)searchText, bonus, "");
    }
    else
    {
        bProcessed |= AddCommonEffect(line, "Alluring Skills Bonus", "Alluring Skills Bonus", bonus, "");
    }
    return bProcessed;
}

bool WikiItemFileProcessor::ProcessAstuteSkills(const std::string& line, const std::string& bonus)
{
    bool bProcessed = false;
    CString searchText;
    if (bonus != "Enhancement")
    {
        searchText.Format("%s Astute Skills Bonus", bonus.c_str());
        bProcessed |= AddCommonEffect(line, "Astute Skills Bonus", (LPCSTR)searchText, bonus, "");
    }
    else
    {
        bProcessed |= AddCommonEffect(line, "Astute Skills Bonus", "Astute Skills Bonus", bonus, "");
    }
    return bProcessed;
}

bool WikiItemFileProcessor::ProcessIncite(const std::string& line, const std::string& bonus)
{
    // Incite
    bool bProcessed = false;
    CString searchText;
    if (bonus != "Enhancement")
    {
        searchText.Format("%s Incite", bonus.c_str());
        bProcessed |= AddCommonEffect(line, "Incite", (LPCSTR)searchText, bonus, "");
    }
    else
    {
        bProcessed |= AddCommonEffect(line, "Incite", "Incite", bonus, "");
    }
    return bProcessed;
}

bool WikiItemFileProcessor::ProcessWizardryNumeric(const std::string& line, const std::string& bonus)
{
    bool bProcessed = false;
    CString searchText;
    // wizardry +xxx
    if (bonus != "Enhancement")
    {
        searchText.Format("%s Wizardry", bonus.c_str());
        bProcessed = AddCommonEffect(line, "WizardryNumber", (LPCSTR)searchText, bonus, "");
    }
    else
    {
        bProcessed = AddCommonEffect(line, "WizardryNumber", "Wizardry", bonus, "");
    }
    return bProcessed;
}

bool WikiItemFileProcessor::ProcessWizardryRomanNumeral(
    const std::string& line, const std::string& numeral, const std::string& bonus, size_t value)
{
    bool bProcessed = false;
    CString text;
    if (bonus != "Enhancement")
    {
        text.Format("%s Wizardry %s", bonus.c_str(), numeral.c_str());
    }
    else
    {
        text.Format("Wizardry %s", numeral.c_str());
    }
    size_t pos = line.find((LPCTSTR)text, 0);
    if (pos != std::string::npos)
    {
        Buff b;
        b.Set_Type("Wizardry");
        b.Set_BonusType(bonus);
        b.Set_Value1(value);
        b.Set_Item(numeral);
        m_item.AddBuff(b);
        bProcessed = true;
    }
    return bProcessed;
}

bool WikiItemFileProcessor::ProcessSpellFocusNumeric(const std::string& line, const std::string& bonus)
{
    bool bProcessed = false;
    CString searchText;
    // Spell Focus Mastery +n
    if (bonus != "Equipment")
    {
        searchText.Format("%s Spell Focus Mastery", bonus.c_str());
        bProcessed = AddCommonEffect(line, "SpellFocusMastery", (LPCSTR)searchText, bonus, "");
    }
    else
    {
        bProcessed = AddCommonEffect(line, "SpellFocusMastery", "Spell Focus Mastery", bonus, "");
    }

    if (!bProcessed)
    {
        // Spell Focus +n
        if (bonus != "Equipment")
        {
            searchText.Format("%s Spell Focus", bonus.c_str());
            bProcessed = AddCommonEffect(line, "SpellFocusNumber", (LPCSTR)searchText, bonus, "");
        }
        else
        {
            bProcessed = AddCommonEffect(line, "SpellFocusNumber", "Spell Focus", bonus, "");
        }
    }
    // look for a specific spell school focus type
    if (!bProcessed)
    {
        for (size_t i = SpellSchool_Unknown + 1; i < SpellSchool_Count && !bProcessed; ++i)
        {
            CString school;
            school = EnumEntryText((SpellSchoolType)i, spellSchoolTypeMap);
            if (bonus != "Equipment")
            {
                searchText.Format("%s %s Focus", bonus.c_str(), (LPCTSTR)school);
                bProcessed = AddCommonEffect(line, "SchoolFocusNumber", (LPCSTR)searchText, bonus, (LPCSTR)school);
            }
            else
            {
                searchText.Format("%s Focus", (LPCTSTR)school);
                bProcessed = AddCommonEffect(line, "SchoolFocusNumber", (LPCSTR)searchText, bonus, (LPCSTR)school);
            }
        }
    }
    return bProcessed;
}

bool WikiItemFileProcessor::ProcessSpellFocusRomanNumeral(const std::string& line, const std::string& numeral, const std::string& bonus, size_t value)
{
    bool bProcessed = false;
    CString searchText;
    if (bonus != "Equipment")
    {
        searchText.Format("%s Spell Focus %s", bonus.c_str(), numeral.c_str());
    }
    else
    {
        searchText.Format("Spell Focus %s", numeral.c_str());
    }
    size_t pos = line.find((LPCTSTR)searchText, 0);
    if (pos != std::string::npos)
    {
        Buff b;
        b.Set_Type("SpellFocus");
        b.Set_BonusType(bonus);
        b.Set_Value1(value);
        b.Set_Item(numeral);
        m_item.AddBuff(b);
        bProcessed = true;
    }
    // look for a specific spell school focus type
    if (!bProcessed)
    {
        for (size_t i = SpellSchool_Unknown + 1; i < SpellSchool_Count && !bProcessed; ++i)
        {
            CString school;
            school = EnumEntryText((SpellSchoolType)i, spellSchoolTypeMap);
            if (bonus != "Equipment")
            {
                searchText.Format("%s %s Focus", bonus.c_str(), (LPCTSTR)school);
            }
            else
            {
                searchText.Format("%s Focus", (LPCTSTR)school);
            }
        }
        pos = line.find((LPCTSTR)searchText, 0);
        if (pos != std::string::npos)
        {
            Buff b;
            b.Set_Type("SchoolFocus");
            b.Set_BonusType(bonus);
            b.Set_Value1(value);
            b.Set_Item(numeral);
            m_item.AddBuff(b);
            bProcessed = true;
        }
    }
    return bProcessed;
}

bool WikiItemFileProcessor::ProcessSeeker(const std::string& line, const std::string& bonus)
{
    bool bProcessed = false;
    CString searchText;
    // wizardry +xxx
    if (bonus != "Enhancement")
    {
        searchText.Format("%s Seeker", bonus.c_str());
        bProcessed = AddCommonEffect(line, "Seeker", (LPCSTR)searchText, bonus, "");
    }
    else
    {
        bProcessed = AddCommonEffect(line, "Seeker", "Seeker", bonus, "");
    }
    return bProcessed;
}

bool WikiItemFileProcessor::ProcessDodge(const std::string& line, const std::string& bonus)
{
    bool bProcessed = false;
    CString searchText;
    // wizardry +xxx
    if (bonus != "Enhancement")
    {
        searchText.Format("%s Dodge", bonus.c_str());
        bProcessed = AddCommonEffect(line, "Dodge", (LPCSTR)searchText, bonus, "");
    }
    else
    {
        bProcessed = AddCommonEffect(line, "Dodge", "Dodge", bonus, "");
    }
    return bProcessed;
}

bool WikiItemFileProcessor::ProcessAttackBonus(const std::string& line, const std::string& bonus)
{
    bool bProcessed = false;
    CString searchText;
    // wizardry +xxx
    if (bonus != "Competence")
    {
        searchText.Format("%s Attack Bonus", bonus.c_str());
        bProcessed = AddCommonEffect(line, "AttackBonus", (LPCSTR)searchText, bonus, "");
    }
    else
    {
        bProcessed = AddCommonEffect(line, "AttackBonus", "Attack Bonus", bonus, "");
    }
    return bProcessed;
}

bool WikiItemFileProcessor::ProcessParrying(const std::string& line, const std::string& bonus)
{
    bool bProcessed = false;
    CString searchText;
    // Spell Focus +n
    if (bonus != "Insightful")
    {
        searchText.Format("%s Parrying", bonus.c_str());
        bProcessed = AddCommonEffect(line, "ParryingNumber", (LPCSTR)searchText, bonus, "");
    }
    else
    {
        bProcessed = AddCommonEffect(line, "ParryingNumber", "Parrying", bonus, "");
    }
    return bProcessed;
}

bool WikiItemFileProcessor::ProcessParryingRomanNumeral(const std::string& line, const std::string& numeral, const std::string& bonus, size_t value)
{
    bool bProcessed = false;
    CString searchText;
    if (bonus != "Insightful")
    {
        searchText.Format("%s Parrying %s", bonus.c_str(), numeral.c_str());
    }
    else
    {
        searchText.Format("Parrying %s", numeral.c_str());
    }
    size_t pos = line.find((LPCTSTR)searchText, 0);
    if (pos != std::string::npos)
    {
        Buff b;
        b.Set_Type("Parrying");
        b.Set_BonusType(bonus);
        b.Set_Value1(value);
        b.Set_Item(numeral);
        m_item.AddBuff(b);
        bProcessed = true;
    }
    return bProcessed;
}

bool WikiItemFileProcessor::ProcessPowerNumeral(const std::string& line, const std::string& numeral, const std::string& bonus, size_t value)
{
    bool bProcessed = false;
    CString searchText;
    searchText.Format("Power %s", numeral.c_str());
    size_t pos = line.find((LPCTSTR)searchText, 0);
    if (pos != std::string::npos)
    {
        Buff b;
        b.Set_Type("Power");
        b.Set_BonusType(bonus);
        b.Set_Value1(value);
        b.Set_Item(numeral);
        m_item.AddBuff(b);
        bProcessed = true;
    }
    return bProcessed;
}

bool WikiItemFileProcessor::ProcessSpellPenetrationNumeral(const std::string& line, const std::string& numeral, const std::string& bonus, size_t value)
{
    bool bProcessed = false;
    CString searchText;
    searchText.Format("%s Spell Penetration %s", bonus.c_str(), numeral.c_str());
    size_t pos = line.find((LPCTSTR)searchText, 0);
    if (pos != std::string::npos)
    {
        Buff b;
        b.Set_Type("SpellPenetrationNumeral");
        b.Set_BonusType(bonus);
        b.Set_Value1(value);
        b.Set_Item(numeral);
        m_item.AddBuff(b);
        bProcessed = true;
    }
    return bProcessed;
}

bool WikiItemFileProcessor::ProcessWillSave(const std::string& line, const std::string& bonus)
{
    bool bProcessed = false;
    CString searchText;
    // Spell Focus +n
    if (bonus != "Resistance")
    {
        searchText.Format("%s Will Save", bonus.c_str());
        bProcessed = AddCommonEffect(line, "WillSave", (LPCSTR)searchText, bonus, "");
    }
    else
    {
        bProcessed = AddCommonEffect(line, "WillSave", "Will Save", bonus, "");
    }
    return bProcessed;
}

bool WikiItemFileProcessor::ProcessFortitudeSave(const std::string& line, const std::string& bonus)
{
    bool bProcessed = false;
    CString searchText;
    // Spell Focus +n
    if (bonus != "Resistance")
    {
        searchText.Format("%s Fortitude Save", bonus.c_str());
        bProcessed = AddCommonEffect(line, "ForitudeSave", (LPCSTR)searchText, bonus, "");
    }
    else
    {
        bProcessed = AddCommonEffect(line, "FortitudeSave", "Fortitude Save", bonus, "");
    }
    return bProcessed;
}

bool WikiItemFileProcessor::ProcessReflexSave(const std::string& line, const std::string& bonus)
{
    bool bProcessed = false;
    CString searchText;
    // Spell Focus +n
    if (bonus != "Resistance")
    {
        searchText.Format("%s Reflex Save", bonus.c_str());
        bProcessed = AddCommonEffect(line, "ReflexSave", (LPCSTR)searchText, bonus, "");
    }
    else
    {
        bProcessed = AddCommonEffect(line, "ReflexSave", "Reflex Save", bonus, "");
    }
    return bProcessed;
}

bool WikiItemFileProcessor::ProcessSpellSave(const std::string& line, const std::string& bonus)
{
    bool bProcessed = false;
    CString searchText;
    // Spell Focus +n
    if (bonus != "Resistance")
    {
        searchText.Format("%s Spell Save", bonus.c_str());
        bProcessed = AddCommonEffect(line, "SpellSave", (LPCSTR)searchText, bonus, "");
    }
    else
    {
        bProcessed = AddCommonEffect(line, "SpellSave", "Spell Save", bonus, "");
    }
    return bProcessed;
}

bool WikiItemFileProcessor::ProcessCurseSave(const std::string& line, const std::string& bonus)
{
    bool bProcessed = false;
    CString searchText;
    // Spell Focus +n
    if (bonus != "Resistance")
    {
        searchText.Format("%s Curse Resistance", bonus.c_str());
        bProcessed = AddCommonEffect(line, "CurseResistance", (LPCSTR)searchText, bonus, "");
    }
    else
    {
        bProcessed = AddCommonEffect(line, "CurseResistance", "Curse Resistance", bonus, "");
    }
    return bProcessed;
}

bool WikiItemFileProcessor::ProcessResistance(const std::string& line, const std::string& bonus)
{
    bool bProcessed = false;
    CString searchText;
    // Spell Focus +n
    if (bonus != "Resistance")
    {
        searchText.Format("%s Resistance", bonus.c_str());
        bProcessed = AddCommonEffect(line, "Resistance", (LPCSTR)searchText, bonus, "");
    }
    else
    {
        bProcessed = AddCommonEffect(line, "Resistance", "Resistance", bonus, "");
    }
    return bProcessed;
}

bool WikiItemFileProcessor::ProcessAbsorption(const std::string& line, const std::string& bonus)
{
    bool bProcessed = false;
    CString searchText;
    // energy absorption
    for (size_t i = Energy_Unknown + 1; i < Energy_Count && !bProcessed; ++i)
    {
        CString energy;
        energy = EnumEntryText((EnergyType)i, energyTypeMap);
        if (bonus != "Enhancement")
        {
            searchText.Format("%s %s Absorption", bonus.c_str(), (LPCTSTR)energy);
            bProcessed = AddCommonEffect(line, "Absorption", (LPCSTR)searchText, bonus, (LPCTSTR)energy);
        }
        else
        {
            searchText.Format("%s Absorption", (LPCTSTR)energy);
            bProcessed = AddCommonEffect(line, "Absorption", (LPCSTR)searchText, bonus, (LPCTSTR)energy);
        }
        if (i == Energy_Lawful && !bProcessed)
        {
            if (bonus != "Enhancement")
            {
                searchText.Format("%s Law Absorption", bonus.c_str());
                bProcessed = AddCommonEffect(line, "Absorption", (LPCSTR)searchText, bonus, (LPCTSTR)energy);
            }
            else
            {
                searchText = "Law Absorption";
                bProcessed = AddCommonEffect(line, "Absorption", (LPCSTR)searchText, bonus, (LPCTSTR)energy);
            }
        }
    }
    return bProcessed;
}

bool WikiItemFileProcessor::ProcessDoublestrike(const std::string& line, const std::string& bonus)
{
    bool bProcessed = false;
    CString searchText;
    if (bonus != "Enhancement")
    {
        searchText.Format("%s Doublestrike", bonus.c_str());
        bProcessed = AddCommonEffect(line, "Doublestrike", (LPCSTR)searchText, bonus, "");
    }
    else
    {
        bProcessed = AddCommonEffect(line, "Doublestrike", "Doublestrike", bonus, "");
    }
    return bProcessed;
}

bool WikiItemFileProcessor::ProcessDoubleshot(const std::string& line, const std::string& bonus)
{
    bool bProcessed = false;
    CString searchText;
    if (bonus != "Enhancement")
    {
        searchText.Format("%s Doubleshot", bonus.c_str());
        bProcessed = AddCommonEffect(line, "Doubleshot", (LPCSTR)searchText, bonus, "");
    }
    else
    {
        bProcessed = AddCommonEffect(line, "Doubleshot", "Doubleshot", bonus, "");
    }
    return bProcessed;
}

bool WikiItemFileProcessor::ProcessSpellPenetration(const std::string& line, const std::string& bonus)
{
    bool bProcessed = false;
    CString searchText;
    if (bonus != "Equipment")
    {
        searchText.Format("%s Spell Penetration", bonus.c_str());
        bProcessed = AddCommonEffect(line, "SpellPenetration", (LPCSTR)searchText, bonus, "");
    }
    else
    {
        bProcessed = AddCommonEffect(line, "SpellPenetration", "Spell Penetration", bonus, "");
    }
    return bProcessed;
}

bool WikiItemFileProcessor::ProcessArmorPiercing(const std::string& line, const std::string& bonus)
{
    bool bProcessed = false;
    CString searchText;
    if (bonus != "Enhancement")
    {
        searchText.Format("%s Armor-Piercing -", bonus.c_str());
        bProcessed = AddCommonEffect(line, "ArmorPiercing", (LPCSTR)searchText, bonus, "");
    }
    else
    {
        bProcessed = AddCommonEffect(line, "ArmorPiercing", "Armor-Piercing -", bonus, "");
    }
    return bProcessed;
}

bool WikiItemFileProcessor::ProcessAccuracy(const std::string& line, const std::string& bonus)
{
    bool bProcessed = false;
    CString searchText;
    if (bonus != "Competence")
    {
        searchText.Format("%s Accuracy", bonus.c_str());
        bProcessed = AddCommonEffect(line, "Accuracy", (LPCSTR)searchText, bonus, "");
    }
    else
    {
        bProcessed = AddCommonEffect(line, "Accuracy", "Accuracy", bonus, "");
    }
    return bProcessed;
}

bool WikiItemFileProcessor::ProcessDeadly(const std::string& line, const std::string& bonus)
{
    bool bProcessed = false;
    CString searchText;
    if (bonus != "Competence")
    {
        searchText.Format("%s Deadly", bonus.c_str());
        bProcessed = AddCommonEffect(line, "Deadly", (LPCSTR)searchText, bonus, "");
    }
    else
    {
        bProcessed = AddCommonEffect(line, "Deadly", "Deadly", bonus, "");
    }
    return bProcessed;
}

bool WikiItemFileProcessor::ProcessEnergyResistance(const std::string& line, const std::string& bonus)
{
    bool bProcessed = false;
    CString searchText;
    // energy absorption
    for (size_t i = Energy_Unknown + 1; i < Energy_Count && !bProcessed; ++i)
    {
        CString energy;
        energy = EnumEntryText((EnergyType)i, energyTypeMap);
        if (bonus != "Enhancement")
        {
            searchText.Format("%s %s Resistance", bonus.c_str(), (LPCTSTR)energy);
            bProcessed = AddCommonEffect(line, "EnergyResistance", (LPCSTR)searchText, bonus, (LPCTSTR)energy);
        }
        else
        {
            searchText.Format("%s Resistance", (LPCTSTR)energy);
            bProcessed = AddCommonEffect(line, "EnergyResistance", (LPCSTR)searchText, bonus, (LPCTSTR)energy);
        }
    }
    return bProcessed;
}

bool WikiItemFileProcessor::ProcessEnchantmentSave(const std::string& line, const std::string& bonus)
{
    bool bProcessed = false;
    CString searchText;
    if (bonus != "Resistance")
    {
        searchText.Format("%s Enchantment Save", bonus.c_str());
        bProcessed = AddCommonEffect(line, "EnchantmentSave", (LPCSTR)searchText, bonus, "");
    }
    else
    {
        bProcessed = AddCommonEffect(line, "EnchantmentSave", "Enchantment Save", bonus, "");
    }
    return bProcessed;
}

bool WikiItemFileProcessor::ProcessNaturalArmor(const std::string& line, const std::string& bonus)
{
    bool bProcessed = false;
    CString searchText;
    if (bonus != "Resistance")
    {
        searchText.Format("%s Natural Armor", bonus.c_str());
        bProcessed = AddCommonEffect(line, "NaturalArmor", (LPCSTR)searchText, bonus, "");
    }
    else
    {
        bProcessed = AddCommonEffect(line, "NaturalArmor", "Natural Armor", bonus, "");
    }
    return bProcessed;
}

bool WikiItemFileProcessor::ProcessFalseLife(const std::string& line, const std::string& bonus)
{
    bool bProcessed = false;
    CString searchText;
    if (bonus != "Enhancement")
    {
        searchText.Format("%s False Life", bonus.c_str());
        bProcessed = AddCommonEffect(line, "FalseLife", (LPCSTR)searchText, bonus, "");
    }
    else
    {
        bProcessed = AddCommonEffect(line, "FalseLife", "False Life", bonus, "");
    }
    return bProcessed;
}

bool WikiItemFileProcessor::ProcessDiversion(const std::string& line, const std::string& bonus)
{
    bool bProcessed = false;
    CString searchText;
    if (bonus != "Enhancement")
    {
        searchText.Format("%s Diversion", bonus.c_str());
        bProcessed = AddCommonEffect(line, "Diversion", (LPCSTR)searchText, bonus, "", 3);
    }
    else
    {
        bProcessed = AddCommonEffect(line, "Diversion", "Diversion", bonus, "", 3);
    }
    return bProcessed;
}

bool WikiItemFileProcessor::ProcessDistantDiversion(const std::string& line, const std::string& bonus)
{
    bool bProcessed = false;
    CString searchText;
    if (bonus != "Enhancement")
    {
        searchText.Format("%s Distant Diversion", bonus.c_str());
        bProcessed = AddCommonEffect(line, "DistantDiversion", (LPCSTR)searchText, bonus, "", 3);
    }
    else
    {
        bProcessed = AddCommonEffect(line, "DistantDiversion", "Distant Diversion", bonus, "", 3);
    }
    return bProcessed;
}

bool WikiItemFileProcessor::ProcessMysticDiversion(const std::string& line, const std::string& bonus)
{
    bool bProcessed = false;
    CString searchText;
    if (bonus != "Enhancement")
    {
        searchText.Format("%s Mystic Diversion", bonus.c_str());
        bProcessed = AddCommonEffect(line, "MysticDiversion", (LPCSTR)searchText, bonus, "", 3);
    }
    else
    {
    bProcessed = AddCommonEffect(line, "MysticDiversion", "Mystic Diversion", bonus, "", 3);
    }
    return bProcessed;
}

bool WikiItemFileProcessor::ProcessHealingAmplification(const std::string& line, const std::string& bonus)
{
    bool bProcessed = false;
    CString searchText;
    searchText.Format("%s Healing Amplification", bonus.c_str());
    bProcessed = AddCommonEffect(line, "HealingAmplification", (LPCSTR)searchText, bonus, "", 3);
    return bProcessed;
}

bool WikiItemFileProcessor::ProcessRepairAmplification(const std::string& line, const std::string& bonus)
{
    bool bProcessed = false;
    CString searchText;
    if (bonus != "Competence")
    {
        searchText.Format("%s Repair Amplification", bonus.c_str());
        bProcessed = AddCommonEffect(line, "RepairAmplification", (LPCSTR)searchText, bonus, "", 3);
    }
    else
    {
        bProcessed = AddCommonEffect(line, "RepairAmplification", "Repair Amplification", bonus, "", 3);
    }
    return bProcessed;
}

bool WikiItemFileProcessor::ProcessNegativeHealingAmplification(const std::string& line, const std::string& bonus)
{
    bool bProcessed = false;
    CString searchText;
    if (bonus != "Competence")
    {
        searchText.Format("%s Negative Amplification", bonus.c_str());
        bProcessed = AddCommonEffect(line, "NegativeAmplification", (LPCSTR)searchText, bonus, "", 3);
    }
    else
    {
        bProcessed = AddCommonEffect(line, "NegativeAmplification", "Negative Amplification", bonus, "", 3);
    }
    return bProcessed;
}

bool WikiItemFileProcessor::ProcessSpeed(const std::string& line)
{
    Buff b;
    b.Set_Type("SpeedRomanNumeral");
    b.Set_BonusType("Enhancement");
    bool bProcessed = false;
    if (line.find("Speed XV") != std::string::npos)
    {
        b.Set_Value1(30);
        b.Set_Value2(15);
        b.Set_Item("XV");
        m_item.AddBuff(b);
        bProcessed = true;
    }
    else if (line.find("Speed XIV") != std::string::npos)
    {
        b.Set_Value1(30);
        b.Set_Value2(14);
        b.Set_Item("XIV");
        m_item.AddBuff(b);
        bProcessed = true;
    }
    else if (line.find("Speed XIII") != std::string::npos)
    {
        b.Set_Value1(30);
        b.Set_Value2(13);
        b.Set_Item("XIII");
        m_item.AddBuff(b);
        bProcessed = true;
    }
    else if (line.find("Speed XII") != std::string::npos)
    {
        b.Set_Value1(30);
        b.Set_Value2(12);
        b.Set_Item("XII");
        m_item.AddBuff(b);
        bProcessed = true;
    }
    else if (line.find("Speed XI") != std::string::npos)
    {
        b.Set_Value1(30);
        b.Set_Value2(11);
        b.Set_Item("XI");
        m_item.AddBuff(b);
        bProcessed = true;
    }
    else if (line.find("Speed X") != std::string::npos)
    {
        b.Set_Value1(30);
        b.Set_Value2(10);
        b.Set_Item("X");
        m_item.AddBuff(b);
        bProcessed = true;
    }
    else if (line.find("Speed IX") != std::string::npos)
    {
        b.Set_Value1(30);
        b.Set_Value2(9);
        b.Set_Item("IX");
        m_item.AddBuff(b);
        bProcessed = true;
    }
    else if (line.find("Speed VIII") != std::string::npos)
    {
        b.Set_Value1(30);
        b.Set_Value2(8);
        b.Set_Item("VIII");
        m_item.AddBuff(b);
        bProcessed = true;
    }
    else if (line.find("Speed VII") != std::string::npos)
    {
        b.Set_Value1(30);
        b.Set_Value2(7);
        b.Set_Item("VII");
        m_item.AddBuff(b);
        bProcessed = true;
    }
    else if (line.find("Speed VI") != std::string::npos)
    {
        b.Set_Value1(30);
        b.Set_Value2(6);
        b.Set_Item("VI");
        m_item.AddBuff(b);
        bProcessed = true;
    }
    else if (line.find("Speed V") != std::string::npos)
    {
        b.Set_Value1(25);
        b.Set_Value2(5);
        b.Set_Item("V");
        m_item.AddBuff(b);
        bProcessed = true;
    }
    else if (line.find("Speed") <= 1)
    {
        // need to parse the string for the 2 numbers
        CString txt(line.c_str());
        txt.Replace("Speed", "");
        int percent = atoi(txt);
        int pos = txt.Find("to movement speed,");
        txt = txt.Right(txt.GetLength() - pos - 18);
        int alacrity = atoi(txt);
        b.Set_Type("Speed");
        b.Set_Value1(percent);
        b.Set_Value2(alacrity);
        m_item.AddBuff(b);
        bProcessed = true;
    }
    return bProcessed;
}

bool WikiItemFileProcessor::AddFavoredWeapon(const std::string& line)
{
    bool bProcessed = false;
    if (line.find("Favored Weapon Favored Weapon") != std::string::npos
            && m_item.HasWeapon())
    {
        WeaponType wt = m_item.Weapon();
        CString weapon = EnumEntryText(wt, weaponTypeMap);
        Buff b;
        b.Set_Type("Favored Weapon");
        b.Set_BonusType("Enhancement");
        b.Set_Item2((LPCTSTR)weapon);
        m_item.AddBuff(b);
        bProcessed = true;
    }
    return bProcessed;
}

bool WikiItemFileProcessor::ProcessDeception(const std::string& line, const std::string& bonus)
{
    bool bProcessed = false;
    CString searchText("Deception");
    if (bonus != "Enhancement")
    {
        searchText.Format("%s Deception", bonus.c_str());
    }
    size_t pos = line.find(searchText);
    if (pos != std::string::npos    // must be found
        && pos < 3)                 // and close to start of the line
    {
        CString txt = line.c_str();
        txt.Replace(searchText, "");
        double v1 = atof(txt);
        txt = txt.Right(txt.GetLength() - txt.Find("bonus to hit and") - 16);
        double v2 = atof(txt);
        Buff b;
        b.Set_Type("Deception");
        b.Set_BonusType(bonus);
        b.Set_Value1(v1);
        b.Set_Value1(v2);
        bProcessed = true;
    }
    return bProcessed;
}

bool WikiItemFileProcessor::ProcessSneakAttack(const std::string& line, const std::string& bonus)
{
    bool bProcessed = false;
    CString searchText("Sneak Attack");
    if (bonus != "Enhancement")
    {
        searchText.Format("%s Sneak Attack Bonus", bonus.c_str());
    }
    size_t pos = line.find(searchText);
    if (pos != std::string::npos    // must be found
        && pos < 3)                 // and close to start of the line
    {
        CString txt = line.c_str();
        txt.Replace(searchText, "");
        double v1 = atof(txt);
        txt = txt.Right(txt.GetLength() - txt.Find("attack bonus and a") - 18);
        double v2 = atof(txt);
        Buff b;
        b.Set_Type("Sneak Attack");
        b.Set_BonusType(bonus);
        b.Set_Value1(v1);
        b.Set_Value1(v2);
        bProcessed = true;
    }
    return bProcessed;
}

bool WikiItemFileProcessor::ProcessCombatAction(const std::string& line, const std::string& bonus, const std::string& type)
{
    bool bProcessed = false;
    CString searchText(type.c_str());
    if (bonus != "Enhancement")
    {
        searchText.Format("%s %s", bonus.c_str(), type.c_str());
    }
    bProcessed = AddCommonEffect(line, type, (LPCSTR)searchText, bonus, "", 3);
    return bProcessed;
}

bool WikiItemFileProcessor::ProcessKeen(const std::string& line)
{
    bool bRecognised = false;
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Keen", "Keen Keen :", "Enhancement", "", 20);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Impact", "Impact Impact: This weapon", "Enhancement", "", 20);
    if (!bRecognised) bRecognised |= AddCommonEffect(line, "Impact", "Impact Impact : Passive", "Enhancement", "", 20);
    if (bRecognised)
    {
        // need to adjust the critical threat range of this weapon to not include the Keen bonus
        if (m_item.HasWeapon())
        {
            WeaponType wt = m_item.Weapon();
            // from the wiki: This feat adds 1, 2, or 3 to critical threat
            // range based on the weapon type's unmodified threat range.
            // +3 for falchion, great crossbow, kukri, rapier, and scimitar.
            // +2 for bastard sword, dagger, greatsword, heavy crossbow, khopesh,
            // light crossbow, long sword, repeating heavy crossbow,
            // repeating light crossbow, short sword, and throwing dagger.
            // +1 to all other weapons.
            // Shields do not benefit from Improved Critical.
            size_t baseRange = 0;     // assume unknown
            switch (wt)
            {
            case Weapon_Falchion:
            case Weapon_GreatCrossbow:
            case Weapon_Kukri:
            case Weapon_Rapier:
            case Weapon_Scimitar:
                baseRange = +3;
                break;
            case Weapon_BastardSword:
            case Weapon_Dagger:
            case Weapon_GreatSword:
            case Weapon_HeavyCrossbow:
            case Weapon_Khopesh:
            case Weapon_LightCrossbow:
            case Weapon_Longsword:
            case Weapon_RepeatingHeavyCrossbow:
            case Weapon_RepeatingLightCrossbow:
            case Weapon_Shortsword:
            case Weapon_ThrowingDagger:
                baseRange = +2;
                break;
            case Weapon_ShieldBuckler:
            case Weapon_ShieldSmall:
            case Weapon_ShieldLarge:
            case Weapon_ShieldTower:
            case Weapon_Orb:
            case Weapon_RuneArm:
                baseRange = 0;
                break;
            default:
                // all other weapon types
                baseRange = +1;
                break;
            }
            size_t range = m_item.CriticalThreatRange();
            range -= baseRange;
            m_item.Set_CriticalThreatRange(range);
        }
    }
    return bRecognised;
}

bool WikiItemFileProcessor::AddCommonEffect(
    const std::string& line,
    const std::string& buffName,
    const std::string& searchText,
    const std::string& bonus,
    const std::string& item,
    size_t minStartPos /*defaults to 10*/)
{
    bool bRecognised = false;
    size_t pos = line.find(searchText);
    if (pos != std::string::npos    // must be found
            && pos < minStartPos)            // and close to start of the line
    {
        Buff b;
        b.Set_Type(buffName);
        b.Set_BonusType(bonus);
        CString text(line.c_str());
        text = text.Right(line.size() - pos - searchText.size());
        double value = atoi(text);
        if (value == 0)
        {
            // bas parse, try for a numeric at the start of the line
            value = atoi(line.c_str());
        }
        // do we have a value?
        if (value != 0)
        {
            b.Set_Value1(value);
        }
        if (item != "")
        {
            b.Set_Item(item);
        }
        m_item.AddBuff(b);
        bRecognised = true;
    }
    return bRecognised;
}
