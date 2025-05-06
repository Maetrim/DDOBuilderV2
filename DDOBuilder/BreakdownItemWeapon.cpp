// BreakdownItemWeapon.cpp
//
#include "stdafx.h"
#include "BreakdownItemWeapon.h"

#include "BreakdownsPane.h"
#include "GlobalSupportFunctions.h"

BreakdownItemWeapon::BreakdownItemWeapon(
        CBreakdownsPane* pPane,
        BreakdownType type,
        WeaponType weaponType,
        const CString & title,
        MfcControls::CTreeListCtrl * treeList,
        HTREEITEM hItem,
        InventorySlotType slot,
        const BasicDice& damageDice,
        size_t weaponCriticalMultiplier) :
    BreakdownItem(type, treeList, hItem),
    m_title(title),
    m_damageDice(damageDice),
    m_weaponEnchantment(pPane, Breakdown_WeaponEnchantment, IsShield(weaponType) ? Effect_ShieldEnchantment : Effect_Weapon_Enchantment, IsShield(weaponType) ? "Shield Enhancement" : "Weapon Enhancement", treeList, NULL, false),
    m_baseDamage(pPane, Breakdown_WeaponBaseDamage, Effect_Weapon_BaseDamage, "Base Damage", treeList, NULL, false),
    m_attackBonus(Breakdown_WeaponAttackBonus, Effect_Weapon_Attack, "Attack Bonus", treeList, NULL, slot == Inventory_Weapon2, false),
    m_damageBonus(Breakdown_WeaponDamageBonus, Effect_Weapon_Damage, "Damage Bonus", treeList, NULL, slot == Inventory_Weapon2, false),
    //m_otherDamageEffects(pPane, Breakdown_WeaponOtherDamageEffects, treeList, NULL, false),
    m_vorpalRange(Breakdown_WeaponVorpalRange, treeList, NULL),
    m_criticalAttackBonus(Breakdown_WeaponCriticalAttackBonus, Effect_Weapon_AttackCritical, "Critical Attack Bonus", treeList, NULL, slot == Inventory_Weapon2, true),
    m_criticalDamageBonus(Breakdown_WeaponCriticalDamageBonus, Effect_Weapon_DamageCritical, "Critical Damage Bonus", treeList, NULL, slot == Inventory_Weapon2, true),
    //m_otherCriticalDamageEffects(pPane, Breakdown_WeaponCriticalOtherDamageEffects, treeList, NULL, false),
    m_criticalThreatRange(Breakdown_WeaponCriticalThreatRange, treeList, NULL),
    m_criticalMultiplier(Breakdown_WeaponCriticalMultiplier,  treeList, NULL, NULL),
    m_criticalMultiplier19To20(Breakdown_WeaponCriticalMultiplier19To20, treeList, NULL, &m_criticalMultiplier),
    m_attackSpeed(pPane, Breakdown_WeaponAttackSpeed, Effect_Weapon_Alacrity, "Attack Speed", treeList, NULL),
    m_ghostTouch(pPane, Breakdown_WeaponGhostTouch, Effect_GhostTouch, "Ghost Touch", treeList, NULL, false),
    m_trueSeeing(pPane, Breakdown_WeaponTrueSeeing, Effect_TrueSeeing, "True Seeing", treeList, NULL, false),
    m_drBypass(pPane, Breakdown_DRBypass, treeList, NULL),
    m_weaponCriticalMuliplier(weaponCriticalMultiplier)
{
    SetInventorySlotType(slot);
    m_weaponEnchantment.SetInventorySlotType(slot);
    m_baseDamage.SetInventorySlotType(slot);
    m_attackBonus.SetInventorySlotType(slot);
    m_damageBonus.SetInventorySlotType(slot);
    //m_otherDamageEffects.SetInventorySlotType(slot);
    m_vorpalRange.SetInventorySlotType(slot);
    m_criticalThreatRange.SetInventorySlotType(slot);
    m_criticalAttackBonus.SetInventorySlotType(slot);
    m_criticalDamageBonus.SetInventorySlotType(slot);
    //m_otherCriticalDamageEffects.SetInventorySlotType(slot);
    m_criticalMultiplier.SetInventorySlotType(slot);
    m_criticalMultiplier19To20.SetInventorySlotType(slot);
    m_attackSpeed.SetInventorySlotType(slot);
    m_ghostTouch.SetInventorySlotType(slot);
    m_trueSeeing.SetInventorySlotType(slot);
    m_drBypass.SetInventorySlotType(slot);

    SetWeapon(weaponType, weaponCriticalMultiplier);
    m_weaponEnchantment.SetWeapon(weaponType, weaponCriticalMultiplier);
    m_baseDamage.SetWeapon(weaponType, weaponCriticalMultiplier);
    m_attackBonus.SetWeapon(weaponType, weaponCriticalMultiplier);
    m_damageBonus.SetWeapon(weaponType, weaponCriticalMultiplier);
    //m_otherDamageEffects.SetWeapon(weaponType, weaponCriticalMultiplier);
    m_vorpalRange.SetWeapon(weaponType, weaponCriticalMultiplier);
    m_criticalThreatRange.SetWeapon(weaponType, weaponCriticalMultiplier);
    m_criticalAttackBonus.SetWeapon(weaponType, weaponCriticalMultiplier);
    m_criticalDamageBonus.SetWeapon(weaponType, weaponCriticalMultiplier);
    //m_otherCriticalDamageEffects.SetWeapon(weaponType, weaponCriticalMultiplier);
    m_criticalMultiplier.SetWeapon(weaponType, weaponCriticalMultiplier);
    m_criticalMultiplier19To20.SetWeapon(weaponType, weaponCriticalMultiplier);
    m_attackSpeed.SetWeapon(weaponType, weaponCriticalMultiplier);
    m_ghostTouch.SetWeapon(weaponType, weaponCriticalMultiplier);
    m_trueSeeing.SetWeapon(weaponType, weaponCriticalMultiplier);
    m_drBypass.SetWeapon(weaponType, weaponCriticalMultiplier);

    // we need to update if any of our sub-items update also
    m_weaponEnchantment.AttachObserver(this);
    m_baseDamage.AttachObserver(this);
    m_attackBonus.AttachObserver(this);
    m_damageBonus.AttachObserver(this);
    //m_otherDamageEffects.AttachObserver(this);
    m_vorpalRange.AttachObserver(this);
    m_criticalThreatRange.AttachObserver(this);
    m_criticalAttackBonus.AttachObserver(this);
    m_criticalDamageBonus.AttachObserver(this);
    //m_otherCriticalDamageEffects.AttachObserver(this);
    m_criticalMultiplier.AttachObserver(this);
    m_criticalMultiplier19To20.AttachObserver(this);
    m_attackSpeed.AttachObserver(this);
    m_ghostTouch.AttachObserver(this);
    m_trueSeeing.AttachObserver(this);
    m_drBypass.AttachObserver(this);


    if (IsShield(weaponType))
    {
        AddTreeItem("Shield Enhancement", "", &m_weaponEnchantment);
    }
    else
    {
        AddTreeItem("Weapon Enhancement", "", &m_weaponEnchantment);
    }
    std::string strDamageDice = (LPCTSTR)m_damageDice.DiceAsText();
    AddTreeItem("Weapon Dice", strDamageDice, NULL);           // no breakdown, just a dice number
    AddTreeItem("Base Damage", "", &m_baseDamage);
    AddTreeItem("Attack Bonus", "", &m_attackBonus);
    AddTreeItem("Damage Bonus", "", &m_damageBonus);
    //AddTreeItem("Other Damage Effects", "", &m_otherDamageEffects);
    AddTreeItem("Critical Attack Bonus", "", &m_criticalAttackBonus);
    AddTreeItem("Critical Damage Bonus", "", &m_criticalDamageBonus);
    //AddTreeItem("Other Critical Damage Effects", "", &m_otherCriticalDamageEffects);
    AddTreeItem("Vorpal Range", "", &m_vorpalRange);
    AddTreeItem("Critical Threat Range", "", &m_criticalThreatRange);
    AddTreeItem("Critical Multiplier", "", &m_criticalMultiplier);
    AddTreeItem("Critical Multiplier (19-20)", "", &m_criticalMultiplier19To20);
    AddTreeItem("Attack Speed", "", &m_attackSpeed);
    AddTreeItem("Ghost Touch", "", &m_ghostTouch);
    AddTreeItem("True Seeing", "", &m_trueSeeing);
    AddTreeItem("DR Bypass", "", &m_drBypass);
}

