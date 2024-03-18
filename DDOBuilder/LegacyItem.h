// LegacyItem.h
//
// An XML object wrapper that holds information on an LegacyItem that can be equipped
#pragma once
#include "XmlLib\DLMacros.h"
#include "EquipmentSlot.h"
#include "ItemAugment.h"
#include "SlotUpgrade.h"

class LegacyItem :
    public XmlLib::SaxContentElement
{
    public:
        LegacyItem(void);
        LegacyItem(const XmlLib::SaxString & objectName);
        void Write(XmlLib::SaxWriter * writer) const;

    protected:
        XmlLib::SaxContentElementInterface * StartElement(
                const XmlLib::SaxString & name,
                const XmlLib::SaxAttributes & attributes);

        virtual void EndElement();

        #define LegacyItem_PROPERTIES(_) \
                DL_STRING(_, Name) \
                DL_OBJECT(_, EquipmentSlot, Slots) \
                DL_OBJECT_VECTOR(_, ItemAugment, Augments) \
                DL_OBJECT_VECTOR(_, SlotUpgrade, SlotUpgrades)

        DL_DECLARE_ACCESS(LegacyItem_PROPERTIES)
        DL_DECLARE_VARIABLES(LegacyItem_PROPERTIES)
};
