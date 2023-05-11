// SetBonusBuff.cpp
//
#include "StdAfx.h"
#include "SetBonusBuff.h"
#include "XmlLib\SaxWriter.h"
#include "GlobalSupportFunctions.h"

#define DL_ELEMENT SetBonusBuff

namespace
{
    const wchar_t f_saxElementName[] = L"Buff";
    DL_DEFINE_NAMES(SetBonusBuff_PROPERTIES)

    const unsigned f_verCurrent = 1;
}

SetBonusBuff::SetBonusBuff() :
    XmlLib::SaxContentElement(f_saxElementName, f_verCurrent)
{
    DL_INIT(SetBonusBuff_PROPERTIES)
}

DL_DEFINE_ACCESS(SetBonusBuff_PROPERTIES)

XmlLib::SaxContentElementInterface * SetBonusBuff::StartElement(
        const XmlLib::SaxString & name,
        const XmlLib::SaxAttributes & attributes)
{
    XmlLib::SaxContentElementInterface * subHandler =
            SaxContentElement::StartElement(name, attributes);

    DL_START(SetBonusBuff_PROPERTIES)

    return subHandler;
}

void SetBonusBuff::EndElement()
{
    // now check out our object
    SaxContentElement::EndElement();
    DL_END(SetBonusBuff_PROPERTIES)
}

void SetBonusBuff::Write(XmlLib::SaxWriter * writer) const
{
    writer->StartElement(ElementName());//, VersionAttributes());
    DL_WRITE(SetBonusBuff_PROPERTIES)
    writer->EndElement();
}

void SetBonusBuff::VerifyObject() const
{
    //bool ok = true;
    //std::stringstream ss;
    // check this SetBonusBuff out for any inconsistencies
}

