// TrainedFeat.h
//
// An XML object wrapper that holds information on an affect that a feat has.
#pragma once
#include "XmlLib\DLMacros.h"

class TrainedFeat :
    public XmlLib::SaxContentElement
{
    public:
        TrainedFeat();
        TrainedFeat(const std::string featName, const std::string& type, size_t level, bool bFeatSwapWarning = false);

        void Write(XmlLib::SaxWriter * writer) const;
        bool operator<(const TrainedFeat & other) const;
        bool operator==(const TrainedFeat & other) const;

        // these are used for forum export only
        size_t Count() const;
        void IncrementCount();
        void SetAlternateFeat(const std::string& featName);

    protected:
        XmlLib::SaxContentElementInterface * StartElement(
                const XmlLib::SaxString & name,
                const XmlLib::SaxAttributes & attributes);

        virtual void EndElement();

        #define TrainedFeat_PROPERTIES(_) \
                DL_STRING(_, FeatName) \
                DL_OPTIONAL_STRING(_, AlternateFeatName) \
                DL_STRING(_, Type) \
                DL_SIMPLE(_, size_t, LevelTrainedAt, 0) \
                DL_FLAG(_, FeatSwapWarning)

        DL_DECLARE_ACCESS(TrainedFeat_PROPERTIES)
        DL_DECLARE_VARIABLES(TrainedFeat_PROPERTIES)

    private:
        void TranslateOldFeatNames();
        size_t m_count;
        friend class FeatsListObject;
};
