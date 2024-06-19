// SelectedTrees.cpp
//
#include "StdAfx.h"
#include "SelectedTrees.h"
#include "XmlLib\SaxWriter.h"
#include "EnhancementTree.h"
#include "GlobalSupportFunctions.h"

#define DL_ELEMENT SelectedTrees

namespace
{
    DL_DEFINE_NAMES(SelectedTrees_PROPERTIES)

    const unsigned f_verCurrent = 1;
    const std::string f_noSelection = "No selection";
}

SelectedTrees::SelectedTrees(const XmlLib::SaxString & elementName, size_t maxTrees) :
    XmlLib::SaxContentElement(elementName, f_verCurrent),
    m_nMaxTrees(maxTrees)
{
    DL_INIT(SelectedTrees_PROPERTIES)
    m_TreeName.resize(m_nMaxTrees, f_noSelection);
}

DL_DEFINE_ACCESS(SelectedTrees_PROPERTIES)

XmlLib::SaxContentElementInterface * SelectedTrees::StartElement(
        const XmlLib::SaxString & name,
        const XmlLib::SaxAttributes & attributes)
{
    XmlLib::SaxContentElementInterface * subHandler =
            SaxContentElement::StartElement(name, attributes);

    DL_START(SelectedTrees_PROPERTIES)

    return subHandler;
}

void SelectedTrees::EndElement()
{
    SaxContentElement::EndElement();
    DL_END(SelectedTrees_PROPERTIES)
    // ensure we have the right number of elements after load
    m_TreeName.resize(m_nMaxTrees, f_noSelection);
    TranslateNamesFromV1();
}

void SelectedTrees::Write(XmlLib::SaxWriter * writer) const
{
    writer->StartElement(ElementName());//, VersionAttributes());
    DL_WRITE(SelectedTrees_PROPERTIES)
    writer->EndElement();
}

bool SelectedTrees::SaxElementIsSelf(
        const XmlLib::SaxString & name,
        const XmlLib::SaxAttributes & attributes)
{
    bool isUs = XmlLib::SaxContentElement::SaxElementIsSelf(name, attributes);
    if (isUs)
    {
        // a load operation is about to start, clear any previous data we held
        m_TreeName.clear();
    }
    return isUs;
}

bool SelectedTrees::IsTreePresent(const std::string& treeName) const
{
    // return true if the tree is present
    bool present = false;
    for (size_t i = 0; i < m_TreeName.size(); ++i)
    {
        if (m_TreeName[i] == treeName)
        {
            present = true;
        }
    }
    return present;
}

const std::string& SelectedTrees::Tree(size_t index) const
{
    ASSERT(index >= 0 && index <= m_TreeName.size());
    return m_TreeName[index];
}

void SelectedTrees::SetTree(size_t index, const std::string& treeName)
{
    ASSERT(index >= 0 && index <= m_TreeName.size());
    m_TreeName[index] = treeName;
}

void SelectedTrees::SetNotSelected(size_t index)
{
    ASSERT(index >= 0 && index <= m_TreeName.size());
    m_TreeName[index] = f_noSelection;
}

bool SelectedTrees::IsNotSelected(const std::string& treeName)
{
    return (treeName == f_noSelection);
}

void SelectedTrees::SwapTrees(
        const std::string& tree1,
        const std::string& tree2)
{
    bool swapped1 = false;
    bool swapped2 = false;
    for (size_t i = 0; i < m_TreeName.size(); ++i)
    {
        if (!swapped1
                && m_TreeName[i] == tree1)
        {
            swapped1 = true;
            m_TreeName[i] = tree2;
        }
        else if (!swapped2
                && m_TreeName[i] == tree2)
        {
            swapped2 = true;
            m_TreeName[i] = tree1;
        }
    }
}

void SelectedTrees::SetTier5Tree(const std::string& treeName)
{
    Set_Tier5Tree(treeName);
}

void SelectedTrees::ClearTier5Tree()
{
    Clear_Tier5Tree();
}

void SelectedTrees::TranslateNamesFromV1()
{
    static std::string nameTranslations[] =
    {
        // old tree name                        new tree name
        "Ravager",                              "Ravager (Barbarian)",
        "Ravager (Ftr)",                        "Ravager (Fighter)",
        "Arch-Mage",                            "Archmage"
    };
    size_t count = sizeof(nameTranslations) / sizeof(std::string);
    if (count % 2 != 0)
    {
        throw "Must be an multiple of 2";
    }
    for (auto&& it: m_TreeName)
    {
        for (size_t i = 0; i < count; i += 2)
        {
            if (it == nameTranslations[i])
            {
                it = nameTranslations[i + 1];
                break;
            }
        }
    }
}
