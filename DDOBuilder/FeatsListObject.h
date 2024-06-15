// FeatsListObject.h
//
// An XML object wrapper that holds information on an affect that a feat has.
#pragma once
#include "XmlLib\DLMacros.h"
#include "TrainedFeat.h"

class FeatsListObject :
    public XmlLib::SaxContentElement
{
    public:
        FeatsListObject(const XmlLib::SaxString & objectName);
        FeatsListObject(const XmlLib::SaxString& objectName, const std::list<TrainedFeat> & feats);
        void Write(XmlLib::SaxWriter * writer) const;

        void Add(const TrainedFeat& feat);

        void TrainFeat(
                const std::string& featName,
                const std::string& type,
                size_t level,
                bool featSwapWarning);
        void TrainAlternateFeat(
                const std::string& featName,
                const std::string& type,
                size_t level);
        std::string RevokeFeat(const std::string& type);
        void RevokeAllFeats(const std::string& type);
        std::string FeatName(const std::string& type) const;

    protected:
        XmlLib::SaxContentElementInterface * StartElement(
                const XmlLib::SaxString & name,
                const XmlLib::SaxAttributes & attributes);

        virtual void EndElement();

        #define FeatsListObject_PROPERTIES(_) \
                DL_OBJECT_LIST(_, TrainedFeat, Feats)

        DL_DECLARE_ACCESS(FeatsListObject_PROPERTIES)
        DL_DECLARE_VARIABLES(FeatsListObject_PROPERTIES)
};
