// EnhacementTree.h
//
#pragma once
#include "XmlLib\DLMacros.h"
#include "EnhancementTreeItem.h"
#include "Requirements.h"
#include <list>
#include <map>

class Build;
class Feat;

class EnhancementTree :
    public XmlLib::SaxContentElement
{
    public:
        static const EnhancementTree & GetTree(const std::string& treeName);
        static std::list<Effect> GetEnhancementEffects(
                const std::string& treeName,
                const std::string& enhancementName,
                const std::string& selection,
                size_t rank);

        EnhancementTree(void);
        void Write(XmlLib::SaxWriter * writer) const;
        const EnhancementTreeItem * FindEnhancementItem(const std::string& enhancementName) const;
        bool MeetRequirements(const Build & build, size_t level) const;

        void SetFilename(const std::string&) {};

        bool operator==(const EnhancementTree & other) const;
        void VerifyObject(
                std::map<std::string, int> * names,
                const std::list<EnhancementTree> & trees) const;

    protected:
        XmlLib::SaxContentElementInterface * StartElement(
                const XmlLib::SaxString & name,
                const XmlLib::SaxAttributes & attributes);

        virtual void EndElement();

        #define EnhancementTree_PROPERTIES(_) \
                DL_STRING(_, Name) \
                DL_SIMPLE(_, size_t, Version, 0) \
                DL_OPTIONAL_OBJECT(_, Requirements, RequirementsToTrain) \
                DL_FLAG(_, IsEpicDestiny) \
                DL_FLAG(_, IsRacialTree) \
                DL_FLAG(_, IsReaperTree) \
                DL_FLAG(_, IsUniversalTree) \
                DL_STRING(_, Icon) \
                DL_STRING(_, Background) \
                DL_OBJECT_LIST(_, EnhancementTreeItem, Items)

        DL_DECLARE_ACCESS(EnhancementTree_PROPERTIES)
        DL_DECLARE_VARIABLES(EnhancementTree_PROPERTIES)

        friend class CEnhancementEditorDialog;
};