BreakdownItemWeapon::~BreakdownItemWeapon()
{
}

void BreakdownItemWeapon::BuildChanged(Character* charData)
{
    BreakdownItem::BuildChanged(charData);
    m_weaponEnchantment.BuildChanged(charData);
    m_baseDamage.BuildChanged(charData);
    m_attackBonus.BuildChanged(charData);
    m_damageBonus.BuildChanged(charData);
    //m_otherDamageEffects.BuildChanged(charData);
    m_vorpalRange.BuildChanged(charData);
    m_criticalThreatRange.BuildChanged(charData);
    m_criticalAttackBonus.BuildChanged(charData);
    m_criticalDamageBonus.BuildChanged(charData);
    //m_otherCriticalDamageEffects.BuildChanged(charData);
    m_criticalMultiplier.BuildChanged(charData);
    m_criticalMultiplier19To20.BuildChanged(charData);
    m_attackSpeed.BuildChanged(charData);
    m_ghostTouch.BuildChanged(charData);
    m_trueSeeing.BuildChanged(charData);
    m_drBypass.BuildChanged(charData);
}

void BreakdownItemWeapon::AddTreeItem(
        const std::string & entry,
        const std::string & total,
        BreakdownItem * pBreakdown)
{
    HTREEITEM hItem = m_pTreeList->InsertItem(
            entry.c_str(),
            m_hItem,
            TVI_LAST);
    m_pTreeList->SetItemText(hItem, 1, total.c_str());
    m_pTreeList->SetItemData(hItem, (DWORD)(void*)pBreakdown);
    if (pBreakdown != NULL)
    {
        pBreakdown->SetHTreeItem(hItem);
    }
}

bool BreakdownItemWeapon::IsCentering() const
{
    bool isCentering = false;
    if (m_pCharacter != NULL)
    {
        isCentering = m_pCharacter->ActiveBuild()->IsWeaponInGroup("Centered", Weapon());
    }
    return isCentering;
}

