// EnhancementSelection.cpp
//
#include "StdAfx.h"
#include "EnhancementSelection.h"
#include "XmlLib\SaxWriter.h"
#include "GlobalSupportFunctions.h"

#define DL_ELEMENT EnhancementSelection

namespace
{
    const wchar_t f_saxElementName[] = L"EnhancementSelection";
    DL_DEFINE_NAMES(EnhancementSelection_PROPERTIES)

    const unsigned f_verCurrent = 1;
}

CImageList EnhancementSelection::sm_enhancementImages;

EnhancementSelection::EnhancementSelection() :
    XmlLib::SaxContentElement(f_saxElementName, f_verCurrent),
    m_imageIndex(-1)
{
    DL_INIT(EnhancementSelection_PROPERTIES)
}

DL_DEFINE_ACCESS(EnhancementSelection_PROPERTIES)

XmlLib::SaxContentElementInterface * EnhancementSelection::StartElement(
        const XmlLib::SaxString & name,
        const XmlLib::SaxAttributes & attributes)
{
    XmlLib::SaxContentElementInterface * subHandler =
            SaxContentElement::StartElement(name, attributes);

    DL_START(EnhancementSelection_PROPERTIES)

    return subHandler;
}

void EnhancementSelection::EndElement()
{
    SaxContentElement::EndElement();
    DL_END(EnhancementSelection_PROPERTIES)
}

void EnhancementSelection::Write(XmlLib::SaxWriter * writer) const
{
    writer->StartElement(ElementName());//, VersionAttributes());
    DL_WRITE(EnhancementSelection_PROPERTIES)
    writer->EndElement();
}

void EnhancementSelection::CreateRequirementStrings(
        const Build & build,
        std::vector<CString> * requirements,
        std::vector<bool> * met,
        size_t level) const
{
    m_RequirementsToTrain.CreateRequirementStrings(build, level, false, requirements, met);
}

bool EnhancementSelection::VerifyObject(
        std::stringstream * ss) const
{
    bool ok = true;
    if (!ImageFileExists("DataFiles\\EnhancementImages\\", Icon()))
    {
        (*ss) << "EnhancementSelection is missing image file \"" << Icon() << "\"\n";
        ok = false;
    }
    // check the effects also
    for (auto&& it: m_Effects)
    {
        ok &= it.VerifyObject(ss);
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
    return ok;
}

void EnhancementSelection::RenderIcon(CDC * pDC, const CRect & itemRect) const
{
    if (m_imageIndex < 0)
    {
        // need to load and add to the image lists
        m_imageIndex = AddImage(Icon());
    }
    sm_enhancementImages.Draw(
            pDC,
            m_imageIndex,
            itemRect.TopLeft(),
            ILD_NORMAL | ILD_TRANSPARENT);
}

bool EnhancementSelection::CostVaries() const
{
    size_t cost = m_CostPerRank[0];
    bool varies = false;
    for (size_t i = 1; i < m_CostPerRank.size(); ++i)
    {
        varies |= (m_CostPerRank[i] != cost);
    }
    return varies;
}

size_t EnhancementSelection::Cost(size_t rank) const
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
    return cost;
}

std::list<Effect> EnhancementSelection::GetEffects(size_t rank) const
{
    std::list<Effect> effects;
    for (auto&& eit : m_Effects)
    {
        if (!eit.HasRank()
            || (eit.HasRank() && eit.Rank() == rank))
        {
            // need to include this Effect
            effects.push_back(eit);
        }
    }
    // if this item has an exclusion group in its requirements, then make an effect
    // to handle it so its tracked correctly
    if (HasRequirementsToTrain())
    {
        std::string enhancementId;
        std::string group;
        m_RequirementsToTrain.GetExclusionGroup(&enhancementId, &group);
        if (group != "")
        {
            // it does have one, create the special effect to handle it
            Effect eg(
                    Effect_ExclusionGroup,
                    enhancementId,
                    group);
            effects.push_back(eg);
        }
    }
    return effects;
}

std::list<Stance> EnhancementSelection::GetStances(size_t rank) const
{
    UNREFERENCED_PARAMETER(rank);
    std::list<Stance> stances;
    for (auto&& sit : m_Stances)
    {
        stances.push_back(sit);
    }
    return stances;
}

int EnhancementSelection::AddImage(const std::string& icon)
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
    return imageIndex1;
}

void EnhancementSelection::UpdateLegacyInfo(const std::string& prepend)
{
    if (m_hasRequirementsToTrain)
    {
        m_RequirementsToTrain.UpdateEnhancementRequirements(prepend);
    }
}

