// BreakdownItemWeaponEffects.cpp
//
#include "stdafx.h"
#include "BreakdownItemWeaponEffects.h"

#include "BreakdownsPane.h"
#include "BreakdownItemWeapon.h"
#include "GlobalSupportFunctions.h"

BreakdownItemWeaponEffects::BreakdownItemWeaponEffects(
        CBreakdownsPane* m_pPane,
        MfcControls::CTreeListCtrl * treeList,
        HTREEITEM hItem) :
    BreakdownItem(Breakdown_WeaponEffectHolder, NULL, NULL),
    m_pPane(m_pPane),
    m_pTreeList(treeList),  // not passed to base class
    m_hItem(hItem),         // not passed to base class
    m_pMainHandWeapon(NULL),
    m_pOffHandWeapon(NULL)
{
    // ensure the vector is the correct size
    m_weaponFeatEffects.resize(Weapon_Count);
    m_weaponItemEffects.resize(Weapon_Count);
    m_weaponEnhancementEffects.resize(Weapon_Count);
    // we need to register ourselves as observers of all the Weapon effects
    m_pPane->RegisterBuildCallbackEffect(Effect_GhostTouch, this);
    m_pPane->RegisterBuildCallbackEffect(Effect_TrueSeeing, this);
    m_pPane->RegisterBuildCallbackEffect(Effect_DRBypass, this);

    m_pPane->RegisterBuildCallbackEffect(Effect_Weapon_Alacrity, this);
    m_pPane->RegisterBuildCallbackEffect(Effect_Weapon_AttackAbility, this);
    m_pPane->RegisterBuildCallbackEffect(Effect_Weapon_AttackBonus, this);
    m_pPane->RegisterBuildCallbackEffect(Effect_Weapon_AttackBonusCritical, this);
    m_pPane->RegisterBuildCallbackEffect(Effect_Weapon_AttackAndDamage, this);
    m_pPane->RegisterBuildCallbackEffect(Effect_Weapon_AttackAndDamageCritical, this);
    m_pPane->RegisterBuildCallbackEffect(Effect_Weapon_BaseDamage, this);
    m_pPane->RegisterBuildCallbackEffect(Effect_Weapon_CriticalMultiplier, this);
    m_pPane->RegisterBuildCallbackEffect(Effect_Weapon_CriticalMultiplier19To20, this);
    m_pPane->RegisterBuildCallbackEffect(Effect_Weapon_CriticalRange, this);
    m_pPane->RegisterBuildCallbackEffect(Effect_Weapon_DamageAbility, this);
    m_pPane->RegisterBuildCallbackEffect(Effect_Weapon_DamageBonus, this);
    m_pPane->RegisterBuildCallbackEffect(Effect_Weapon_DamageBonusCritical, this);
    m_pPane->RegisterBuildCallbackEffect(Effect_Weapon_Enchantment, this);
    m_pPane->RegisterBuildCallbackEffect(Effect_Weapon_Keen, this);
    m_pPane->RegisterBuildCallbackEffect(Effect_Weapon_VorpalRange, this);
    m_pPane->RegisterBuildCallbackEffect(Effect_Weapon_OtherDamageBonus, this);
    m_pPane->RegisterBuildCallbackEffect(Effect_Weapon_OtherDamageBonusCritical, this);
    m_pPane->RegisterBuildCallbackEffect(Effect_Weapon_OtherDamageBonusClass, this);
    m_pPane->RegisterBuildCallbackEffect(Effect_Weapon_OtherDamageBonusCriticalClass, this);
    m_pPane->RegisterBuildCallbackEffect(Effect_WeaponAlacrityClass, this);
    m_pPane->RegisterBuildCallbackEffect(Effect_WeaponAttackAbilityClass, this);
    m_pPane->RegisterBuildCallbackEffect(Effect_WeaponBaseDamageBonusClass, this);
    m_pPane->RegisterBuildCallbackEffect(Effect_WeaponDamageAbilityClass, this);
    m_pPane->RegisterBuildCallbackEffect(Effect_WeaponDamageBonusCriticalStat, this);
    m_pPane->RegisterBuildCallbackEffect(Effect_WeaponDamageBonusStat, this);
    m_pPane->RegisterBuildCallbackEffect(Effect_WeaponProficiencyClass, this);
    m_pPane->RegisterBuildCallbackEffect(Effect_WeaponAttackBonusClass, this);
    m_pPane->RegisterBuildCallbackEffect(Effect_WeaponAttackBonusCriticalClass, this);
    m_pPane->RegisterBuildCallbackEffect(Effect_WeaponDamageBonusClass, this);
    m_pPane->RegisterBuildCallbackEffect(Effect_WeaponDamageBonusCriticalClass, this);
    m_pPane->RegisterBuildCallbackEffect(Effect_Weapon_KeenClass, this);
    m_pPane->RegisterBuildCallbackEffect(Effect_WeaponCriticalMultiplierClass, this);
    m_pPane->RegisterBuildCallbackEffect(Effect_WeaponCriticalRangeClass, this);
    m_pPane->RegisterBuildCallbackEffect(Effect_Weapon_EnchantmentClass, this);
    m_pPane->RegisterBuildCallbackEffect(Effect_WeaponAttackBonusDamageType, this);
    m_pPane->RegisterBuildCallbackEffect(Effect_WeaponAttackBonusCriticalDamageType, this);
    m_pPane->RegisterBuildCallbackEffect(Effect_WeaponDamageBonusDamageType, this);
    m_pPane->RegisterBuildCallbackEffect(Effect_WeaponDamageBonusCriticalDamageType, this);
    m_pPane->RegisterBuildCallbackEffect(Effect_Weapon_KeenDamageType, this);
}