void BreakdownItemWeapon::SetCharacter(Character * pCharacter)
{
    BreakdownItem::BuildChanged(pCharacter);
    m_weaponEnchantment.BuildChanged(pCharacter);        // we handle this for them
    m_baseDamage.BuildChanged(pCharacter);        // we handle this for them
    m_attackBonus.BuildChanged(pCharacter);        // we handle this for them
    m_damageBonus.BuildChanged(pCharacter);        // we handle this for them
    //m_otherDamageEffects.BuildChanged(pCharacter);
    m_vorpalRange.BuildChanged(pCharacter);        // we handle this for them
    m_criticalThreatRange.BuildChanged(pCharacter);        // we handle this for them
    m_criticalAttackBonus.BuildChanged(pCharacter);        // we handle this for them
    m_criticalDamageBonus.BuildChanged(pCharacter);        // we handle this for them
    //m_otherCriticalDamageEffects.BuildChanged(pCharacter);        // we handle this for them
    m_criticalMultiplier.BuildChanged(pCharacter);        // we handle this for them
    m_criticalMultiplier19To20.BuildChanged(pCharacter);        // we handle this for them
    m_attackSpeed.BuildChanged(pCharacter);
    m_ghostTouch.BuildChanged(pCharacter);
    m_trueSeeing.BuildChanged(pCharacter);
    m_drBypass.BuildChanged(pCharacter);
}

// required overrides
CString BreakdownItemWeapon::Title() const
{
    if (IsCentering())
    {
        return m_title + " (Centering)";
    }
    return m_title;
}

CString BreakdownItemWeapon::Value() const
{
    CString valueRegular;
    // regular hit value
    valueRegular.Format(
            "%.2f[%s]+%d %s",
            m_baseDamage.Total(),
            (LPCTSTR)m_damageDice.DiceAsText(),
            (int)m_damageBonus.Total(),
            "NYI"); // m_otherDamageEffects.Value());
    // critical hit value
    //CString valueCrit;
    //valueCrit.Format(
    //        "%s (%.2f[%s]+%d) * %d %s",
    //        m_criticalThreatRange.Value(),
    //        m_baseDamage.Total(),
    //        m_damageDice.Description(1).c_str(),
    //        (int)m_criticalDamageBonus.Total(),
    //        (int)m_criticalMultiplier.Total(),
    //        m_otherCriticalDamageEffects.Value());
    //valueCrit.Replace("-20", "-18");
    //CString valueCrit1920;
    //valueCrit1920.Format(
    //        "19-20 (%.2f[%s]+%d) * %d %s",
    //        m_baseDamage.Total(),
    //        m_damageDice.Description(1).c_str(),
    //        (int)m_criticalDamageBonus.Total(),
    //        (int)m_criticalMultiplier19To20.Total(),
    //        m_otherCriticalDamageEffects.Value());
    CString value;
    value.Format("%s; %s; %s", (LPCTSTR)valueRegular, "NYI", "NYI"); // valueCrit, valueCrit1920);
    return value;
}

void BreakdownItemWeapon::CreateOtherEffects()
{
    // does nothing in this holder class
}

bool BreakdownItemWeapon::AffectsUs(const Effect& effect) const
{
    UNREFERENCED_PARAMETER(effect);
    bool isUs = true;
    return isUs;
}

void BreakdownItemWeapon::FeatEffectApplied(
        Build* pBuild,
        const Effect& effect)
{
    // handle special affects that change our list of available stats
    if (AffectsUs(effect))
    {
        // pass through to all our sub breakdowns
        m_weaponEnchantment.FeatEffectApplied(pBuild, effect);
        m_baseDamage.FeatEffectApplied(pBuild, effect);
        m_attackBonus.FeatEffectApplied(pBuild, effect);
        m_damageBonus.FeatEffectApplied(pBuild, effect);
        //m_otherDamageEffects.FeatEffectApplied(pBuild, effect);
        m_vorpalRange.FeatEffectApplied(pBuild, effect);
        m_criticalThreatRange.FeatEffectApplied(pBuild, effect);
        m_criticalAttackBonus.FeatEffectApplied(pBuild, effect);
        m_criticalDamageBonus.FeatEffectApplied(pBuild, effect);
        //m_otherCriticalDamageEffects.FeatEffectApplied(pBuild, effect);
        m_criticalMultiplier.FeatEffectApplied(pBuild, effect);
        m_criticalMultiplier19To20.FeatEffectApplied(pBuild, effect);
        m_attackSpeed.FeatEffectApplied(pBuild, effect);
        m_ghostTouch.FeatEffectApplied(pBuild, effect);
        m_trueSeeing.FeatEffectApplied(pBuild, effect);
        m_drBypass.FeatEffectApplied(pBuild, effect);
    }
}

