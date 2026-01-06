// EnhancementTreeItem.cpp
//
#include "StdAfx.h"
#include "EnhancementTreeItem.h"
#include "XmlLib\SaxWriter.h"
#include "Build.h"
#include "GlobalSupportFunctions.h"
#include "TrainedEnhancement.h"
#include "EnhancementTree.h"

#define DL_ELEMENT EnhancementTreeItem

namespace
{
    const wchar_t f_saxElementName[] = L"EnhancementTreeItem";
    DL_DEFINE_NAMES(EnhancementTreeItem_PROPERTIES)

    const unsigned f_verCurrent = 1;
}

CImageList EnhancementTreeItem::sm_enhancementImages;
CImageList EnhancementTreeItem::sm_disabledEnhancementImages;

EnhancementTreeItem::EnhancementTreeItem() :
    XmlLib::SaxContentElement(f_saxElementName, f_verCurrent),
    m_imageIndex(-1)
{
    DL_INIT(EnhancementTreeItem_PROPERTIES)
}

DL_DEFINE_ACCESS(EnhancementTreeItem_PROPERTIES)

XmlLib::SaxContentElementInterface * EnhancementTreeItem::StartElement(
        const XmlLib::SaxString & name,
        const XmlLib::SaxAttributes & attributes)
{
    XmlLib::SaxContentElementInterface * subHandler =
            SaxContentElement::StartElement(name, attributes);

    DL_START(EnhancementTreeItem_PROPERTIES)
    return subHandler;
}

void EnhancementTreeItem::EndElement()
{
    SaxContentElement::EndElement();
    DL_END(EnhancementTreeItem_PROPERTIES)
}

void EnhancementTreeItem::Write(XmlLib::SaxWriter * writer) const
{
    writer->StartElement(ElementName());//, VersionAttributes());
    DL_WRITE(EnhancementTreeItem_PROPERTIES)
    writer->EndElement();
}

void EnhancementTreeItem::CreateRequirementStrings(
        const Build & build,
        std::vector<CString> * requirements,
        std::vector<bool> * met,
        size_t level) const
{
    if (HasRequirementsToTrain())
    {
        m_RequirementsToTrain.CreateRequirementStrings(build, level, false, requirements, met);
    }
}

bool EnhancementTreeItem::VerifyObject(
        std::stringstream * ss,
        const std::list<EnhancementTree> & trees) const
{
    bool ok = true;
    std::stringstream lss;
    if (!ImageFileExists("DataFiles\\EnhancementImages\\", Icon()))
    {
        lss << Name() << " is missing icon file " << Icon() << "\n";
        ok = false;
    }
    if (HasRequirementsToTrain())
    {
        ok &= m_RequirementsToTrain.VerifyObject(&lss);
    }
    if (HasSelections())
    {
        // check each of the selections also
        ok &= m_Selections.VerifyObject(&lss, trees);
    }
    // check the effects also
    for (auto&& it: m_Effects)
    {
        ok &= it.VerifyObject(&lss);
    }
    // verify its DC objects
    for (auto&& it: m_EffectDC)
    {
        ok &= it.VerifyObject(ss);
    }
    // verify its Stance objects
    for (auto&& it: m_Stances)
    {
        ok &= it.VerifyObject(ss);
    }
    if (!ok)
    {
        (*ss) << "---" << m_Name << "\n";
        (*ss) << lss.str();
    }
    return ok;
}

bool EnhancementTreeItem::MeetRequirements(
        const Build & build,
        const std::string& selection,
        const std::string& treeName,
        size_t level,
        size_t spentInTree) const
{
    bool met = true;
    // must also meet the requirements of the item
    if (met)
    {
        if (HasRequirementsToTrain())
        {
            met = m_RequirementsToTrain.Met(
                build,
                level,
                false,
                Inventory_Unknown,
                Weapon_Unknown,
                Weapon_Unknown);
        }
    }
    // cannot train this enhancement if its tier5 and not from the same tier 5
    // tree if one has already been trained
    if (met)
    {
        met = !IsTier5Blocked(build, treeName);
        if (met && selection != "")
        {
            // check if we can train this selection
            std::list<EnhancementSelection> selections = m_Selections.Selections();
            std::list<EnhancementSelection>::const_iterator it = selections.begin();
            while (it != selections.end())
            {
                if ((*it).Name() == selection)
                {
                    // this is the one we need to check
                    if ((*it).HasMinSpent())
                    {
                        met &= ((*it).MinSpent() <= spentInTree);
                    }
                    if (HasRequirementsToTrain())
                    {
                        if ((*it).HasRequirementsToTrain())
                        {
                            met &= (*it).RequirementsToTrain().Met(
                                    build,
                                    level,
                                    false,
                                    Inventory_Unknown,
                                    Weapon_Unknown,
                                    Weapon_Unknown);
                        }
                    }
                    // and were done
                    break;
                }
                ++it;
            }
        }
    }
    return met;
}

