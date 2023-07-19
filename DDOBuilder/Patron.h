// Patron.h
//
#pragma once
#include "XmlLib\DLMacros.h"
#include "PatronTypes.h"

class Patron :
    public XmlLib::SaxContentElement
{
    public:
        Patron(void);
        void Write(XmlLib::SaxWriter * writer) const;

        void SetMaxFavor(int maxfavor);
        int MaxFavor() const;

        bool VerifyObject() const;

    protected:
        XmlLib::SaxContentElementInterface * StartElement(
                const XmlLib::SaxString & name,
                const XmlLib::SaxAttributes & attributes);

        virtual void EndElement();

        #define Patron_PROPERTIES(_) \
                DL_ENUM(_, PatronType, Name, Patron_Unknown, patronTypeMap) \
                DL_VECTOR(_, int, FavorTiers) \
                DL_STRING(_, AssociatedFavorFeat)

        DL_DECLARE_ACCESS(Patron_PROPERTIES)
        DL_DECLARE_VARIABLES(Patron_PROPERTIES)

        int m_maxFavor;
};
