// BasicDice.h
//
// An XML object wrapper that holds information on dice to be rolled
#pragma once
#include "XmlLib\DLMacros.h"

class BasicDice :
    public XmlLib::SaxContentElement
{
    public:
        BasicDice(const XmlLib::SaxString& elementName);
        BasicDice(const XmlLib::SaxString& elementName, size_t version);
        void Write(XmlLib::SaxWriter * writer) const;

        CString DiceAsText() const;
        double MinDamage() const;
        double MaxDamage() const;
        double AverageDamage() const;

        void Set(int number, int sides, int bonus);

    protected:
        XmlLib::SaxContentElementInterface * StartElement(
                const XmlLib::SaxString & name,
                const XmlLib::SaxAttributes & attributes);

        virtual void EndElement();

        // eg. (3d6 + 10)
        #define BasicDice_PROPERTIES(_) \
                DL_OPTIONAL_SIMPLE(_, int, Number, 1) \
                DL_OPTIONAL_SIMPLE(_, int, Sides, 6) \
                DL_OPTIONAL_SIMPLE(_, int, Bonus, 1)

        DL_DECLARE_ACCESS(BasicDice_PROPERTIES)
        DL_DECLARE_VARIABLES(BasicDice_PROPERTIES)
};