BreakdownItemWeaponEffects::~BreakdownItemWeaponEffects()
{
    delete m_pMainHandWeapon;
    m_pMainHandWeapon = NULL;
    delete m_pOffHandWeapon;
    m_pOffHandWeapon = NULL;
}

void BreakdownItemWeaponEffects::SetCharacter(
    Character * charData)
{
    BreakdownItem::BuildChanged(charData);
    if (charData == NULL)
    {
        // remove any weapons listed from the previous character
        EquippedGear gear;      // empty gear
        WeaponsChanged(gear);
    }
    // clear any effects on a character change
    for (size_t i = 0; i < Weapon_Count; ++i)
    {
        m_weaponFeatEffects[i].clear();
        m_weaponItemEffects[i].clear();
        m_weaponEnhancementEffects[i].clear();
    }
    if (m_pMainHandWeapon != NULL)
    {
        m_pMainHandWeapon->BuildChanged(charData);
    }
    if (m_pOffHandWeapon != NULL)
    {
        m_pOffHandWeapon->BuildChanged(charData);
    }
}

// required overrides
CString BreakdownItemWeaponEffects::Title() const
{
    CString title("This item not displayed");
    return title;
}

CString BreakdownItemWeaponEffects::Value() const
{
    return "Not displayed";
}

void BreakdownItemWeaponEffects::CreateOtherEffects()
{
    // does nothing in this holder class
}

bool BreakdownItemWeaponEffects::AffectsUs(const Effect&) const
{
    // always affects us, as we are registered with weapon effects only
    return true;
}

void BreakdownItemWeaponEffects::AddToAffectedWeapons(
        std::vector<std::list<Effect>> * list,
        const Effect & effect,
        NotificationType nt)
{
    // see which sub-weapons (if any) this effect applies to
    // if it add it to the list
    for (size_t i = Weapon_StartOfList; i < Weapon_Count; ++i)
    {
        bool affectsWeapon = AffectsThisWeapon((WeaponType)i, effect);
        // find in the list and remove
        if (affectsWeapon)
        {
            AddToAffectedWeapon(list, effect, (WeaponType)i, nt);
        }
    }
}

void BreakdownItemWeaponEffects::RemoveFromAffectedWeapons(
        std::vector<std::list<Effect>> * list,
        const Effect & effect,
        NotificationType nt)
{
    // see which sub-weapons (if any) this effect applies to
    // if it affects remove it from the list
    for (size_t i = Weapon_StartOfList; i < Weapon_Count; ++i)
    {
        bool affectsWeapon = AffectsThisWeapon((WeaponType)i, effect);
        // find in the list and remove
        if (affectsWeapon)
        {
            RemoveFromAffectedWeapon(list, effect, (WeaponType)i, nt);
        }
    }
}

