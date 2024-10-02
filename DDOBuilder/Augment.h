// Augment.h
//
// An XML object wrapper that holds information on which equipment slot(s)
// and item can be equipped to.
#pragma once
#include "XmlLib\DLMacros.h"
#include "Effect.h"
#include "WeaponClassTypes.h"
#include "Stance.h"

class Augment :
    public XmlLib::SaxContentElement
{
    public:
        Augment(void);
        void Write(XmlLib::SaxWriter * writer) const;

        bool IsCompatibleWithSlot(const std::string& augmentType, bool bMatchingColourOnly, const std::string& selectedAugment) const;
        void AddImage(CImageList * pIL) const;
        bool operator<(const Augment & other) const;
        std::string CompoundDescription() const;

        void VerifyObject() const;

        void SetFilename(const std::string&) {}; // dummy

    protected:
        XmlLib::SaxContentElementInterface * StartElement(
                const XmlLib::SaxString & name,
                const XmlLib::SaxAttributes & attributes);

        virtual void EndElement();

        #define Augment_PROPERTIES(_) \
                DL_STRING(_, Name) \
                DL_STRING(_, Description) \
                DL_OPTIONAL_SIMPLE(_, size_t, MinLevel, 0) \
                DL_OPTIONAL_STRING(_, Icon) \
                DL_STRING_LIST(_, Type) \
                DL_STRING_LIST(_, AddAugment) \
                DL_OPTIONAL_STRING(_, GrantAugment) \
                DL_OPTIONAL_STRING(_, GrantConditionalAugment) \
                DL_OPTIONAL_ENUM(_, WeaponClassType, WeaponClass, WeaponClass_Unknown, weaponClassTypeMap) \
                DL_FLAG(_, DualValues) \
                DL_FLAG(_, EnterValue) \
                DL_FLAG(_, ChooseLevel) \
                DL_OPTIONAL_VECTOR(_, int, Levels) \
                DL_OPTIONAL_VECTOR(_, double, LevelValue) \
                DL_OPTIONAL_VECTOR(_, double, LevelValue2) \
                DL_STRING_LIST(_, SetBonus) \
                DL_FLAG(_, SuppressSetBonus) \
                DL_OBJECT_LIST(_, Stance, StanceData) \
                DL_STRING_LIST(_, EffectDescription) \
                DL_OBJECT_LIST(_, Effect, Effects)

        DL_DECLARE_ACCESS(Augment_PROPERTIES)
        DL_DECLARE_VARIABLES(Augment_PROPERTIES)

        friend class CItemSelectDialog;
};
