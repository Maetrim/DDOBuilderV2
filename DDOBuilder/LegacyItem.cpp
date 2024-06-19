// LegacyItem.cpp
//
#include "StdAfx.h"
#include "LegacyItem.h"
#include "GlobalSupportFunctions.h"

#define DL_ELEMENT LegacyItem

namespace
{
    const wchar_t f_saxElementName[] = L"Item";
    DL_DEFINE_NAMES(LegacyItem_PROPERTIES)

    const unsigned f_verCurrent = 1;
}

LegacyItem::LegacyItem() :
    XmlLib::SaxContentElement(f_saxElementName, f_verCurrent),
    m_Slots(L"EquipmentSlot")
{
    DL_INIT(LegacyItem_PROPERTIES)
}

LegacyItem::LegacyItem(const XmlLib::SaxString & objectName) :
    XmlLib::SaxContentElement(objectName, f_verCurrent),
    m_Slots(L"EquipmentSlot")
{
    DL_INIT(LegacyItem_PROPERTIES)
}

DL_DEFINE_ACCESS(LegacyItem_PROPERTIES)

XmlLib::SaxContentElementInterface * LegacyItem::StartElement(
        const XmlLib::SaxString & name,
        const XmlLib::SaxAttributes & attributes)
{
    XmlLib::SaxContentElementInterface * subHandler =
            SaxContentElement::StartElement(name, attributes);

    DL_START(LegacyItem_PROPERTIES)

    return subHandler;
}

void LegacyItem::EndElement()
{
    SaxContentElement::EndElement();
    DL_END(LegacyItem_PROPERTIES)
    if (m_Name.find("Legendary Greensteel") != std::string::npos)
    {
        m_Name = ReplaceAll(
            m_Name,
            "Legendary Greensteel",
            "Legendary Green Steel");
        m_Name = ReplaceAll(
            m_Name,
            "Helmet",
            "Helm");
    }
}

void LegacyItem::Write(XmlLib::SaxWriter * writer) const
{
    writer->StartElement(ElementName());//, VersionAttributes());
    DL_WRITE(LegacyItem_PROPERTIES)
    writer->EndElement();
}
