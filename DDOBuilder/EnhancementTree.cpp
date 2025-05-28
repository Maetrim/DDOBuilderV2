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

XmlLib::SaxContentElementInterface* EnhancementTree::StartElement(
    const XmlLib::SaxString& name,
    const XmlLib::SaxAttributes& attributes)
{
    XmlLib::SaxContentElementInterface* subHandler =
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

    // rename enhancement items for a legacy tree
    if (HasLegacy())
    {
        UpdateLegacyInfo();
    }
}

void EnhancementTree::Write(XmlLib::SaxWriter* writer) const
{
    writer->StartElement(ElementName());//, VersionAttributes());
    //DL_WRITE(EnhancementTree_PROPERTIES)
    // expanded MACRO version to allow comments to be placed in output
    writer->WriteSimpleElement(f_saxName, m_Name);
    writer->WriteSimpleElement(f_saxVersion, m_Version);
    if (m_hasRequirementsToTrain) { m_RequirementsToTrain.Write(writer); }
    if (m_hasIsEpicDestiny) { writer->WriteEmptyElement(f_saxIsEpicDestiny); }
    if (m_hasIsRacialTree) { writer->WriteEmptyElement(f_saxIsRacialTree); }
    if (m_hasIsReaperTree) { writer->WriteEmptyElement(f_saxIsReaperTree); }
    if (m_hasIsUniversalTree) { writer->WriteEmptyElement(f_saxIsUniversalTree); }
    writer->WriteSimpleElement(f_saxIcon, m_Icon);
    writer->WriteSimpleElement(f_saxBackground, m_Background);
    if (!m_Items.empty())
    {
        int last = -1;
        std::list<EnhancementTreeItem>::const_iterator iter;
        for (iter = m_Items.begin(); iter != m_Items.end(); ++iter)
        {
            if ((int)iter->YPosition() != last)
            {
                last = iter->YPosition();
                switch (last)
                {
                    case 0: writer->WriteComment(L"Core Items"); break;
                    case 1: writer->WriteComment(L"Tier 1"); break;
                    case 2: writer->WriteComment(L"Tier 2"); break;
                    case 3: writer->WriteComment(L"Tier 3"); break;
                    case 4: writer->WriteComment(L"Tier 4"); break;
                    case 5: writer->WriteComment(L"Tier 5"); break;
                }
            }
            iter->Write(writer);
        }
    }
    writer->EndElement();
}

bool EnhancementTree::operator<(const EnhancementTree & other) const
{
    bool bBefore =  (Name() < other.Name());
    return bBefore;
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
        met = m_RequirementsToTrain.Met(build, level, false, Inventory_Unknown, Weapon_Unknown, Weapon_Unknown);
    }
    return met;
}

void EnhancementTree::SetFilename(const std::string& filename)
{
    m_filename = filename;
}

void EnhancementTree::Save() const
{
    XmlLib::SaxWriter writer;
    // write out the tree as xml
    if (writer.Open(m_filename))
    {
        writer.StartDocument(L"Enhancements");
        Write(&writer);
        writer.EndDocument();
    }
}

EnhancementTreeItem* EnhancementTree::FindItemByPosition(
    size_t x,
    size_t y) const
{
    EnhancementTreeItem* pItem = NULL;
    for (auto&& iit : m_Items)
    {
        if (iit.XPosition() == x && iit.YPosition() == y)
        {
            const EnhancementTreeItem* pConstItem = &iit;
            pItem = const_cast<EnhancementTreeItem*>(pConstItem);
        }
    }
    return pItem;
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

void EnhancementTree::UpdateLegacyInfo()
{
    // all enhancement need to also be renamed so no clashes
    std::stringstream ss;
    ss << "V" << Version();
    std::string prepend = ss.str();
    for (auto&& it : m_Items)
    {
        std::stringstream si;
        si << prepend << it.InternalName();
        it.Set_InternalName(si.str());
        // we also need to update any requirements for enhancements that require enhancements
        if (it.HasRequirementsToTrain())
        {
            it.m_RequirementsToTrain.UpdateEnhancementRequirements(prepend);
        }
        it.UpdateLegacyInfo(prepend, this);
    }
}
