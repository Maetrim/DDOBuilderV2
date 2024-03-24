// LegacyEquippedGear.cpp
//
#include "StdAfx.h"
#include "LegacyEquippedGear.h"

#define DL_ELEMENT LegacyEquippedGear

namespace
{
    const wchar_t f_saxElementName[] = L"EquippedGear";
    DL_DEFINE_NAMES(LegacyEquippedGear_PROPERTIES)

    const unsigned f_verCurrent = 1;
}

LegacyEquippedGear::LegacyEquippedGear() :
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
    m_Ring2(L"Ring2")
{
    DL_INIT(LegacyEquippedGear_PROPERTIES)
}

DL_DEFINE_ACCESS(LegacyEquippedGear_PROPERTIES)

XmlLib::SaxContentElementInterface * LegacyEquippedGear::StartElement(
        const XmlLib::SaxString & name,
        const XmlLib::SaxAttributes & attributes)
{
    XmlLib::SaxContentElementInterface * subHandler =
            SaxContentElement::StartElement(name, attributes);

    DL_START(LegacyEquippedGear_PROPERTIES)

    return subHandler;
}

void LegacyEquippedGear::EndElement()
{
    m_hasSentientIntelligence = true;   // assume for very old files to load

    SaxContentElement::EndElement();
    DL_END(LegacyEquippedGear_PROPERTIES)
}

void LegacyEquippedGear::Write(XmlLib::SaxWriter * writer) const
{
    writer->StartElement(ElementName());//, VersionAttributes());
    DL_WRITE(LegacyEquippedGear_PROPERTIES)
    writer->EndElement();
}

bool LegacyEquippedGear::HasItemInSlot(InventorySlotType slot) const
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

LegacyItem LegacyEquippedGear::ItemInSlot(InventorySlotType slot) const
{
    LegacyItem noItem;
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