void BreakdownItemWeapon::FeatEffectRevoked(
    Build* pBuild,
    const Effect& effect)
{
    // handle special affects that change our list of available stats
    if (AffectsUs(effect))
    {
        // pass through to all our sub breakdowns
        m_weaponEnchantment.FeatEffectRevoked(pBuild, effect);
        m_baseDamage.FeatEffectRevoked(pBuild, effect);
        m_attackBonus.FeatEffectRevoked(pBuild, effect);
        m_damageBonus.FeatEffectRevoked(pBuild, effect);
        //m_otherDamageEffects.FeatEffectRevoked(pBuild, effect);
        m_vorpalRange.FeatEffectRevoked(pBuild, effect);
        m_criticalThreatRange.FeatEffectRevoked(pBuild, effect);
        m_criticalAttackBonus.FeatEffectRevoked(pBuild, effect);
        m_criticalDamageBonus.FeatEffectRevoked(pBuild, effect);
        //m_otherCriticalDamageEffects.FeatEffectRevoked(pBuild, effect);
        m_criticalMultiplier.FeatEffectRevoked(pBuild, effect);
        m_criticalMultiplier19To20.FeatEffectRevoked(pBuild, effect);
        m_attackSpeed.FeatEffectRevoked(pBuild, effect);
        m_ghostTouch.FeatEffectRevoked(pBuild, effect);
        m_trueSeeing.FeatEffectRevoked(pBuild, effect);
        m_drBypass.FeatEffectRevoked(pBuild, effect);
    }
}

void BreakdownItemWeapon::ItemEffectApplied(
    Build* pBuild,
    const Effect& effect)
{
    // handle special affects that change our list of available stats
    if (AffectsUs(effect))
    {
        // pass through to all our sub breakdowns
        m_weaponEnchantment.ItemEffectApplied(pBuild, effect);
        m_baseDamage.ItemEffectApplied(pBuild, effect);
        m_attackBonus.ItemEffectApplied(pBuild, effect);
        m_damageBonus.ItemEffectApplied(pBuild, effect);
        //m_otherDamageEffects.ItemEffectApplied(pBuild, effect);
        m_vorpalRange.ItemEffectApplied(pBuild, effect);
        m_criticalThreatRange.ItemEffectApplied(pBuild, effect);
        m_criticalAttackBonus.ItemEffectApplied(pBuild, effect);
        m_criticalDamageBonus.ItemEffectApplied(pBuild, effect);
        //m_otherCriticalDamageEffects.ItemEffectApplied(pBuild, effect);
        m_criticalMultiplier.ItemEffectApplied(pBuild, effect);
        m_criticalMultiplier19To20.ItemEffectApplied(pBuild, effect);
        m_attackSpeed.ItemEffectApplied(pBuild, effect);
        m_ghostTouch.ItemEffectApplied(pBuild, effect);
        m_trueSeeing.ItemEffectApplied(pBuild, effect);
        m_drBypass.ItemEffectApplied(pBuild, effect);
    }
}

void BreakdownItemWeapon::ItemEffectRevoked(
    Build* pBuild,
    const Effect& effect)
{
    // handle special affects that change our list of available stats
    if (AffectsUs(effect))
    {
        // pass through to all our sub breakdowns
        m_weaponEnchantment.ItemEffectRevoked(pBuild, effect);
        m_baseDamage.ItemEffectRevoked(pBuild, effect);
        m_attackBonus.ItemEffectRevoked(pBuild, effect);
        m_damageBonus.ItemEffectRevoked(pBuild, effect);
        //m_otherDamageEffects.ItemEffectRevoked(pBuild, effect);
        m_vorpalRange.ItemEffectRevoked(pBuild, effect);
        m_criticalThreatRange.ItemEffectRevoked(pBuild, effect);
        m_criticalAttackBonus.ItemEffectRevoked(pBuild, effect);
        m_criticalDamageBonus.ItemEffectRevoked(pBuild, effect);
        //m_otherCriticalDamageEffects.ItemEffectRevoked(pBuild, effect);
        m_criticalMultiplier.ItemEffectRevoked(pBuild, effect);
        m_criticalMultiplier19To20.ItemEffectRevoked(pBuild, effect);
        m_attackSpeed.ItemEffectRevoked(pBuild, effect);
        m_ghostTouch.ItemEffectRevoked(pBuild, effect);
        m_trueSeeing.ItemEffectRevoked(pBuild, effect);
        m_drBypass.ItemEffectRevoked(pBuild, effect);
    }
}

void BreakdownItemWeapon::EnhancementTrained(
    Build* pBuild,
    const EnhancementItemParams& item)
{
    // pass through to all our sub breakdowns
    m_weaponEnchantment.EnhancementTrained(pBuild, item);
    m_baseDamage.EnhancementTrained(pBuild, item);
    m_attackBonus.EnhancementTrained(pBuild, item);
    m_damageBonus.EnhancementTrained(pBuild, item);
    //m_otherDamageEffects.EnhancementTrained(pBuild, item);
    m_vorpalRange.EnhancementTrained(pBuild, item);
    m_criticalThreatRange.EnhancementTrained(pBuild, item);
    m_criticalAttackBonus.EnhancementTrained(pBuild, item);
    m_criticalDamageBonus.EnhancementTrained(pBuild, item);
    //m_otherCriticalDamageEffects.EnhancementTrained(pBuild, item);
    m_criticalMultiplier.EnhancementTrained(pBuild, item);
    m_criticalMultiplier19To20.EnhancementTrained(pBuild, item);
    m_attackSpeed.EnhancementTrained(pBuild, item);
    m_ghostTouch.EnhancementTrained(pBuild, item);
    m_trueSeeing.EnhancementTrained(pBuild, item);
    m_drBypass.EnhancementTrained(pBuild, item);
}

