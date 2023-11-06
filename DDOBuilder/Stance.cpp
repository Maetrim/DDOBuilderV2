// Stance.cpp
//
#include "StdAfx.h"
#include "Stance.h"
#include "XmlLib\SaxWriter.h"
#include "GlobalSupportFunctions.h"

#define DL_ELEMENT Stance

namespace
{
    const wchar_t f_saxElementName[] = L"Stance";
    DL_DEFINE_NAMES(Stance_PROPERTIES)

    const unsigned f_verCurrent = 1;
}

Stance::Stance() :
    XmlLib::SaxContentElement(f_saxElementName, f_verCurrent)
{
    DL_INIT(Stance_PROPERTIES)
}

Stance::Stance(
        const std::string& name,
        const std::string& icon,
        const std::string& description,
        const std::string& group,
        bool bAutoControlled) :
    XmlLib::SaxContentElement(f_saxElementName, f_verCurrent),
    m_hasName(true),
    m_Name(name),
    m_hasIcon(true),
    m_Icon(icon),
    m_Group(group),
    m_hasDescription(true),
    m_Description(description),
    m_hasActiveRequirements(false)
{
    m_hasGroup = (group != "");
    m_hasAutoControlled = bAutoControlled;
}

DL_DEFINE_ACCESS(Stance_PROPERTIES)

XmlLib::SaxContentElementInterface * Stance::StartElement(
        const XmlLib::SaxString & name,
        const XmlLib::SaxAttributes & attributes)
{
    XmlLib::SaxContentElementInterface * subHandler =
            SaxContentElement::StartElement(name, attributes);

    DL_START(Stance_PROPERTIES)

    return subHandler;
}

void Stance::EndElement()
{
    SaxContentElement::EndElement();
    DL_END(Stance_PROPERTIES)
}

void Stance::Write(XmlLib::SaxWriter * writer) const
{
    writer->StartElement(ElementName());//, VersionAttributes());
    DL_WRITE(Stance_PROPERTIES)
    writer->EndElement();
}

bool Stance::VerifyObject(std::stringstream * ss) const
{
    bool ok = true;
    if (!ImageFileExists("DataFiles\\UIImages\\", Icon())
            && !ImageFileExists("DataFiles\\FeatImages\\", Icon())
            && !ImageFileExists("DataFiles\\EnhancementImages\\", Icon()))
    {
        (*ss) << "Stance " << m_Name << " is missing image file \"" << Icon() << "\"\n";
        ok = false;
    }
    if (HasAutoControlled())
    {
        if (!m_hasActiveRequirements)
        {
            (*ss) << "Stance " << m_Name << " is auto controlled and missing requirements\n";
            ok = false;
        }
        else
        {
            ok &= m_ActiveRequirements.VerifyObject(ss);
        }
    }
    return ok;
}

bool Stance::operator==(const Stance & other) const
{
    return (m_Name == other.m_Name)
            && (m_Icon == other.m_Icon);
}

void Stance::AddRequirement(const Requirement& requirement)
{
    m_ActiveRequirements.SetRequirement(requirement);
    m_hasActiveRequirements = true;
}

