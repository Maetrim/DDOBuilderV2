// ConditionalGroup.cpp
//
#include "StdAfx.h"
#include "ConditionalGroup.h"
#include "XmlLib\SaxWriter.h"
#include "GlobalSupportFunctions.h"
#include "Build.h"

#define DL_ELEMENT ConditionalGroup

namespace
{
    const wchar_t f_saxElementName[] = L"ConditionalGroup";
    DL_DEFINE_NAMES(ConditionalGroup_PROPERTIES)

    const unsigned f_verCurrent = 1;
}

ConditionalGroup::ConditionalGroup() :
    XmlLib::SaxContentElement(f_saxElementName, f_verCurrent)
{
    DL_INIT(ConditionalGroup_PROPERTIES)
}

ConditionalGroup::ConditionalGroup(const XmlLib::SaxString & objectName) :
    XmlLib::SaxContentElement(objectName, f_verCurrent)
{
    DL_INIT(ConditionalGroup_PROPERTIES)
}

DL_DEFINE_ACCESS(ConditionalGroup_PROPERTIES)

XmlLib::SaxContentElementInterface * ConditionalGroup::StartElement(
        const XmlLib::SaxString & name,
        const XmlLib::SaxAttributes & attributes)
{
    XmlLib::SaxContentElementInterface * subHandler =
            SaxContentElement::StartElement(name, attributes);

    DL_START(ConditionalGroup_PROPERTIES)

    return subHandler;
}

void ConditionalGroup::EndElement()
{
    SaxContentElement::EndElement();
    DL_END(ConditionalGroup_PROPERTIES)
}

void ConditionalGroup::Write(XmlLib::SaxWriter * writer) const
{
    writer->StartElement(ElementName());//, VersionAttributes());
    DL_WRITE(ConditionalGroup_PROPERTIES)
    writer->EndElement();
}

bool ConditionalGroup::VerifyObject(std::stringstream* ss) const
{
    UNREFERENCED_PARAMETER(ss);
    bool ok = true;
    //const ConditionalGroup& ConditionalGroup = FindConditionalGroup(Type());
    //if (ConditionalGroup.Type() == "ConditionalGroupNotFound")
    //{
    //    (*ss) << "---Has missing ConditionalGroup \"" << Type() << "\"\n";
    //    ok = false;
    //}
    return ok;
}