void BreakdownItemWeapon::EnhancementRevoked(
    Build* pBuild,
    const EnhancementItemParams& item)
{
    // pass through to all our sub breakdowns
    m_weaponEnchantment.EnhancementRevoked(pBuild, item);
    m_baseDamage.EnhancementRevoked(pBuild, item);
    m_attackBonus.EnhancementRevoked(pBuild, item);
    m_damageBonus.EnhancementRevoked(pBuild, item);
    //m_otherDamageEffects.EnhancementRevoked(pBuild, item);
    m_vorpalRange.EnhancementRevoked(pBuild, item);
    m_criticalThreatRange.EnhancementRevoked(pBuild, item);
    m_criticalAttackBonus.EnhancementRevoked(pBuild, item);
    m_criticalDamageBonus.EnhancementRevoked(pBuild, item);
    //m_otherCriticalDamageEffects.EnhancementRevoked(pBuild, item);
    m_criticalMultiplier.EnhancementRevoked(pBuild, item);
    m_criticalMultiplier19To20.EnhancementRevoked(pBuild, item);
    m_attackSpeed.EnhancementRevoked(pBuild, item);
    m_ghostTouch.EnhancementRevoked(pBuild, item);
    m_trueSeeing.EnhancementRevoked(pBuild, item);
    m_drBypass.EnhancementRevoked(pBuild, item);
}

void BreakdownItemWeapon::EnhancementEffectApplied(
    Build* pBuild,
    const Effect& effect)
{
    // handle special affects that change our list of available stats
    if (AffectsUs(effect))
    {
        // pass through to all our sub breakdowns
        m_weaponEnchantment.EnhancementEffectApplied(pBuild, effect);
        m_baseDamage.EnhancementEffectApplied(pBuild, effect);
        m_attackBonus.EnhancementEffectApplied(pBuild, effect);
        m_damageBonus.EnhancementEffectApplied(pBuild, effect);
        //m_otherDamageEffects.EnhancementEffectApplied(pBuild, effect);
        m_vorpalRange.EnhancementEffectApplied(pBuild, effect);
        m_criticalThreatRange.EnhancementEffectApplied(pBuild, effect);
        m_criticalAttackBonus.EnhancementEffectApplied(pBuild, effect);
        m_criticalDamageBonus.EnhancementEffectApplied(pBuild, effect);
        //m_otherCriticalDamageEffects.EnhancementEffectApplied(pBuild, effect);
        m_criticalMultiplier.EnhancementEffectApplied(pBuild, effect);
        m_criticalMultiplier19To20.EnhancementEffectApplied(pBuild, effect);
        m_attackSpeed.EnhancementEffectApplied(pBuild, effect);
        m_ghostTouch.EnhancementEffectApplied(pBuild, effect);
        m_trueSeeing.EnhancementEffectApplied(pBuild, effect);
        m_drBypass.EnhancementEffectApplied(pBuild, effect);
    }
}

void BreakdownItemWeapon::EnhancementEffectRevoked(
    Build* pBuild,
    const Effect& effect)
{
    // handle special affects that change our list of available stats
    if (AffectsUs(effect))
    {
        // pass through to all our sub breakdowns
        m_weaponEnchantment.EnhancementEffectRevoked(pBuild, effect);
        m_baseDamage.EnhancementEffectRevoked(pBuild, effect);
        m_attackBonus.EnhancementEffectRevoked(pBuild, effect);
        m_damageBonus.EnhancementEffectRevoked(pBuild, effect);
        //m_otherDamageEffects.EnhancementEffectRevoked(pBuild, effect);
        m_vorpalRange.EnhancementEffectRevoked(pBuild, effect);
        m_criticalThreatRange.EnhancementEffectRevoked(pBuild, effect);
        m_criticalAttackBonus.EnhancementEffectRevoked(pBuild, effect);
        m_criticalDamageBonus.EnhancementEffectRevoked(pBuild, effect);
        //m_otherCriticalDamageEffects.EnhancementEffectRevoked(pBuild, effect);
        m_criticalMultiplier.EnhancementEffectRevoked(pBuild, effect);
        m_criticalMultiplier19To20.EnhancementEffectRevoked(pBuild, effect);
        m_attackSpeed.EnhancementEffectRevoked(pBuild, effect);
        m_ghostTouch.EnhancementEffectRevoked(pBuild, effect);
        m_trueSeeing.EnhancementEffectRevoked(pBuild, effect);
        m_drBypass.EnhancementEffectRevoked(pBuild, effect);
    }
}

void BreakdownItemWeapon::UpdateTotalChanged(
        BreakdownItem * item,
        BreakdownType type)
{
    // do base class stuff also
    BreakdownItem::UpdateTotalChanged(item, type);
    Populate();
}

void BreakdownItemWeapon::SetWeaponTypes(
        WeaponType wtMain,
        WeaponType wtOffhand) 
{
    BreakdownItem::SetWeaponTypes(wtMain, wtOffhand);
    m_weaponEnchantment.SetWeaponTypes(wtMain, wtOffhand);
    m_baseDamage.SetWeaponTypes(wtMain, wtOffhand);
    m_attackBonus.SetWeaponTypes(wtMain, wtOffhand);
    m_damageBonus.SetWeaponTypes(wtMain, wtOffhand);
    //m_otherDamageEffects.SetWeaponTypes(wtMain, wtOffhand);
    m_criticalThreatRange.SetWeaponTypes(wtMain, wtOffhand);
    m_vorpalRange.SetWeaponTypes(wtMain, wtOffhand);
    m_criticalAttackBonus.SetWeaponTypes(wtMain, wtOffhand);
    m_criticalDamageBonus.SetWeaponTypes(wtMain, wtOffhand);
    //m_otherCriticalDamageEffects.SetWeaponTypes(wtMain, wtOffhand);
    m_criticalMultiplier.SetWeaponTypes(wtMain, wtOffhand);
    m_criticalMultiplier19To20.SetWeaponTypes(wtMain, wtOffhand);
    m_drBypass.SetWeaponTypes(wtMain, wtOffhand);
    m_attackSpeed.SetWeaponTypes(wtMain, wtOffhand);
    m_ghostTouch.SetWeaponTypes(wtMain, wtOffhand);
    m_trueSeeing.SetWeaponTypes(wtMain, wtOffhand);
}