void BreakdownItemWeaponEffects::AddToAffectedWeapon(
    std::vector<std::list<Effect>>* list,
    const Effect& effect,
    WeaponType wt,
    NotificationType nt)
{
    if (wt == Weapon_All)
    {
        AddToAffectedWeapons(list, effect, nt);
    }
    else
    {
        (*list)[wt].push_back(effect);
        if (m_pMainHandWeapon != NULL
            && m_pMainHandWeapon->Weapon() == wt
            && effect.HasWeapon1())
        {
            // also apply to the active breakdowns
            switch (nt)
            {
            case NT_Feat: m_pMainHandWeapon->FeatEffectApplied(NULL, effect); break;
            case NT_Item: m_pMainHandWeapon->ItemEffectApplied(NULL, effect); break;
            case NT_ItemWeapon: m_pMainHandWeapon->ItemEffectApplied(NULL, effect); break;
            case NT_Enhancement: m_pMainHandWeapon->EnhancementEffectApplied(NULL, effect); break;
            }
        }
        if (m_pOffHandWeapon != NULL
            && m_pOffHandWeapon->Weapon() == wt
            && effect.HasWeapon2())
        {
            // also apply to the active breakdowns
            switch (nt)
            {
            case NT_Feat: m_pOffHandWeapon->FeatEffectApplied(NULL, effect); break;
            case NT_Item: m_pOffHandWeapon->ItemEffectApplied(NULL, effect); break;
            case NT_ItemWeapon: m_pOffHandWeapon->ItemEffectApplied(NULL, effect); break;
            case NT_Enhancement: m_pOffHandWeapon->EnhancementEffectApplied(NULL, effect); break;
            }
        }
    }
}

void BreakdownItemWeaponEffects::RemoveFromAffectedWeapon(
    std::vector<std::list<Effect>>* list,
    const Effect& effect,
    WeaponType wt,
    NotificationType nt)
{
    if (wt == Weapon_All)
    {
        RemoveFromAffectedWeapons(list, effect, nt);
    }
    else
    {
        std::list<Effect>::iterator it = (*list)[wt].begin();
        while (it != (*list)[wt].end())
        {
            if ((*it) == effect)
            {
                // this is it
                it = (*list)[wt].erase(it);
                if (m_pMainHandWeapon != NULL
                    && m_pMainHandWeapon->Weapon() == wt
                    && effect.HasWeapon1())
                {
                    // also apply to the active breakdowns
                    switch (nt)
                    {
                    case NT_Feat: m_pMainHandWeapon->FeatEffectRevoked(NULL, effect); break;
                    case NT_Item: m_pMainHandWeapon->ItemEffectRevoked(NULL, effect); break;
                    case NT_ItemWeapon: m_pMainHandWeapon->ItemEffectRevoked(NULL, effect); break;
                    case NT_Enhancement: m_pMainHandWeapon->EnhancementEffectRevoked(NULL, effect); break;
                    }
                }
                if (m_pOffHandWeapon != NULL
                    && m_pOffHandWeapon->Weapon() == wt
                    && effect.HasWeapon2())
                {
                    // also apply to the active breakdowns
                    switch (nt)
                    {
                    case NT_Feat: m_pOffHandWeapon->FeatEffectRevoked(NULL, effect); break;
                    case NT_Item: m_pOffHandWeapon->ItemEffectRevoked(NULL, effect); break;
                    case NT_ItemWeapon: m_pOffHandWeapon->ItemEffectRevoked(NULL, effect); break;
                    case NT_Enhancement: m_pOffHandWeapon->EnhancementEffectRevoked(NULL, effect); break;
                    }
                }
                break; // and were done
            }
            ++it;
        }
    }
}