bool EnhancementTreeItem::IsAllowed(
        const Build & build,
        const std::string& selection,
        size_t level,
        size_t spentInTree) const
{
    // an item is not allowed if it has:
    // 1: A feat requirement that is not met
    bool bIsAllowed = true;
    if (HasRequirementsToTrain())
    {
        bIsAllowed = m_RequirementsToTrain.MetHardRequirements(
                build,
                level,
                false);
    }
    if (bIsAllowed && selection != "")
    {
        // check if we can train this selection
        std::list<EnhancementSelection> selections = m_Selections.Selections();
        std::list<EnhancementSelection>::const_iterator it = selections.begin();
        while (it != selections.end())
        {
            if ((*it).Name() == selection)
            {
                // this is the one we need to check
                if ((*it).HasMinSpent())
                {
                    bIsAllowed &= ((*it).MinSpent() <= spentInTree);
                }
                if ((*it).HasRequirementsToTrain())
                {
                    bIsAllowed &= (*it).RequirementsToTrain().MetHardRequirements(
                            build,
                            level,
                            false);
                }
                // and were done
                break;
            }
            ++it;
        }
    }
    return bIsAllowed;
}

bool EnhancementTreeItem::IsTier5Blocked(
    const Build& build,
    const std::string& treeName) const
{
    bool bTier5Blocked = false;
    if (HasTier5())                  // are we a tier 5 enhancement?
    {
        const EnhancementTree& tree = GetEnhancementTree(treeName);
        if (tree.HasIsEpicDestiny())
        {
            if (build.DestinySelectedTrees().HasTier5Tree()
                && treeName != build.DestinySelectedTrees().Tier5Tree())
            {
                // not allowed this tier 5 enhancement
                bTier5Blocked = true;
            }
        }
        else if (!tree.HasIsRacialTree() && !tree.HasIsReaperTree())
        {
            // must be a universal or an enhancement tree
            if (build.EnhancementSelectedTrees().HasTier5Tree()
                && treeName != build.EnhancementSelectedTrees().Tier5Tree())
            {
                // not allowed this tier 5 enhancement
                bTier5Blocked = true;
            }
        }
    }
    return bTier5Blocked;
}

bool EnhancementTreeItem::CanTrain(
        const Build & build,
        size_t spentInTree,
        TreeType type,
        size_t level) const
{
    // must not be trained to max ranks
    const TrainedEnhancement * te = build.IsTrained(InternalName(), "");
    size_t trainedRanks = (te != NULL) ? te->Ranks() : 0;
    std::string selection = (te != NULL && te->HasSelection()) ? te->Selection() : "";
    bool canTrain = (trainedRanks < Ranks(selection));
    canTrain &= (spentInTree >= MinSpent(selection));
    // if we have no selection, and we have a selector, only enabled if at least
    // one of the sub selector items can be trained
    if (selection == ""
        && HasSelections())
    {
        canTrain &= Selections().HasTrainableOption(build, spentInTree);
    }
    // verify any enhancements we are dependent on have enough trained ranks also
    if (HasRequirementsToTrain())
    {
        canTrain &= m_RequirementsToTrain.Met(build,
                level,
                false,
                Inventory_Unknown,
                Weapon_Unknown,
                Weapon_Unknown);
    }
    // must have enough action points to buy it
    int availableAP = build.AvailableActionPoints(level+1, type);
    canTrain &= (availableAP >= (int)Cost(selection, trainedRanks));
    return canTrain;
}

void EnhancementTreeItem::RenderIcon(
        const Build & build,
        CDC * pDC,
        const CRect & itemRect) const
{
    std::string icon = "NoImage";
    // assume its the items root icon
    icon = Icon();
    // check to see if the enhancement is already trained
    const TrainedEnhancement * te = build.IsTrained(InternalName(), "");
    if (m_imageIndex < 0)
    {
        // need to load and add to the image lists
        m_imageIndex = AddImage(Icon());
    }

    if (te != NULL)
    {
        if (te->HasSelection())
        {
            std::string sel = te->Selection();
            m_Selections.RenderIcon(sel, pDC, itemRect);
        }
        else
        {
            sm_enhancementImages.Draw(
                    pDC,
                    m_imageIndex,
                    itemRect.TopLeft(),
                    ILD_NORMAL | ILD_TRANSPARENT);
        }
    }
    else
    {
        sm_disabledEnhancementImages.Draw(
                pDC,
                m_imageIndex,
                itemRect.TopLeft(),
                ILD_NORMAL | ILD_TRANSPARENT);
    }
}

std::string EnhancementTreeItem::SelectionIcon(
    const std::string& selection) const
{
    std::string iconName;
    iconName = m_Selections.SelectedIcon(selection);
    return iconName;
}

