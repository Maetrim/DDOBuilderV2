// Quest.h
//
#pragma once
#include "XmlLib\DLMacros.h"
#include "PatronTypes.h"

class Quest :
    public XmlLib::SaxContentElement
{
    public:
        Quest(void);
        void Write(XmlLib::SaxWriter * writer) const;

        bool VerifyObject(std::stringstream * ss) const;

    protected:
        XmlLib::SaxContentElementInterface * StartElement(
                const XmlLib::SaxString & name,
                const XmlLib::SaxAttributes & attributes);

        virtual void EndElement();

        #define Quest_PROPERTIES(_) \
                DL_STRING(_, Name) \
                DL_ENUM(_, PatronType, Patron, Patron_Unknown, patronTypeMap) \
                DL_STRING(_, AdventurePack) \
                DL_SIMPLE(_, int, Favor, 0) \
                DL_VECTOR(_, int, Levels)

        DL_DECLARE_ACCESS(Quest_PROPERTIES)
        DL_DECLARE_VARIABLES(Quest_PROPERTIES)
};
