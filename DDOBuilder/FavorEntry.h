// FavorEntry.h
//
#pragma once
#include "XmlLib\DLMacros.h"
#include "FavorTypes.h"

class FavorEntry :
    public XmlLib::SaxContentElement
{
    public:
        FavorEntry(void);
        void Write(XmlLib::SaxWriter * writer) const;

        bool VerifyObject(std::stringstream * ss) const;

    protected:
        XmlLib::SaxContentElementInterface * StartElement(
                const XmlLib::SaxString & name,
                const XmlLib::SaxAttributes & attributes);

        virtual void EndElement();

        #define FavorEntry_PROPERTIES(_) \
                DL_STRING(_, Quest) \
                DL_ENUM(_, FavorType, Heroic, Favor_Unknown, favorTypeMap) \
                DL_ENUM(_, FavorType, Epic, Favor_Unknown, favorTypeMap) \
                DL_ENUM(_, FavorType, Legendary, Favor_Unknown, favorTypeMap)

        DL_DECLARE_ACCESS(FavorEntry_PROPERTIES)
        DL_DECLARE_VARIABLES(FavorEntry_PROPERTIES)
};
