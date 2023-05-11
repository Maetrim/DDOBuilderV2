// SkillTomes.cpp
//
#include "StdAfx.h"
#include "SkillTomes.h"
#include "XmlLib\SaxWriter.h"
#include "GlobalSupportFunctions.h"
#include "LogPane.h"

#define DL_ELEMENT SkillTomes

namespace
{
    const wchar_t f_saxElementName[] = L"SkillTomes";
    DL_DEFINE_NAMES(SkillTomes_PROPERTIES)

    const unsigned f_verCurrent = 1;
}

SkillTomes::SkillTomes() :
    XmlLib::SaxContentElement(f_saxElementName, f_verCurrent)
{
    DL_INIT(SkillTomes_PROPERTIES)
}

DL_DEFINE_ACCESS(SkillTomes_PROPERTIES)

XmlLib::SaxContentElementInterface * SkillTomes::StartElement(
        const XmlLib::SaxString & name,
        const XmlLib::SaxAttributes & attributes)
{
    XmlLib::SaxContentElementInterface * subHandler =
            SaxContentElement::StartElement(name, attributes);

    DL_START(SkillTomes_PROPERTIES)

    return subHandler;
}

void SkillTomes::EndElement()
{
    SaxContentElement::EndElement();
    DL_END(SkillTomes_PROPERTIES)
}

void SkillTomes::Write(XmlLib::SaxWriter * writer) const
{
    writer->StartElement(ElementName());//, VersionAttributes());
    DL_WRITE(SkillTomes_PROPERTIES)
    writer->EndElement();
}

void SkillTomes::SetSkillTome(SkillType skill, size_t value)
{
    size_t old = SkillTomeValue(skill);
    switch (skill)
    {
    case Skill_Balance:
        Set_Balance(value);
        break;
    case Skill_Bluff:
        Set_Bluff(value);
        break;
    case Skill_Concentration:
        Set_Concentration(value);
        break;
    case Skill_Diplomacy:
        Set_Diplomacy(value);
        break;
    case Skill_DisableDevice:
        Set_DisableDevice(value);
        break;
    case Skill_Haggle:
        Set_Haggle(value);
        break;
    case Skill_Heal:
        Set_Heal(value);
        break;
    case Skill_Hide:
        Set_Hide(value);
        break;
    case Skill_Intimidate:
        Set_Intimidate(value);
        break;
    case Skill_Jump:
        Set_Jump(value);
        break;
    case Skill_Listen:
        Set_Listen(value);
        break;
    case Skill_MoveSilently:
        Set_MoveSilently(value);
        break;
    case Skill_OpenLock:
        Set_OpenLock(value);
        break;
    case Skill_Perform:
        Set_Perform(value);
        break;
    case Skill_Repair:
        Set_Repair(value);
        break;
    case Skill_Search:
        Set_Search(value);
        break;
    case Skill_SpellCraft:
        Set_SpellCraft(value);
        break;
    case Skill_Spot:
        Set_Spot(value);
        break;
    case Skill_Swim:
        Set_Swim(value);
        break;
    case Skill_Tumble:
        Set_Tumble(value);
        break;
    case Skill_UMD:
        Set_UMD(value);
        break;
    }
    if (value != old)
    {
        std::stringstream ss;
        ss << "Skill tome for skill ";
        ss << EnumEntryText(skill, skillTypeMap);
        ss << " changed from " << old;
        ss << " to " << value;
        GetLog().AddLogEntry(ss.str().c_str());
    }
}

size_t SkillTomes::SkillTomeValue(SkillType skill) const
{
    size_t value = 0;
    switch (skill)
    {
    case Skill_Balance:
        value = Balance();
        break;
    case Skill_Bluff:
        value = Bluff();
        break;
    case Skill_Concentration:
        value = Concentration();
        break;
    case Skill_Diplomacy:
        value = Diplomacy();
        break;
    case Skill_DisableDevice:
        value = DisableDevice();
        break;
    case Skill_Haggle:
        value = Haggle();
        break;
    case Skill_Heal:
        value = Heal();
        break;
    case Skill_Hide:
        value = Hide();
        break;
    case Skill_Intimidate:
        value = Intimidate();
        break;
    case Skill_Jump:
        value = Jump();
        break;
    case Skill_Listen:
        value = Listen();
        break;
    case Skill_MoveSilently:
        value = MoveSilently();
        break;
    case Skill_OpenLock:
        value = OpenLock();
        break;
    case Skill_Perform:
        value = Perform();
        break;
    case Skill_Repair:
        value = Repair();
        break;
    case Skill_Search:
        value = Search();
        break;
    case Skill_SpellCraft:
        value = SpellCraft();
        break;
    case Skill_Spot:
        value = Spot();
        break;
    case Skill_Swim:
        value = Swim();
        break;
    case Skill_Tumble:
        value = Tumble();
        break;
    case Skill_UMD:
        value = UMD();
        break;
    }
    return value;
}
