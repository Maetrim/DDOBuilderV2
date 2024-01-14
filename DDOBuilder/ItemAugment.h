// ItemAugment.h
//
// An XML object wrapper that holds information on which equipment slot(s)
// and item can be equipped to.
#pragma once
#include "XmlLib\DLMacros.h"
#include "Augment.h"

class ItemAugment :
    public XmlLib::SaxContentElement
{
    public:
        ItemAugment(void);
        void Write(XmlLib::SaxWriter * writer) const;

        const Augment& GetSelectedAugment() const;

        bool VerifyObject(std::stringstream * ss) const;

        void SetType(const std::string& type);
        void ClearSelectedAugment();
        void ClearValue();
        void SetSelectedAugment(const std::string& name);
        void SetSelectedLevelIndex(int level);
        void SetValue(double value);
        void SetValue2(double value);

    protected:
        XmlLib::SaxContentElementInterface * StartElement(
                const XmlLib::SaxString & name,
                const XmlLib::SaxAttributes & attributes);

        virtual void EndElement();

        #define ItemAugment_PROPERTIES(_) \
                DL_STRING(_, Type) \
                DL_OPTIONAL_STRING(_, SelectedAugment) \
                DL_OPTIONAL_SIMPLE(_, int, SelectedLevelIndex, 0) \
                DL_OPTIONAL_SIMPLE(_, double, Value, 0.0) \
                DL_OPTIONAL_SIMPLE(_, double, Value2, 0.0) \
                DL_OBJECT_LIST(_, Augment, ItemSpecificAugments)

        DL_DECLARE_ACCESS(ItemAugment_PROPERTIES)
        DL_DECLARE_VARIABLES(ItemAugment_PROPERTIES)

        friend class WikiItemFileProcessor;
        friend class Build;
        friend class EquippedGear;
};
