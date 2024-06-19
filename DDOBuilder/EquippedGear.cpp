// EquippedGear.cpp
//
#include "StdAfx.h"
#include "EquippedGear.h"
#include "XmlLib\SaxWriter.h"
#include "GlobalSupportFunctions.h"
#include "LogPane.h"
#include "Character.h"
#include "DDOBuilder.h"

#define DL_ELEMENT EquippedGear

namespace
{
    const wchar_t f_saxElementName[] = L"EquippedGear";
    DL_DEFINE_NAMES(EquippedGear_PROPERTIES)

    const unsigned f_verCurrent = 1;
}

EquippedGear::EquippedGear(const std::string& name) :
    XmlLib::SaxContentElement(f_saxElementName, f_verCurrent),
    m_Helmet(L"Helmet"),
    m_Necklace(L"Necklace"),
    m_Trinket(L"Trinket"),
    m_Cloak(L"Cloak"),
    m_Belt(L"Belt"),
    m_Goggles(L"Goggles"),
    m_Gloves(L"Gloves"),
    m_Boots(L"Boots"),
    m_Bracers(L"Bracers"),
    m_Armor(L"Armor"),
    m_MainHand(L"MainHand"),
    m_OffHand(L"OffHand"),
    m_Quiver(L"Quiver"),
    m_Arrow(L"Arrow"),
    m_Ring1(L"Ring1"),
    m_Ring2(L"Ring2"),
    m_CosmeticArmor(L"CosmeticArmor"),
    m_CosmeticCloak(L"CosmeticCloak"),
    m_CosmeticHelm(L"CosmeticHelm"),
    m_CosmeticWeapon1(L"CosmeticWeapon1"),
    m_CosmeticWeapon2(L"CosmeticWeapon2")
{
    DL_INIT(EquippedGear_PROPERTIES)
    m_Name = name;
    // make sure we have the default artifact filigree items
    while (m_ArtifactFiligrees.size() < MAX_ARTIFACT_FILIGREE)
    {
        m_ArtifactFiligrees.push_back(ArtifactFiligree());
    }
}

DL_DEFINE_ACCESS(EquippedGear_PROPERTIES)

XmlLib::SaxContentElementInterface * EquippedGear::StartElement(
        const XmlLib::SaxString & name,
        const XmlLib::SaxAttributes & attributes)
{
    XmlLib::SaxContentElementInterface * subHandler =
            SaxContentElement::StartElement(name, attributes);

    DL_START(EquippedGear_PROPERTIES)

    return subHandler;
}

void EquippedGear::EndElement()
{
    // make sure we have the right amount of artifact filigree objects
    while (m_ArtifactFiligrees.size() > MAX_ARTIFACT_FILIGREE)
    {
        m_ArtifactFiligrees.pop_front();    // oldest are the ones not loaded
    }
    while (m_ArtifactFiligrees.size() < MAX_ARTIFACT_FILIGREE)
    {
        m_ArtifactFiligrees.push_back(ArtifactFiligree());
    }
    SaxContentElement::EndElement();
    DL_END(EquippedGear_PROPERTIES)
}

void EquippedGear::Write(XmlLib::SaxWriter * writer) const
{
    writer->StartElement(ElementName());//, VersionAttributes());
    DL_WRITE(EquippedGear_PROPERTIES)
    writer->EndElement();
}

void EquippedGear::SetName(const std::string& name)
{
    Set_Name(name);
}

void EquippedGear::UpdateImages()
{
    CDDOBuilderApp* pApp = static_cast<CDDOBuilderApp*>(AfxGetApp());
    for (size_t i = Inventory_Unknown + 1; i < Inventory_FinalDrawnItem; ++i)
    {
        InventorySlotType ist = static_cast<InventorySlotType>(i);
        if (HasItemInSlot(ist))
        {
            Item item = ItemInSlot(ist);
            if (item.HasIcon())
            {
                std::string icon = item.Icon();
                if (pApp->m_imagesMap.find(icon) != pApp->m_imagesMap.end())
                {
                    item.SetIconIndex(pApp->m_imagesMap[icon]);
                    UpdateItem(ist, item);
                }
            }
        }
    }
}

