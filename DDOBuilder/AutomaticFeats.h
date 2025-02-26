// AutomaticFeats.h
//
// An XML object wrapper that holds information on which automatic feats are
// gained at the specified class or race level
#pragma once
#include "XmlLib\DLMacros.h"
#include "Effect.h"
#include "WeaponClassTypes.h"
#include "Stance.h"

class AutomaticFeats :
    public XmlLib::SaxContentElement
{
    public:
        AutomaticFeats(void);
        void Write(XmlLib::SaxWriter * writer) const;

        void VerifyObject() const;

    protected:
        XmlLib::SaxContentElementInterface * StartElement(
                const XmlLib::SaxString & name,
                const XmlLib::SaxAttributes & attributes);

        virtual void EndElement();

        #define AutomaticFeats_PROPERTIES(_) \
                DL_SIMPLE(_, size_t, Level, 0) \
                DL_STRING_LIST(_, Feats)

        DL_DECLARE_ACCESS(AutomaticFeats_PROPERTIES)
        DL_DECLARE_VARIABLES(AutomaticFeats_PROPERTIES)
};
