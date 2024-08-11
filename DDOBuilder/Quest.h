// Quest.h
//
#pragma once
#include "XmlLib\DLMacros.h"
#include "PatronTypes.h"
#include "QuestDifficultyTypes.h"

class Quest :
    public XmlLib::SaxContentElement
{
    public:
        Quest(void);
        Quest(const std::string& name);
        void Write(XmlLib::SaxWriter * writer) const;

        static void SetSortInfo(int column, bool bAscending);

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

        int Favor(QuestDifficulty diff) const;

        void SetDifficulty(QuestDifficulty diff);
        QuestDifficulty GetDifficulty() const;

        bool VerifyObject() const;
        bool operator<(const Quest& other) const;

    protected:
        XmlLib::SaxContentElementInterface * StartElement(
                const XmlLib::SaxString & name,
                const XmlLib::SaxAttributes & attributes);

        virtual void EndElement();

        #define Quest_PROPERTIES(_) \
                DL_STRING(_, Name) \
                DL_OPTIONAL_STRING(_, EpicName) \
                DL_ENUM(_, PatronType, Patron, Patron_Unknown, patronTypeMap) \
                DL_STRING(_, AdventurePack) \
                DL_FLAG(_, Solo) \
                DL_FLAG(_, Casual) \
                DL_FLAG(_, Normal) \
                DL_FLAG(_, Hard) \
                DL_FLAG(_, Elite) \
                DL_FLAG(_, Reaper) \
                DL_FLAG(_, IsRaid) \
                DL_SIMPLE(_, int, Favor, 0) \
                DL_VECTOR(_, size_t, Levels) \
                DL_FLAG(_, IgnoreForTotalFavor)

        DL_DECLARE_ACCESS(Quest_PROPERTIES)
        DL_DECLARE_VARIABLES(Quest_PROPERTIES)

        QuestDifficulty m_difficulty;

        static int gm_column;
        static bool gm_bAscending;

        friend class CDDOBuilderApp;
};
