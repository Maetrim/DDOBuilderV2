// EnhancementSelection.h
//
#pragma once
#include "XmlLib\DLMacros.h"
#include "DC.h"
#include "EnhancementSelection.h"
#include "Effect.h"
#include "Requirements.h"
#include "Stance.h"

class Build;
class EnhancementTree;
class Feat;

class EnhancementSelection :
    public XmlLib::SaxContentElement
{
    public:
        EnhancementSelection(void);
        void Write(XmlLib::SaxWriter * writer) const;
        void RenderIcon(CDC * pDC, const CRect & itemRect) const;
        void CreateRequirementStrings(
                const Build & build,
                std::vector<CString> * requirements,
                std::vector<bool> * met,
                size_t level) const;

        bool CostVaries() const;
        size_t Cost(size_t rank) const;

        std::list<Effect> GetEffects(size_t rank) const;
        std::list<Stance> GetStances(size_t rank) const;

        bool VerifyObject(
                std::stringstream * ss) const;
    protected:
        XmlLib::SaxContentElementInterface * StartElement(
                const XmlLib::SaxString & name,
                const XmlLib::SaxAttributes & attributes);

        virtual void EndElement();

        #define EnhancementSelection_PROPERTIES(_) \
                DL_STRING(_, Name) \
                DL_STRING(_, Description) \
                DL_STRING(_, Icon) \
                DL_VECTOR(_, size_t, CostPerRank) \
                DL_FLAG(_, Clickie) \
                DL_OPTIONAL_SIMPLE(_, size_t, MinSpent, 0) \
                DL_OPTIONAL_SIMPLE(_, size_t, Ranks, 0) \
                DL_OPTIONAL_OBJECT(_, Requirements, RequirementsToTrain) \
                DL_OBJECT_LIST(_, Stance, Stances) \
                DL_OBJECT_LIST(_, Effect, Effects) \
                DL_OBJECT_LIST(_, DC, EffectDC)

        DL_DECLARE_ACCESS(EnhancementSelection_PROPERTIES)
        DL_DECLARE_VARIABLES(EnhancementSelection_PROPERTIES)

    private:
        mutable int m_imageIndex;
        static int AddImage(const std::string& icon);

        static CImageList sm_enhancementImages;

        friend class CEnhancementEditorDialog;
};
