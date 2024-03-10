// Selector.h
//
#pragma once
#include "XmlLib\DLMacros.h"
#include "EnhancementSelection.h"

class EnhancementTree;
class Feat;

class Selector :
    public XmlLib::SaxContentElement
{
    public:
        Selector(void);
        void Write(XmlLib::SaxWriter * writer) const;
        std::string SelectedIcon(const std::string& selectionName) const;
        void RenderIcon(const std::string& selection, CDC * pDC, const CRect & itemRect) const;
        std::string Selector::DisplayName(const std::string& selection) const;

        std::list<Effect> GetEffects(const std::string& selection, size_t rank) const;
        std::list<Stance> GetStances(const std::string&selection, size_t rank) const;

        std::list<DC> EffectDCs(const std::string& selection) const;
        std::list<Stance> Stances(const std::string& selection) const;

        size_t MinSpent(const std::string& selection, size_t defaultMinSpent) const;
        bool CostVaries(const std::string& selection) const;
        size_t Cost(const std::string& selection, size_t rank) const;
        size_t Ranks(const std::string& selection, size_t defaultRank) const;
        const std::vector<size_t>& ItemCosts(const std::string& selection) const;
        bool IsSelectionClickie(const std::string& selection) const;
        bool HasTrainableOption(const Build& build, size_t spentInTree) const;

        bool VerifyObject(
                std::stringstream * ss,
                const std::list<EnhancementTree> & trees) const;
    protected:
        XmlLib::SaxContentElementInterface * StartElement(
                const XmlLib::SaxString & name,
                const XmlLib::SaxAttributes & attributes);

        virtual void EndElement();

        #define Selector_PROPERTIES(_) \
                DL_STRING_LIST(_, Exclusions) \
                DL_OBJECT_LIST(_, EnhancementSelection, Selections)

        DL_DECLARE_ACCESS(Selector_PROPERTIES)
        DL_DECLARE_VARIABLES(Selector_PROPERTIES)

        friend class CEnhancementEditorDialog;
};
