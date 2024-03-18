// CompletedQuest.h
//
#pragma once
#include "XmlLib\DLMacros.h"
#include "QuestDifficultyTypes.h"

class CompletedQuest :
    public XmlLib::SaxContentElement
{
    public:
        CompletedQuest(void);
        void Write(XmlLib::SaxWriter * writer) const;

    protected:
        XmlLib::SaxContentElementInterface * StartElement(
                const XmlLib::SaxString & name,
                const XmlLib::SaxAttributes & attributes);

        virtual void EndElement();

        #define CompletedQuest_PROPERTIES(_) \
                DL_STRING(_, Name) \
                DL_SIMPLE(_, size_t, Level, 0) \
                DL_ENUM(_, QuestDifficulty, Difficulty, QD_Unknown, questDifficultyTypeMap)

        DL_DECLARE_ACCESS(CompletedQuest_PROPERTIES)
        DL_DECLARE_VARIABLES(CompletedQuest_PROPERTIES)
};
