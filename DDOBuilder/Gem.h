// Gem.h
//
// An XML object wrapper that holds information on which equipment slot(s)
// and item can be equipped to.
#pragma once
#include "XmlLib\DLMacros.h"
#include "Effect.h"
#include "WeaponClassTypes.h"
#include "Stance.h"

class Gem :
    public XmlLib::SaxContentElement
{
    public:
        Gem(void);
        void Write(XmlLib::SaxWriter * writer) const;

        bool IsCompatibleWithSlot(const std::string& augmentType) const;
        void AddImage(CImageList * pIL) const;
        bool operator<(const Gem & other) const;

        void VerifyObject() const;

    protected:
        XmlLib::SaxContentElementInterface * StartElement(
                const XmlLib::SaxString & name,
                const XmlLib::SaxAttributes & attributes);

        virtual void EndElement();

        #define Gem_PROPERTIES(_) \
                DL_STRING(_, Name) \
                DL_STRING(_, Description) \
                DL_OPTIONAL_STRING(_, Icon)

        DL_DECLARE_ACCESS(Gem_PROPERTIES)
        DL_DECLARE_VARIABLES(Gem_PROPERTIES)
};
