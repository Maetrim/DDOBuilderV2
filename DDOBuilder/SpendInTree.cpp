// SpendInTree.cpp
//
#include "StdAfx.h"
#include "SpendInTree.h"
#include "XmlLib\SaxWriter.h"
#include <algorithm>
#include "GlobalSupportFunctions.h"
#include "EnhancementTreeItem.h"
#include "LogPane.h"

#define DL_ELEMENT SpendInTree

namespace
{
    DL_DEFINE_NAMES(SpendInTree_PROPERTIES)

    const unsigned f_verCurrent = 1;

    bool SortEnhancementEntry(const TrainedEnhancement& a, const TrainedEnhancement& b)
    {
        return  (a.RequiredAps() > b.RequiredAps());
    }
}

SpendInTree::SpendInTree(const XmlLib::SaxString & elementName, TreeType type) :
    XmlLib::SaxContentElement(elementName, f_verCurrent),
    m_pointsSpent(0),
    m_type(type)
{
    DL_INIT(SpendInTree_PROPERTIES)
}

SpendInTree::SpendInTree(
        const XmlLib::SaxString & elementName,
        const std::string& treeName,
        TreeType type,
        size_t version) :
    XmlLib::SaxContentElement(elementName, version),
    m_pointsSpent(0),
    m_type(type)
{
    DL_INIT(SpendInTree_PROPERTIES)
    m_TreeName = treeName;
    m_TreeVersion = version;
}

DL_DEFINE_ACCESS(SpendInTree_PROPERTIES)

XmlLib::SaxContentElementInterface * SpendInTree::StartElement(
        const XmlLib::SaxString & name,
        const XmlLib::SaxAttributes & attributes)
{
    XmlLib::SaxContentElementInterface * subHandler =
            SaxContentElement::StartElement(name, attributes);

    DL_START(SpendInTree_PROPERTIES)

    return subHandler;
}

void SpendInTree::EndElement()
{
    SaxContentElement::EndElement();
    DL_END(SpendInTree_PROPERTIES)
    TranslateNamesFromV1();
    // also make sure every loaded TrainedEnhancement has the correct
    // cached cost values for the enhancements it references
    m_pointsSpent = 0;
    for (auto&& teit: m_Enhancements)
    {
        const EnhancementTreeItem* pTreeItem = FindEnhancement(TreeName(), teit.EnhancementName());
        if (pTreeItem != NULL)
        {
            std::string selection = teit.HasSelection() ? teit.Selection() : "";
            teit.SetCost(pTreeItem->ItemCosts(selection));
            teit.SetRequiredAps(pTreeItem->MinSpent(selection));
        }
        else
        {
            std::vector<size_t> defaultCost(1, 1);
            teit.SetCost(defaultCost);
            teit.SetRequiredAps(0);
            std::stringstream ss;
            ss << "Trained enhancement from tree \""
                << TreeName()
                << "\" not found. Using default cost of 1 per rank.";
            GetLog().AddLogEntry(ss.str().c_str());
        }
        // also need to track how many points are spent
        for (size_t rank = 0; rank < teit.Ranks(); ++rank)
        {
            m_pointsSpent += teit.Cost(rank);
        }
    }
}

void SpendInTree::Write(XmlLib::SaxWriter * writer) const
{
    writer->StartElement(ElementName());//, VersionAttributes());
    DL_WRITE(SpendInTree_PROPERTIES)
    writer->EndElement();
}

void SpendInTree::SetTree(
        const std::string& treeName,
        size_t treeVersion)
{
    Set_TreeName(treeName);
    Set_TreeVersion(treeVersion);
}

size_t SpendInTree::TrainedRanks(
        const std::string& enhancementName,
        std::string* pSelection) const
{
    size_t ranks = 0;
    std::list<TrainedEnhancement>::const_iterator it = m_Enhancements.begin();
    while (ranks == 0 && it != m_Enhancements.end())
    {
        if ((*it).EnhancementName() == enhancementName)
        {
            ranks = (*it).Ranks();
            if ((*it).HasSelection())
            {
                *pSelection = (*it).Selection();
            }
            break;
        }
        ++it;
    }
    return ranks;
}

bool SpendInTree::CanRevokeAtTier(size_t minSpent, size_t cost) const
{
    bool canRevoke = true;
    // note that the enhancement are sorted by their MinSpent tier

    // need to evaluate all items at later tiers
    size_t spentBelow = 0;
    size_t spentSame = 0;
    size_t spentAbove = 0;
    for (auto&& it : m_Enhancements)
    {
        if (it.RequiredAps() < minSpent)
        {
            // its at the below tier, count it as APs spent
            spentBelow += it.TotalCost();
        }
        else if (it.RequiredAps() == minSpent)
        {
            // its at the same tier, count its APs spent
            spentSame += it.TotalCost();
        }
        else
        {
            // we need to do a recursive call at the deeper tier
            // to see if revoke can go ahead
            canRevoke = EnoughPointsSpentAtLowerTiers(it.RequiredAps(), cost);
            if (canRevoke)
            {
                // later tier revoke is ok, but are there enough Aps
                // spent at this level to allow a revoke and let later tiers
                // stay ok?
                canRevoke = (((spentBelow + spentSame + spentAbove) - cost) >= it.RequiredAps());
                spentAbove += it.TotalCost();
            }
        }
        if (!canRevoke)
        {
            break;
        }
    }
    return canRevoke;
}

