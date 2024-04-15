// EnhancementTreeItem.h
//
#pragma once
#include "XmlLib\DLMacros.h"
#include "DC.h"
#include "Effect.h"
#include "Requirements.h"
#include "Selector.h"
#include "Stance.h"

class Build;
class EnhancementTree;
class Feat;
class SpendInTree;

class EnhancementTreeItem :
    public XmlLib::SaxContentElement
{
    public:
        EnhancementTreeItem(void);
        void Write(XmlLib::SaxWriter * writer) const;
        std::string DisplayName(const std::string& selection) const;
        std::string ActiveIcon(const Build & build) const;
        void RenderIcon(const Build & build, CDC * pDC, const CRect & itemRect) const;
        std::string SelectionIcon(const std::string& selection) const;
        bool MeetRequirements(
                const Build & build,
                const std::string& selection,
                const std::string& treeName,
                size_t level,
                size_t spentInTree) const;
        bool IsAllowed(
                const Build & build,
                const std::string& selection,
                size_t level,
                size_t spentInTree) const;
        bool IsTier5Blocked(
                const Build & build,
                const std::string& treeName) const;
        bool CanTrain(
                const Build & build,
                size_t spentInTree,
                TreeType type,
                size_t level) const;
        bool CanRevoke(const SpendInTree* pTreeSpend) const;
        void CreateRequirementStrings(
                const Build & build,
                std::vector<CString> * requirements,
                std::vector<bool> * met,
                size_t level) const;
        std::list<Stance> Stances(const std::string& selection) const;
        bool CostVaries(const std::string& selection) const;
        size_t Cost(const std::string& selection, size_t rank) const;
        const std::vector<size_t>& ItemCosts(const std::string& selection) const;
        bool IsSelectionClickie(const std::string& selection) const;

        std::list<Effect> GetEffects(const std::string&selection, size_t rank) const;
        std::list<Stance> GetStances(const std::string&selection, size_t rank) const;
        std::list<DC> GetDCs(const std::string& selection) const;

        bool RequiresEnhancement(const std::string& name) const;
        bool RequiresEnhancement(const std::string& ourSelection, const std::string& name) const;
        size_t MinSpent(const std::string& selection) const;
        size_t Ranks(const std::string& selection) const;

        bool VerifyObject(
                std::stringstream * ss,
                const std::list<EnhancementTree> & trees) const;

    protected:
        XmlLib::SaxContentElementInterface * StartElement(
                const XmlLib::SaxString & name,
                const XmlLib::SaxAttributes & attributes);

        virtual void EndElement();

        #define EnhancementTreeItem_PROPERTIES(_) \
                DL_STRING(_, Name) \
                DL_STRING(_, InternalName) \
                DL_STRING(_, Description) \
                DL_STRING(_, Icon) \
                DL_SIMPLE(_, size_t, XPosition, 0) \
                DL_SIMPLE(_, size_t, YPosition, 0) \
                DL_VECTOR(_, size_t, CostPerRank) \
                DL_SIMPLE(_, size_t, Ranks, 1) \
                DL_SIMPLE(_, size_t, MinSpent, 0) \
                DL_FLAG(_, Clickie) \
                DL_FLAG(_, ArrowLeft) \
                DL_FLAG(_, ArrowRight) \
                DL_FLAG(_, ArrowUp) \
                DL_FLAG(_, LongArrowUp) \
                DL_FLAG(_, ExtraLongArrowUp) \
                DL_FLAG(_, Tier5) \
                DL_OBJECT_LIST(_, Stance, Stances) \
                DL_OPTIONAL_OBJECT(_, Requirements, RequirementsToTrain) \
                DL_OPTIONAL_OBJECT(_, Selector, Selections) \
                DL_OBJECT_LIST(_, Effect, Effects) \
                DL_OBJECT_VECTOR(_, DC, EffectDC)

        DL_DECLARE_ACCESS(EnhancementTreeItem_PROPERTIES)
        DL_DECLARE_VARIABLES(EnhancementTreeItem_PROPERTIES)

    private:
        static int AddImage(const std::string& icon);
        std::string Name(const std::string& selection) const;

        mutable int m_imageIndex;           // same for both normal and disabled

        static CImageList sm_enhancementImages;
        static CImageList sm_disabledEnhancementImages;

        friend class CEnhancementTreeDialog;
};
