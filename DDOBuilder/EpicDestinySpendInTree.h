// EpicDestinySpendInTree.h
//
// An XML object wrapper that holds information on enhancements trained in a specific tree
#pragma once
#include "XmlLib\DLMacros.h"
#include "TrainedEnhancement.h"

class EpicDestinySpendInTree :
    public XmlLib::SaxContentElement
{
    public:
        EpicDestinySpendInTree();
        void Write(XmlLib::SaxWriter * writer) const;

        size_t TrainEnhancement(
                const std::string & enhancementName,
                const std::string & selection,
                size_t cost,
                bool isTier5,
                size_t * ranks); // returns actual cost, may differ to what is passed in
        int RevokeEnhancement(
                std::string * revokedEnhancement,
                std::string * revokedEnhancementSelection,
                size_t * ranks); // revokes last enhancement trained in tree (if any)
        void SetSpent(size_t apsSpent);
        size_t Spent() const;
    protected:
        XmlLib::SaxContentElementInterface * StartElement(
                const XmlLib::SaxString & name,
                const XmlLib::SaxAttributes & attributes);

        virtual void EndElement();

        #define EpicDestinySpendInTree_PROPERTIES(_) \
                DL_STRING(_, TreeName) \
                DL_SIMPLE(_, size_t, TreeVersion, 0) \
                DL_OBJECT_LIST(_, TrainedEnhancement, Enhancements)

        DL_DECLARE_ACCESS(EpicDestinySpendInTree_PROPERTIES)
        DL_DECLARE_VARIABLES(EpicDestinySpendInTree_PROPERTIES)
    private:
        size_t NextBuyIndex() const;
        size_t m_pointsSpent;       // run time only as prices can change between runs on enhancement updates

        friend class Character;
        friend class Build;
};