bool EquippedGear::HasItemInSlot(InventorySlotType slot) const
{
    switch (slot)
    {
    case Inventory_Arrows:          return HasArrow();
    case Inventory_Armor:           return HasArmor();
    case Inventory_Belt:            return HasBelt();
    case Inventory_Boots:           return HasBoots();
    case Inventory_Bracers:         return HasBracers();
    case Inventory_Cloak:           return HasCloak();
    case Inventory_Gloves:          return HasGloves();
    case Inventory_Goggles:         return HasGoggles();
    case Inventory_Helmet:          return HasHelmet();
    case Inventory_Necklace:        return HasNecklace();
    case Inventory_Quiver:          return HasQuiver();
    case Inventory_Ring1:           return HasRing1();
    case Inventory_Ring2:           return HasRing2();
    case Inventory_Trinket:         return HasTrinket();
    case Inventory_Weapon1:         return HasMainHand();
    case Inventory_Weapon2:         return HasOffHand();
    case Inventory_CosmeticArmor:   return HasCosmeticArmor();
    case Inventory_CosmeticCloak:   return HasCosmeticCloak();
    case Inventory_CosmeticHelm:    return HasCosmeticHelm();
    case Inventory_CosmeticWeapon1: return HasCosmeticWeapon1();
    case Inventory_CosmeticWeapon2: return HasCosmeticWeapon2();
    }
    return false;
}

Item EquippedGear::ItemInSlot(InventorySlotType slot) const
{
    Item noItem;
    switch (slot)
    {
    case Inventory_Arrows:
        if (HasArrow())
        {
            return Arrow();
        }
        break;
    case Inventory_Armor:
        if (HasArmor())
        {
            return Armor();
        }
        break;
    case Inventory_Belt:
        if (HasBelt())
        {
            return Belt();
        }
        break;
    case Inventory_Boots:
        if (HasBoots())
        {
            return Boots();
        }
        break;
    case Inventory_Bracers:
        if (HasBracers())
        {
            return Bracers();
        }
        break;
    case Inventory_Cloak:
        if (HasCloak())
        {
            return Cloak();
        }
        break;
    case Inventory_Gloves:
        if (HasGloves())
        {
            return Gloves();
        }
        break;
    case Inventory_Goggles:
        if (HasGoggles())
        {
            return Goggles();
        }
        break;
    case Inventory_Helmet:
        if (HasHelmet())
        {
            return Helmet();
        }
        break;
    case Inventory_Necklace:
        if (HasNecklace())
        {
            return Necklace();
        }
        break;
    case Inventory_Quiver:
        if (HasQuiver())
        {
            return Quiver();
        }
        break;
    case Inventory_Ring1:
        if (HasRing1())
        {
            return Ring1();
        }
        break;
    case Inventory_Ring2:
        if (HasRing2())
        {
            return Ring2();
        }
        break;
    case Inventory_Trinket:
        if (HasTrinket())
        {
            return Trinket();
        }
        break;
    case Inventory_Weapon1:
        if (HasMainHand())
        {
            return MainHand();
        }
        break;
    case Inventory_Weapon2:
        if (HasOffHand())
        {
            return OffHand();
        }
        break;
    case Inventory_CosmeticArmor:
        if (HasCosmeticArmor())
        {
            return CosmeticArmor();
        }
        break;
    case Inventory_CosmeticCloak:
        if (HasCosmeticCloak())
        {
            return CosmeticCloak();
        }
        break;
    case Inventory_CosmeticHelm:
        if (HasCosmeticHelm())
        {
            return CosmeticHelm();
        }
        break;
    case Inventory_CosmeticWeapon1:
        if (HasCosmeticWeapon1())
        {
            return CosmeticWeapon1();
        }
        break;
    case Inventory_CosmeticWeapon2:
        if (HasCosmeticWeapon2())
        {
            return CosmeticWeapon2();
        }
        break;
    }
    return noItem;
}

