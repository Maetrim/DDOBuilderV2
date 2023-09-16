// Patron.cpp
//
#include "StdAfx.h"
#include "Patron.h"
#include "XmlLib\SaxWriter.h"
#include "GlobalSupportFunctions.h"
#include "LogPane.h"

#define DL_ELEMENT Patron

namespace
{
    const wchar_t f_saxElementName[] = L"Patron";
    DL_DEFINE_NAMES(Patron_PROPERTIES)

    const unsigned f_verCurrent = 1;
}

Patron::Patron() :
    XmlLib::SaxContentElement(f_saxElementName, f_verCurrent),
    m_maxFavor(0)
{
    DL_INIT(Patron_PROPERTIES)
}

Patron::Patron(const std::string& name) :
    XmlLib::SaxContentElement(f_saxElementName, f_verCurrent),
    m_maxFavor(0)
{
    UNREFERENCED_PARAMETER(name);
    DL_INIT(Patron_PROPERTIES)
    m_hasName = true;
    m_Name = Patron_Unknown;
}

DL_DEFINE_ACCESS(Patron_PROPERTIES)

XmlLib::SaxContentElementInterface * Patron::StartElement(
        const XmlLib::SaxString & name,
        const XmlLib::SaxAttributes & attributes)
{
    XmlLib::SaxContentElementInterface * subHandler =
            SaxContentElement::StartElement(name, attributes);

    DL_START(Patron_PROPERTIES)

    return subHandler;
}

void Patron::EndElement()
{
    SaxContentElement::EndElement();
    DL_END(Patron_PROPERTIES)
}

void Patron::Write(XmlLib::SaxWriter * writer) const
{
    writer->StartElement(ElementName());//, VersionAttributes());
    DL_WRITE(Patron_PROPERTIES)
    writer->EndElement();
}

bool Patron::VerifyObject() const
{
    bool ok = true;

    std::stringstream ss;
    if (Name() == Patron_Unknown)
    {
        ss << "Patron \"" << Name() << "\" has bad name field\n";
        ok = false;
    }
    if (!ok)
    {
        GetLog().AddLogEntry(ss.str().c_str());
    }
    return ok;
}

void Patron::SetMaxFavor(int maxfavor)
{
    m_maxFavor = maxfavor;
}

int Patron::MaxFavor() const
{
    return m_maxFavor;
}
