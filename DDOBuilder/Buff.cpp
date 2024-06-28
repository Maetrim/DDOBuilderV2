// Buff.cpp
//
#include "StdAfx.h"
#include "Buff.h"
#include "XmlLib\SaxWriter.h"
#include "GlobalSupportFunctions.h"
#include "Build.h"

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

bool Buff::VerifyObject(std::stringstream* ss) const
{
    bool ok = true;
    const Buff& buff = FindBuff(Type());
    if (buff.Type() == "BuffNotFound")
    {
        (*ss) << "---Has missing Buff \"" << Type() << "\"\n";
        ok = false;
    }
    return ok;
}

CString Buff::MakeDescription() const
{
    CString description;
    size_t count = DisplayText().size();
    for (auto&& it: DisplayText())
    {
        CString str = it.c_str();
        if (HasValue1())
        {
            CString value;
            if (HasNegativeValues())
            {
                value.Format("%+d", -static_cast<int>(Value1()));
            }
            else
            {
                value.Format("%+d", static_cast<int>(Value1()));
            }
            str.Replace("%v1", value);
        }
        if (HasValue2())
        {
            CString value;
            if (HasNegativeValues())
            {
                value.Format("%+d", -static_cast<int>(Value2()));
            }
            else
            {
                value.Format("%+d", static_cast<int>(Value2()));
            }
            str.Replace("%v2", value);
        }
        if (HasBonusType())
        {
            str.Replace("%b1", BonusType().c_str());
        }
        if (HasDescription1())
        {
            str.Replace("%i1", Description1().c_str());
        }
        else if (HasItem())
        {
            str.Replace("%i1", Item().c_str());
        }
        if (HasIgnore())
        {
            // only remove the ignore if it is at the start of the description
            int pos = str.Find(Ignore().c_str());
            if (pos == 0)
            {
                str = str.Right(str.GetLength() - (Ignore().size() + 1));
            }
        }
        BreakUpLongLines(str);
        description += str;
        count--;
        if (count > 0)
        {
            description += "\r\n";
        }
    }

    return description;
}

void Buff::UpdatedEffects(std::list<Effect>* effects, bool bNegativeValues) const
{
    if (HasBonusType()
            && m_BonusType != "")
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
    if (HasItem2())
    {
        // set the Item type field for all the effects that have them
        for (auto&& eit : *effects)
        {
            std::list<std::string> items = eit.Item();
            items.push_back(Item2());
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
            {
                if (bNegativeValues)
                {
                    eit.SetAmount(-Value1());
                }
                else
                {
                    eit.SetAmount(Value1());
                }
            }
            else
            {
                if (bNegativeValues)
                {
                    eit.SetAmount(-Value2());
                }
                else
                {
                    eit.SetAmount(Value2());
                }
            }
            ++index;
            if (HasRequirementsToUse())
            {
                eit.SetRequirements(RequirementsToUse());
            }
        }
    }
    else if (HasValue1())
    {
        // value1 applies to all effects
        for (auto&& eit : *effects)
        {
            if (bNegativeValues)
            {
                eit.SetAmount(-Value1());
            }
            else
            {
                eit.SetAmount(Value1());
            }
            if (HasRequirementsToUse())
            {
                eit.SetRequirements(RequirementsToUse());
            }
        }
    }
}