void BreakdownItemWeapon::ClassChanged(
    Build* pBuild, const std::string& classFrom, const std::string& classTo, size_t level)
{
    BreakdownItem::ClassChanged(pBuild, classFrom, classTo, level);
    // pass through to all our sub breakdowns
    m_weaponEnchantment.ClassChanged(pBuild, classFrom, classTo, level);
    m_baseDamage.ClassChanged(pBuild, classFrom, classTo, level);
    m_attackBonus.ClassChanged(pBuild, classFrom, classTo, level);
    m_damageBonus.ClassChanged(pBuild, classFrom, classTo, level);
    //m_otherDamageEffects.ClassChanged(pBuild, classFrom, classTo, level);
    m_vorpalRange.ClassChanged(pBuild, classFrom, classTo, level);
    m_criticalThreatRange.ClassChanged(pBuild, classFrom, classTo, level);
    m_criticalAttackBonus.ClassChanged(pBuild, classFrom, classTo, level);
    m_criticalDamageBonus.ClassChanged(pBuild, classFrom, classTo, level);
    //m_otherCriticalDamageEffects.ClassChanged(pBuild, classFrom, classTo, level);
    m_criticalMultiplier.ClassChanged(pBuild, classFrom, classTo, level);
    m_criticalMultiplier19To20.ClassChanged(pBuild, classFrom, classTo, level);
    m_attackSpeed.ClassChanged(pBuild, classFrom, classTo, level);
    m_ghostTouch.ClassChanged(pBuild, classFrom, classTo, level);
    m_trueSeeing.ClassChanged(pBuild, classFrom, classTo, level);
    m_drBypass.ClassChanged(pBuild, classFrom, classTo, level);
}

void BreakdownItemWeapon::FeatTrained(
        Build* pBuild,
        const std::string& featName)
{
    BreakdownItem::FeatTrained(pBuild, featName);
    // pass through to all our sub breakdowns
    m_weaponEnchantment.FeatTrained(pBuild, featName);
    m_baseDamage.FeatTrained(pBuild, featName);
    m_attackBonus.FeatTrained(pBuild, featName);
    m_damageBonus.FeatTrained(pBuild, featName);
    //m_otherDamageEffects.FeatTrained(pBuild, featName);
    m_vorpalRange.FeatTrained(pBuild, featName);
    //m_criticalThreatRange.FeatTrained(pBuild, featName);
    m_criticalAttackBonus.FeatTrained(pBuild, featName);
    m_criticalDamageBonus.FeatTrained(pBuild, featName);
    //m_otherCriticalDamageEffects.FeatTrained(pBuild, featName);
    m_criticalMultiplier.FeatTrained(pBuild, featName);
    m_criticalMultiplier19To20.FeatTrained(pBuild, featName);
    m_attackSpeed.FeatTrained(pBuild, featName);
    m_ghostTouch.FeatTrained(pBuild, featName);
    m_trueSeeing.FeatTrained(pBuild, featName);
    m_drBypass.FeatTrained(pBuild, featName);
}

void BreakdownItemWeapon::FeatRevoked(
    Build* pBuild,
    const std::string& featName)
{
    BreakdownItem::FeatRevoked(pBuild, featName);
    // pass through to all our sub breakdowns
    m_weaponEnchantment.FeatRevoked(pBuild, featName);
    m_baseDamage.FeatRevoked(pBuild, featName);
    m_attackBonus.FeatRevoked(pBuild, featName);
    m_damageBonus.FeatRevoked(pBuild, featName);
    //m_otherDamageEffects.FeatRevoked(pBuild, featName);
    m_vorpalRange.FeatRevoked(pBuild, featName);
    m_criticalThreatRange.FeatRevoked(pBuild, featName);
    m_criticalAttackBonus.FeatRevoked(pBuild, featName);
    m_criticalDamageBonus.FeatRevoked(pBuild, featName);
    //m_otherCriticalDamageEffects.FeatRevoked(pBuild, featName);
    m_criticalMultiplier.FeatRevoked(pBuild, featName);
    m_criticalMultiplier19To20.FeatRevoked(pBuild, featName);
    m_attackSpeed.FeatRevoked(pBuild, featName);
    m_ghostTouch.FeatRevoked(pBuild, featName);
    m_trueSeeing.FeatRevoked(pBuild, featName);
    m_drBypass.FeatRevoked(pBuild, featName);
}