bool BreakdownItemWeaponEffects::AffectsThisWeapon(
        WeaponType wt,
        const Effect & effect)
{
    // determine whether this effect is applied to this weapon
    bool isUs = false;
    Build* pBuild = m_pCharacter->ActiveBuild();
    if (pBuild != NULL)
    {
        EffectType t = effect.Type().front();
        switch (t)
        {
        case Effect_Weapon_Alacrity:
        case Effect_Weapon_AttackAbility:
        case Effect_Weapon_AttackBonus:
        case Effect_Weapon_AttackBonusCritical:
        case Effect_Weapon_AttackAndDamage:
        case Effect_Weapon_AttackAndDamageCritical:
        case Effect_Weapon_BaseDamage:
        case Effect_Weapon_CriticalMultiplier:
        case Effect_Weapon_CriticalMultiplier19To20:
        case Effect_Weapon_CriticalRange:
        case Effect_Weapon_DamageAbility:
        case Effect_Weapon_DamageBonus:
        case Effect_Weapon_DamageBonusCritical:
        case Effect_Weapon_Enchantment:
        case Effect_Weapon_Keen:
        case Effect_Weapon_VorpalRange:
        case Effect_Weapon_OtherDamageBonus:
        case Effect_Weapon_OtherDamageBonusCritical:
        case Effect_WeaponDamageBonusCriticalStat:
        case Effect_WeaponDamageBonusStat:
        case Effect_DRBypass:
            // look for the individual weapon type
            for (auto&& iit : effect.Item())
            {
                WeaponType w = TextToEnumEntry(iit, weaponTypeMap, false);
                if (w == Weapon_All
                    || w == wt)
                {
                    isUs = true;
                }
            }
            break;
        case Effect_WeaponAttackBonusDamageType:
        case Effect_WeaponAttackBonusCriticalDamageType:
        case Effect_WeaponDamageBonusDamageType:
        case Effect_WeaponDamageBonusCriticalDamageType:
        case Effect_Weapon_KeenDamageType:
            // check that the weapon we have is in that damage group type
            for (auto&& iit : effect.Item())
            {
                WeaponDamageType wdt = TextToEnumEntry(iit, weaponDamageTypeMap); // throws in fail, fix that effect!
                if (wdt != WeaponDamage_Unknown)
                {
                    if (pBuild->IsWeaponInGroup(iit, wt))
                    {
                        isUs = true;
                    }
                }
            }
            break;
        case Effect_Weapon_OtherDamageBonusClass:
        case Effect_Weapon_OtherDamageBonusCriticalClass:
        case Effect_WeaponAlacrityClass:
        case Effect_WeaponAttackAbilityClass:
        case Effect_WeaponBaseDamageBonusClass:
        case Effect_WeaponDamageAbilityClass:
        case Effect_WeaponProficiencyClass:
        case Effect_WeaponAttackBonusClass:
        case Effect_WeaponAttackBonusCriticalClass:
        case Effect_WeaponDamageBonusClass:
        case Effect_WeaponDamageBonusCriticalClass:
        case Effect_Weapon_KeenClass:
        case Effect_WeaponCriticalMultiplierClass:
        case Effect_WeaponCriticalRangeClass:
        case Effect_Weapon_EnchantmentClass:
            // check each mentioned weapon group
            for (auto&& iit : effect.Item())
            {
                if (pBuild->IsWeaponInGroup(iit, wt))
                {
                    isUs = true;
                }
            }
            break;
        case Effect_GhostTouch:
        case Effect_TrueSeeing:
            isUs = true;
            break;
        }
    }
    return isUs;
}

void BreakdownItemWeaponEffects::FeatEffectApplied(
        Build*,
        const Effect& effect)
{
    // handle special affects that change our list of available stats
    AddToAffectedWeapons(&m_weaponFeatEffects, effect, NT_Feat);
}

void BreakdownItemWeaponEffects::FeatEffectRevoked(
    Build*,
    const Effect& effect)
{
    // handle special affects that change our list of available stats
    RemoveFromAffectedWeapons(&m_weaponFeatEffects, effect, NT_Feat);
}

void BreakdownItemWeaponEffects::ItemEffectApplied(
    Build*,
    const Effect& effect)
{
    // handle special affects that change our list of available stats
    AddToAffectedWeapons(&m_weaponItemEffects, effect, NT_Item);
}

void BreakdownItemWeaponEffects::ItemEffectRevoked(
    Build*,
    const Effect& effect)
{
    // handle special affects that change our list of available stats
    RemoveFromAffectedWeapons(&m_weaponItemEffects, effect, NT_Item);
}

void BreakdownItemWeaponEffects::ItemEffectApplied(
    Build*,
    const Effect& effect,
    WeaponType wt)
{
    // handle special affects that change our list of available stats
    AddToAffectedWeapon(&m_weaponItemEffects, effect, wt, NT_ItemWeapon);
}

void BreakdownItemWeaponEffects::ItemEffectRevoked(
    Build*,
    const Effect& effect,
    WeaponType wt)
{
    // handle special affects that change our list of available stats
    RemoveFromAffectedWeapon(&m_weaponItemEffects, effect, wt, NT_ItemWeapon);
}

void BreakdownItemWeaponEffects::EnhancementEffectApplied(
    Build*,
    const Effect& effect)
{
    // handle special affects that change our list of available stats
    AddToAffectedWeapons(&m_weaponEnhancementEffects, effect, NT_Enhancement);
}

void BreakdownItemWeaponEffects::EnhancementEffectRevoked(
    Build*,
    const Effect& effect)
{
    // handle special affects that change our list of available stats
    RemoveFromAffectedWeapons(&m_weaponEnhancementEffects, effect, NT_Enhancement);
}