bool SpendInTree::EnoughPointsSpentAtLowerTiers(size_t minSpent, size_t cost) const
{
    // need to evaluate all items at lower tiers
    size_t spentBelow = 0;
    for (auto&& it : m_Enhancements)
    {
        if (it.RequiredAps() < minSpent)
        {
            // its at the below tier, count it as APs spent
            spentBelow += it.TotalCost();
        }
    }
    return ((spentBelow - cost) >= minSpent) || (minSpent == 1 && spentBelow != 1);
}

bool SpendInTree::HasTrainedDependants(
    const std::string& enhancementName,
    size_t ranksTrained) const
{
    bool hasDependants = false;
    // look through the list of enhancement items trained
    // if we find one which has a requirement on the "enhancementName" item
    // it must have lower trained ranks than "enhancementName" to allow the
    // revoke to go ahead.
    for (auto&& it : m_Enhancements)
    {
        if (it.HasRequirementOf(enhancementName))
        {
            hasDependants = (it.Ranks() >= ranksTrained); // cannot revoke if has equal or more ranks
        }
    }
    return hasDependants;
}

size_t SpendInTree::TrainEnhancement(
        const std::string& enhancementName,
        const std::string& selection,
        const std::vector<size_t>& cost,
        size_t minSpent,
        bool isTier5,
        size_t * ranks)
{
    size_t buyCost = 0;
    // increment the ranks of this enhancement if already present
    TrainedEnhancement * item = NULL;
    // iterate the list to see if its present
    std::list<TrainedEnhancement>::iterator it = m_Enhancements.begin();
    while (item == NULL && it != m_Enhancements.end())
    {
        if ((*it).EnhancementName() == enhancementName)
        {
            item = &(*it);
            break;
        }
        ++it;
    }
    if (item != NULL)
    {
        buyCost = item->Cost(item->Ranks());
        m_pointsSpent += buyCost;
        item->AddRank(isTier5);
    }
    else
    {
        TrainedEnhancement te;
        te.Set_EnhancementName(enhancementName);
        if (!selection.empty())
        {
            te.Set_Selection(selection);
        }
        te.AddRank(isTier5);            // create the first rank trained
        te.SetCost(cost);               // cached value
        te.SetRequiredAps(minSpent);    // cached value
        m_Enhancements.push_back(te);
        item = &m_Enhancements.back();
        buyCost = cost[0];              // always 1st rank cost
        m_pointsSpent += buyCost;
    }
    *ranks = item->Ranks();
    m_Enhancements.sort();
    return buyCost;
}

int SpendInTree::RevokeEnhancement(
        const std::string& revokedEnhancement,
        std::string * revokedEnhancementSelection,
        size_t * ranks)
{
    int ap = 0;     // return number of AP recovered

    // find the item in the list
    TrainedEnhancement * item = NULL;
    std::list<TrainedEnhancement>::iterator it = m_Enhancements.begin();
    while (item == NULL && it != m_Enhancements.end())
    {
        if ((*it).EnhancementName() == revokedEnhancement)
        {
            item = &(*it);
            break;
        }
        ++it;
    }
    // decrement the ranks of this enhancement if already present, remove if ranks == 0
    if (item != NULL)
    {
        if (item->HasSelection())
        {
            *revokedEnhancementSelection = item->Selection();
        }
        else
        {
            *revokedEnhancementSelection = "";
        }
        *ranks = item->Ranks();     // return the rank being revoked
        item->RevokeRank();
        ap = item->Cost(item->Ranks());
        m_pointsSpent -= ap;    // return AP for use
        if (item->Ranks() == 0)
        {
            // fully revokes, remove item from list
            m_Enhancements.erase(it);
        }
    }
    return ap;
}

bool SpendInTree::HasTier5() const
{
    // return true if any of the trained enhancements are tier 5
    bool hasTier5 = false;
    std::list<TrainedEnhancement>::const_iterator it = m_Enhancements.begin();
    while (it != m_Enhancements.end())
    {
        if ((*it).HasIsTier5())
        {
            hasTier5 = true;
            break;  // no need to check the rest
        }
        ++it;
    }
    return hasTier5;
}

size_t SpendInTree::Spent() const
{
    return m_pointsSpent;
}

TreeType SpendInTree::Type() const
{
    return m_type;
}

void SpendInTree::SetSpent(size_t apsSpent)
{
    m_pointsSpent = apsSpent;
}

void SpendInTree::TranslateNamesFromV1()
{
    static std::string nameTranslations[] =
    {
        // old tree name                        new tree name
        "Ravager",                              "Ravager (Barbarian)",
        "Ravager (Ftr)",                        "Ravager (Fighter)"
    };
    size_t count = sizeof(nameTranslations) / sizeof(std::string);
    if (count % 2 != 0)
    {
        throw "Must be an multiple of 2";
    }
    for (size_t i = 0; i < count; i += 2)
    {
        if (m_TreeName == nameTranslations[i])
        {
            m_TreeName = nameTranslations[i + 1];
            break;
        }
    }
}
