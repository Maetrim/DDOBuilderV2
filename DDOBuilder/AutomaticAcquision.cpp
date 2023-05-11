// AutomaticAcquision.cpp
//
#include "StdAfx.h"
#include "AutomaticAcquision.h"

#define DL_ELEMENT AutomaticAcquision

namespace
{
    DL_DEFINE_NAMES(RequirementsBase_PROPERTIES)
    DL_DEFINE_NAMES(AutomaticAcquision_PROPERTIES)
    const wchar_t f_saxElementName[] = L"AutomaticAcquisition";
    const unsigned f_verCurrent = 1;
}

AutomaticAcquision::AutomaticAcquision() :
    RequirementsBase(f_saxElementName, f_verCurrent)
{
    DL_INIT(AutomaticAcquision_PROPERTIES)
}

DL_DEFINE_ACCESS(AutomaticAcquision_PROPERTIES)

XmlLib::SaxContentElementInterface* AutomaticAcquision::StartElement(
    const XmlLib::SaxString& name,
    const XmlLib::SaxAttributes& attributes)
{
    XmlLib::SaxContentElementInterface* subHandler =
        SaxContentElement::StartElement(name, attributes);

    DL_START_MULTIPLE(RequirementsBase_PROPERTIES)
    DL_START_MULTIPLE_END(AutomaticAcquision_PROPERTIES)

    return subHandler;
}

void AutomaticAcquision::EndElement()
{
    SaxContentElement::EndElement();
    DL_END(RequirementsBase_PROPERTIES)
    DL_END(AutomaticAcquision_PROPERTIES)
}

void AutomaticAcquision::Write(XmlLib::SaxWriter* writer) const
{
    writer->StartElement(ElementName(), VersionAttributes());
    DL_WRITE(RequirementsBase_PROPERTIES)
    DL_WRITE(AutomaticAcquision_PROPERTIES)
    writer->EndElement();
}

void AutomaticAcquision::SetRequirement(const Requirement& requirement)
{
    m_Requires.push_back(requirement);
}

