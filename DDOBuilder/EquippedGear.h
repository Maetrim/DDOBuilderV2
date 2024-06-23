// EquippedGear.h
//
// An XML object wrapper that holds information on all item equipped
#pragma once
#include "XmlLib\DLMacros.h"
#include "Item.h"
#include "InventorySlotTypes.h"
#include "WeaponFiligree.h"
#include "ArtifactFiligree.h"

class Build;

class EquippedGear :
    public XmlLib::SaxContentElement
{
    public:
        EquippedGear(const std::string& name = "");
        void Write(XmlLib::SaxWriter * writer) const;

        void SetName(const std::string& name);
        void UpdateImages();

        bool HasItemInSlot(InventorySlotType slot) const;
        Item ItemInSlot(InventorySlotType slot) const;
        bool IsSlotRestricted(InventorySlotType slot, Build* pBuild) const;
        std::list<Item> SetItem(InventorySlotType slot,
                Build* pBuild,
                const Item & item); // returns list of revoked items (if any)
        void ClearItem(InventorySlotType slot);

        bool HasMinorArtifact() const;

        void SetNumFiligrees(size_t count);

        void SetPersonality(const std::string& name);
        void ClearPersonality();

        std::string GetFiligree(size_t fi) const;
        bool IsRareFiligree(size_t fi) const;

        void SetFiligree(size_t fi, const std::string& name);
        void SetFiligreeRare(size_t fi, bool isRare);

        std::string GetArtifactFiligree(size_t fi) const;
        bool IsRareArtifactFiligree(size_t fi) const;

        void SetArtifactFiligree(size_t fi, const std::string& name);
        void SetArtifactFiligreeRare(size_t fi, bool isRare);

        bool ImportFromFile(const CString& filename);
        bool ImportFromClipboard();

        WeaponType Weapon1() const;
        WeaponType Weapon2() const;

    protected:
        XmlLib::SaxContentElementInterface * StartElement(
                const XmlLib::SaxString & name,
                const XmlLib::SaxAttributes & attributes);

        virtual void EndElement();

        #define EquippedGear_PROPERTIES(_) \
                DL_STRING (_, Name) \
                DL_OPTIONAL_OBJECT(_, Item, Helmet) \
                DL_OPTIONAL_OBJECT(_, Item, Necklace) \
                DL_OPTIONAL_OBJECT(_, Item, Trinket) \
                DL_OPTIONAL_OBJECT(_, Item, Cloak) \
                DL_OPTIONAL_OBJECT(_, Item, Belt) \
                DL_OPTIONAL_OBJECT(_, Item, Goggles) \
                DL_OPTIONAL_OBJECT(_, Item, Gloves) \
                DL_OPTIONAL_OBJECT(_, Item, Boots) \
                DL_OPTIONAL_OBJECT(_, Item, Bracers) \
                DL_OPTIONAL_OBJECT(_, Item, Armor) \
                DL_OPTIONAL_OBJECT(_, Item, MainHand) \
                DL_OPTIONAL_OBJECT(_, Item, OffHand) \
                DL_OPTIONAL_OBJECT(_, Item, Quiver) \
                DL_OPTIONAL_OBJECT(_, Item, Arrow) \
                DL_OPTIONAL_OBJECT(_, Item, Ring1) \
                DL_OPTIONAL_OBJECT(_, Item, Ring2) \
                DL_OPTIONAL_OBJECT(_, Item, CosmeticArmor) \
                DL_OPTIONAL_OBJECT(_, Item, CosmeticCloak) \
                DL_OPTIONAL_OBJECT(_, Item, CosmeticHelm) \
                DL_OPTIONAL_OBJECT(_, Item, CosmeticWeapon1) \
                DL_OPTIONAL_OBJECT(_, Item, CosmeticWeapon2) \
                DL_OPTIONAL_STRING(_, Personality) \
                DL_SIMPLE(_, size_t, NumFiligrees, 0) \
                DL_OBJECT_LIST(_, WeaponFiligree, Filigrees) \
                DL_OBJECT_LIST(_, ArtifactFiligree, ArtifactFiligrees)

        DL_DECLARE_ACCESS(EquippedGear_PROPERTIES)
        DL_DECLARE_VARIABLES(EquippedGear_PROPERTIES)

        size_t ExtractLine(CString* line, size_t pif, char* buffer, size_t bufferSize);
        bool ProcessFileLine(CString line);
        bool ProcessClipboardLine(CString line, size_t pif, char* buffer, size_t uSize);
        void ApplyFileItemAugment(Item* pItem, CString augmentText);
        void ApplyClipboardItemAugment(Item* pItem, CString augmentText);
        void UpdateItem(InventorySlotType slot, const Item& item);

        friend class CEquipmentPane;
};
