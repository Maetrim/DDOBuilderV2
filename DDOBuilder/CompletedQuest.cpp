// CompletedQuest.cpp
//
#include "StdAfx.h"
#include "CompletedQuest.h"
#include "XmlLib\SaxWriter.h"

#define DL_ELEMENT CompletedQuest

namespace
{
    const wchar_t f_saxElementName[] = L"CompletedQuest";
    DL_DEFINE_NAMES(CompletedQuest_PROPERTIES)

    const unsigned f_verCurrent = 1;
}

CompletedQuest::CompletedQuest() :
    XmlLib::SaxContentElement(f_saxElementName, f_verCurrent)
{
    DL_INIT(CompletedQuest_PROPERTIES)
}

DL_DEFINE_ACCESS(CompletedQuest_PROPERTIES)

XmlLib::SaxContentElementInterface * CompletedQuest::StartElement(
        const XmlLib::SaxString & name,
        const XmlLib::SaxAttributes & attributes)
{
    XmlLib::SaxContentElementInterface * subHandler =
            SaxContentElement::StartElement(name, attributes);

    DL_START(CompletedQuest_PROPERTIES)

    return subHandler;
}

void CompletedQuest::EndElement()
{
    SaxContentElement::EndElement();
    DL_END(CompletedQuest_PROPERTIES)
}

void CompletedQuest::Write(XmlLib::SaxWriter * writer) const
{
    writer->StartElement(ElementName());//, VersionAttributes());
    DL_WRITE(CompletedQuest_PROPERTIES)
    writer->EndElement();
}
