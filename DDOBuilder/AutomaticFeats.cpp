// AutomaticFeats.cpp
//
#include "StdAfx.h"
#include "AutomaticFeats.h"

#include "XmlLib\SaxWriter.h"

#define DL_ELEMENT AutomaticFeats

namespace
{
    const wchar_t f_saxElementName[] = L"AutomaticFeats";
    DL_DEFINE_NAMES(AutomaticFeats_PROPERTIES)

    const unsigned f_verCurrent = 1;
}

AutomaticFeats::AutomaticFeats() :
    XmlLib::SaxContentElement(f_saxElementName, f_verCurrent)
{
    DL_INIT(AutomaticFeats_PROPERTIES)
}

DL_DEFINE_ACCESS(AutomaticFeats_PROPERTIES)

XmlLib::SaxContentElementInterface * AutomaticFeats::StartElement(
        const XmlLib::SaxString & name,
        const XmlLib::SaxAttributes & attributes)
{
    XmlLib::SaxContentElementInterface * subHandler =
            SaxContentElement::StartElement(name, attributes);

    DL_START(AutomaticFeats_PROPERTIES)

    return subHandler;
}

void AutomaticFeats::EndElement()
{
    SaxContentElement::EndElement();
    DL_END(AutomaticFeats_PROPERTIES)
    m_Feats.sort(); // ensure alphabetical order
}

void AutomaticFeats::Write(XmlLib::SaxWriter * writer) const
{
    writer->StartElement(ElementName());//, VersionAttributes());
    DL_WRITE(AutomaticFeats_PROPERTIES)
    writer->EndElement();
}
