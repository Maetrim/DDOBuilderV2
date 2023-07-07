// FavorEntry.cpp
//
#include "StdAfx.h"
#include "FavorEntry.h"
#include "XmlLib\SaxWriter.h"

#define DL_ELEMENT FavorEntry

namespace
{
    const wchar_t f_saxElementName[] = L"FavorEntry";
    DL_DEFINE_NAMES(FavorEntry_PROPERTIES)

    const unsigned f_verCurrent = 1;
}

FavorEntry::FavorEntry() :
    XmlLib::SaxContentElement(f_saxElementName, f_verCurrent)
{
    DL_INIT(FavorEntry_PROPERTIES)
}

DL_DEFINE_ACCESS(FavorEntry_PROPERTIES)

XmlLib::SaxContentElementInterface * FavorEntry::StartElement(
        const XmlLib::SaxString & name,
        const XmlLib::SaxAttributes & attributes)
{
    XmlLib::SaxContentElementInterface * subHandler =
            SaxContentElement::StartElement(name, attributes);

    DL_START(FavorEntry_PROPERTIES)

    return subHandler;
}

void FavorEntry::EndElement()
{
    SaxContentElement::EndElement();
    DL_END(FavorEntry_PROPERTIES)
}

void FavorEntry::Write(XmlLib::SaxWriter * writer) const
{
    writer->StartElement(ElementName());//, VersionAttributes());
    DL_WRITE(FavorEntry_PROPERTIES)
    writer->EndElement();
}

bool FavorEntry::VerifyObject(std::stringstream * ss) const
{
    bool ok = true;
    return ok;
}
