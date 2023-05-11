// TrainedFiligree.cpp
//
#include "StdAfx.h"
#include "TrainedFiligree.h"
#include "XmlLib\SaxWriter.h"

#define DL_ELEMENT TrainedFiligree

namespace
{
    DL_DEFINE_NAMES(TrainedFiligree_PROPERTIES)
}

TrainedFiligree::TrainedFiligree(const XmlLib::SaxString& elementName, unsigned verCurrent) :
    XmlLib::SaxContentElement(elementName, verCurrent)
{
    DL_INIT(TrainedFiligree_PROPERTIES)
}

DL_DEFINE_ACCESS(TrainedFiligree_PROPERTIES)

XmlLib::SaxContentElementInterface * TrainedFiligree::StartElement(
        const XmlLib::SaxString & name,
        const XmlLib::SaxAttributes & attributes)
{
    XmlLib::SaxContentElementInterface * subHandler =
            SaxContentElement::StartElement(name, attributes);

    DL_START(TrainedFiligree_PROPERTIES)

    return subHandler;
}

void TrainedFiligree::EndElement()
{
    SaxContentElement::EndElement();
    DL_END(TrainedFiligree_PROPERTIES)
}

void TrainedFiligree::Write(XmlLib::SaxWriter * writer) const
{
    writer->StartElement(ElementName());//, VersionAttributes());
    DL_WRITE(TrainedFiligree_PROPERTIES)
    writer->EndElement();
}

bool TrainedFiligree::operator==(const TrainedFiligree & other) const
{
    bool bEqual = (m_Name == other.m_Name);
    return bEqual;
}

