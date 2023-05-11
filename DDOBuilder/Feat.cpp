// Feat.cpp
//
#include "StdAfx.h"
#include "Feat.h"
#include "XmlLib\SaxWriter.h"
#include "GlobalSupportFunctions.h"
#include "LogPane.h"

#define DL_ELEMENT Feat

namespace
{
    const wchar_t f_saxElementName[] = L"Feat";
    DL_DEFINE_NAMES(Feat_PROPERTIES)

    const unsigned f_verCurrent = 1;
}

CImageList Feat::sm_featImages;

Feat::Feat() :
    XmlLib::SaxContentElement(f_saxElementName, f_verCurrent),
    m_imageIndex(-1)
{
    DL_INIT(Feat_PROPERTIES)
}

Feat::Feat(const Feat& other) :
    XmlLib::SaxContentElement(f_saxElementName, f_verCurrent),
    m_imageIndex(-1)
{
    DL_INIT(Feat_PROPERTIES)
    m_Name = other.m_Name;
    m_hasName = other.m_hasName;
    m_Description = other.m_Description;
    m_hasDescription = other.m_hasDescription;
    m_Icon = other.m_Icon;
    m_hasIcon = other.m_hasIcon;
    m_Group = other.m_Group;
    m_Sphere = other.m_Sphere;
    m_hasSphere = other.m_hasSphere;
    m_RequirementsToTrain = other.m_RequirementsToTrain;
    m_hasRequirementsToTrain = other.m_hasRequirementsToTrain;
    m_AutomaticAssignment = other.m_AutomaticAssignment;
    m_Acquire = other.m_Acquire;
    m_MaxTimesAcquire = other.m_MaxTimesAcquire;
    m_hasMaxTimesAcquire = other.m_hasMaxTimesAcquire;
    m_Stances = other.m_Stances;
    m_DCs = other.m_DCs;
    m_Effects = other.m_Effects;
    m_imageIndex = other.m_imageIndex;
}

Feat::Feat(
        const std::string& name,
        const std::string& description,
        const std::string& icon) :
    XmlLib::SaxContentElement(f_saxElementName, f_verCurrent),
    m_Name(name),
    m_hasName(true),
    m_Description(description),
    m_hasDescription(true),
    m_Icon(icon),
    m_hasIcon(true),
    m_imageIndex(-1)
{
    DL_INIT(Feat_PROPERTIES)
    m_hasMaxTimesAcquire = true;
}

DL_DEFINE_ACCESS(Feat_PROPERTIES)

XmlLib::SaxContentElementInterface * Feat::StartElement(
        const XmlLib::SaxString & name,
        const XmlLib::SaxAttributes & attributes)
{
    XmlLib::SaxContentElementInterface * subHandler =
            SaxContentElement::StartElement(name, attributes);

    DL_START(Feat_PROPERTIES)

    return subHandler;
}

void Feat::EndElement()
{
    // now check out our object
    SaxContentElement::EndElement();
    DL_END(Feat_PROPERTIES)
    // the default value is 1 if not present is ok
    m_hasMaxTimesAcquire = true;
    // all Efefcts must have a DisplayName field
    // if it does not have one, it gets set to the name of this Feat
    for (auto&& eit : m_Effects)
    {
        if (!eit.HasDisplayName())
        {
            eit.SetDisplayName(Name());
        }
    }
}

void Feat::Write(XmlLib::SaxWriter * writer) const
{
    writer->StartElement(ElementName());//, VersionAttributes());
    DL_WRITE(Feat_PROPERTIES)
    writer->EndElement();
}

bool Feat::operator<(const Feat & other) const
{
    // (assumes all feat names are unique)
    // sort by name
    return (Name() < other.Name());
}

void Feat::AddImage(CImageList * pIL) const
{
    CImage image;
    HRESULT result = LoadImageFile(
            "DataFiles\\FeatImages\\",
            Icon(),
            &image,
            CSize(32, 32));
    if (result == S_OK)
    {
        CBitmap bitmap;
        bitmap.Attach(image.Detach());
        m_imageIndex = pIL->Add(&bitmap, c_transparentColour);  // standard mask color (purple)
    }
    else
    {
        // load unknown image imstead
        result = LoadImageFile(
                "DataFiles\\FeatImages\\",
                "Unknown",
                &image,
                CSize(32, 32));
        CBitmap bitmap;
        bitmap.Attach(image.Detach());
        m_imageIndex = pIL->Add(&bitmap, c_transparentColour);  // standard mask color (purple)
    }
}

void Feat::CreateRequirementStrings(
    const Build& build,
    std::vector<CString>* requirements,
    std::vector<bool>* met,
    size_t level) const
{
    if (m_hasRequirementsToTrain)
    {
        m_RequirementsToTrain.CreateRequirementStrings(
                build,
                level,
                true,
                requirements,
                met);
    }
}

void Feat::VerifyObject() const
{
    bool ok = true;
    std::stringstream ss;
    ss << "=====" << m_Name << "=====\n";
    // check this feat out for any inconsistencies
    if (Acquire() == FeatAcquisition_Train
            && m_Group.size() == 0)
    {
        ss << "Feat is missing Group options\n";
        ok = false;
    }
    if (!ImageFileExists("DataFiles\\FeatImages\\", Icon()))
    {
        ss << "Feat is missing icon file " << Icon() << "\n";
        ok = false;
    }
    if (HasRequirementsToTrain())
    {
        ok &= m_RequirementsToTrain.VerifyObject(&ss);
    }
    for (auto&& aait: m_AutomaticAssignment)
    {
        ok &= aait.VerifyObject(&ss);
    }
    for (auto&& sit : m_Stances)
    {
        ok &= sit.VerifyObject(&ss);
    }
    for (auto&& dit : m_DCs)
    {
        ok &= dit.VerifyObject(&ss);
    }
    auto eit = m_Effects.begin();
    while (eit != m_Effects.end())
    {
        ok &= (*eit).VerifyObject(&ss);
        ++eit;
    }
    if (!ok)
    {
        GetLog().AddLogEntry(ss.str().c_str());
    }
}

void Feat::CreateFeatImageList()
{
    // create the image list for the feats
    const std::map<std::string, Feat>& allFeats = StandardFeats();
    sm_featImages.Create(
            32,
            32,
            ILC_COLOR32,
            0,
            allFeats.size());
    for (auto&& it : allFeats)
    {
        it.second.AddImage(&sm_featImages);
    }
}

CImageList & Feat::FeatImageList()
{
    return sm_featImages;
}

int Feat::FeatImageIndex() const
{
    return m_imageIndex;
}

void Feat::AddGroup(const std::string& group)
{
    m_Group.push_back(group);
}

void Feat::SetName(const std::string& newName)
{
    Set_Name(newName);
    // also updat eth names in all effects
    for (auto&& eit: m_Effects)
    {
        eit.SetDisplayName(newName);
    }
}

void Feat::AddAutomaticAcquisition(const Requirement& requirement)
{
    AutomaticAcquisition aa;
    aa.SetRequirement(requirement);
    m_AutomaticAssignment.push_back(aa);
}

void Feat::SetRequirements(const Requirements& requirements)
{
    m_RequirementsToTrain = requirements;
    m_hasRequirementsToTrain = true;
}