void BreakdownItemWeaponEffects::SliderChanged(
    Build* pBuild,
    const std::string& sliderName,
    int newValue)
{
    if (m_pMainHandWeapon != NULL)
    {
        m_pMainHandWeapon->SliderChanged(pBuild, sliderName, newValue);
    }

    if (m_pOffHandWeapon != NULL)
    {
        m_pOffHandWeapon->SliderChanged(pBuild, sliderName, newValue);
    }
}

void BreakdownItemWeaponEffects::StanceActivated(
    Build* pBuild,
    const std::string& stanceName)
{
    if (m_pMainHandWeapon != NULL)
    {
        m_pMainHandWeapon->StanceActivated(pBuild, stanceName);
    }

    if (m_pOffHandWeapon != NULL)
    {
        m_pOffHandWeapon->StanceActivated(pBuild, stanceName);
    }
}

void BreakdownItemWeaponEffects::StanceDeactivated(
    Build* pBuild,
    const std::string& stanceName)
{
    if (m_pMainHandWeapon != NULL)
    {
        m_pMainHandWeapon->StanceDeactivated(pBuild, stanceName);
    }

    if (m_pOffHandWeapon != NULL)
    {
        m_pOffHandWeapon->StanceDeactivated(pBuild, stanceName);
    }
}

void BreakdownItemWeaponEffects::WeaponsChanged(const EquippedGear & gear)
{
    // first remove any previous weapons breakdowns (if present)
    m_pTreeList->DeleteSubItems(m_hItem);
    delete m_pMainHandWeapon;
    m_pMainHandWeapon = NULL;
    delete m_pOffHandWeapon;
    m_pOffHandWeapon = NULL;

    // now create the new weapon breakdowns (if required)
    if (gear.HasItemInSlot(Inventory_Weapon1))
    {
        m_pMainHandWeapon = CreateWeaponBreakdown(
                Breakdown_MainHand,
                gear.ItemInSlot(Inventory_Weapon1),
                Inventory_Weapon1);
    }
    else
    {
        // all non weapon breakdowns
        for (size_t i = Breakdown_Unknown + 1; i < Breakdown_EndOfNormalBreakdowns; ++i)
        {
            BreakdownItem * pBreakdown = FindBreakdown((BreakdownType)i);
            if (pBreakdown != NULL)
            {
                pBreakdown->SetWeapon(Weapon_Unknown, 0); // crit multiplier does not matter here
            }
        }
    }
    if (gear.HasItemInSlot(Inventory_Weapon2))
    {
        m_pOffHandWeapon = CreateWeaponBreakdown(
                Breakdown_OffHand,
                gear.ItemInSlot(Inventory_Weapon2),
                Inventory_Weapon2);
    }
    m_pTreeList->Expand(m_hItem, TVE_EXPAND);
    m_pTreeList->RedrawWindow();    // ensure view updates
}

bool BreakdownItemWeaponEffects::AreWeaponsCentering() const
{
    bool isCentered = true;
    if (m_pMainHandWeapon != NULL
            && !m_pMainHandWeapon->IsCentering())
    {
        isCentered = false;
    }
    if (m_pOffHandWeapon != NULL
            && !m_pOffHandWeapon->IsCentering())
    {
        isCentered = false;
    }
    return isCentered;
}

