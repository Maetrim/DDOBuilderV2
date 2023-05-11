// Bonus.cpp
//
#include "StdAfx.h"
#include "Bonus.h"
#include "XmlLib\SaxWriter.h"
#include "GlobalSupportFunctions.h"
#include "LogPane.h"

#define DL_ELEMENT Bonus

namespace
{
    const wchar_t f_saxElementName[] = L"Bonus";
    DL_DEFINE_NAMES(Bonus_PROPERTIES)

    const unsigned f_verCurrent = 1;
}

Bonus::Bonus() :
    XmlLib::SaxContentElement(f_saxElementName, f_verCurrent)
{
    DL_INIT(Bonus_PROPERTIES)
}

DL_DEFINE_ACCESS(Bonus_PROPERTIES)

XmlLib::SaxContentElementInterface * Bonus::StartElement(
        const XmlLib::SaxString & name,
        const XmlLib::SaxAttributes & attributes)
{
    XmlLib::SaxContentElementInterface * subHandler =
            SaxContentElement::StartElement(name, attributes);

    DL_START(Bonus_PROPERTIES)

    return subHandler;
}

void Bonus::EndElement()
{
    // now check out our object
    SaxContentElement::EndElement();
    DL_END(Bonus_PROPERTIES)
}

void Bonus::Write(XmlLib::SaxWriter * writer) const
{
    writer->StartElement(ElementName());//, VersionAttributes());
    DL_WRITE(Bonus_PROPERTIES)
    writer->EndElement();
}

void Bonus::VerifyObject() const
{
    bool ok = true;
    std::stringstream ss;
    ss << "=====" << m_Name << "=====\n";

    if (!ok)
    {
        GetLog().AddLogEntry(ss.str().c_str());
    }
}
