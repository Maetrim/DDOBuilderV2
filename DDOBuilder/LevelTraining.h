// LevelTraining.h
//
// An XML object wrapper that holds information on an affect that a LevelTraining has.
#pragma once
#include "XmlLib\DLMacros.h"
#include "TrainedFeat.h"
#include "TrainedSkill.h"

class LevelTraining :
    public XmlLib::SaxContentElement
{
    public:
        LevelTraining(void);
        void Write(XmlLib::SaxWriter * writer) const;

        void TrainFeat(
                const std::string& featName,
                const std::string& type,
                size_t level,
                bool featSwapWarning);
        void TrainAlternateFeat(
            const std::string& featName,
            const std::string& type);
        std::string RevokeFeat(const std::string& type);
        void RevokeAllFeats(const std::string& type);
        std::string FeatName(const std::string& type) const;

        void TrainSkill(SkillType skill);
        void RevokeSkill(SkillType skill);

        // as not in macros we still need to support the standard macro access functions
        // that would have been generated for us.
        const std::list<TrainedFeat> & AutomaticFeats() const;
        void Set_AutomaticFeats(const std::list<TrainedFeat> & newValue);

    protected:
        XmlLib::SaxContentElementInterface * StartElement(
                const XmlLib::SaxString & name,
                const XmlLib::SaxAttributes & attributes);

        virtual void EndElement();

        #define LevelTraining_PROPERTIES(_) \
                DL_OPTIONAL_STRING(_, Class) \
                DL_SIMPLE(_, size_t, SkillPointsAvailable, 0) \
                DL_SIMPLE(_, size_t, SkillPointsSpent, 0) \
                DL_OBJECT_LIST(_, TrainedFeat, TrainedFeats) \
                DL_OBJECT_LIST(_, TrainedSkill, TrainedSkills)

        DL_DECLARE_ACCESS(LevelTraining_PROPERTIES)
        DL_DECLARE_VARIABLES(LevelTraining_PROPERTIES)

        // if this was included in the macros, they would be saved to the file,
        // we do not want these saved as they are regenerated automatically on load.
        std::list<TrainedFeat> m_AutomaticFeats;

        friend class Build;
};
