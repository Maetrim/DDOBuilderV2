// AttackChain.h
//
// An XML object wrapper that holds information on which attacks in what order are done
#pragma once
#include "XmlLib\DLMacros.h"

class AttackChain :
    public XmlLib::SaxContentElement
{
    public:
        AttackChain(void);
        void Write(XmlLib::SaxWriter * writer) const;

        void SetName(const std::string& newName);
        void SetAttacks(const std::list<std::string>& attacks);

    protected:
        XmlLib::SaxContentElementInterface * StartElement(
                const XmlLib::SaxString & name,
                const XmlLib::SaxAttributes & attributes);

        virtual void EndElement();

        #define AttackChain_PROPERTIES(_) \
                DL_STRING(_, Name) \
                DL_STRING_LIST(_, Attacks)

        DL_DECLARE_ACCESS(AttackChain_PROPERTIES)
        DL_DECLARE_VARIABLES(AttackChain_PROPERTIES)
};
