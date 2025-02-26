// Feat.h
//
// An XML object wrapper that holds information on an affect that a feat has.
#pragma once
#include "XmlLib\DLMacros.h"

#include "Attack.h"
#include "AutomaticAcquisition.h"
#include "ConditionalGroup.h"
#include "DC.h"
#include "Effect.h"
#include "FeatAcquisitionTypes.h"
#include "Requirements.h"
#include "SubItem.h"
#include "Stance.h"

class Build;

class Feat :
    public XmlLib::SaxContentElement
{
    public:
        Feat(void);
        Feat(const Feat& other);
        Feat(const std::string& name, const std::string& description, const std::string& icon);

        void Write(XmlLib::SaxWriter * writer) const;
        bool operator<(const Feat & other) const;
        void AddImage(CImageList * pIL) const;
        void CreateRequirementStrings(
                const Build& build,
                std::vector<CString>* requirements,
                std::vector<bool>* met,
                size_t level) const;

        void VerifyObject() const;
        static void CreateFeatImageList();
        static CImageList & FeatImageList();
        int FeatImageIndex() const;

        void AddGroup(const std::string& group);

        // special support functions for dynamically created feats
        void SetName(const std::string& newName);
        void AddAutomaticAcquisition(const Requirement& requirement);
        void SetRequirements(const Requirements& requirements);
    protected:
        XmlLib::SaxContentElementInterface * StartElement(
                const XmlLib::SaxString & name,
                const XmlLib::SaxAttributes & attributes);

        virtual void EndElement();

        #define Feat_PROPERTIES(_) \
                DL_STRING(_, Name) \
                DL_STRING(_, Description) \
                DL_OBJECT_LIST(_, SubItem, SubItems) \
                DL_STRING(_, Icon) \
                DL_STRING_LIST(_, Group) \
                DL_OPTIONAL_OBJECT(_, ConditionalGroup, ConditionalGroups) \
                DL_OPTIONAL_STRING(_, Sphere) \
                DL_OPTIONAL_OBJECT(_, Requirements, RequirementsToTrain) \
                DL_OBJECT_LIST(_, AutomaticAcquisition, AutomaticAssignment) \
                DL_ENUM(_, FeatAcquisitionType, Acquire, FeatAcquisition_Unknown, featAcquisitionMap) \
                DL_OPTIONAL_SIMPLE(_, size_t, MaxTimesAcquire, 1) \
                DL_OBJECT_LIST(_, Attack, Attacks) \
                DL_OBJECT_LIST(_, Stance, Stances) \
                DL_OBJECT_LIST(_, DC, DCs) \
                DL_OBJECT_LIST(_, Effect, Effects)
        //***DONT FORGET TO UPDATE THE COPY CONSTRUCTOR***

        DL_DECLARE_ACCESS(Feat_PROPERTIES)
        DL_DECLARE_VARIABLES(Feat_PROPERTIES)

        mutable int m_imageIndex;
        static CImageList sm_featImages;
};
