// Item.h
//
// An XML object wrapper that holds information on an item that can be equipped
#pragma once
#include "XmlLib\DLMacros.h"
#include "ArmorTypes.h"
#include "DamageReductionTypes.h"
#include "BaseDice.h"
#include "Buff.h"
#include "EquipmentSlot.h"
#include "InventorySlotTypes.h"
#include "ItemAugment.h"
#include "Requirements.h"
#include "SlotUpgrade.h"
//#include "Stance.h"
#include "WeaponTypes.h"

class Item :
    public XmlLib::SaxContentElement
{
    public:
        Item(void);
        Item(const XmlLib::SaxString & objectName);
        void Write(XmlLib::SaxWriter * writer) const;

        bool CanEquipToSlot(InventorySlotType slot) const;
        void VerifyObject() const;
        bool ContainsSearchText(const std::string& searchText, const Build* pBuild) const;

        void CopyUserSetValues(const Item & original);
        std::string ItemType() const;
        void SetIconIndex(size_t index);
        size_t IconIndex() const;
        void SetFilename(const std::string& filename);
        const std::string& Filename() const;
        std::vector<CString> BuffDescriptions(const Build* pBuild) const;
        void AddSetBonus(const std::string& setName);
        void AddItemAugment(const ItemAugment& ia);
        void AddRaceRequirement(const std::string& race);
        void AddClassRequirement(const std::string& feat);
        void AddFeatRequirement(const std::string& feat);
        void AddFeatAnySourceRequirement(const std::string& c);
        void AddBuff(const Buff& b);
        size_t RealCriticalThreatRange() const;
        bool FindEffect(EffectType et) const;
        int BuffValue(EffectType et) const;
        bool HasSetBonus(const std::string& setBonusName) const;

        void SetAugments(const std::vector<ItemAugment>& augments);
        bool IsRaidItem() const;
        void SetIsRaidItem(bool bIsRaidItem);
    protected:
        XmlLib::SaxContentElementInterface * StartElement(
                const XmlLib::SaxString & name,
                const XmlLib::SaxAttributes & attributes);

        virtual void EndElement();

        #define Item_PROPERTIES(_) \
                DL_FLAG(_, NoAutoUpdate) \
                DL_FLAG(_, UserSetsLevel) \
                DL_STRING(_, Name) \
                DL_OPTIONAL_STRING(_, Icon) \
                DL_STRING(_, Description) \
                DL_OPTIONAL_STRING(_, DropLocation) \
                DL_SIMPLE(_, size_t, MinLevel, 0) \
                DL_OBJECT(_, EquipmentSlot, Slots) \
                DL_OPTIONAL_OBJECT(_, EquipmentSlot, RestrictedSlots) \
                DL_OPTIONAL_OBJECT(_, Requirements, RequirementsToUse) \
                DL_OPTIONAL_ENUM(_, WeaponType, Weapon, Weapon_Unknown, weaponTypeMap) \
                DL_OPTIONAL_ENUM(_, ArmorType, Armor, Armor_Unknown, armorTypeMap) \
                DL_ENUM_LIST(_, AbilityType, AttackModifier, Ability_Unknown, abilityTypeMap) \
                DL_ENUM_LIST(_, AbilityType, DamageModifier, Ability_Unknown, abilityTypeMap) \
                DL_ENUM_LIST(_, DamageReductionType, DRBypass, DR_Unknown, drTypeMap) \
                DL_OPTIONAL_SIMPLE(_, double, WeaponDamage, 1) \
                DL_OPTIONAL_OBJECT(_, BaseDice, DamageDice) \
                DL_OPTIONAL_SIMPLE(_, size_t, CriticalMultiplier, 0) \
                DL_OPTIONAL_SIMPLE(_, size_t, CriticalThreatRange, 0) \
                DL_OPTIONAL_SIMPLE(_, int, ArmorBonus, 0) \
                DL_OPTIONAL_SIMPLE(_, int, MithralBody, 0) \
                DL_OPTIONAL_SIMPLE(_, int, AdamantineBody, 0) \
                DL_OPTIONAL_SIMPLE(_, size_t, MaximumDexterityBonus, 0) \
                DL_OPTIONAL_SIMPLE(_, int, ArmorCheckPenalty, 0) \
                DL_OPTIONAL_SIMPLE(_, int, ArcaneSpellFailure, 0) \
                DL_OPTIONAL_SIMPLE(_, size_t, ShieldBonus, 0) \
                DL_OPTIONAL_SIMPLE(_, size_t, DamageReduction, 0) \
                DL_OPTIONAL_STRING(_, Material) \
                DL_OBJECT_VECTOR(_, Effect, Effects) \
                DL_OBJECT_VECTOR(_, Buff, Buffs) \
                DL_STRING_LIST(_, SetBonus) \
                DL_OBJECT_VECTOR(_, ItemAugment, Augments) \
                DL_OBJECT_VECTOR(_, SlotUpgrade, SlotUpgrades) \
                DL_FLAG(_, IsAcceptsSentience) \
                DL_FLAG(_, IsGreensteel) \
                DL_FLAG(_, MinorArtifact)

        DL_DECLARE_ACCESS(Item_PROPERTIES)
        DL_DECLARE_VARIABLES(Item_PROPERTIES)

        size_t m_iconIndex;
        std::string m_filename;
        bool m_bIsRaindItem;

        friend class WikiItemFileProcessor;
        friend class CItemSelectDialog;
        friend class CFindGearDialog;
        friend class CItemImageDialog;
        friend class CWeaponImageDialog;
        friend class Build;
        friend class EquippedGear;
};
