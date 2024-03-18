// LegacyEquippedGear.h
//
// An XML object wrapper that holds information on all item equipped
#pragma once
#include "XmlLib\DLMacros.h"
#include "LegacyItem.h"
#include "LegacySentientJewel.h"
#include "InventorySlotTypes.h"

class Build;

class LegacyEquippedGear :
    public XmlLib::SaxContentElement
{
    public:
        LegacyEquippedGear();
        void Write(XmlLib::SaxWriter * writer) const;

        bool HasItemInSlot(InventorySlotType slot) const;
        LegacyItem ItemInSlot(InventorySlotType slot) const;

    protected:
        XmlLib::SaxContentElementInterface * StartElement(
                const XmlLib::SaxString & name,
                const XmlLib::SaxAttributes & attributes);

        virtual void EndElement();

        #define LegacyEquippedGear_PROPERTIES(_) \
                DL_STRING (_, Name) \
                DL_OPTIONAL_OBJECT(_, LegacyItem, Helmet) \
                DL_OPTIONAL_OBJECT(_, LegacyItem, Necklace) \
                DL_OPTIONAL_OBJECT(_, LegacyItem, Trinket) \
                DL_OPTIONAL_OBJECT(_, LegacyItem, Cloak) \
                DL_OPTIONAL_OBJECT(_, LegacyItem, Belt) \
                DL_OPTIONAL_OBJECT(_, LegacyItem, Goggles) \
                DL_OPTIONAL_OBJECT(_, LegacyItem, Gloves) \
                DL_OPTIONAL_OBJECT(_, LegacyItem, Boots) \
                DL_OPTIONAL_OBJECT(_, LegacyItem, Bracers) \
                DL_OPTIONAL_OBJECT(_, LegacyItem, Armor) \
                DL_OPTIONAL_OBJECT(_, LegacyItem, MainHand) \
                DL_OPTIONAL_OBJECT(_, LegacyItem, OffHand) \
                DL_OPTIONAL_OBJECT(_, LegacyItem, Quiver) \
                DL_OPTIONAL_OBJECT(_, LegacyItem, Arrow) \
                DL_OPTIONAL_OBJECT(_, LegacyItem, Ring1) \
                DL_OPTIONAL_OBJECT(_, LegacyItem, Ring2) \
                DL_OPTIONAL_OBJECT(_, LegacyItem, CosmeticArmor) \
                DL_OPTIONAL_OBJECT(_, LegacyItem, CosmeticCloak) \
                DL_OPTIONAL_OBJECT(_, LegacyItem, CosmeticHelm) \
                DL_OPTIONAL_OBJECT(_, LegacyItem, CosmeticWeapon1) \
                DL_OPTIONAL_OBJECT(_, LegacyItem, CosmeticWeapon2) \
                DL_OBJECT(_, LegacySentientJewel, SentientIntelligence)

        DL_DECLARE_ACCESS(LegacyEquippedGear_PROPERTIES)
        DL_DECLARE_VARIABLES(LegacyEquippedGear_PROPERTIES)
};
