// EnhancementTree.cpp
//
#include "StdAfx.h"
#include "EnhancementTree.h"
#include "XmlLib\SaxWriter.h"
#include "GlobalSupportFunctions.h"
#include "LogPane.h"
#include "Feat.h"

#define DL_ELEMENT EnhancementTree

namespace
{
    const wchar_t f_saxElementName[] = L"EnhancementTree";
    DL_DEFINE_NAMES(EnhancementTree_PROPERTIES)

    const unsigned f_verCurrent = 1;
}

EnhancementTree::EnhancementTree() :
    XmlLib::SaxContentElement(f_saxElementName, f_verCurrent)
{
    DL_INIT(EnhancementTree_PROPERTIES)
    m_Icon = "NoIcon";  // default to a blank icon
    m_Background = "NoTreeBackground";
}

DL_DEFINE_ACCESS(EnhancementTree_PROPERTIES)

XmlLib::SaxContentElementInterface * EnhancementTree::StartElement(
        const XmlLib::SaxString & name,
        const XmlLib::SaxAttributes & attributes)
{
    XmlLib::SaxContentElementInterface * subHandler =
            SaxContentElement::StartElement(name, attributes);

    DL_START(EnhancementTree_PROPERTIES)

    ASSERT(subHandler != NULL || wasFlag);
    return subHandler;
}

void EnhancementTree::EndElement()
{
    // now check out our object
    SaxContentElement::EndElement();
    DL_END(EnhancementTree_PROPERTIES)
}

void EnhancementTree::Write(XmlLib::SaxWriter * writer) const
{
    writer->StartElement(ElementName());//, VersionAttributes());
    DL_WRITE(EnhancementTree_PROPERTIES)
    writer->EndElement();
}

bool EnhancementTree::operator==(const EnhancementTree & other) const
{
    // only need to compare the name
    return (Name() == other.Name());
}

bool EnhancementTree::MeetRequirements(
        const Build & build,
        size_t level) const
{
    bool met = true;
    if (HasRequirementsToTrain())
    {
        met = m_RequirementsToTrain.Met(build, level, false, Weapon_Unknown, Weapon_Unknown);
    }
    return met;
}

void EnhancementTree::VerifyObject(
        std::map<std::string, int> * names,
        const std::list<EnhancementTree> & trees) const
{
    bool ok = true;
    std::stringstream ss;
    ss << "=====" << m_Name << "=====\n";
    if (!ImageFileExists("DataFiles\\UIImages\\", Icon()))
    {
        ss << "EnhancementTree is missing image file \"" << Icon() << "\"\n";
        ok = false;
    }
    if (HasRequirementsToTrain())
    {
        ok |= m_RequirementsToTrain.VerifyObject(&ss);
    }
    // check this enhancement tree out for any inconsistencies
    std::list<EnhancementTreeItem>::const_iterator it = m_Items.begin();
    while (it != m_Items.end())
    {
        ok &= (*it).VerifyObject(&ss, trees);
        if (names->find((*it).InternalName()) != names->end())
        {
            ss << "EnhancementTree item " << (*it).InternalName() << " is a duplicate\n";
            ok = false;
        }
        else
        {
            // add this one to the map of used names
            (*names)[(*it).InternalName()] = 0;
        }
        ++it;
    }
    if (!ok)
    {
        GetLog().AddLogEntry(ss.str().c_str());
    }
}

const EnhancementTreeItem * EnhancementTree::FindEnhancementItem(
        const std::string& enhancementName) const
{
    const EnhancementTreeItem * pItem = NULL;
    std::list<EnhancementTreeItem>::const_iterator it = m_Items.begin();
    while (it != m_Items.end())
    {
        if ((*it).InternalName() == enhancementName)
        {
            pItem = &(*it);
            break;      // found
        }
        ++it;
    }
    return pItem;
}

const EnhancementTree& EnhancementTree::GetTree(
    const std::string& treeName)
{
    static EnhancementTree emptyTree;
    const std::list<EnhancementTree>& trees = EnhancementTrees();
    // find the tree we want
    auto tit = trees.begin();
    while (tit != trees.end())
    {
        if ((*tit).Name() == treeName)
        {
            // this is the one we want
            break;
        }
        ++tit;
    }
    if (tit != trees.end())
    {
        return (*tit);
    }
    // make sure on failed to find a tree we return a valid blank object
    emptyTree.Set_Background("NoTreeBackground");
    emptyTree.Set_Icon("NoIcon");
    return emptyTree;
}

std::list<Effect> EnhancementTree::GetEnhancementEffects(
    const std::string& treeName,
    const std::string& enhancementName,
    const std::string& selection,
    size_t rank)
{
    const EnhancementTree& tree = GetTree(treeName);

    std::list<Effect> effects;      // assume no effects
    // we have the tree, now find the enhancement
    const EnhancementTreeItem* pItem = tree.FindEnhancementItem(enhancementName);
    if (pItem != NULL)
    {
        // we found the tree item
        effects = pItem->GetEffects(selection, rank);
    }
    return effects;
}