bool EnhancementTreeItem::CanRevoke(
        const SpendInTree* pTreeSpend) const
{
    bool canRevoke = false;
    // This enhancement cannot be revoked if:
    if (pTreeSpend != NULL)
    {
        // 1. No ranks have been spent in this enhancement
        std::string selection;
        size_t ranks = pTreeSpend->TrainedRanks(InternalName(), &selection);
        if (ranks != 0)
        {
            // 2. Revoking a rank of this enhancement would reduce the APs spent in the tree
            // below those required for a higher tier enhancement to be trained

            // sum how many Action points have been spent in this min AP tier
            canRevoke = pTreeSpend->CanRevokeAtTier(MinSpent(selection), Cost(selection, ranks));

            if (canRevoke)
            {
                // 3. Another enhancement that is dependent on this enhancement being trained has
                // equal or more ranks than this enhancement
                canRevoke = !pTreeSpend->HasTrainedDependants(InternalName(), ranks);
            }
        }
    }
    return canRevoke;
}

std::string EnhancementTreeItem::DisplayName(
        const std::string& selection) const
{
    std::string name;
    name = Name();
    if (HasSelections())
    {
        // we need to look up the effects for a selection
        name += ": ";
        name += m_Selections.DisplayName(selection);
    }
    return name;
}

std::string EnhancementTreeItem::ActiveIcon(
        const Build & build) const
{
    std::string icon = "NoImage";
    // assume its the items root icon
    icon = Icon();
    // check to see if the enhancement is already trained
    const TrainedEnhancement * te = build.IsTrained(InternalName(), "");
    if (te != NULL)
    {
        if (te->HasSelection())
        {
            // get the selector icon instead
            std::string sel = te->Selection();
            icon = m_Selections.SelectedIcon(sel);
        }
    }
    return icon;
}

std::list<DC> EnhancementTreeItem::GetDCs(const std::string& selection) const
{
    // an enhancement may have specific sub-selection DCs
    std::list<DC> dcs;
    if (HasSelections())
    {
        // we need to look up the DCs for a selection
        dcs = m_Selections.EffectDCs(selection);
    }
    // even if it had a sub-selection it may still have DCs that
    // always apply regardless of the sub-selection
    dcs.insert(dcs.end(), m_EffectDC.begin(), m_EffectDC.end());
    return dcs;
}

std::list<Stance> EnhancementTreeItem::Stances(const std::string& selection) const
{
    // an enhancement may have specific sub-selection stances
    std::list<Stance> stances;
    if (HasSelections())
    {
        // we need to look up the effects for a selection
        stances = m_Selections.Stances(selection);
    }
    // even if it had a sub-selection it may still have stances that
    // always apply regardless of the sub-selection
    stances.insert(stances.end(), m_Stances.begin(), m_Stances.end());
    return stances;
}

bool EnhancementTreeItem::CostVaries(const std::string& selection) const
{
    bool varies = false;
    if (selection != ""
            && HasSelections())
    {
        varies = m_Selections.CostVaries(selection);
    }
    else
    {
        size_t cost = m_CostPerRank[0];
        for (size_t i = 1; i < m_CostPerRank.size(); ++i)
        {
            varies |= (m_CostPerRank[i] != cost);
        }
    }
    return varies;
}

size_t EnhancementTreeItem::Cost(
        const std::string& selection,
        size_t rank) const
{
    size_t cost = 0;
    if (rank < m_CostPerRank.size())
    {
        cost = m_CostPerRank[rank];
    }
    else
    {
        cost = m_CostPerRank[0];
    }
    if (selection != ""
            && HasSelections())
    {
        cost = m_Selections.Cost(selection, rank);
    }
    return cost;
}

const std::vector<size_t>& EnhancementTreeItem::ItemCosts(
        const std::string& selection) const
{
    if (selection != ""
            && HasSelections())
    {
        return m_Selections.ItemCosts(selection);
    }
    else
    {
        return m_CostPerRank;
    }
}


bool EnhancementTreeItem::IsSelectionClickie(const std::string& selection) const
{
    return m_Selections.IsSelectionClickie(selection);
}

std::list<Effect> EnhancementTreeItem::GetEffects(
        const std::string& selection,
        size_t rank) const
{
    // an enhancement may have specific sub-selection effects
    std::list<Effect> effects;
    if (HasSelections())
    {
        // we need to look up the effects for a selection
        effects = m_Selections.GetEffects(selection, rank);
        for (auto&& eit : effects)
        {
            if (!eit.HasDisplayName())
            {
                // make sure we know the source of this Effect
                eit.SetDisplayName(Name(selection));
            }
        }
    }
    // even if it had a sub-selection it may still have effects that
    // always apply regardless of the sub-selection
    for (auto&& eit : m_Effects)
    {
        if (!eit.HasRank()
                || (eit.HasRank() && eit.Rank() == rank))
        {
            // need to include this Effect
            Effect e = eit;
            if (!e.HasDisplayName())
            {
                // make sure we know the source of this Effect
                e.SetDisplayName(Name(selection));
            }
            effects.push_back(e);
        }
    }
    return effects;
}

