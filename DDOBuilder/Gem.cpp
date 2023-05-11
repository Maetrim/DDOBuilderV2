// Gem.cpp
//
#include "StdAfx.h"
#include "Gem.h"
#include "GlobalSupportFunctions.h"

#include "XmlLib\SaxWriter.h"

#define DL_ELEMENT Gem

namespace
{
    const wchar_t f_saxElementName[] = L"Gem";
    DL_DEFINE_NAMES(Gem_PROPERTIES)

    const unsigned f_verCurrent = 1;
}

Gem::Gem() :
    XmlLib::SaxContentElement(f_saxElementName, f_verCurrent)
{
    DL_INIT(Gem_PROPERTIES)
}

DL_DEFINE_ACCESS(Gem_PROPERTIES)

XmlLib::SaxContentElementInterface * Gem::StartElement(
        const XmlLib::SaxString & name,
        const XmlLib::SaxAttributes & attributes)
{
    XmlLib::SaxContentElementInterface * subHandler =
            SaxContentElement::StartElement(name, attributes);

    DL_START(Gem_PROPERTIES)

    return subHandler;
}

void Gem::EndElement()
{
    SaxContentElement::EndElement();
    DL_END(Gem_PROPERTIES)
}

void Gem::Write(XmlLib::SaxWriter * writer) const
{
    writer->StartElement(ElementName());//, VersionAttributes());
    DL_WRITE(Gem_PROPERTIES)
    writer->EndElement();
}

bool Gem::operator<(const Gem & other) const
{
    // (assumes all augment names are unique)
    // sort by name
    return (Name() < other.Name());
}

void Gem::VerifyObject() const
{
    bool ok = true;
    std::stringstream ss;
    ss << "=====" << m_Name << "=====\n";
    if (HasIcon())
    {
        if (!ImageFileExists("DataFiles\\SentientGemImages\\", Icon()))
        {
            ss << "Gem is missing image file \"" << Icon() << "\"\n";
            ok = false;
        }
    }

    if (!ok)
    {
        ::OutputDebugString(ss.str().c_str());
    }
}
