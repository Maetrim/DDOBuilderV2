// TrainedFiligree.h
//
// An XML object wrapper that holds information on a selected TrainedFiligree
#pragma once
#include "XmlLib\DLMacros.h"

class TrainedFiligree :
    public XmlLib::SaxContentElement
{
    public:
        TrainedFiligree(const XmlLib::SaxString& elementName, unsigned verCurrent);
        void Write(XmlLib::SaxWriter * writer) const;

        bool operator==(const TrainedFiligree & other) const;

        void TranslateOldNamesFromV1();

    protected:
        XmlLib::SaxContentElementInterface * StartElement(
                const XmlLib::SaxString & name,
                const XmlLib::SaxAttributes & attributes);

        virtual void EndElement();

        #define TrainedFiligree_PROPERTIES(_) \
                DL_STRING(_, Name) \
                DL_FLAG(_, Rare)

        DL_DECLARE_ACCESS(TrainedFiligree_PROPERTIES)
        DL_DECLARE_VARIABLES(TrainedFiligree_PROPERTIES)

        friend class EquippedGear;
        friend class LegacySentientJewel;
};
