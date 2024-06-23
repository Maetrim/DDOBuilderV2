// EquipmentSlot.h
//
// An XML object wrapper that holds information on which equipment slot(s)
// and item can be equipped to.
#pragma once
#include "XmlLib\DLMacros.h"
#include "InventorySlotTypes.h"

class EquipmentSlot :
    public XmlLib::SaxContentElement
{
    public:
        EquipmentSlot(const XmlLib::SaxString & objectName);
        void Write(XmlLib::SaxWriter * writer) const;

        bool HasSlot(InventorySlotType slot) const;

    protected:
        XmlLib::SaxContentElementInterface * StartElement(
                const XmlLib::SaxString & name,
                const XmlLib::SaxAttributes & attributes);

        virtual void EndElement();

        #define EquipmentSlot_PROPERTIES(_) \
                DL_FLAG(_, Armor) \
                DL_FLAG(_, Arrow) \
                DL_FLAG(_, Belt) \
                DL_FLAG(_, Boots) \
                DL_FLAG(_, Bracers) \
                DL_FLAG(_, Cloak) \
                DL_FLAG(_, Gloves) \
                DL_FLAG(_, Goggles) \
                DL_FLAG(_, Helmet) \
                DL_FLAG(_, Necklace) \
                DL_FLAG(_, Quiver) \
                DL_FLAG(_, Ring) \
                DL_FLAG(_, Trinket) \
                DL_FLAG(_, Weapon1) \
                DL_FLAG(_, Weapon2) \
                DL_FLAG(_, CosmeticWeapon1) \
                DL_FLAG(_, CosmeticWeapon2) \
                DL_FLAG(_, CosmeticArmor) \
                DL_FLAG(_, CosmeticCloak) \
                DL_FLAG(_, CosmeticHelm)

        DL_DECLARE_ACCESS(EquipmentSlot_PROPERTIES)
        DL_DECLARE_VARIABLES(EquipmentSlot_PROPERTIES)

        friend class WikiItemFileProcessor;
};