bool EquippedGear::IsSlotRestricted(
        InventorySlotType slot,
        Build* pBuild) const
{
    UNREFERENCED_PARAMETER(pBuild);
    bool bRestricted = false;

    if (slot == Inventory_Weapon2
            && HasItemInSlot(Inventory_Weapon1)
            && !CanEquipTo2ndWeapon(pBuild, ItemInSlot(Inventory_Weapon1)))
    {
        // Weapon equipped in main hand that precludes weapon in off hand
        // do not permit selection of an item in the off hand slot
        bRestricted = true;
    }
    else
    {
        // check to see if this restricted by any item in any other slot
        for (size_t i = Inventory_Unknown + 1; i < Inventory_Count; ++i)
        {
            if (HasItemInSlot((InventorySlotType)i)
                    && ItemInSlot((InventorySlotType)i).HasRestrictedSlots()
                    && ItemInSlot((InventorySlotType)i).RestrictedSlots().HasSlot(slot))
            {
                bRestricted = true;
            }
        }
    }
    return bRestricted;
}

std::list<Item> EquippedGear::SetItem(
        InventorySlotType slot,
        Build* pBuild,
        const Item & item)
{
    std::list<Item> revokedItems;
    bool itemsRemoved = false;
    std:: stringstream ss;
    switch (slot)
    {
    case Inventory_Arrows:  Set_Arrow(item); break;
    case Inventory_Armor:   Set_Armor(item); break;
    case Inventory_Belt:    Set_Belt(item); break;
    case Inventory_Boots:   Set_Boots(item); break;
    case Inventory_Bracers: Set_Bracers(item); break;
    case Inventory_Cloak:   Set_Cloak(item); break;
    case Inventory_Gloves:  Set_Gloves(item); break;
    case Inventory_Goggles: Set_Goggles(item); break;
    case Inventory_Helmet:  Set_Helmet(item); break;
    case Inventory_Necklace: Set_Necklace(item); break;
    case Inventory_Quiver:  Set_Quiver(item); break;
    case Inventory_Ring1:   Set_Ring1(item); break;
    case Inventory_Ring2:   Set_Ring2(item); break;
    case Inventory_Trinket: Set_Trinket(item); break;
    case Inventory_Weapon1: Set_MainHand(item); break;
    case Inventory_Weapon2: Set_OffHand(item); break;
    case Inventory_CosmeticArmor:   Set_CosmeticArmor(item); break;
    case Inventory_CosmeticCloak:   Set_CosmeticCloak(item); break;
    case Inventory_CosmeticHelm:    Set_CosmeticHelm(item); break;
    case Inventory_CosmeticWeapon1: Set_CosmeticWeapon1(item); break;
    case Inventory_CosmeticWeapon2: Set_CosmeticWeapon2(item); break;
    default: ASSERT(FALSE); break;
    }
     // if the item just equipped is a Minor Artifact, make sure
    // we do not have any other minor artifacts equipped.
    if (item.HasMinorArtifact())
    {
        // need to check all other slots
        for (size_t s = Inventory_Unknown + 1; s < Inventory_Count; ++s)
        {
            // make sure we do not remove the just equipped item
            if (s != static_cast<size_t>(slot))
            {
                if (HasItemInSlot((InventorySlotType)s))
                {
                    if (ItemInSlot((InventorySlotType)s).HasMinorArtifact())
                    {
                        // we do have at least 1 other minor artifact, remove it
                        ss << "You can only have a single Minor Artifact equipped at any time.\r\n"
                            "The following item was removed: ";
                        ss << ItemInSlot((InventorySlotType)s).Name();
                        revokedItems.push_back(ItemInSlot((InventorySlotType)s));
                        ClearItem((InventorySlotType)s);
                        itemsRemoved = true;
                    }
                }
            }
        }
    }
   if (slot == Inventory_Weapon1
            && !CanEquipTo2ndWeapon(pBuild, item)
            && HasItemInSlot(Inventory_Weapon2))
    {
        ss << "The following item was removed because you cannot have an item in your off hand:\r\n\r\n";
        ss << ItemInSlot(Inventory_Weapon2).Name();
        // item in this slot now stops an item in weapon slot 2
        revokedItems.push_back(ItemInSlot(Inventory_Weapon2));
        ClearItem(Inventory_Weapon2);
        itemsRemoved = true;
    }
    if (itemsRemoved)
    {
        AfxMessageBox(ss.str().c_str(), MB_ICONEXCLAMATION);
    }
    return revokedItems;
}

