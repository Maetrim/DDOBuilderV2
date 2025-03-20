// Item.cpp
//
#include "StdAfx.h"
#include "Item.h"
#include "XmlLib\SaxWriter.h"
#include "GlobalSupportFunctions.h"
#include "LogPane.h"
#include "Buff.h"
#include "SetBonus.h"
#include "DDOBuilder.h"
#include "Build.h"

#define DL_ELEMENT Item

namespace
{
    const wchar_t f_saxElementName[] = L"Item";
    DL_DEFINE_NAMES(Item_PROPERTIES)

    const unsigned f_verCurrent = 1;
}

Item::Item() :
    XmlLib::SaxContentElement(f_saxElementName, f_verCurrent),
    m_Slots(L"EquipmentSlot"),
    m_RestrictedSlots(L"RestrictedSlots"),
    m_iconIndex(0),         // defaults to no image
    m_bIsRaindItem(false)
{
    DL_INIT(Item_PROPERTIES)
}

Item::Item(const XmlLib::SaxString & objectName) :
    XmlLib::SaxContentElement(objectName, f_verCurrent),
    m_Slots(L"EquipmentSlot"),
    m_RestrictedSlots(L"RestrictedSlots"),
    m_iconIndex(0),         // defaults to no image
    m_bIsRaindItem(false)
{
    DL_INIT(Item_PROPERTIES)
}

DL_DEFINE_ACCESS(Item_PROPERTIES)

XmlLib::SaxContentElementInterface * Item::StartElement(
        const XmlLib::SaxString & name,
        const XmlLib::SaxAttributes & attributes)
{
    XmlLib::SaxContentElementInterface * subHandler =
            SaxContentElement::StartElement(name, attributes);

    DL_START(Item_PROPERTIES)

    return subHandler;
}

void Item::EndElement()
{
    SaxContentElement::EndElement();
    DL_END(Item_PROPERTIES)
}

void Item::Write(XmlLib::SaxWriter * writer) const
{
    writer->StartElement(ElementName());//, VersionAttributes());
    DL_WRITE(Item_PROPERTIES)
    writer->EndElement();
}

bool Item::CanEquipToSlot(InventorySlotType slot) const
{
    bool canEquipToSlot = false;
    switch (slot)
    {
    case Inventory_Arrows:
        canEquipToSlot = m_Slots.HasArrow();
        break;
    case Inventory_CosmeticArmor:
        canEquipToSlot = m_Slots.HasCosmeticArmor();
        break;
    case Inventory_Armor:
        canEquipToSlot = m_Slots.HasArmor();
        break;
    case Inventory_ArmorCloth:
        canEquipToSlot = m_Slots.HasArmor() && Armor() == Armor_Cloth;
        break;
    case Inventory_ArmorLight:
        canEquipToSlot = m_Slots.HasArmor() && Armor() == Armor_Light;
        break;
    case Inventory_ArmorMedium:
        canEquipToSlot = m_Slots.HasArmor() && Armor() == Armor_Medium;
        break;
    case Inventory_ArmorHeavy:
        canEquipToSlot = m_Slots.HasArmor() && Armor() == Armor_Heavy;
        break;
    case Inventory_ArmorDocent:
        canEquipToSlot = m_Slots.HasArmor() && Armor() == Armor_Docent;
        break;
    case Inventory_Belt:
        canEquipToSlot = m_Slots.HasBelt();
        break;
    case Inventory_Boots:
        canEquipToSlot = m_Slots.HasBoots();
        break;
    case Inventory_Bracers:
        canEquipToSlot = m_Slots.HasBracers();
        break;
    case Inventory_CosmeticCloak:
        canEquipToSlot = m_Slots.HasCosmeticCloak();
        break;
    case Inventory_Cloak:
        canEquipToSlot = m_Slots.HasCloak();
        break;
    case Inventory_Gloves:
        canEquipToSlot = m_Slots.HasGloves();
        break;
    case Inventory_Goggles:
        canEquipToSlot = m_Slots.HasGoggles();
        break;
    case Inventory_CosmeticHelm:
        canEquipToSlot = m_Slots.HasCosmeticHelm();
        break;
    case Inventory_Helmet:
        canEquipToSlot = m_Slots.HasHelmet();
        break;
    case Inventory_Necklace:
        canEquipToSlot = m_Slots.HasNecklace();
        break;
    case Inventory_Quiver:
        canEquipToSlot = m_Slots.HasQuiver();
        break;
    case Inventory_Ring1:
    case Inventory_Ring2:
        canEquipToSlot = m_Slots.HasRing();
        break;
    case Inventory_Trinket:
        canEquipToSlot = m_Slots.HasTrinket();
        break;
    case Inventory_Weapon1:
        canEquipToSlot = m_Slots.HasWeapon1();
        break;
    case Inventory_Weapon2:
        canEquipToSlot = m_Slots.HasWeapon2();
        break;
    case Inventory_CosmeticWeapon1:
        canEquipToSlot = m_Slots.HasWeapon1();
        break;
    case Inventory_CosmeticWeapon2:
        canEquipToSlot = m_Slots.HasWeapon2();
        break;
    }
    return canEquipToSlot;
}

