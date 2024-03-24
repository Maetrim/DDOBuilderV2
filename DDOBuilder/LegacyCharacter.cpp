// LegacyCharacter.cpp
//
#include "StdAfx.h"
#include "LegacyCharacter.h"

#define DL_ELEMENT LegacyCharacter

namespace
{
    const wchar_t f_saxElementName[] = L"Character";
    DL_DEFINE_NAMES(LegacyCharacter_PROPERTIES)
    const unsigned f_verCurrent = 1;
}

LegacyCharacter::LegacyCharacter() :
    XmlLib::SaxContentElement(f_saxElementName, f_verCurrent),
    m_SpecialFeats(L"SpecialFeats")
{
    DL_INIT(LegacyCharacter_PROPERTIES)
}

DL_DEFINE_ACCESS(LegacyCharacter_PROPERTIES)

XmlLib::SaxContentElementInterface * LegacyCharacter::StartElement(
        const XmlLib::SaxString & name,
        const XmlLib::SaxAttributes & attributes)
{
    XmlLib::SaxContentElementInterface * subHandler =
            SaxContentElement::StartElement(name, attributes);

    DL_START(LegacyCharacter_PROPERTIES)
    if (subHandler == NULL && name == f_saxElementName)
    {
        subHandler = this;
    }

    return subHandler;
}

void LegacyCharacter::EndElement()
{
    m_hasLevel32 = true;    // assume for  very old files to load (default to strength)
    m_hasLevel36 = true;
    m_hasLevel40 = true;
    m_hasDestinyTrees = true;    // assume for  very old files to load
    SaxContentElement::EndElement();
    DL_END(LegacyCharacter_PROPERTIES)
}