void EquippedGear::ClearItem(InventorySlotType slot)
{
    switch (slot)
    {
    case Inventory_Arrows:  Clear_Arrow(); break;
    case Inventory_Armor:   Clear_Armor(); break;
    case Inventory_Belt:    Clear_Belt(); break;
    case Inventory_Boots:   Clear_Boots(); break;
    case Inventory_Bracers: Clear_Bracers(); break;
    case Inventory_Cloak:   Clear_Cloak(); break;
    case Inventory_Gloves:  Clear_Gloves(); break;
    case Inventory_Goggles: Clear_Goggles(); break;
    case Inventory_Helmet:  Clear_Helmet(); break;
    case Inventory_Necklace: Clear_Necklace(); break;
    case Inventory_Quiver:  Clear_Quiver(); break;
    case Inventory_Ring1:   Clear_Ring1(); break;
    case Inventory_Ring2:   Clear_Ring2(); break;
    case Inventory_Trinket: Clear_Trinket(); break;
    case Inventory_Weapon1: Clear_MainHand(); break;
    case Inventory_Weapon2: Clear_OffHand(); break;
    case Inventory_CosmeticArmor:   Clear_CosmeticArmor(); break;
    case Inventory_CosmeticCloak:   Clear_CosmeticCloak(); break;
    case Inventory_CosmeticHelm:    Clear_CosmeticHelm(); break;
    case Inventory_CosmeticWeapon1: Clear_CosmeticWeapon1(); break;
    case Inventory_CosmeticWeapon2: Clear_CosmeticWeapon2(); break;
    default: ASSERT(FALSE); break;
    }
}

bool EquippedGear::HasMinorArtifact() const
{
    bool bHas = false;
    // need to check all gear slots
    for (size_t s = Inventory_Unknown + 1; s < Inventory_Count; ++s)
    {
        if (HasItemInSlot((InventorySlotType)s))
        {
            if (ItemInSlot((InventorySlotType)s).HasMinorArtifact())
            {
                // we do have at least 1 minor artifact
                bHas = true;
            }
        }
    }
    return bHas;
}

std::string EquippedGear::GetFiligree(size_t fi) const
{
    std::string name;
    if (fi < m_Filigrees.size())
    {
        std::list<WeaponFiligree>::const_iterator it = m_Filigrees.begin();
        std::advance(it, fi);
        name = (*it).Name();
    }
    return name;
}

bool EquippedGear::IsRareFiligree(size_t fi) const
{
    bool bRare = false;
    if (fi < m_Filigrees.size())
    {
        std::list<WeaponFiligree>::const_iterator it = m_Filigrees.begin();
        std::advance(it, fi);
        bRare = (*it).HasRare();
    }
    return bRare;
}

void EquippedGear::SetNumFiligrees(size_t count)
{
    if (m_NumFiligrees != count)
    {
        std::stringstream ss;
        ss << "Number of weapon filigrees changed from " << m_NumFiligrees << " to " << count << ".";
        // ensure we do not have too many filigrees setup
        m_NumFiligrees = count;
        // remove any extra filigrees we no longer have space for
        while (m_Filigrees.size() > count)
        {
            if (m_Filigrees.back().Name() != "")
            {
                ss << "\r\n    Filigree \"" << m_Filigrees.back().Name() << "\" was removed.";
            }
            m_Filigrees.pop_back();
        }
        // add any required blank filigrees
        while (m_Filigrees.size() < count)
        {
            m_Filigrees.push_back(WeaponFiligree());  // add a blank filigree
        }
        GetLog().AddLogEntry(ss.str().c_str());
    }
}