void Item::VerifyObject() const
{
    bool ok = true;
    std::stringstream ss;
    ss << "=====" << m_Name << "=====\n";
    if (HasIcon())
    {
        CDDOBuilderApp* pApp = dynamic_cast<CDDOBuilderApp*>(AfxGetApp());
        if (pApp->m_itemImagesMap.find(Icon()) == pApp->m_itemImagesMap.end())
        {
            ss << "Item is missing image file \"" << Icon() << "\"\n";
            ok = false;
        }
    }
    else
    {
        ss << "Item is missing Icon field\n";
        ok = false;
    }
    if (HasWeapon())
    {
        if (Weapon() != Weapon_RuneArm)
        {
            if (!HasCriticalThreatRange())
            {
                ss << "Item is missing Critical Threat Range\n";
                ok = false;
            }
            if (!HasCriticalMultiplier())
            {
                ss << "Item is missing Critical Multiplier\n";
                ok = false;
            }
            if (m_hasDamageDice)
            {
                //ok &= m_DamageDice.VerifyObject(&ss);
            }
        }
    }
    // check the item effects also
    std::vector<Effect>::const_iterator it = m_Effects.begin();
    while (it != m_Effects.end())
    {
        ok &= (*it).VerifyObject(&ss);
        ++it;
    }
    // verify any buffs
    std::vector<Buff>::const_iterator bit = m_Buffs.begin();
    while (bit != m_Buffs.end())
    {
        ok &= (*bit).VerifyObject(&ss);
        ++bit;
    }
    // verify its DC objects
    //std::list<DC>::const_iterator edcit = m_EffectDC.begin();
    //while (edcit != m_EffectDC.end())
    //{
    //    ok &= (*edcit).VerifyObject(&ss);
    //    ++edcit;
    //}
    // verify any augments
    std::vector<ItemAugment>::const_iterator iacit = m_Augments.begin();
    while (iacit != m_Augments.end())
    {
        ok &= (*iacit).VerifyObject(&ss);
        ++iacit;
    }
    // check any set bonuses exist
    const std::list<::SetBonus> & loadedSets = SetBonuses();
    std::list<std::string>::const_iterator sbit = m_SetBonus.begin();
    while (sbit != m_SetBonus.end())
    {
        bool bFound = false;
        std::list<::SetBonus>::const_iterator sit = loadedSets.begin();
        while (!bFound && sit != loadedSets.end())
        {
            bFound = ((*sit).Type() == (*sbit));
            ++sit;
        }
        if (!bFound)
        {
            ok = false;
            ss << "Has unknown set bonus \"" << (*sbit) << "\"\n";
        }
        ++sbit;
    }

    if (!ok)
    {
        GetLog().AddLogEntry(ss.str().c_str());
    }
}

void Item::CopyUserSetValues(const Item & original)
{
    // when updating an item to the latest version there are certain user values
    // which we need to copy
    // augments/upgrade slots
    m_Augments = original.m_Augments;
    m_SlotUpgrades = original.m_SlotUpgrades;
}