void BreakdownItemWeapon::SliderChanged(
    Build* pBuild,
    const std::string& sliderName,
    int newValue)
{
    BreakdownItem::SliderChanged(pBuild, sliderName, newValue);
    // pass through to all our sub breakdowns
    m_weaponEnchantment.SliderChanged(pBuild, sliderName, newValue);
    m_baseDamage.SliderChanged(pBuild, sliderName, newValue);
    m_attackBonus.SliderChanged(pBuild, sliderName, newValue);
    m_damageBonus.SliderChanged(pBuild, sliderName, newValue);
    //m_otherDamageEffects.SliderChanged(pBuild, sliderName, newValue);
    m_vorpalRange.SliderChanged(pBuild, sliderName, newValue);
    m_criticalThreatRange.SliderChanged(pBuild, sliderName, newValue);
    m_criticalAttackBonus.SliderChanged(pBuild, sliderName, newValue);
    m_criticalDamageBonus.SliderChanged(pBuild, sliderName, newValue);
    //m_otherCriticalDamageEffects.SliderChanged(pBuild, sliderName, newValue);
    m_criticalMultiplier.SliderChanged(pBuild, sliderName, newValue);
    m_criticalMultiplier19To20.SliderChanged(pBuild, sliderName, newValue);
    m_attackSpeed.SliderChanged(pBuild, sliderName, newValue);
    m_ghostTouch.SliderChanged(pBuild, sliderName, newValue);
    m_trueSeeing.SliderChanged(pBuild, sliderName, newValue);
    m_drBypass.SliderChanged(pBuild, sliderName, newValue);
}

void BreakdownItemWeapon::StanceActivated(
    Build* pBuild,
    const std::string& stanceName)
{
    BreakdownItem::StanceActivated(pBuild, stanceName);
    // pass through to all our sub breakdowns
    m_weaponEnchantment.StanceActivated(pBuild, stanceName);
    m_baseDamage.StanceActivated(pBuild, stanceName);
    m_attackBonus.StanceActivated(pBuild, stanceName);
    m_damageBonus.StanceActivated(pBuild, stanceName);
    //m_otherDamageEffects.StanceActivated(pBuild, stanceName);
    m_vorpalRange.StanceActivated(pBuild, stanceName);
    m_criticalThreatRange.StanceActivated(pBuild, stanceName);
    m_criticalAttackBonus.StanceActivated(pBuild, stanceName);
    m_criticalDamageBonus.StanceActivated(pBuild, stanceName);
    //m_otherCriticalDamageEffects.StanceActivated(pBuild, stanceName);
    m_criticalMultiplier.StanceActivated(pBuild, stanceName);
    m_criticalMultiplier19To20.StanceActivated(pBuild, stanceName);
    m_attackSpeed.StanceActivated(pBuild, stanceName);
    m_ghostTouch.StanceActivated(pBuild, stanceName);
    m_trueSeeing.StanceActivated(pBuild, stanceName);
    m_drBypass.StanceActivated(pBuild, stanceName);
}

void BreakdownItemWeapon::StanceDeactivated(
    Build* pBuild,
    const std::string& stanceName)
{
    BreakdownItem::StanceDeactivated(pBuild, stanceName);
    // pass through to all our sub breakdowns
    m_weaponEnchantment.StanceDeactivated(pBuild, stanceName);
    m_baseDamage.StanceDeactivated(pBuild, stanceName);
    m_attackBonus.StanceDeactivated(pBuild, stanceName);
    m_damageBonus.StanceDeactivated(pBuild, stanceName);
    //m_otherDamageEffects.StanceDeactivated(pBuild, stanceName);
    m_vorpalRange.StanceDeactivated(pBuild, stanceName);
    m_criticalThreatRange.StanceDeactivated(pBuild, stanceName);
    m_criticalAttackBonus.StanceDeactivated(pBuild, stanceName);
    m_criticalDamageBonus.StanceDeactivated(pBuild, stanceName);
    //m_otherCriticalDamageEffects.StanceDeactivated(pBuild, stanceName);
    m_criticalMultiplier.StanceDeactivated(pBuild, stanceName);
    m_criticalMultiplier19To20.StanceDeactivated(pBuild, stanceName);
    m_attackSpeed.StanceDeactivated(pBuild, stanceName);
    m_ghostTouch.StanceDeactivated(pBuild, stanceName);
    m_trueSeeing.StanceDeactivated(pBuild, stanceName);
    m_drBypass.StanceDeactivated(pBuild, stanceName);
}

void BreakdownItemWeapon::AddForumExportData(std::stringstream & forumExport)
{
    UNREFERENCED_PARAMETER(forumExport);
    forumExport << Title() << "\r\n";
    CString valueRegular;
    // regular hit value
    valueRegular.Format(
            "On Hit         %.2f[%s]+%d\r\n",
            m_baseDamage.Total(),
            (LPCTSTR)m_damageDice.DiceAsText(),
            (int)m_damageBonus.Total());
    forumExport << valueRegular;
    // critical hit value
    CString valueCrit;
    valueCrit.Format(
            "Critical %s (%.2f[%s]+%d) * %d\r\n",
            (LPCTSTR)m_criticalThreatRange.Value(),
            m_baseDamage.Total(),
            (LPCTSTR)m_damageDice.DiceAsText(),
            (int)m_criticalDamageBonus.Total(),
            (int)m_criticalMultiplier.Total());
    valueCrit.Replace("-20", "-18");
    forumExport << valueCrit;
    CString valueCrit1920;
    valueCrit1920.Format(
            "Critical 19-20 (%.2f[%s]+%d) * %d\r\n",
            m_baseDamage.Total(),
            (LPCTSTR)m_damageDice.DiceAsText(),
            (int)m_criticalDamageBonus.Total(),
            (int)m_criticalMultiplier19To20.Total());
    forumExport << valueCrit1920;
    forumExport << "DR Bypass: " << m_drBypass.Value();
    forumExport << "\r\n";
    if (m_ghostTouch.Total() > 0)
    {
        forumExport << "You bypass Incorporeality miss chance with this weapon\r\n";
    }
    if (m_trueSeeing.Total() > 0)
    {
        forumExport << "You bypass Concealment miss chance with this weapon\r\n";
    }
    forumExport << "\r\n";
}