void EquippedGear::SetPersonality(const std::string& name)
{
    std::stringstream ss;
    ss << "Sentient Gem personality of \""
            << name
            << "\" set for gear set \""
            << Name() << "\"";
    GetLog().AddLogEntry(ss.str().c_str());
    Set_Personality(name);
}

void EquippedGear::ClearPersonality()
{
    std::stringstream ss;
    ss << "Sentient Gem personality of \""
            << Personality()
            << "\" cleared from gear set \""
            << Name() << "\"";
    GetLog().AddLogEntry(ss.str().c_str());
    Clear_Personality();
}

void EquippedGear::SetFiligree(size_t fi, const std::string& name)
{
    if (fi < m_NumFiligrees)
    {
        std::list<WeaponFiligree>::iterator it = m_Filigrees.begin();
        std::advance(it, fi);
        std::stringstream ss;
        ss << "Weapon Filigree " << (fi + 1);
        if (it->Name() == "")
        {
            ss << " set to \"" << name << "\".";
        }
        else
        {
            if (name == "")
            {
                ss << " was cleared from \"" << it->Name() << "\".";
            }
            else
            {
                ss << " changed from \"" << it->Name() << "\" to \"" << name << "\".";
            }
        }
        GetLog().AddLogEntry(ss.str().c_str());
        (*it).Set_Name(name);
    }
}

void EquippedGear::SetFiligreeRare(size_t fi, bool isRare)
{
    if (fi < m_NumFiligrees)
    {
        std::list<WeaponFiligree>::iterator it = m_Filigrees.begin();
        std::advance(it, fi);
        std::stringstream ss;
        ss << "Weapon Filigree " << (fi + 1) << " of \"" << it->Name() << "\"";
        if (isRare)
        {
            ss << " was set to Rare state";
            (*it).Set_Rare();
        }
        else
        {
            ss << " had its Rare state revoked";
            (*it).Clear_Rare();
        }
        GetLog().AddLogEntry(ss.str().c_str());
    }
}

std::string EquippedGear::GetArtifactFiligree(size_t fi) const
{
    std::string name;
    if (fi < m_ArtifactFiligrees.size())
    {
        std::list<ArtifactFiligree>::const_iterator it = m_ArtifactFiligrees.begin();
        std::advance(it, fi);
        name = (*it).Name();
    }
    return name;
}

bool EquippedGear::IsRareArtifactFiligree(size_t fi) const
{
    bool bRare = false;
    if (fi < m_ArtifactFiligrees.size())
    {
        std::list<ArtifactFiligree>::const_iterator it = m_ArtifactFiligrees.begin();
        std::advance(it, fi);
        bRare = (*it).HasRare();
    }
    return bRare;
}

void EquippedGear::SetArtifactFiligree(size_t fi, const std::string& name)
{
    if (fi < MAX_ARTIFACT_FILIGREE)
    {
        std::list<ArtifactFiligree>::iterator it = m_ArtifactFiligrees.begin();
        std::advance(it, fi);
        std::stringstream ss;
        ss << "Artifact Filigree " << (fi + 1);
        if (it->Name() == "")
        {
            ss << " set to \"" << name << "\".";
        }
        else
        {
            if (name == "")
            {
                ss << " was cleared from \"" << it->Name() << "\".";
            }
            else
            {
                ss << " changed from \"" << it->Name() << "\" to \"" << name << "\".";
            }
        }
        GetLog().AddLogEntry(ss.str().c_str());
        (*it).Set_Name(name);
    }
}

