// SelectedTrees.h
//
#pragma once
#include "XmlLib\DLMacros.h"

class SelectedTrees :
    public XmlLib::SaxContentElement
{
    public:
        SelectedTrees(const XmlLib::SaxString & elementName, size_t maxTrees);
        void Write(XmlLib::SaxWriter * writer) const;

        virtual bool SaxElementIsSelf(
                const XmlLib::SaxString & name,
                const XmlLib::SaxAttributes & attributes) override;

        bool IsTreePresent(const std::string& treeName) const;
        const std::string& Tree(size_t index) const;
        void SetTree(size_t index, const std::string& treeName);
        void SetNotSelected(size_t index);
        void SwapTrees(const std::string& tree1, const std::string& tree2);
        void SetTier5Tree(const std::string& treeName);
        void ClearTier5Tree();

        static bool IsNotSelected(const std::string& treeName);

    protected:
        XmlLib::SaxContentElementInterface * StartElement(
                const XmlLib::SaxString & name,
                const XmlLib::SaxAttributes & attributes);

        virtual void EndElement();

        #define SelectedTrees_PROPERTIES(_) \
                DL_STRING_VECTOR(_, TreeName) \
                DL_OPTIONAL_STRING(_, Tier5Tree)

        DL_DECLARE_ACCESS(SelectedTrees_PROPERTIES)
        DL_DECLARE_VARIABLES(SelectedTrees_PROPERTIES)
    private:
        void TranslateNamesFromV1();
        size_t m_nMaxTrees;
};
