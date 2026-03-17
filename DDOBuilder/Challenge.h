// Challenge.h
//
#pragma once
#include "XmlLib\DLMacros.h"
#include "PatronTypes.h"
#include "QuestDifficultyTypes.h"

class Challenge :
    public XmlLib::SaxContentElement
{
    public:
        Challenge(void);
        Challenge(const std::string& name);
        void Write(XmlLib::SaxWriter * writer) const;

        static void SetSortInfo(int column, bool bAscending);

        enum FavorIndex
        {
            FI_notRun = 0,
            FI_1Star,
            FI_2Star,
            FI_3Star,
            FI_4Star,
            FI_5Star,
            FI_6Star,

            FI_count
        };

        int MaxFavor() const;
        int Favor(QuestDifficulty diff) const;

        void SetDifficulty(QuestDifficulty diff);
        QuestDifficulty GetDifficulty() const;

        bool VerifyObject() const;
        bool operator<(const Challenge& other) const;

    protected:
        XmlLib::SaxContentElementInterface * StartElement(
                const XmlLib::SaxString & name,
                const XmlLib::SaxAttributes & attributes);

        virtual void EndElement();

        #define Challenge_PROPERTIES(_) \
                DL_STRING(_, Name) \
                DL_ENUM(_, PatronType, Patron, Patron_Unknown, patronTypeMap) \
                DL_STRING(_, AdventurePack) \
                DL_VECTOR(_, size_t, LevelRange)

        DL_DECLARE_ACCESS(Challenge_PROPERTIES)
        DL_DECLARE_VARIABLES(Challenge_PROPERTIES)

        QuestDifficulty m_difficulty;

        static int gm_column;
        static bool gm_bAscending;

        friend class CDDOBuilderApp;
};