void EquippedGear::SetArtifactFiligreeRare(size_t fi, bool isRare)
{
    if (fi < MAX_ARTIFACT_FILIGREE)
    {
        std::list<ArtifactFiligree>::iterator it = m_ArtifactFiligrees.begin();
        std::advance(it, fi);
        std::stringstream ss;
        ss << "Artifact Filigree " << (fi + 1) << " of \"" << it->Name() << "\"";
        if (isRare)
        {
            ss << " was set to Rare state";
            (*it).Set_Rare();
        }
        else
        {
            ss << " had its Rare state revoked";
            (*it).Clear_Rare();
        }
        GetLog().AddLogEntry(ss.str().c_str());
    }
}

bool EquippedGear::ImportFromFile(const CString& filename)
{
    bool bSuccess = false;
    // first read the entire content of the import file into a buffer
    CFile importFile;
    if (importFile.Open(filename, CFile::modeRead))
    {
        CString text;
        text.Format("Processing gear import file: %s", (LPCTSTR)filename);
        GetLog().AddLogEntry(text);
        ULONGLONG size = importFile.GetLength();
        if (size < 1024 * 20)       // 20k
        {
            size_t uSize = (size_t)size;
            char buffer[1024*20];
            size_t pif = 0;         // position in file being parsed
            importFile.Read(buffer, uSize);
            while (pif < uSize)
            {
                CString lineOfText;
                pif = ExtractLine(&lineOfText, pif, buffer, uSize);
                if (lineOfText == "")
                {
                    // ignore the rest of the file
                    break;
                }
                ProcessLine(lineOfText);
            }
            bSuccess = true;
        }
        else
        {
            // the file being imported is far too big, just report an error
            CString errMsg;
            errMsg.Format("The import file \"%s\" is too large to process", (LPCTSTR)filename);
            AfxMessageBox(errMsg, MB_ICONERROR);
            GetLog().AddLogEntry(errMsg);
        }
    }
    else
    {
        CString errMsg;
        errMsg.Format("Failed to open the import file \"%s\"", (LPCTSTR)filename);
        AfxMessageBox(errMsg, MB_ICONERROR);
        GetLog().AddLogEntry(errMsg);
    }
    return bSuccess;
}

size_t EquippedGear::ExtractLine(
        CString* line,
        size_t pif,
        char* buffer,
        size_t bufferSize)
{
    bool eol = false;
    while (pif < bufferSize && !eol)
    {
        char ch = buffer[pif];
        eol = (ch == '\n');
        if (!eol)
        {
            if (ch != '\r')
            {
                *line += ch;
            }
        }
        // always move to the next character
        ++pif;
    }
    return pif;
}

