// Filigree.h
//
// An XML object wrapper that holds information on a selected Filigree
#pragma once
#include "XmlLib\DLMacros.h"
#include "Effect.h"

class Filigree :
    public XmlLib::SaxContentElement
{
    public:
        Filigree(void);
        void Write(XmlLib::SaxWriter * writer) const;

        std::list<Effect> NormalEffects() const;
        std::list<Effect> RareEffects() const;

        void SetFilename(const std::string&) {};

        bool operator<(const Filigree & other) const;
        bool operator==(const Filigree & other) const;
        void VerifyObject() const;

    protected:
        XmlLib::SaxContentElementInterface * StartElement(
                const XmlLib::SaxString & name,
                const XmlLib::SaxAttributes & attributes);

        virtual void EndElement();

        #define Filigree_PROPERTIES(_) \
                DL_STRING(_, Name) \
                DL_STRING(_, Description) \
                DL_STRING(_, Icon) \
                DL_STRING(_, Menu) \
                DL_OBJECT_LIST(_, Effect, Effects) \
                DL_STRING_LIST(_, SetBonus)

        DL_DECLARE_ACCESS(Filigree_PROPERTIES)
        DL_DECLARE_VARIABLES(Filigree_PROPERTIES)

        friend class EquippedGear;
};
