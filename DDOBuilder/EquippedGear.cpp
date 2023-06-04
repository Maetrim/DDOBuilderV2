// EquippedGear.cpp
//
#include "StdAfx.h"
#include "EquippedGear.h"
#include "XmlLib\SaxWriter.h"
#include "GlobalSupportFunctions.h"
#include "LogPane.h"
#include "Character.h"

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

void EquippedGear::SetItem(
        InventorySlotType slot,
        Build* pBuild,
        const Item & item)
{
    UNREFERENCED_PARAMETER(pBuild);
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
        ClearItem(Inventory_Weapon2);
        itemsRemoved = true;
    }
    if (itemsRemoved)
    {
        AfxMessageBox(ss.str().c_str(), MB_ICONEXCLAMATION);
    }
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