bool EquippedGear::ProcessLine(CString line)
{
    CString lineCopy(line);
    bool bSuccess = false;
    InventorySlotType st = Inventory_Unknown;
    // first try and determine which slot this line references
    if (line.Find("Eye:", 0) >= 0)
    {
        st = Inventory_Goggles;
        line.Replace("Eye:", "");
    }
    else if (line.Find("Head:", 0) >= 0)
    {
        st = Inventory_Helmet;
        line.Replace("Head:", "");
    }
    else if (line.Find("Neck:", 0) >= 0)
    {
        st = Inventory_Necklace;
        line.Replace("Neck:", "");
    }
    else if (line.Find("Trinket:", 0) >= 0)
    {
        st = Inventory_Trinket;
        line.Replace("Trinket:", "");
    }
    else if (line.Find("Body:", 0) >= 0)
    {
        st = Inventory_Armor;
        line.Replace("Body:", "");
    }
    else if (line.Find("Back:", 0) >= 0)
    {
        st = Inventory_Cloak;
        line.Replace("Back:", "");
    }
    else if (line.Find("Wrist:", 0) >= 0)
    {
        st = Inventory_Bracers;
        line.Replace("Wrist:", "");
    }
    else if (line.Find("Waist:", 0) >= 0)
    {
        st = Inventory_Belt;
        line.Replace("Waist:", "");
    }
    else if (line.Find("Finger1:", 0) >= 0)
    {
        st = Inventory_Ring1;
        line.Replace("Finger1:", "");
    }
    else if (line.Find("Feet:", 0) >= 0)
    {
        st = Inventory_Boots;
        line.Replace("Feet:", "");
    }
    else if (line.Find("Hand:", 0) >= 0)
    {
        st = Inventory_Gloves;
        line.Replace("Hand:", "");
    }
    else if (line.Find("Finger2:", 0) >= 0)
    {
        st = Inventory_Ring2;
        line.Replace("Finger2:", "");
    }
    else if (line.Find("Weapon:", 0) >= 0)
    {
        st = Inventory_Weapon1;
        line.Replace("Weapon:", "");
    }
    else if (line.Find("Offhand:", 0) >= 0)
    {
        st = Inventory_Weapon2;
        line.Replace("Offhand:", "");
    }
    if (st != Inventory_Unknown)
    {
        // inventory slot was recognised, now find the item and apply
        // any enhancement selection that may be present

        // the remaining text is the item name up to the first '{' character
        CString itemName(line);
        int pos = line.Find('{', 0);
        if (pos >= 0)
        {
            itemName = line.Left(pos);
        }
        Item i = FindItem((LPCSTR)itemName);
        if (i.Name() == (LPCSTR)itemName)
        {
            CString text;
            text.Format("   Processed item \"%s\"", (LPCTSTR)itemName);
            GetLog().AddLogEntry(text);
            // item was found, process any enhancement options
            line.Replace(itemName, "");     // remove the item name
            int sil = line.Find('{', 0);
            int pil = line.Find('}', 0);
            while (sil >= 0 && pil >= 0)
            {
                CString augmentText = line.Mid(sil + 1, pil - sil);
                ApplyItemAugment(&i, augmentText);
                // setup for next iteration
                sil = line.Find('{', pil);
                pil = line.Find('}', pil+1);
            }

            // finally set the item in gear
            SetItem(st, NULL, i);
        }
        bSuccess = true;
    }
    else
    {
        CString text;
        text.Format("   Failed to process import line \"%s\"", (LPCTSTR)lineCopy);
        GetLog().AddLogEntry(text);
    }
    return bSuccess;
}

