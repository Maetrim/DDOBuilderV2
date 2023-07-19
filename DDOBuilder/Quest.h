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

        enum FavorIndex
        {
            FI_solo = 0,
            FI_casual,
            FI_normal,
            FI_hard,
            FI_elite,
            FI_reaper,

            FI_count
        };

        int MaxFavor() const;

        bool VerifyObject() const;

    protected:
        XmlLib::SaxContentElementInterface * StartElement(
                const XmlLib::SaxString & name,
                const XmlLib::SaxAttributes & attributes);

        virtual void EndElement();

        #define Quest_PROPERTIES(_) \
                DL_STRING(_, Name) \
                DL_ENUM(_, PatronType, Patron, Patron_Unknown, patronTypeMap) \
                DL_STRING(_, AdventurePack) \
                DL_FLAG(_, Solo) \
                DL_FLAG(_, Casual) \
                DL_FLAG(_, Normal) \
                DL_FLAG(_, Hard) \
                DL_FLAG(_, Elite) \
                DL_FLAG(_, Reaper) \
                DL_VECTOR(_, int, Favor) \
                DL_VECTOR(_, int, Levels)

        DL_DECLARE_ACCESS(Quest_PROPERTIES)
        DL_DECLARE_VARIABLES(Quest_PROPERTIES)
};
