// Buff.cpp
//
#include "StdAfx.h"
#include "Buff.h"
#include "XmlLib\SaxWriter.h"
#include "GlobalSupportFunctions.h"

#define DL_ELEMENT Buff

namespace
{
    const wchar_t f_saxElementName[] = L"Buff";
    DL_DEFINE_NAMES(Buff_PROPERTIES)

    const unsigned f_verCurrent = 1;
}

Buff::Buff() :
    XmlLib::SaxContentElement(f_saxElementName, f_verCurrent)
{
    DL_INIT(Buff_PROPERTIES)
}

Buff::Buff(const XmlLib::SaxString & objectName) :
    XmlLib::SaxContentElement(objectName, f_verCurrent)
{
    DL_INIT(Buff_PROPERTIES)
}

DL_DEFINE_ACCESS(Buff_PROPERTIES)

XmlLib::SaxContentElementInterface * Buff::StartElement(
        const XmlLib::SaxString & name,
        const XmlLib::SaxAttributes & attributes)
{
    XmlLib::SaxContentElementInterface * subHandler =
            SaxContentElement::StartElement(name, attributes);

    DL_START(Buff_PROPERTIES)

    return subHandler;
}

void Buff::EndElement()
{
    SaxContentElement::EndElement();
    DL_END(Buff_PROPERTIES)
}

void Buff::Write(XmlLib::SaxWriter * writer) const
{
    writer->StartElement(ElementName());//, VersionAttributes());
    DL_WRITE(Buff_PROPERTIES)
    writer->EndElement();
}

void Buff::VerifyObject() const
{
    bool ok = true;
    //std::stringstream ss;
    //ss << "=====" << m_Name << "=====\n";
    //if (!ImageFileExists("DataFiles\\BuffImages\\", Icon()))
    //{
    //    ss << "Buff is missing image file \"" << Icon() << "\"\n";
    //    ok = false;
    //}

    if (!ok)
    {
        //::OutputDebugString(ss.str().c_str());
    }
}

CString Buff::MakeDescription() const
{
    CString description(m_DisplayText.c_str());
    if (HasValue1())
    {
        CString value;
        value.Format("%+d", static_cast<int>(Value1()));
        description.Replace("%v1", value);
    }
    if (HasValue2())
    {
        CString value;
        value.Format("%+d", static_cast<int>(Value2()));
        description.Replace("%v2", value);
    }
    if (HasBonusType())
    {
        description.Replace("%b1", BonusType().c_str());
    }
    if (HasItem())
    {
        description.Replace("%i1", Item().c_str());
    }
    if (HasIgnore())
    {
        // only remove the ignore if it is at the start of the description
        int pos = description.Find(Ignore().c_str());
        if (pos == 0)
        {
            description = description.Right(description.GetLength() - (Ignore().size() + 1));
        }
    }
    BreakUpLongLines(description);

    return description;
}

void Buff::UpdatedEffects(std::list<Effect>* effects) const
{
    if (HasBonusType())
    {
        // set the bonus type for all the effects
        for (auto&& eit : *effects)
        {
            eit.SetBonus(BonusType());
        }
    }
    if (HasItem())
    {
        // set the Item type field for all the effects that have them
        std::list<std::string> items;
        items.push_back(Item());
        for (auto&& eit : *effects)
        {
            eit.SetItem(items);
        }
    }
    if (HasValue1() && HasValue2())
    {
        // v1 applies to first effect, value 2 to second (if there is one)
        size_t index = 0;
        for (auto&& eit : *effects)
        {
            if (index % 2 == 0)
                eit.SetAmount(Value1());
            else
                eit.SetAmount(Value2());
            ++index;
        }
    }
    else if (HasValue1())
    {
        // value1 applies to all effects
        for (auto&& eit : *effects)
        {
            eit.SetAmount(Value1());
        }
    }
}
