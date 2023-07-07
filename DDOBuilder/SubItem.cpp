// SubItem.cpp
//
#include "StdAfx.h"
#include "SubItem.h"
#include "XmlLib\SaxWriter.h"
#include "GlobalSupportFunctions.h"

#define DL_ELEMENT SubItem

namespace
{
    const wchar_t f_saxElementName[] = L"SubItem";
    DL_DEFINE_NAMES(SubItem_PROPERTIES)

    const unsigned f_verCurrent = 1;
}

SubItem::SubItem() :
    XmlLib::SaxContentElement(f_saxElementName, f_verCurrent)
{
    DL_INIT(SubItem_PROPERTIES)
}

DL_DEFINE_ACCESS(SubItem_PROPERTIES)

XmlLib::SaxContentElementInterface * SubItem::StartElement(
        const XmlLib::SaxString & name,
        const XmlLib::SaxAttributes & attributes)
{
    XmlLib::SaxContentElementInterface * subHandler =
            SaxContentElement::StartElement(name, attributes);

    DL_START(SubItem_PROPERTIES)

    return subHandler;
}

void SubItem::EndElement()
{
    SaxContentElement::EndElement();
    DL_END(SubItem_PROPERTIES)
}

void SubItem::Write(XmlLib::SaxWriter * writer) const
{
    writer->StartElement(ElementName());//, VersionAttributes());
    DL_WRITE(SubItem_PROPERTIES)
    writer->EndElement();
}

bool SubItem::VerifyObject(std::stringstream * ss) const
{
    bool ok = true;
    if (!ImageFileExists("DataFiles\\FeatImages\\", Icon()))
    {
        (*ss) << "SubItem " << m_Name << " is missing image file \"" << Icon() << "\"\n";
        ok = false;
    }
    return ok;
}

bool SubItem::operator==(const SubItem & other) const
{
    return (m_Name == other.m_Name)
            && (m_Icon == other.m_Icon);
}
