// ItemBuff.cpp
//
#include "StdAfx.h"
#include "ItemBuff.h"
#include "XmlLib\SaxWriter.h"

#define DL_ELEMENT ItemBuff

namespace
{
    const wchar_t f_saxElementName[] = L"ItemBuff";
    DL_DEFINE_NAMES(ItemBuff_PROPERTIES)

    const unsigned f_verCurrent = 1;
}

ItemBuff::ItemBuff() :
    XmlLib::SaxContentElement(f_saxElementName, f_verCurrent)
{
    DL_INIT(ItemBuff_PROPERTIES)
}

ItemBuff::ItemBuff(const XmlLib::SaxString & objectName) :
    XmlLib::SaxContentElement(objectName, f_verCurrent)
{
    DL_INIT(ItemBuff_PROPERTIES)
}

DL_DEFINE_ACCESS(ItemBuff_PROPERTIES)

XmlLib::SaxContentElementInterface * ItemBuff::StartElement(
        const XmlLib::SaxString & name,
        const XmlLib::SaxAttributes & attributes)
{
    XmlLib::SaxContentElementInterface * subHandler =
            SaxContentElement::StartElement(name, attributes);

    DL_START(ItemBuff_PROPERTIES)

    return subHandler;
}

void ItemBuff::EndElement()
{
    SaxContentElement::EndElement();
    DL_END(ItemBuff_PROPERTIES)
}

void ItemBuff::Write(XmlLib::SaxWriter * writer) const
{
    writer->StartElement(ElementName());//, VersionAttributes());
    DL_WRITE(ItemBuff_PROPERTIES)
    writer->EndElement();
}
void ItemBuff::VerifyObject() const
{
    bool ok = true;
    //std::stringstream ss;
    //ss << "=====" << m_Name << "=====\n";
    //if (!ImageFileExists("DataFiles\\ItemBuffImages\\", Icon()))
    //{
    //    ss << "ItemBuff is missing image file \"" << Icon() << "\"\n";
    //    ok = false;
    //}

    if (!ok)
    {
        //::OutputDebugString(ss.str().c_str());
    }
}