bool Item::ContainsSearchText(const std::string& searchText, const Build* pBuild) const
{
    bool bHasSearchText = true;
    // break up the search text into individual fields and search for each
    std::string parsedItem;
    std::stringstream ss(searchText);
    while (bHasSearchText && std::getline(ss, parsedItem, ' '))
    {
        bHasSearchText = false; // must have all terms
        // we have to search all of the following text fields of this item:
        // Name
        // Description
        // Drop Location
        // EffectDescription(s)
        // Set Bonuses
        bHasSearchText |= SearchForText(Name(), parsedItem);
        bHasSearchText |= SearchForText(ItemType(), parsedItem);
        bHasSearchText |= SearchForText(Description(), parsedItem);
        if (HasMaterial())
        {
            bHasSearchText |= SearchForText(Material(), parsedItem);
        }
        if (HasMinorArtifact())
        {
            bHasSearchText |= SearchForText("Minor Artifact", parsedItem);
        }
        if (m_hasDropLocation)
        {
            bHasSearchText |= SearchForText(DropLocation(), parsedItem);
        }

        std::vector<CString> buffs = BuffDescriptions(pBuild);
        auto bit = buffs.begin();
        while (!bHasSearchText && bit != buffs.end())
        {
            bHasSearchText |= SearchForText((LPCTSTR)(*bit), parsedItem);
            ++bit;
        }
        std::list<std::string>::const_iterator it = m_SetBonus.begin();
        while (!bHasSearchText && it != m_SetBonus.end())
        {
            const ::SetBonus& set = FindSetBonus((*it));
            bHasSearchText |= SearchForText(set.Type(), parsedItem);
            ++it;
        }
    }
    return bHasSearchText;
}

std::string Item::ItemType() const
{
    std::stringstream ss;
    bool bFirst = true;
    for (size_t i = Inventory_Unknown; i < Inventory_Count; ++i)
    {
        if (Slots().HasSlot(static_cast<InventorySlotType>(i)))
        {
            if (!bFirst)
            {
                ss << ", ";
            }
            if (i == Inventory_Armor && HasArmor())
            {
                ss << EnumEntryText(Armor(), armorTypeMap);
                ss << " ";
            }
            if ((i == Inventory_Weapon1 || i == Inventory_Weapon2)
                    && HasWeapon())
            {
                ss << EnumEntryText(Weapon(), weaponTypeMap);
                ss << " ";
            }
            ss << EnumEntryText(static_cast<InventorySlotType>(i), InventorySlotTypeMap);
            bFirst = false;
        }
    }

    return ss.str();
}

void Item::SetIconIndex(size_t index)
{
    m_iconIndex = index;
}

size_t Item::IconIndex() const
{
    return m_iconIndex;
}

void Item::SetFilename(const std::string& filename)
{
    m_filename = filename;
}

const std::string& Item::Filename() const
{
    return m_filename;
}

std::vector<CString> Item::BuffDescriptions(const Build* pBuild) const
{
    std::vector<CString> buffs;
    for (auto&& it : m_Buffs)
    {
        // we can use front here as there is always only 1 entry
        // for an item entry. EffectInfo's can have multiple types
        // when in the Buff list only
        Buff buff = FindBuff(it.Type());
        if (it.HasBonusType()) buff.Set_BonusType(it.BonusType());
        if (it.HasIgnore()) buff.Set_Ignore(it.Ignore());
        if (it.HasValue1()) buff.Set_Value1(it.Value1());
        if (it.HasValue2()) buff.Set_Value2(it.Value2());
        if (it.HasItem()) buff.Set_Item(it.Item());
        if (it.HasDescription1()) buff.Set_Description1(it.Description1());
        buffs.push_back(buff.MakeDescription());
        if (it.HasRequirementsToUse())
        {
            std::vector<bool> metIgnored;
            it.RequirementsToUse().CreateRequirementStrings(*pBuild, pBuild->Level(), true, &buffs, &metIgnored);
        }
    }
    return buffs;
}

void Item::AddSetBonus(const std::string& setName)
{
    m_SetBonus.push_back(setName);
}

void Item::AddItemAugment(const ItemAugment& ia)
{
    m_Augments.push_back(ia);
}

