// AttackBonus.cpp
//
#include "StdAfx.h"
#include "AttackBonus.h"

#include "XmlLib\SaxWriter.h"

#define DL_ELEMENT AttackBonus

namespace
{
    DL_DEFINE_NAMES(AttackBonus_PROPERTIES)

    const unsigned f_verCurrent = 1;
}

AttackBonus::AttackBonus(const XmlLib::SaxString& elementName) :
    XmlLib::SaxContentElement(elementName, f_verCurrent)
{
    DL_INIT(AttackBonus_PROPERTIES)
}

DL_DEFINE_ACCESS(AttackBonus_PROPERTIES)

XmlLib::SaxContentElementInterface * AttackBonus::StartElement(
        const XmlLib::SaxString & name,
        const XmlLib::SaxAttributes & attributes)
{
    XmlLib::SaxContentElementInterface * subHandler =
            SaxContentElement::StartElement(name, attributes);

    DL_START(AttackBonus_PROPERTIES)

    return subHandler;
}

void AttackBonus::EndElement()
{
    SaxContentElement::EndElement();
    DL_END(AttackBonus_PROPERTIES)
}

void AttackBonus::Write(XmlLib::SaxWriter * writer) const
{
    writer->StartElement(ElementName());//, VersionAttributes());
    DL_WRITE(AttackBonus_PROPERTIES)
    writer->EndElement();
}
