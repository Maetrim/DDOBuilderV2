// WeaponGroup.h
//
// A class that handles list of weapons that belong to a specific grouping
// grouping will be things like "Light Weapons", "Favored Weapons", "Focus Weapons"
#pragma once
#include "XmlLib\DLMacros.h"

#include "WeaponTypes.h"
#include <list>
#include "WeaponGroupRequirements.h"

class WeaponGroup :
    public XmlLib::SaxContentElement
{
    public:
        WeaponGroup();
        WeaponGroup(const std::string & name);
        void Write(XmlLib::SaxWriter * writer) const;

        bool HasWeapon(WeaponType wt, const Build& build) const;
        void AddWeapon(WeaponType wt);
        void RemoveWeapon(WeaponType wt);

        void AddWeapon(WeaponType wt, const Requirements& requirements);
        void RemoveWeapon(WeaponType wt, const Requirements& requirements);

        void AddMergeGroup(const std::string& groupName);
        void RemoveMergeGroup(const std::string& groupName);

        bool operator<(const WeaponGroup & other) const;

        void VerifyObject() const;
        void DumpToLog(const Build& build) const;
    protected:
        XmlLib::SaxContentElementInterface * StartElement(
                const XmlLib::SaxString & name,
                const XmlLib::SaxAttributes & attributes);

        virtual void EndElement();

        #define WeaponGroup_PROPERTIES(_) \
                DL_STRING(_, Name) \
                DL_ENUM_LIST(_, WeaponType, Weapon, Weapon_Unknown, weaponTypeMap)

        DL_DECLARE_ACCESS(WeaponGroup_PROPERTIES)
        DL_DECLARE_VARIABLES(WeaponGroup_PROPERTIES)
    private:
        bool HasWeapon(WeaponType wt) const;

        std::list<WeaponGroupRequirements> m_weaponsWithRequirements;
        std::list<std::string> m_mergedGroups;
};
