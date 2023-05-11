// ClassSpell.cpp
//
#include "StdAfx.h"
#include "ClassSpell.h"
#include "XmlLib\SaxWriter.h"

#define DL_ELEMENT ClassSpell

namespace
{
    const wchar_t f_saxElementName[] = L"ClassSpell";
    DL_DEFINE_NAMES(ClassSpell_PROPERTIES)

    const unsigned f_verCurrent = 1;
}

ClassSpell::ClassSpell() :
    XmlLib::SaxContentElement(f_saxElementName, f_verCurrent)
{
    DL_INIT(ClassSpell_PROPERTIES)
}

ClassSpell::ClassSpell(int level, size_t cost, size_t mcl) :
    XmlLib::SaxContentElement(f_saxElementName, f_verCurrent)
{
    DL_INIT(ClassSpell_PROPERTIES)
    m_Level = level;
    m_hasLevel = true;
    m_Cost = cost;
    m_hasCost = true;
    m_MaxCasterLevel = mcl;
    m_hasMaxCasterLevel = true;
}

DL_DEFINE_ACCESS(ClassSpell_PROPERTIES)

XmlLib::SaxContentElementInterface * ClassSpell::StartElement(
        const XmlLib::SaxString & name,
        const XmlLib::SaxAttributes & attributes)
{
    XmlLib::SaxContentElementInterface * subHandler =
            SaxContentElement::StartElement(name, attributes);

    DL_START(ClassSpell_PROPERTIES)

    return subHandler;
}

void ClassSpell::EndElement()
{
    SaxContentElement::EndElement();
    DL_END(ClassSpell_PROPERTIES)
}

void ClassSpell::Write(XmlLib::SaxWriter * writer) const
{
    writer->StartElement(ElementName());//, VersionAttributes());
    DL_WRITE(ClassSpell_PROPERTIES)
    writer->EndElement();
}