void EquippedGear::ApplyItemAugment(Item* pItem, CString augmentText)
{
    // example augmentText is:
    // "enhancement Strength 21"
    // "enhancement Dexterity 8"
    // "insight Constitution 6"
    // "enhancement Doubleshot 8"
    // "enhancement Sneak Attack Damage 16"

    // break the line up into it's components and we need to find all text
    // items in a given augments compatible augment descriptions
    std::vector<CString> augmentComponents;
    augmentComponents.reserve(10);
    int pis = 0;    // position in string
    int start = pis;
    while (pis >= 0)
    {
        int space = augmentText.Find(' ', pis);
        int bracket = augmentText.Find('}', pis);
        if (space != -1 && space < bracket)
        {
            pis = space;
        }
        else
        {
            pis = bracket;
        }
        if (pis >= 0)
        {
            CString element = augmentText.Mid(start, pis - start);
            element.MakeLower();
            augmentComponents.push_back(element);
            ++pis;      // skip the ' ' we found last
        }
        start = pis;
    }
    // now we have the elements, search all augment slots in order on this item
    // to see if we can assign the selection to it, if it does not already have one
    int levelIndex = 0;
    bool bPlaced = false;
    std::vector<ItemAugment> augments = pItem->Augments();
    for (size_t i = 0; !bPlaced && i < augments.size(); ++i)
    {
        if (!augments[i].HasSelectedAugment())
        {
            // this augment has no selection, see if we can place this augment here
            // first find the list of compatible augments for this augment slot
            std::list<Augment> compatibleAugments;
            if (augments[i].ItemSpecificAugments().size() > 0)
            {
                compatibleAugments = augments[i].ItemSpecificAugments();
            }
            else
            {
                compatibleAugments = CompatibleAugments(augments[i].Type(), MAX_GAME_LEVEL);
            }
            std::list<Augment>::const_iterator it = compatibleAugments.begin();
            while (!bPlaced && it != compatibleAugments.end())
            {
                CString description = (*it).Description().c_str();
                description.MakeLower();
                bPlaced = true;     // assume, gets cleared if not compatible
                for (size_t j = 0; bPlaced && j < augmentComponents.size(); ++j)
                {
                    // all text augments component items must be found in the description text
                    int value = atoi(augmentComponents[j]);
                    if ((*it).HasChooseLevel()
                        && value != 0)
                    {
                        // need to match by value here
                        std::vector<double> values = (*it).LevelValue();
                        for (auto&& vit : values)
                        {
                            if (vit == value)
                            {
                                bPlaced = true;
                            }
                            else
                            {
                                ++levelIndex;
                            }
                        }
                        if (levelIndex == static_cast<int>(values.size()))
                        {
                            bPlaced = false;
                        }
                    }
                    else
                    {
                        // match by text
                        bPlaced = (description.Find(augmentComponents[j]) >= 0);
                    }
                }
                if (bPlaced)
                {
                    // this augment goes in this slot. Place it
                    augments[i].Set_SelectedAugment((*it).Name());
                    if ((*it).HasChooseLevel())
                    {
                        augments[i].Set_SelectedLevelIndex(levelIndex);
                    }
                }
                ++it;
            }
        }
    }
    pItem->Set_Augments(augments);
    CString text;
    augmentText.Replace("}", "");
    if (bPlaced)
    {
        text.Format("      Recognised augment \"%s\"", (LPCTSTR)augmentText);
    }
    else
    {
        text.Format("      Augment not recognised \"%s\"", (LPCTSTR)augmentText);
    }
    GetLog().AddLogEntry(text);
}

void EquippedGear::UpdateItem(
    InventorySlotType slot,
    const Item& item)
{
    switch (slot)
    {
        case Inventory_Arrows:  Set_Arrow(item); break;
        case Inventory_Armor:   Set_Armor(item); break;
        case Inventory_Belt:    Set_Belt(item); break;
        case Inventory_Boots:   Set_Boots(item); break;
        case Inventory_Bracers: Set_Bracers(item); break;
        case Inventory_Cloak:   Set_Cloak(item); break;
        case Inventory_Gloves:  Set_Gloves(item); break;
        case Inventory_Goggles: Set_Goggles(item); break;
        case Inventory_Helmet:  Set_Helmet(item); break;
        case Inventory_Necklace: Set_Necklace(item); break;
        case Inventory_Quiver:  Set_Quiver(item); break;
        case Inventory_Ring1:   Set_Ring1(item); break;
        case Inventory_Ring2:   Set_Ring2(item); break;
        case Inventory_Trinket: Set_Trinket(item); break;
        case Inventory_Weapon1: Set_MainHand(item); break;
        case Inventory_Weapon2: Set_OffHand(item); break;
        case Inventory_CosmeticArmor:   Set_CosmeticArmor(item); break;
        case Inventory_CosmeticCloak:   Set_CosmeticCloak(item); break;
        case Inventory_CosmeticHelm:    Set_CosmeticHelm(item); break;
        case Inventory_CosmeticWeapon1: Set_CosmeticWeapon1(item); break;
        case Inventory_CosmeticWeapon2: Set_CosmeticWeapon2(item); break;
        default: ASSERT(FALSE); break;
    }
}

WeaponType EquippedGear::Weapon1() const
{
    WeaponType wt = Weapon_Empty;
    if (HasItemInSlot(Inventory_Weapon1))
    {
        wt = MainHand().Weapon();
    }
    return wt;
}

WeaponType EquippedGear::Weapon2() const
{
    WeaponType wt = Weapon_Empty;
    if (HasItemInSlot(Inventory_Weapon2))
    {
        wt = OffHand().Weapon();
    }
    return wt;
}
