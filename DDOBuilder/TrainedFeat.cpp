// TrainedFeat.cpp
//
#include "StdAfx.h"
#include "TrainedFeat.h"
#include "XmlLib\SaxWriter.h"
#include "GlobalSupportFunctions.h"

#define DL_ELEMENT TrainedFeat

namespace
{
    const wchar_t f_saxElementName[] = L"TrainedFeat";
    DL_DEFINE_NAMES(TrainedFeat_PROPERTIES)

    const unsigned f_verCurrent = 1;
}

TrainedFeat::TrainedFeat() :
    XmlLib::SaxContentElement(f_saxElementName, f_verCurrent),
    m_count(1)
{
    DL_INIT(TrainedFeat_PROPERTIES)
}

TrainedFeat::TrainedFeat(
        const std::string featName,
        const std::string& type,
        size_t level,
        bool bFeatSwapWarning) :
    XmlLib::SaxContentElement(f_saxElementName, f_verCurrent),
    m_count(1)
{
    DL_INIT(TrainedFeat_PROPERTIES)
    m_hasFeatName = true;
    m_FeatName = featName;
    m_hasType = true;
    m_Type = type;
    m_hasLevelTrainedAt = true;
    m_LevelTrainedAt = level;
    if (bFeatSwapWarning)
    {
        Set_FeatSwapWarning();
    }
}

DL_DEFINE_ACCESS(TrainedFeat_PROPERTIES)

XmlLib::SaxContentElementInterface * TrainedFeat::StartElement(
        const XmlLib::SaxString & name,
        const XmlLib::SaxAttributes & attributes)
{
    XmlLib::SaxContentElementInterface * subHandler =
            SaxContentElement::StartElement(name, attributes);

    DL_START(TrainedFeat_PROPERTIES)

    return subHandler;
}

void TrainedFeat::EndElement()
{
    SaxContentElement::EndElement();
    DL_END(TrainedFeat_PROPERTIES)
}

void TrainedFeat::Write(XmlLib::SaxWriter * writer) const
{
    writer->StartElement(ElementName());//, VersionAttributes());
    DL_WRITE(TrainedFeat_PROPERTIES)
    writer->EndElement();
}

bool TrainedFeat::operator<(const TrainedFeat & other) const
{
    if (Type() == other.Type())
    {
        return (FeatName() < other.FeatName());
    }
    else
    {
        return (Type() < other.Type());
    }
}

bool TrainedFeat::operator==(const TrainedFeat & other) const
{
    // ignore revoked feat for comparison
    return m_FeatName == other.m_FeatName
            && m_Type == other.m_Type
            && m_LevelTrainedAt == other.m_LevelTrainedAt;
}

size_t TrainedFeat::Count() const
{
    return m_count;
}

void TrainedFeat::IncrementCount()
{
    ++m_count;
}
