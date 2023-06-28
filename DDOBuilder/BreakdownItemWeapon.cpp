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
    m_baseDamage(pPane, Breakdown_WeaponBaseDamage, Effect_Weapon_BaseDamage, "Base Damage", treeList, NULL, false),
    m_attackBonus(Breakdown_WeaponAttackBonus, Effect_Weapon_AttackBonus, "Attack Bonus", treeList, NULL, slot == Inventory_Weapon2, false),
    m_damageBonus(Breakdown_WeaponDamageBonus, Effect_Weapon_DamageBonus, "Damage Bonus", treeList, NULL, slot == Inventory_Weapon2, false),
    //m_otherDamageEffects(pPane, Breakdown_WeaponOtherDamageEffects, treeList, NULL, false),
    m_criticalAttackBonus(Breakdown_WeaponCriticalAttackBonus, Effect_Weapon_AttackBonusCritical, "Critical Attack Bonus", treeList, NULL, slot == Inventory_Weapon2, true),
    m_criticalDamageBonus(Breakdown_WeaponCriticalDamageBonus, Effect_Weapon_DamageBonusCritical, "Critical Damage Bonus", treeList, NULL, slot == Inventory_Weapon2, true),
    //m_otherCriticalDamageEffects(pPane, Breakdown_WeaponCriticalOtherDamageEffects, treeList, NULL, false),
    m_criticalThreatRange(Breakdown_WeaponCriticalThreatRange, treeList, NULL),
    m_criticalMultiplier(Breakdown_WeaponCriticalMultiplier,  treeList, NULL, NULL),
    m_criticalMultiplier19To20(Breakdown_WeaponCriticalMultiplier19To20, treeList, NULL, &m_criticalMultiplier),
    m_attackSpeed(pPane, Breakdown_WeaponAttackSpeed, Effect_Weapon_Alacrity, "Attack Speed", treeList, NULL, false),
    m_ghostTouch(pPane, Breakdown_WeaponGhostTouch, Effect_GhostTouch, "Ghost Touch", treeList, NULL, false),
    m_trueSeeing(pPane, Breakdown_WeaponTrueSeeing, Effect_TrueSeeing, "True Seeing", treeList, NULL, false),
    m_drBypass(pPane, Breakdown_DRBypass, treeList, NULL),
    m_weaponCriticalMuliplier(weaponCriticalMultiplier)
{
    SetWeapon(weaponType, weaponCriticalMultiplier);
    m_baseDamage.SetWeapon(weaponType, weaponCriticalMultiplier);
    m_attackBonus.SetWeapon(weaponType, weaponCriticalMultiplier);
    m_damageBonus.SetWeapon(weaponType, weaponCriticalMultiplier);
    //m_otherDamageEffects.SetWeapon(weaponType, weaponCriticalMultiplier);
    ////////m_vorpalRange.SetWeapon(weaponType, weaponCriticalMultiplier);
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
    m_baseDamage.AttachObserver(this);
    m_attackBonus.AttachObserver(this);
    m_damageBonus.AttachObserver(this);
    //m_otherDamageEffects.AttachObserver(this);
    ////////m_vorpalRange.AttachObserver(this);
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

    std::string strDamageDice = (LPCTSTR)m_damageDice.DiceAsText();

    AddTreeItem("Weapon Dice", strDamageDice, NULL);           // no breakdown, just a dice number
    AddTreeItem("Base Damage", "", &m_baseDamage);
    AddTreeItem("Attack Bonus", "", &m_attackBonus);
    AddTreeItem("Damage Bonus", "", &m_damageBonus);
    //AddTreeItem("Other Damage Effects", "", &m_otherDamageEffects);
    AddTreeItem("Critical Attack Bonus", "", &m_criticalAttackBonus);
    AddTreeItem("Critical Damage Bonus", "", &m_criticalDamageBonus);
    //AddTreeItem("Other Critical Damage Effects", "", &m_otherCriticalDamageEffects);
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
    m_baseDamage.BuildChanged(charData);
    m_attackBonus.BuildChanged(charData);
    m_damageBonus.BuildChanged(charData);
    //m_otherDamageEffects.BuildChanged(charData);
    ////////m_vorpalRange.BuildChanged(charData);
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
    m_baseDamage.BuildChanged(pCharacter);        // we handle this for them
    m_attackBonus.BuildChanged(pCharacter);        // we handle this for them
    m_damageBonus.BuildChanged(pCharacter);        // we handle this for them
    //m_otherDamageEffects.BuildChanged(pCharacter);
    ///////m_vorpalRange.BuildChanged(pCharacter);        // we handle this for them
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
        m_baseDamage.FeatEffectApplied(pBuild, effect);
        m_attackBonus.FeatEffectApplied(pBuild, effect);
        m_damageBonus.FeatEffectApplied(pBuild, effect);
        //m_otherDamageEffects.FeatEffectApplied(pBuild, effect);
        ///////m_vorpalRange.FeatEffectApplied(pBuild, effect);
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
        m_baseDamage.FeatEffectRevoked(pBuild, effect);
        m_attackBonus.FeatEffectRevoked(pBuild, effect);
        m_damageBonus.FeatEffectRevoked(pBuild, effect);
        //m_otherDamageEffects.FeatEffectRevoked(pBuild, effect);
        ///////m_vorpalRange.FeatEffectRevoked(pBuild, effect);
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
        m_baseDamage.ItemEffectApplied(pBuild, effect);
        m_attackBonus.ItemEffectApplied(pBuild, effect);
        m_damageBonus.ItemEffectApplied(pBuild, effect);
        //m_otherDamageEffects.ItemEffectApplied(pBuild, effect);
        ///////m_vorpalRange.ItemEffectApplied(pBuild, effect);
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
        m_baseDamage.ItemEffectRevoked(pBuild, effect);
        m_attackBonus.ItemEffectRevoked(pBuild, effect);
        m_damageBonus.ItemEffectRevoked(pBuild, effect);
        //m_otherDamageEffects.ItemEffectRevoked(pBuild, effect);
        ////////m_vorpalRange.ItemEffectRevoked(pBuild, effect);
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

void BreakdownItemWeapon::EnhancementEffectApplied(
    Build* pBuild,
    const Effect& effect)
{
    // handle special affects that change our list of available stats
    if (AffectsUs(effect))
    {
        // pass through to all our sub breakdowns
        m_baseDamage.EnhancementEffectApplied(pBuild, effect);
        m_attackBonus.EnhancementEffectApplied(pBuild, effect);
        m_damageBonus.EnhancementEffectApplied(pBuild, effect);
        //m_otherDamageEffects.EnhancementEffectApplied(pBuild, effect);
        ////////m_vorpalRange.EnhancementEffectApplied(pBuild, effect);
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
        m_baseDamage.EnhancementEffectRevoked(pBuild, effect);
        m_attackBonus.EnhancementEffectRevoked(pBuild, effect);
        m_damageBonus.EnhancementEffectRevoked(pBuild, effect);
        //m_otherDamageEffects.EnhancementEffectRevoked(pBuild, effect);
        ////////m_vorpalRange.EnhancementEffectRevoked(pBuild, effect);
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

void BreakdownItemWeapon::ClassChanged(
    Build* pBuild, const std::string& classFrom, const std::string& classTo, size_t level)
{
    BreakdownItem::ClassChanged(pBuild, classFrom, classTo, level);
    // pass through to all our sub breakdowns
    m_baseDamage.ClassChanged(pBuild, classFrom, classTo, level);
    m_attackBonus.ClassChanged(pBuild, classFrom, classTo, level);
    m_damageBonus.ClassChanged(pBuild, classFrom, classTo, level);
    //m_otherDamageEffects.ClassChanged(pBuild, classFrom, classTo, level);
    /////////m_vorpalRange.ClassChanged(pBuild, classFrom, classTo, level);
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

//void BreakdownItemWeapon::UpdateAPSpentInTreeChanged(
//        Character * pBuild,
//        const std::string & treeName)
//{
//    BreakdownItem::UpdateAPSpentInTreeChanged(pBuild, treeName);
//    // pass through to all our sub breakdowns
//    m_baseDamage.UpdateAPSpentInTreeChanged(pBuild, treeName);
//    m_attackBonus.UpdateAPSpentInTreeChanged(pBuild, treeName);
//    m_damageBonus.UpdateAPSpentInTreeChanged(pBuild, treeName);
//    m_otherDamageEffects.UpdateAPSpentInTreeChanged(pBuild, treeName);
//    //m_vorpalRange.UpdateAPSpentInTreeChanged(pBuild, treeName);
//    m_criticalThreatRange.UpdateAPSpentInTreeChanged(pBuild, treeName);
//    m_criticalAttackBonus.UpdateAPSpentInTreeChanged(pBuild, treeName);
//    m_criticalDamageBonus.UpdateAPSpentInTreeChanged(pBuild, treeName);
//    m_otherCriticalDamageEffects.UpdateAPSpentInTreeChanged(pBuild, treeName);
//    m_criticalMultiplier.UpdateAPSpentInTreeChanged(pBuild, treeName);
//    m_criticalMultiplier19To20.UpdateAPSpentInTreeChanged(pBuild, treeName);
//    m_attackSpeed.UpdateAPSpentInTreeChanged(pBuild, treeName);
//    m_ghostTouch.UpdateAPSpentInTreeChanged(pBuild, treeName);
//    m_trueSeeing.UpdateAPSpentInTreeChanged(pBuild, treeName);
//    m_drBypass.UpdateAPSpentInTreeChanged(pBuild, treeName);
//}
//
//void BreakdownItemWeapon::UpdateEnhancementTrained(
//        Character * pBuild,
//        const std::string & enhancementName,
//        const std::string & selection,
//        bool isTier5)
//{
//    BreakdownItem::UpdateEnhancementTrained(pBuild, selection, isTier5);
//    // pass through to all our sub breakdowns
//    m_baseDamage.UpdateEnhancementTrained(pBuild, selection, isTier5);
//    m_attackBonus.UpdateEnhancementTrained(pBuild, selection, isTier5);
//    m_damageBonus.UpdateEnhancementTrained(pBuild, selection, isTier5);
//    m_otherDamageEffects.UpdateEnhancementTrained(pBuild, selection, isTier5);
//    //m_vorpalRange.UpdateEnhancementTrained(pBuild, selection, isTier5);
//    m_criticalThreatRange.UpdateEnhancementTrained(pBuild, selection, isTier5);
//    m_criticalAttackBonus.UpdateEnhancementTrained(pBuild, selection, isTier5);
//    m_criticalDamageBonus.UpdateEnhancementTrained(pBuild, selection, isTier5);
//    m_otherCriticalDamageEffects.UpdateEnhancementTrained(pBuild, selection, isTier5);
//    m_criticalMultiplier.UpdateEnhancementTrained(pBuild, selection, isTier5);
//    m_criticalMultiplier19To20.UpdateEnhancementTrained(pBuild, selection, isTier5);
//    m_attackSpeed.UpdateEnhancementTrained(pBuild, selection, isTier5);
//    m_ghostTouch.UpdateEnhancementTrained(pBuild, selection, isTier5);
//    m_trueSeeing.UpdateEnhancementTrained(pBuild, selection, isTier5);
//    m_drBypass.UpdateEnhancementTrained(pBuild, selection, isTier5);
//    if (enhancementName == "KenseiOneWithTheBlade"
//            || enhancementName == "KenseiExoticWeaponMastery")
//    {
//        // these two enhancements can affect the centering state
//        Populate();
//    }
//}
//
//void BreakdownItemWeapon::UpdateEnhancementRevoked(
//        Character * pBuild,
//        const std::string & enhancementName,
//        const std::string & selection,
//        bool isTier5)
//{
//    BreakdownItem::UpdateEnhancementRevoked(pBuild, selection, isTier5);
//    // pass through to all our sub breakdowns
//    m_baseDamage.UpdateEnhancementRevoked(pBuild, selection, isTier5);
//    m_attackBonus.UpdateEnhancementRevoked(pBuild, selection, isTier5);
//    m_damageBonus.UpdateEnhancementRevoked(pBuild, selection, isTier5);
//    m_otherDamageEffects.UpdateEnhancementRevoked(pBuild, selection, isTier5);
//    //m_vorpalRange.UpdateEnhancementRevoked(pBuild, selection, isTier5);
//    m_criticalThreatRange.UpdateEnhancementRevoked(pBuild, selection, isTier5);
//    m_criticalAttackBonus.UpdateEnhancementRevoked(pBuild, selection, isTier5);
//    m_criticalDamageBonus.UpdateEnhancementRevoked(pBuild, selection, isTier5);
//    m_otherCriticalDamageEffects.UpdateEnhancementRevoked(pBuild, selection, isTier5);
//    m_criticalMultiplier.UpdateEnhancementRevoked(pBuild, selection, isTier5);
//    m_criticalMultiplier19To20.UpdateEnhancementRevoked(pBuild, selection, isTier5);
//    m_attackSpeed.UpdateEnhancementRevoked(pBuild, selection, isTier5);
//    m_ghostTouch.UpdateEnhancementRevoked(pBuild, selection, isTier5);
//    m_trueSeeing.UpdateEnhancementRevoked(pBuild, selection, isTier5);
//    m_drBypass.UpdateEnhancementRevoked(pBuild, selection, isTier5);
//    if (enhancementName == "KenseiOneWithTheBlade"
//            || enhancementName == "KenseiExoticWeaponMastery")
//    {
//        // these two enhancements can affect the centering state
//        Populate();
//    }
//}

void BreakdownItemWeapon::FeatTrained(
        Build* pBuild,
        const std::string& featName)
{
    BreakdownItem::FeatTrained(pBuild, featName);
    // pass through to all our sub breakdowns
    m_baseDamage.FeatTrained(pBuild, featName);
    m_attackBonus.FeatTrained(pBuild, featName);
    m_damageBonus.FeatTrained(pBuild, featName);
    //m_otherDamageEffects.FeatTrained(pBuild, featName);
    ////////m_vorpalRange.FeatTrained(pBuild, featName);
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
    m_baseDamage.FeatRevoked(pBuild, featName);
    m_attackBonus.FeatRevoked(pBuild, featName);
    m_damageBonus.FeatRevoked(pBuild, featName);
    //m_otherDamageEffects.FeatRevoked(pBuild, featName);
    /////////m_vorpalRange.FeatRevoked(pBuild, featName);
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

//void BreakdownItemWeapon::UpdateStanceActivated(
//        Character * pBuild,
//        const std::string & stanceName)
//{
//    // pass through to all our sub breakdowns
//    m_baseDamage.UpdateStanceActivated(pBuild, stanceName);
//    m_attackBonus.UpdateStanceActivated(pBuild, stanceName);
//    m_damageBonus.UpdateStanceActivated(pBuild, stanceName);
//    m_otherDamageEffects.UpdateStanceActivated(pBuild, stanceName);
//    //m_vorpalRange.UpdateStanceActivated(pBuild, stanceName);
//    m_criticalThreatRange.UpdateStanceActivated(pBuild, stanceName);
//    m_criticalAttackBonus.UpdateStanceActivated(pBuild, stanceName);
//    m_criticalDamageBonus.UpdateStanceActivated(pBuild, stanceName);
//    m_otherCriticalDamageEffects.UpdateStanceActivated(pBuild, stanceName);
//    m_criticalMultiplier.UpdateStanceActivated(pBuild, stanceName);
//    m_criticalMultiplier19To20.UpdateStanceActivated(pBuild, stanceName);
//    m_attackSpeed.UpdateStanceActivated(pBuild, stanceName);
//    m_ghostTouch.UpdateStanceActivated(pBuild, stanceName);
//    m_trueSeeing.UpdateStanceActivated(pBuild, stanceName);
//    m_drBypass.UpdateStanceActivated(pBuild, stanceName);
//}
//
//void BreakdownItemWeapon::UpdateStanceDeactivated(
//        Character * pBuild,
//        const std::string & stanceName)
//{
//    // pass through to all our sub breakdowns
//    m_baseDamage.UpdateStanceDeactivated(pBuild, stanceName);
//    m_attackBonus.UpdateStanceDeactivated(pBuild, stanceName);
//    m_damageBonus.UpdateStanceDeactivated(pBuild, stanceName);
//    m_otherDamageEffects.UpdateStanceDeactivated(pBuild, stanceName);
//    //m_vorpalRange.UpdateStanceDeactivated(pBuild, stanceName);
//    m_criticalThreatRange.UpdateStanceDeactivated(pBuild, stanceName);
//    m_criticalAttackBonus.UpdateStanceDeactivated(pBuild, stanceName);
//    m_criticalDamageBonus.UpdateStanceDeactivated(pBuild, stanceName);
//    m_otherCriticalDamageEffects.UpdateStanceDeactivated(pBuild, stanceName);
//    m_criticalMultiplier.UpdateStanceDeactivated(pBuild, stanceName);
//    m_criticalMultiplier19To20.UpdateStanceDeactivated(pBuild, stanceName);
//    m_attackSpeed.UpdateStanceDeactivated(pBuild, stanceName);
//    m_ghostTouch.UpdateStanceDeactivated(pBuild, stanceName);
//    m_trueSeeing.UpdateStanceDeactivated(pBuild, stanceName);
//    m_drBypass.UpdateStanceDeactivated(pBuild, stanceName);
//}

void BreakdownItemWeapon::AddForumExportData(std::stringstream & forumExport)
{
    UNREFERENCED_PARAMETER(forumExport);
    //forumExport << Title() << "\r\n";
    //CString valueRegular;
    //// regular hit value
    //valueRegular.Format(
    //        "On Hit         %.2f[%s]+%d %s\r\n",
    //        m_baseDamage.Total(),
    //        m_damageDice.Description(1).c_str(),
    //        (int)m_damageBonus.Total(),
    //        m_otherDamageEffects.Value());
    //forumExport << valueRegular;
    //// critical hit value
    //CString valueCrit;
    //valueCrit.Format(
    //        "Critical %s (%.2f[%s]+%d) * %d %s\r\n",
    //        m_criticalThreatRange.Value(),
    //        m_baseDamage.Total(),
    //        m_damageDice.Description(1).c_str(),
    //        (int)m_criticalDamageBonus.Total(),
    //        (int)m_criticalMultiplier.Total(),
    //        m_otherCriticalDamageEffects.Value());
    //valueCrit.Replace("-20", "-18");
    //forumExport << valueCrit;
    //CString valueCrit1920;
    //valueCrit1920.Format(
    //        "Critical 19-20 (%.2f[%s]+%d) * %d %s\r\n",
    //        m_baseDamage.Total(),
    //        m_damageDice.Description(1).c_str(),
    //        (int)m_criticalDamageBonus.Total(),
    //        (int)m_criticalMultiplier19To20.Total(),
    //        m_otherCriticalDamageEffects.Value());
    //forumExport << valueCrit1920;
    //forumExport << "DR Bypass: " << m_drBypass.Value();
    //forumExport << "\r\n";
    //if (m_ghostTouch.Total() > 0)
    //{
    //    forumExport << "You bypass Incorporeality miss chance with this weapon\r\n";
    //}
    //if (m_trueSeeing.Total() > 0)
    //{
    //    forumExport << "You bypass Concealment miss chance with this weapon\r\n";
    //}
    //forumExport << "\r\n";
}

BreakdownItem * BreakdownItemWeapon::GetWeaponBreakdown(BreakdownType bt)
{
    UNREFERENCED_PARAMETER(bt);
    BreakdownItem * pBI = NULL;
    //switch (bt)
    //{
    //case Breakdown_WeaponBaseDamage:
    //    pBI = &m_baseDamage;
    //    break;
    //case Breakdown_WeaponAttackBonus:
    //    pBI = &m_attackBonus;
    //    break;
    //case Breakdown_WeaponDamageBonus:
    //    pBI = &m_damageBonus;
    //    break;
    //case Breakdown_WeaponOtherDamageEffects:
    //    pBI = &m_otherDamageEffects;
    //    break;
    //case Breakdown_WeaponCriticalAttackBonus:
    //    pBI = &m_criticalAttackBonus;
    //    break;
    //case Breakdown_WeaponCriticalDamageBonus:
    //    pBI = &m_criticalDamageBonus;
    //    break;
    //case Breakdown_WeaponCriticalOtherDamageEffects:
    //    pBI = &m_otherCriticalDamageEffects;
    //    break;
    //case Breakdown_WeaponCriticalThreatRange:
    //    pBI = &m_criticalThreatRange;
    //    break;
    //case Breakdown_WeaponCriticalMultiplier:
    //    pBI = &m_criticalMultiplier;
    //    break;
    //case Breakdown_WeaponCriticalMultiplier19To20:
    //    pBI = &m_criticalMultiplier19To20;
    //    break;
    //case Breakdown_WeaponAttackSpeed:
    //    pBI = &m_attackSpeed;
    //    break;
    //case Breakdown_WeaponGhostTouch:
    //    pBI = &m_ghostTouch;
    //    break;
    //case Breakdown_WeaponTrueSeeing:
    //    pBI = &m_trueSeeing;
    //    break;
    //case Breakdown_DRBypass:
    //    pBI = &m_drBypass;
    //    break;
    //}
    return pBI;
}