BreakdownItem * BreakdownItemWeapon::GetWeaponBreakdown(BreakdownType bt)
{
    UNREFERENCED_PARAMETER(bt);
    BreakdownItem * pBI = NULL;
    switch (bt)
    {
    case Breakdown_WeaponEnchantment:
        pBI = &m_weaponEnchantment;
        break;
    case Breakdown_WeaponBaseDamage:
        pBI = &m_baseDamage;
        break;
    case Breakdown_WeaponAttackBonus:
        pBI = &m_attackBonus;
        break;
    case Breakdown_WeaponDamageBonus:
        pBI = &m_damageBonus;
        break;
    //case Breakdown_WeaponOtherDamageEffects:
    //    pBI = &m_otherDamageEffects;
    //    break;
    case Breakdown_WeaponCriticalAttackBonus:
        pBI = &m_criticalAttackBonus;
        break;
    case Breakdown_WeaponCriticalDamageBonus:
        pBI = &m_criticalDamageBonus;
        break;
    //case Breakdown_WeaponCriticalOtherDamageEffects:
    //    pBI = &m_otherCriticalDamageEffects;
    //    break;
    case Breakdown_WeaponVorpalRange:
        pBI = &m_vorpalRange;
        break;
    case Breakdown_WeaponCriticalThreatRange:
        pBI = &m_criticalThreatRange;
        break;
    case Breakdown_WeaponCriticalMultiplier:
        pBI = &m_criticalMultiplier;
        break;
    case Breakdown_WeaponCriticalMultiplier19To20:
        pBI = &m_criticalMultiplier19To20;
        break;
    case Breakdown_WeaponAttackSpeed:
        pBI = &m_attackSpeed;
        break;
    case Breakdown_WeaponGhostTouch:
        pBI = &m_ghostTouch;
        break;
    case Breakdown_WeaponTrueSeeing:
        pBI = &m_trueSeeing;
        break;
    case Breakdown_DRBypass:
        pBI = &m_drBypass;
        break;
    }
    return pBI;
}

void BreakdownItemWeapon::PopulateStartValues()
{
    m_weaponEnchantment.Populate();
    m_baseDamage.Populate();
    m_attackBonus.Populate();
    m_damageBonus.Populate();
    //m_otherDamageEffects.Populate();
    m_criticalThreatRange.Populate();
    m_vorpalRange.Populate();
    m_criticalAttackBonus.Populate();
    m_criticalDamageBonus.Populate();
    //m_otherCriticalDamageEffects.Populate();
    m_criticalMultiplier.Populate();
    m_criticalMultiplier19To20.Populate();
    m_drBypass.Populate();
    m_attackSpeed.Populate();
    m_ghostTouch.Populate();
    m_trueSeeing.Populate();
}

void BreakdownItemWeapon::LinkUp()
{
    // make sure all items are correctly monitoring each other as required
    m_weaponEnchantment.LinkUp();
    m_baseDamage.LinkUp();
    m_attackBonus.LinkUp();
    m_damageBonus.LinkUp();
    //m_otherDamageEffects.LinkUp();
    m_criticalThreatRange.LinkUp();
    m_vorpalRange.LinkUp();
    m_criticalAttackBonus.LinkUp();
    m_criticalDamageBonus.LinkUp();
    //m_otherCriticalDamageEffects.LinkUp();
    m_criticalMultiplier.LinkUp();
    m_criticalMultiplier19To20.LinkUp();
    m_drBypass.LinkUp();
    m_attackSpeed.LinkUp();
    m_ghostTouch.LinkUp();
    m_trueSeeing.LinkUp();

    // also ensure all items have been created correctly
    m_weaponEnchantment.CreateOtherEffects();
    m_baseDamage.CreateOtherEffects();
    m_attackBonus.CreateOtherEffects();
    m_damageBonus.CreateOtherEffects();
    //m_otherDamageEffects.CreateOtherEffects();
    m_criticalThreatRange.CreateOtherEffects();
    m_vorpalRange.CreateOtherEffects();
    m_criticalAttackBonus.CreateOtherEffects();
    m_criticalDamageBonus.CreateOtherEffects();
    //m_otherCriticalDamageEffects.CreateOtherEffects();
    m_criticalMultiplier.CreateOtherEffects();
    m_criticalMultiplier19To20.CreateOtherEffects();
    m_drBypass.CreateOtherEffects();
    m_attackSpeed.CreateOtherEffects();
    m_ghostTouch.CreateOtherEffects();
    m_trueSeeing.CreateOtherEffects();
}

BasicDice BreakdownItemWeapon::WeaponDice() const
{
    return m_damageDice;
}
