// FeatSlot.cpp
//
#include "StdAfx.h"
#include "FeatSlot.h"
#include "XmlLib\SaxWriter.h"

#define DL_ELEMENT FeatSlot

namespace
{
    const wchar_t f_saxElementName[] = L"FeatSlot";
    DL_DEFINE_NAMES(FeatSlot_PROPERTIES)

    const unsigned f_verCurrent = 1;
}

FeatSlot::FeatSlot() :
    XmlLib::SaxContentElement(f_saxElementName, f_verCurrent)
{
    DL_INIT(FeatSlot_PROPERTIES)
}

FeatSlot::FeatSlot(
        size_t level,
        const std::string& featType,
        bool bSingular,
        bool bAutoPopulate) :
    XmlLib::SaxContentElement(f_saxElementName, f_verCurrent),
    m_Level(level),
    m_hasLevel(true),
    m_FeatType(featType),
    m_hasFeatType(true),
    m_hasSingular(bSingular),
    m_hasAutoPopulate(bAutoPopulate)
{
}

DL_DEFINE_ACCESS(FeatSlot_PROPERTIES)

XmlLib::SaxContentElementInterface * FeatSlot::StartElement(
        const XmlLib::SaxString & name,
        const XmlLib::SaxAttributes & attributes)
{
    XmlLib::SaxContentElementInterface * subHandler =
            SaxContentElement::StartElement(name, attributes);

    DL_START(FeatSlot_PROPERTIES)

    return subHandler;
}

void FeatSlot::EndElement()
{
    SaxContentElement::EndElement();
    DL_END(FeatSlot_PROPERTIES)
}

void FeatSlot::Write(XmlLib::SaxWriter * writer) const
{
    writer->StartElement(ElementName());//, VersionAttributes());
    DL_WRITE(FeatSlot_PROPERTIES)
    writer->EndElement();
}

bool FeatSlot::operator==(const FeatSlot& other) const
{
    // Feat Slots are only matched by name
    return m_FeatType == other.m_FeatType;
}

void FeatSlot::VerifyObject() const
{
    //bool ok = true;
    //std::stringstream ss;
}