void Item::AddRaceRequirement(const std::string& race)
{
    Requirement raceRequirement(Requirement_Race, race, 1);
    m_RequirementsToUse.AddRequirement(raceRequirement);
    m_hasRequirementsToUse = true;
}

void Item::AddClassRequirement(const std::string& c)
{
    Requirement classRequirement(Requirement_BaseClass, c);
    m_RequirementsToUse.AddRequirement(classRequirement);
    m_hasRequirementsToUse = true;
}

void Item::AddFeatRequirement(const std::string& feat)
{
    Requirement featRequirement(Requirement_Feat, feat);
    m_RequirementsToUse.AddRequirement(featRequirement);
    m_hasRequirementsToUse = true;
}

void Item::AddFeatAnySourceRequirement(const std::string& feat)
{
    Requirement featRequirement(Requirement_FeatAnySource, feat, 1);
    m_RequirementsToUse.AddRequirement(featRequirement);
    m_hasRequirementsToUse = true;
}

void Item::AddBuff(const Buff& b)
{
    m_Buffs.push_back(b);
}

void Item::SetAugments(const std::vector<ItemAugment>& augments)
{
    Set_Augments(augments);
}

bool Item::IsRaidItem() const
{
    return m_bIsRaindItem;
}

void Item::SetIsRaidItem(bool bIsRaidItem)
{
    m_bIsRaindItem = bIsRaidItem;
}

size_t Item::RealCriticalThreatRange() const
{
    size_t criticalRange = CriticalThreatRange(); // default
    bool bHasKeen = false;
    bHasKeen = FindEffect(Effect_Weapon_Keen);
    if (bHasKeen)
    {
        criticalRange += WeaponBaseCriticalRange(Weapon());
    }
    return criticalRange;
}

bool Item::FindEffect(EffectType et) const
{
    bool isEffectType = false;
    for (auto&& eit: m_Effects)
    {
        isEffectType |= eit.IsType(et);
    }
    if (!isEffectType)
    {
        for (auto&& bit: m_Buffs)
        {
            Buff buff = FindBuff(bit.Type());
            const std::list<Effect>& effects = buff.Effects();
            for (auto&& eit: effects)
            {
                isEffectType |= eit.IsType(et);
            }
        }
    }
    return isEffectType;
}

int Item::BuffValue(EffectType et) const
{
    int value = 0;
    bool bFound = false;
    for (auto&& eit: m_Effects)
    {
        if (eit.IsType(et))
        {
            value = static_cast<int>(eit.Amount().front());
            bFound = true;
        }
    }
    if (!bFound)
    {
        for (auto&& bit: m_Buffs)
        {
            Buff buff = FindBuff(bit.Type());
            std::list<Effect> effects = buff.Effects();
            bit.UpdatedEffects(&effects, false);
            for (auto&& eit: effects)
            {
                if (eit.IsType(et))
                {
                    value = static_cast<int>(eit.Amount().front());
                    bFound = true;
                }
                if (bFound) break;
            }
            if (bFound) break;
        }
    }
    return value;
}

bool Item::HasSetBonus(
    const std::string& setBonusName) const
{
    bool bHasSetBonus = false;
    bool bNativeSetBonusSuppressed = false;
    // we need to check all out augment slots for set bonus first
    // as its possible one of those could have one that suppresses the
    // items fixed set bonuses (if any)
    for (auto&& ait: m_Augments)
    {
        if (ait.HasSelectedAugment())
        {
            CString augmentName(ait.SelectedAugment().c_str());
            // augment name may need to be updated to include the actual bonus value
            const Augment& augment = FindAugmentByName((LPCTSTR)augmentName, this);
            for (auto&& sbit : augment.SetBonus())
            {
                if (sbit == setBonusName)
                {
                    bHasSetBonus = true;
                }
            }
            if (augment.HasSuppressSetBonus())
            {
                bNativeSetBonusSuppressed = true;
            }
        }
    }
    // now check the native set bonuses if not suppressed
    if (!bNativeSetBonusSuppressed)
    {
        for (auto&& sbit: m_SetBonus)
        {
            if (sbit == setBonusName)
            {
                bHasSetBonus = true;
            }
        }
    }
    return bHasSetBonus;
}