BreakdownItemWeapon * BreakdownItemWeaponEffects::CreateWeaponBreakdown(
        BreakdownType bt,
        const Item & item,
        InventorySlotType ist)
{
    BreakdownItemWeapon * pWeaponBreakdown = NULL;
    std::string name = item.Name();
    WeaponType wt = item.Weapon();
    // some breakdowns can be dependent on the weapon type
    if (bt == Breakdown_MainHand)
    {
        // all non weapon breakdowns
        for (size_t i = Breakdown_Unknown + 1; i < Breakdown_EndOfNormalBreakdowns; ++i)
        {
            BreakdownItem * pBreakdown = FindBreakdown((BreakdownType)i);
            if (pBreakdown != NULL)
            {
                pBreakdown->SetWeapon(wt, 0); // crit multiplier does not matter here
            }
        }
    }
    HTREEITEM hItem = m_pTreeList->InsertItem(
            name.c_str(),
            m_hItem,
            TVI_LAST);
    BaseDice defaultDice;
    pWeaponBreakdown = new BreakdownItemWeapon(
            m_pPane,
            bt,
            wt,
            name.c_str(),
            m_pTreeList,
            hItem,
            bt == Breakdown_MainHand
                ? Inventory_Weapon1
                : Inventory_Weapon2,
            item.HasDamageDice()
                ? item.DamageDice()
                : defaultDice,
             item.HasCriticalMultiplier()
                ? item.CriticalMultiplier()
                : 1);
    m_pTreeList->SetItemData(hItem, (DWORD)(void*)pWeaponBreakdown);
    pWeaponBreakdown->SetCharacter(m_pCharacter);

    // apply the items stat attack modifiers (if any)
    for (auto&& amit : item.AttackModifier())
    {
        Effect e(Effect_Weapon_AttackAbility, "", "Weapon Enchantment", 1);
        std::list<std::string> items;
        items.push_back((LPCTSTR)EnumEntryText(amit, abilityTypeMap));
        items.push_back((LPCTSTR)EnumEntryText(item.Weapon(), weaponTypeMap));
        e.SetItem(items);
        pWeaponBreakdown->ItemEffectApplied(
                m_pCharacter->ActiveBuild(),
                e);
    }
    // apply the items stat damage modifiers (if any)
    for (auto&& dmit : item.DamageModifier())
    {
        Effect e(Effect_Weapon_DamageAbility, "", "Weapon Enchantment", 1);
        std::list<std::string> items;
        items.push_back((LPCTSTR)EnumEntryText(dmit, abilityTypeMap));
        items.push_back((LPCTSTR)EnumEntryText(item.Weapon(), weaponTypeMap));
        e.SetItem(items);
        pWeaponBreakdown->ItemEffectApplied(
            m_pCharacter->ActiveBuild(),
            e);
    }

    // for this weapon, filter in all the cached effects
    for (auto&& wfeit: m_weaponFeatEffects[wt])
    {
        if (   (ist == Inventory_Weapon1 && wfeit.HasWeapon1())
            || (ist == Inventory_Weapon2 && wfeit.HasWeapon2()))
        {
            pWeaponBreakdown->FeatEffectApplied(
                    m_pCharacter->ActiveBuild(),
                    wfeit);
        }
    }
    for (auto&& wieit : m_weaponItemEffects[wt])
    {
        if ((ist == Inventory_Weapon1 && wieit.HasWeapon1())
            || (ist == Inventory_Weapon2 && wieit.HasWeapon2()))
        {
            pWeaponBreakdown->ItemEffectApplied(
                    m_pCharacter->ActiveBuild(),
                    wieit);
        }
    }
    for (auto&& weeit : m_weaponEnhancementEffects[wt])
    {
        if ((ist == Inventory_Weapon1 && weeit.HasWeapon1())
            || (ist == Inventory_Weapon2 && weeit.HasWeapon2()))
        {
            pWeaponBreakdown->EnhancementEffectApplied(
                    m_pCharacter->ActiveBuild(),
                    weeit);
        }
    }

    return pWeaponBreakdown;
}

void BreakdownItemWeaponEffects::AddForumExportData(std::stringstream & forumExport)
{
    if (m_pMainHandWeapon != NULL)
    {
        forumExport << "Main Hand: ";
        m_pMainHandWeapon->AddForumExportData(forumExport);
    }
    if (m_pOffHandWeapon != NULL)
    {
        forumExport << "Off Hand: ";
        m_pOffHandWeapon->AddForumExportData(forumExport);
    }
}

BreakdownItem * BreakdownItemWeaponEffects::GetWeaponBreakdown(bool bMainhand, BreakdownType bt)
{
    BreakdownItem * pBI = NULL;
    if (bMainhand
            && m_pMainHandWeapon != NULL)
    {
        pBI = m_pMainHandWeapon->GetWeaponBreakdown(bt);
    }
    if (!bMainhand
            && m_pMainHandWeapon != NULL)
    {
        pBI = m_pOffHandWeapon->GetWeaponBreakdown(bt);
    }
    return pBI;
}

void BreakdownItemWeaponEffects::BuildChanged(Character* charData)
{
    // clear any effects on a character change
    for (size_t i = 0; i < Weapon_Count; ++i)
    {
        m_weaponFeatEffects[i].clear();
        m_weaponItemEffects[i].clear();
        m_weaponEnhancementEffects[i].clear();
    }
    if (m_pMainHandWeapon != NULL)
    {
        m_pMainHandWeapon->BuildChanged(charData);
    }
    if (m_pOffHandWeapon != NULL)
    {
        m_pOffHandWeapon->BuildChanged(charData);
    }
    BreakdownItem::BuildChanged(charData);
}