std::list<Stance> EnhancementTreeItem::GetStances(
        const std::string& selection,
        size_t rank) const
{
    // an enhancement may have specific sub-selection effects
    std::list<Stance> stances;
    if (HasSelections())
    {
        // we need to look up the stances for a selection
        stances = m_Selections.GetStances(selection, rank);
    }
    // even if it had a sub-selection it may still have stances that
    // always apply regardless of the sub-selection
    for (auto&& sit : m_Stances)
    {
        stances.push_back(sit);
    }
    return stances;
}

bool EnhancementTreeItem::HasRequirementsToTrain(const std::string& selection) const
{
    bool bHas = false;
    for (auto&& it : m_Selections.Selections())
    {
        if (it.Name() == selection)
        {
            bHas = it.HasRequirementsToTrain();
        }
    }
    return bHas;
}

bool EnhancementTreeItem::RequiresEnhancement(const std::string& name) const
{
    bool bRequiresIt = false;
    if (HasRequirementsToTrain())
    {
        bRequiresIt = m_RequirementsToTrain.RequiresEnhancement(name);
    }
    return bRequiresIt;
}

bool EnhancementTreeItem::RequiresEnhancement(
    const std::string& ourSelection,
    const std::string& name) const
{
    bool bRequiresIt = false;
    for (auto&& it: m_Selections.Selections())
    {
        if (it.Name() == ourSelection)
        {
            if (it.HasRequirementsToTrain())
            {
                bRequiresIt = it.RequirementsToTrain().RequiresEnhancement(name);
            }
        }
    }
    return bRequiresIt;
}

size_t EnhancementTreeItem::MinSpent(const std::string& selection) const
{
    size_t min = MinSpent();    // assume default
    if (selection != "")
    {
        // find the selection and use its MinSepnt if it has one
        min = m_Selections.MinSpent(selection, min);    // returns min if no MinSpent on item or not found
    }
    return min;
}

size_t EnhancementTreeItem::Ranks(const std::string& selection) const
{
    size_t ranks = Ranks();
    if (selection != "")
    {
        // find the selection and use its MinSepnt if it has one
        ranks = m_Selections.Ranks(selection, ranks);
    }
    return ranks;
}

std::list<Attack> EnhancementTreeItem::Attacks(const std::string& selection) const
{
    std::list<Attack> allAttacks = Attacks();
    if (HasSelections())
    {
        // we need to look up the effects for a selection
        std::list<Attack> selAttacks = m_Selections.Attacks(selection);
        allAttacks.insert(allAttacks.end(), selAttacks.begin(), selAttacks.end());
    }
    return allAttacks;
}

std::string EnhancementTreeItem::Name(const std::string& selection) const
{
    std::stringstream ss;
    ss << Name();
    if (selection != "")
    {
        ss << ": ";
        ss << selection;
    }
    return ss.str();
}

int EnhancementTreeItem::AddImage(const std::string& icon)
{
    static bool firstTime = true;
    if (firstTime)
    {
        sm_enhancementImages.Create(
                32,
                32,
                ILC_COLOR32 | ILC_MASK,
                0,
                6 * 5 * 100);       // standard tree is 6 * 5, 100 possible trees
        sm_disabledEnhancementImages.Create(
                32,
                32,
                ILC_COLOR32 | ILC_MASK,
                0,
                6 * 5 * 100);       // standard tree is 6 * 5, 100 possible trees
        firstTime = false;
    }
    CImage image;
    // load the display image for this item
    LoadImageFile(
            "DataFiles\\EnhancementImages\\",
            icon,
            &image,
            CSize(32, 32));
    CBitmap bitmap;
    bitmap.Attach(image.Detach());
    int imageIndex1 = sm_enhancementImages.Add(&bitmap, c_transparentColour);  // standard mask color (purple)
    // now do the disabled image
    LoadImageFile(
            "DataFiles\\EnhancementImages\\",
            icon,
            &image,
            CSize(32, 32));
    MakeGrayScale(&image, c_transparentColour);
    CBitmap disabledBitmap;
    disabledBitmap.Attach(image.Detach());
    sm_disabledEnhancementImages.Add(&disabledBitmap, c_transparentColour);  // standard mask color (purple)
    return imageIndex1;
}

void EnhancementTreeItem::UpdateLegacyInfo(const std::string& prepend, EnhancementTree* pTree)
{
    if (HasSelections())
    {
        m_Selections.UpdateLegacyInfo(prepend, pTree);
    }
}

