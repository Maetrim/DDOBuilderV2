// Effect.h
//
#pragma once
#include "XmlLib\DLMacros.h"
#include "Dice.h"
#include "Requirements.h"
#include "SpellSchoolTypes.h"
#include "TacticalTypes.h"
#include "WeaponTypes.h"

class Build;
class Character;

// add new feat effects here and update the map for the text saved for that
// effect type so it will be loaded/saved correctly.
enum EffectType
{
    Effect_Unknown = 0,
    Effect_NotModeled,
    Effect_AbilityBonus,
    Effect_ACBonus,
    Effect_ACBonusShield,
    Effect_AddGroupWeapon,
    Effect_ArcaneSpellFailure,
    Effect_ArcaneSpellFailureShields,
    Effect_ArmorACBonus,
    Effect_ArmorCheckPenalty,
    Effect_ArmorCheckPenaltyShield,
    Effect_BlockingDR,
    Effect_CasterLevel,
    Effect_CasterLevelSpell,
    Effect_CasterLevelSchool,
    Effect_CreateSlider,
    Effect_DamageAbilityMultiplier,
    Effect_DamageAbilityMultiplierOffhand,
    Effect_DestinyAPBonus,
    Effect_DestinyTree,
    Effect_Displacement,
    Effect_DivineGrace,
    Effect_DragonmarkUse,
    Effect_DodgeBonus,
    Effect_DodgeBonusTowerShield,
    Effect_DodgeBypass,
    Effect_DodgeCapBonus,
    Effect_DoubleShot,
    Effect_DoubleStrike,
    Effect_DoublestrikeOffhand,
    Effect_DR,
    Effect_DRBypass,
    Effect_EldritchBlastD6,
    Effect_EldritchBlastD8,
    Effect_EnhancementTree,
    Effect_EnchantArmor,
    Effect_EnergyAbsorbance,
    Effect_EnergyResistance,
    Effect_ExcludeFeatSelection,
    Effect_ExclusionGroup,
    Effect_ExtraActionBoost,
    Effect_ExtraLayOnHands,
    Effect_ExtraRage,
    Effect_ExtraRemoveDisease,
    Effect_ExtraSmite,
    Effect_ExtraTurns,
    Effect_ExtraWildEmpathy,
    Effect_FalseLife,
    Effect_FatePoint,
    Effect_Fortification,
    Effect_FortificationBypass,
    Effect_GhostTouch,
    Effect_GrantFeat,
    Effect_GrantSpell,
    Effect_Guard,
    Effect_HealingAmplification,
    Effect_HelplessDamage,
    Effect_HelplessDamageReduction,
    Effect_HirelingAbilityBonus,
    Effect_HirelingConcealment,
    Effect_HirelingHitpoints,
    Effect_HirelingFortification,
    Effect_HirelingPRR,
    Effect_HirelingMRR,
    Effect_HirelingDodge,
    Effect_HirelingMeleePower,
    Effect_HirelingRangedPower,
    Effect_HirelingSpellPower,
    Effect_HirelingSaveBonus,
    Effect_HirelingGrantFeat,
    Effect_Hitpoints,
    Effect_HitpointsReaper,
    Effect_HitpointsStyleBonus,
    Effect_ImbueDice,
    Effect_Immunity,
    Effect_ImplementBonus,
    Effect_ImplementInYourHands,
    Effect_Incorporeality,
    Effect_ItemClickie,
    Effect_KiCritical,
    Effect_KiHit,
    Effect_KiMaximum,
    Effect_KiPassive,
    Effect_LOHRegenRate,
    Effect_MaxCasterLevel,
    Effect_MaxCasterLevelSchool,
    Effect_MaxCasterLevelSpell,
    Effect_MaxDexBonus,
    Effect_MaxDexBonusTowerShield,
    Effect_MeleePower,
    Effect_MergeGroups,
    Effect_MissileDeflection,
    Effect_MissileDeflectionBypass,
    Effect_MRR,
    Effect_MRRCap,
    Effect_MovementSpeed,
    Effect_NaturalArmor,
    Effect_NegativeHealingAmplification,
    Effect_NegativeLevel,
    Effect_OffHandAttackBonus,
    Effect_OverrideBAB,
    Effect_PactDice,
    Effect_PointBlankShotRange,
    Effect_PRR,
    Effect_RangedPower,
    Effect_RangedSneakAttackDamage,
    Effect_RangedSneakAttackRange,
    Effect_RAPBonus,
    Effect_Regeneration,
    Effect_RepairAmplification,
    Effect_RuneArmChargeRate,
    Effect_RuneArmStableCharge,
    Effect_RustSusceptability,
    Effect_SaveBonus,
    Effect_SaveBonusAbility,
    Effect_SaveNoFailOn1,
    Effect_SecondaryShieldBash,
    Effect_ShieldEnchantment,
    Effect_SkillBonus,
    Effect_SkillBonusAbility,
    Effect_SLACharge,
    Effect_SneakAttackAttack,
    Effect_SneakAttackDamage,
    Effect_SneakAttackDice,
    Effect_SneakAttackRange,
    Effect_SongCount,
    Effect_SongDuration,
    Effect_SongACBonus,
    Effect_SongDodgeBonus,
    Effect_SongSaveBonus,
    Effect_SongAttackBonus,
    Effect_SongDoublestrike,
    Effect_SongDoubleshot,
    Effect_SongDamageBonus,
    Effect_SongUniversalSpellPower,
    Effect_SongSpellPenetration,
    Effect_SongCasterLevel,
    Effect_SongSkillBonus,
    Effect_SongPRR,
    Effect_SongMRR,
    Effect_SongHealingAmp,
    Effect_SongNegativeHealingAmp,
    Effect_SongRepairAmp,
    Effect_SpellCostReduction,
    Effect_SpellCriticalDamage,
    Effect_SpellDC,
    Effect_SpellLikeAbility,
    Effect_SpellListAddition,
    Effect_SpellLore,
    Effect_SpellPenetrationBonus,
    Effect_SpellPoints,
    Effect_SpellPower,
    Effect_SpellPowerReplacement,
    Effect_SpellResistance,
    Effect_Strikethrough,
    Effect_TacticalDC,
    Effect_ThreatBonusMelee,
    Effect_ThreatBonusRanged,
    Effect_ThreatBonusSpell,
    Effect_TrueSeeing,
    Effect_TumbleCharge,
    Effect_TurnBonus,
    Effect_TurnDiceBonus,
    Effect_TurnLevelBonus,
    Effect_TurnMaxDice,
    Effect_UAPBonus,
    Effect_UnconsciousRange,
    Effect_UniversalSpellCriticalDamage,
    Effect_UniversalSpellLore,
    Effect_UniversalSpellPower,
    Effect_WildsurgeChance,

    Effect_SpellPointCostPercent,
    // metamagic costs
    Effect_MetamagicCostAccelerate,
    Effect_MetamagicCostEschewMaterials,
    Effect_MetamagicCostEmbolden,
    Effect_MetamagicCostEmpower,
    Effect_MetamagicCostEmpowerHealing,
    Effect_MetamagicCostEnlarge,
    Effect_MetamagicCostExtend,
    Effect_MetamagicCostHeighten,
    Effect_MetamagicCostIntensify,
    Effect_MetamagicCostMaximize,
    Effect_MetamagicCostQuicken,

    // weapon effects for specified weapons in Item(s)
    Effect_Weapon_Alacrity,
    Effect_Weapon_AttackAbility,
    Effect_Weapon_BaseDamage,
    Effect_Weapon_CriticalMultiplier,
    Effect_Weapon_CriticalMultiplier19To20,
    Effect_Weapon_CriticalRange,
    Effect_Weapon_DamageAbility,
    Effect_Weapon_Enchantment,
    Effect_Weapon_Keen,
    Effect_Weapon_VorpalRange,

    // weapon attack/damage bonuses
    Effect_Weapon_Attack,
    Effect_Weapon_AttackAndDamage,
    Effect_Weapon_Damage,

    Effect_Weapon_AttackCritical,
    Effect_Weapon_DamageCritical,
    Effect_Weapon_AttackAndDamageCritical,

    Effect_Weapon_OtherDamageBonus,
    Effect_Weapon_OtherDamageBonusCritical,
    Effect_Weapon_OtherDamageBonusClass,
    Effect_Weapon_OtherDamageBonusCriticalClass,

    // weapon effects for specified classes of weapons in Item(s) (Such as Melee)
    Effect_WeaponAlacrityClass,
    Effect_WeaponAttackAbilityClass,
    Effect_WeaponDamageAbilityClass,
    Effect_WeaponDamageBonusCriticalStat,
    Effect_WeaponDamageBonusStat,
    Effect_WeaponProficiencyClass,
    Effect_WeaponAttackBonusClass,
    Effect_WeaponAttackBonusCriticalClass,
    Effect_WeaponDamageBonusClass,
    Effect_WeaponDamageBonusCriticalClass,
    Effect_WeaponCriticalMultiplierClass,
    Effect_WeaponCriticalRangeClass,
    Effect_Weapon_EnchantmentClass,

    // weapon effects for specified types of weapon damage in Item(s) (e.g. Piercing)
    Effect_WeaponAttackBonusDamageType,
    Effect_WeaponAttackBonusCriticalDamageType,
    Effect_WeaponDamageBonusDamageType,
    Effect_WeaponDamageBonusCriticalDamageType,
    Effect_Weapon_KeenDamageType,
};

const XmlLib::enumMapEntry<EffectType> effectTypeMap[] =
{
    {Effect_Unknown, L"Unknown"},
    {Effect_NotModeled, L"NotModeled"},
    {Effect_AbilityBonus, L"AbilityBonus"},
    {Effect_ACBonus, L"ACBonus"},
    {Effect_ACBonusShield, L"ACBonusShield"},
    {Effect_AddGroupWeapon, L"AddGroupWeapon"},
    {Effect_ArcaneSpellFailure, L"ArcaneSpellFailure"},
    {Effect_ArcaneSpellFailureShields, L"ArcaneSpellFailureShields"},
    {Effect_ArmorACBonus, L"ArmorACBonus"},
    {Effect_ArmorCheckPenalty, L"ArmorCheckPenalty"},
    {Effect_ArmorCheckPenaltyShield, L"ArmorCheckPenaltyShield"},
    {Effect_BlockingDR, L"BlockingDR"},
    {Effect_CasterLevel, L"CasterLevel"},
    {Effect_CasterLevelSpell, L"CasterLevelSpell"},
    {Effect_CasterLevelSchool, L"CasterLevelSchool"},
    {Effect_CreateSlider, L"CreateSlider"},
    {Effect_DamageAbilityMultiplier, L"DamageAbilityMultiplier"},
    {Effect_DamageAbilityMultiplierOffhand, L"DamageAbilityMultiplierOffhand"},
    {Effect_DestinyAPBonus, L"DestinyAPBonus"},
    {Effect_DestinyTree, L"DestinyTree"},
    {Effect_Displacement, L"Displacement"},
    {Effect_DivineGrace, L"DivineGrace"},
    {Effect_DragonmarkUse, L"DragonmarkUse"},
    {Effect_DodgeBonus, L"DodgeBonus"},
    {Effect_DodgeBonusTowerShield, L"DodgeBonusTowerShield"},
    {Effect_DodgeBypass, L"DodgeBypass"},
    {Effect_DodgeCapBonus, L"DodgeCapBonus"},
    {Effect_DoubleShot, L"Doubleshot"},
    {Effect_DoubleStrike, L"Doublestrike"},
    {Effect_DoublestrikeOffhand, L"DoublestrikeOffhand"},
    {Effect_DR, L"DR"},
    {Effect_DRBypass, L"DRBypass"},
    {Effect_EldritchBlastD6, L"EldritchBlastD6"},
    {Effect_EldritchBlastD8, L"EldritchBlastD8"},
    {Effect_PactDice, L"PactDice"},
    {Effect_EnhancementTree, L"EnhancementTree"},
    {Effect_EnchantArmor, L"EnchantArmor"},
    {Effect_EnergyAbsorbance, L"EnergyAbsorbance"},
    {Effect_EnergyResistance, L"EnergyResistance"},
    {Effect_ExcludeFeatSelection, L"ExcludeFeatSelection"},
    {Effect_ExclusionGroup, L"ExclusionGroup"},
    {Effect_ExtraActionBoost, L"ExtraActionBoost"},
    {Effect_ExtraLayOnHands, L"ExtraLayOnHands"},
    {Effect_ExtraRage, L"ExtraRage"},
    {Effect_ExtraRemoveDisease, L"ExtraRemoveDisease"},
    {Effect_ExtraSmite, L"ExtraSmite"},
    {Effect_ExtraTurns, L"ExtraTurns"},
    {Effect_ExtraWildEmpathy, L"ExtraWildEmpathy"},
    {Effect_FatePoint, L"FatePoint"},
    {Effect_FalseLife, L"FalseLife"},
    {Effect_Fortification, L"Fortification"},
    {Effect_FortificationBypass, L"FortificationBypass"},
    {Effect_GhostTouch, L"GhostTouch"},
    {Effect_GrantFeat, L"GrantFeat"},
    {Effect_GrantSpell, L"GrantSpell"},
    {Effect_Guard, L"Guard"},
    {Effect_HealingAmplification, L"HealingAmplification"},
    {Effect_HelplessDamage, L"HelplessDamage"},
    {Effect_HelplessDamageReduction, L"HelplessDamageReduction"},
    {Effect_HirelingAbilityBonus, L"HirelingAbilityBonus"},
    {Effect_HirelingConcealment, L"HirelingConcealment"},
    {Effect_HirelingHitpoints, L"HirelingHitpoints"},
    {Effect_HirelingFortification, L"HirelingFortification"},
    {Effect_HirelingPRR, L"HirelingPRR"},
    {Effect_HirelingMRR, L"HirelingMRR"},
    {Effect_HirelingDodge, L"HirelingDodge"},
    {Effect_HirelingMeleePower, L"HirelingMeleePower"},
    {Effect_HirelingRangedPower, L"HirelingRangedPower"},
    {Effect_HirelingSpellPower, L"HirelingSpellPower"},
    {Effect_HirelingSaveBonus, L"HirelingSaveBonus"},
    {Effect_HirelingGrantFeat, L"HirelingGrantFeat"},
    {Effect_Hitpoints, L"Hitpoints"},
    {Effect_HitpointsReaper, L"HitpointsReaper"},
    {Effect_HitpointsStyleBonus, L"HitpointsStyleBonus"},
    {Effect_ImbueDice, L"ImbueDice"},
    {Effect_Immunity, L"Immunity"},
    {Effect_ImplementBonus, L"ImplementBonus"},
    {Effect_ImplementInYourHands, L"ImplementInYourHands"},
    {Effect_Incorporeality, L"Incorporeality"},
    {Effect_ItemClickie, L"ItemClickie"},
    {Effect_KiCritical, L"KiCritical"},
    {Effect_KiHit, L"KiHit"},
    {Effect_KiMaximum, L"KiMaximum"},
    {Effect_KiPassive, L"KiPassive"},
    {Effect_LOHRegenRate, L"LOHRegenRate"},
    {Effect_MaxCasterLevel, L"MaxCasterLevel"},
    {Effect_MaxCasterLevelSchool, L"MaxCasterLevelSchool"},
    {Effect_MaxCasterLevelSpell, L"MaxCasterLevelSpell"},
    {Effect_MaxDexBonus, L"MaxDexBonus"},
    {Effect_MaxDexBonusTowerShield, L"MaxDexBonusTowerShield"},
    {Effect_MeleePower, L"MeleePower"},
    {Effect_MergeGroups, L"MergeGroups"},
    {Effect_MissileDeflection, L"MissileDeflection"},
    {Effect_MissileDeflectionBypass, L"MissileDeflectionBypass"},
    {Effect_MRR, L"MRR"},
    {Effect_MRRCap, L"MRRCap"},
    {Effect_MovementSpeed, L"MovementSpeed"},
    {Effect_NaturalArmor, L"NaturalArmor"},
    {Effect_NegativeHealingAmplification, L"NegativeHealingAmplification"},
    {Effect_NegativeLevel, L"NegativeLevel"},
    {Effect_OffHandAttackBonus, L"OffHandAttackBonus"},
    {Effect_OverrideBAB, L"OverrideBAB"},
    {Effect_PointBlankShotRange, L"PointBlankShotRange"},
    {Effect_PRR, L"PRR"},
    {Effect_RangedPower, L"RangedPower"},
    {Effect_RangedSneakAttackDamage, L"RangedSneakAttackDamage"},
    {Effect_RangedSneakAttackRange, L"RangedSneakAttackRange"},
    {Effect_RAPBonus, L"RAPBonus"},
    {Effect_Regeneration, L"Regeneration"},
    {Effect_RepairAmplification, L"RepairAmplification"},
    {Effect_RuneArmChargeRate, L"RuneArmChargeRate"},
    {Effect_RuneArmStableCharge, L"RuneArmStableCharge"},
    {Effect_RustSusceptability, L"RustSusceptability"},
    {Effect_SaveBonus, L"SaveBonus"},
    {Effect_SaveBonusAbility, L"SaveBonusAbility"},
    {Effect_SaveNoFailOn1, L"SaveNoFailOn1"},
    {Effect_SecondaryShieldBash, L"SecondaryShieldBash"},
    {Effect_ShieldEnchantment, L"ShieldEnchantment"},
    {Effect_SkillBonus, L"SkillBonus"},
    {Effect_SkillBonusAbility, L"SkillBonusAbility"},
    {Effect_SLACharge, L"SLACharge"},
    {Effect_SneakAttackAttack, L"SneakAttackAttack"},
    {Effect_SneakAttackDamage, L"SneakAttackDamage"},
    {Effect_SneakAttackDice, L"SneakAttackDice"},
    {Effect_SneakAttackRange, L"SneakAttackRange"},
    {Effect_SongCount, L"SongCount"},
    {Effect_SongDuration, L"SongDuration"},
    {Effect_SongACBonus, L"SongACBonus"},
    {Effect_SongDodgeBonus, L"SongDodgeBonus"},
    {Effect_SongSaveBonus, L"SongSaveBonus"},
    {Effect_SongAttackBonus, L"SongAttackBonus"},
    {Effect_SongDoublestrike, L"SongDoublestrike"},
    {Effect_SongDoubleshot, L"SongDoubleshot"},
    {Effect_SongDamageBonus, L"SongDamageBonus"},
    {Effect_SongUniversalSpellPower, L"SongUniversalSpellPower"},
    {Effect_SongSpellPenetration, L"SongSpellPenetration"},
    {Effect_SongCasterLevel, L"SongCasterLevel"},
    {Effect_SongSkillBonus, L"SongSkillBonus" },
    {Effect_SongPRR, L"SongPRR"},
    {Effect_SongMRR, L"SongMRR"},
    {Effect_SongHealingAmp, L"SongHealingAmp"},
    {Effect_SongNegativeHealingAmp, L"SongNegativeHealingAmp"},
    {Effect_SongRepairAmp, L"SongRepairAmp"},
    {Effect_SpellCostReduction, L"SpellCostReduction"},
    {Effect_SpellCriticalDamage, L"SpellCriticalDamage"},
    {Effect_SpellDC, L"SpellDC"},
    {Effect_SpellLikeAbility, L"SpellLikeAbility"},
    {Effect_SpellListAddition , L"SpellListAddition"},
    {Effect_SpellLore, L"SpellLore"},
    {Effect_SpellPenetrationBonus, L"SpellPenetrationBonus"},
    {Effect_SpellPoints, L"SpellPoints"},
    {Effect_SpellPower, L"SpellPower"},
    {Effect_SpellPowerReplacement, L"SpellPowerReplacement"},
    {Effect_SpellResistance, L"SpellResistance"},
    {Effect_Strikethrough, L"Strikethrough"},
    {Effect_TacticalDC, L"TacticalDC"},
    {Effect_ThreatBonusMelee, L"ThreatBonusMelee"},
    {Effect_ThreatBonusRanged, L"ThreatBonusRanged"},
    {Effect_ThreatBonusSpell, L"ThreatBonusSpell"},
    {Effect_TrueSeeing, L"TrueSeeing"},
    {Effect_TumbleCharge, L"TumbleCharge"},
    {Effect_TurnBonus, L"TurnBonus"},
    {Effect_TurnDiceBonus, L"TurnDiceBonus"},
    {Effect_TurnLevelBonus, L"TurnLevelBonus"},
    {Effect_TurnMaxDice, L"TurnMaxDice"},
    {Effect_UAPBonus, L"UAPBonus"},
    {Effect_UniversalSpellCriticalDamage, L"UniversalSpellCriticalDamage"},
    {Effect_UnconsciousRange, L"UnconsciousRange"},
    {Effect_UniversalSpellLore, L"UniversalSpellLore"},
    {Effect_UniversalSpellPower, L"UniversalSpellPower"},
    {Effect_WildsurgeChance, L"WildsurgeChance"},

    {Effect_SpellPointCostPercent, L"SpellPointCostPercent"},
    // metamagic costs
    {Effect_MetamagicCostAccelerate, L"MetamagicCostAccelerate"},
    {Effect_MetamagicCostEschewMaterials, L"MetamagicCostEschewMaterials"},
    {Effect_MetamagicCostEmbolden, L"MetamagicCostEmbolden"},
    {Effect_MetamagicCostEmpower, L"MetamagicCostEmpower"},
    {Effect_MetamagicCostEmpowerHealing, L"MetamagicCostEmpowerHealing"},
    {Effect_MetamagicCostEnlarge, L"MetamagicCostEnlarge"},
    {Effect_MetamagicCostExtend, L"MetamagicCostExtend"},
    {Effect_MetamagicCostHeighten, L"MetamagicCostHeighten"},
    {Effect_MetamagicCostIntensify, L"MetamagicCostIntensify"},
    {Effect_MetamagicCostMaximize, L"MetamagicCostMaximize"},
    {Effect_MetamagicCostQuicken, L"MetamagicCostQuicken"},

    // weapon effects for specified weapons in Item(s)
    {Effect_Weapon_Alacrity, L"Weapon_Alacrity"},
    {Effect_Weapon_AttackAbility, L"Weapon_AttackAbility"},
    {Effect_Weapon_BaseDamage, L"Weapon_BaseDamage"},
    {Effect_Weapon_CriticalMultiplier, L"Weapon_CriticalMultiplier"},
    {Effect_Weapon_CriticalMultiplier19To20, L"Weapon_CriticalMultiplier19To20"},
    {Effect_Weapon_CriticalRange, L"Weapon_CriticalRange"},
    {Effect_Weapon_DamageAbility, L"Weapon_DamageAbility"},
    {Effect_Weapon_Enchantment, L"Weapon_Enchantment"},
    {Effect_Weapon_Keen, L"Weapon_Keen"},
    {Effect_Weapon_VorpalRange, L"Weapon_VorpalRange"},

    {Effect_Weapon_Attack, L"Weapon_Attack" },
    {Effect_Weapon_AttackAndDamage, L"Weapon_AttackAndDamage"},
    {Effect_Weapon_Damage, L"Weapon_Damage"},

    {Effect_Weapon_AttackCritical, L"Weapon_AttackCritical"},
    {Effect_Weapon_DamageCritical, L"Weapon_DamageCritical"},
    {Effect_Weapon_AttackAndDamageCritical, L"Weapon_AttackAndDamageCritical"},

    {Effect_Weapon_OtherDamageBonus, L"WeaponOtherDamageBonus"},
    {Effect_Weapon_OtherDamageBonusCritical, L"WeaponOtherDamageBonusCritical"},
    {Effect_Weapon_OtherDamageBonusClass, L"WeaponOtherDamageBonusClass"},
    {Effect_Weapon_OtherDamageBonusCriticalClass, L"WeaponOtherDamageBonusCriticalClass"},

    // weapon effects for specified classes of weapons in Item(s) (Such as Melee)
    {Effect_WeaponAlacrityClass, L"WeaponAlacrityClass"},
    {Effect_WeaponAttackAbilityClass, L"WeaponAttackAbilityClass" },
    {Effect_WeaponDamageAbilityClass, L"WeaponDamageAbilityClass" },
    {Effect_WeaponDamageBonusCriticalStat, L"WeaponDamageBonusCriticalStat" },
    {Effect_WeaponDamageBonusStat, L"WeaponDamageBonusStat" },
    {Effect_WeaponProficiencyClass, L"WeaponProficiencyClass"},
    {Effect_WeaponAttackBonusClass, L"WeaponAttackBonusClass"},
    {Effect_WeaponAttackBonusCriticalClass, L"WeaponAttackBonusCriticalClass"},
    {Effect_WeaponDamageBonusClass, L"WeaponDamageBonusClass"},
    {Effect_WeaponDamageBonusCriticalClass, L"WeaponDamageBonusCriticalClass"},
    {Effect_WeaponCriticalMultiplierClass, L"WeaponCriticalMultiplierClass"},
    {Effect_WeaponCriticalRangeClass, L"WeaponCriticalRangeClass"},
    {Effect_Weapon_EnchantmentClass, L"Weapon_EnchantmentClass"},


    {Effect_WeaponAttackBonusDamageType, L"WeaponAttackBonusDamageType"},
    {Effect_WeaponAttackBonusCriticalDamageType, L"WeaponAttackBonusCriticalDamageType"},
    {Effect_WeaponDamageBonusDamageType, L"WeaponDamageBonusDamageType"},
    {Effect_WeaponDamageBonusCriticalDamageType, L"WeaponDamageBonusCriticalDamageType"},
    {Effect_Weapon_KeenDamageType, L"WeaponKeenDamageType"},
    {EffectType(0), NULL}
};

enum AmountType
{
    Amount_Unknown = 0,
    Amount_NotNeeded,           // no amount field
    Amount_Simple,              // its just a direct single number (vector size is 1)
    Amount_Stacks,              // look up based on number of stacks of this effect
    Amount_TotalLevel,          // index into vector based on total character level (0 based)
    Amount_BaseClassLevel,      // index into vector based on class level (0 based)
    Amount_ClassLevel,          // index into vector based on class level (0 based)
    Amount_ClassCasterLevel,    // index into vector based on class caster level (0 based)
    Amount_APCount,             // stacks based on AP spent in tree "Item"
    Amount_AbilityValue,        // Stacks are equal to the specific abilities total inherent value (RACE + Buy + Inherent + Level ups)
    Amount_AbilityTotal,        // Stacks are equal to the specific abilities total value
    Amount_AbilityMod,          // Stacks are equal to the specific abilities mod value
    Amount_HalfAbilityMod,      // Stacks are equal to the specific abilities mod value / 2
    Amount_ThirdAbilityMod,     // Stacks are equal to the specific abilities mod value / 3
    Amount_Slider,              // Defining a slider initial value and range
    Amount_SliderValue,         // Stacks are equal to the specified slider value
    Amount_SliderValueLookup,   // Stacks are equal to the specified slider value multiplied by a look up value
    Amount_FeatCount,           // Stacks are equal to the number times a specific feat has been trained
    Amount_SetBonusCount,       // Stacks are equal to the count of the specified set bonus
    Amount_SLA,                 // Item SLA, caster level, charges, recharge/day
    Amount_SpellInfo,           // 3 entries that specify spell level, cost and MCL
    Amount_Dice,                // Damage Dice are defined
    Amount_CriticialDice,       // Damage Dice are defined, look up based on crit multiplier
    Amount_BAB,                 // amount is multiplied by BAB value
};
const XmlLib::enumMapEntry<AmountType> amountTypeMap[] =
{
    {Amount_Unknown, L"Unknown"},
    {Amount_NotNeeded, L"NotNeeded"},
    {Amount_Simple, L"Simple"},
    {Amount_Stacks, L"Stacks"},
    {Amount_TotalLevel, L"TotalLevel"},
    {Amount_BaseClassLevel, L"BaseClassLevel"},
    {Amount_ClassLevel, L"ClassLevel"},     // Item is the class name
    {Amount_ClassCasterLevel, L"ClassCasterLevel"}, // Item is the class name
    {Amount_APCount, L"APCount"},           // Item is the tree name
    {Amount_AbilityValue, L"AbilityValue"}, // Item is the ability name
    {Amount_AbilityTotal, L"AbilityTotal"}, // Item is the ability name
    {Amount_AbilityMod, L"AbilityMod"},     // Item is the ability name
    {Amount_HalfAbilityMod, L"HalfAbilityMod"},     // Item is the ability name
    {Amount_ThirdAbilityMod, L"ThirdAbilityMod"},     // Item is the ability name
    {Amount_Slider, L"Slider"},             // Item is the Slider name
    {Amount_SliderValue, L"SliderValue"},   // Item is the Slider name
    {Amount_SliderValueLookup, L"SliderValueLookup"},   // Item is a Slider name, and StackSource used to lookup the value used
    {Amount_FeatCount, L"FeatCount"},       // Item is the Feat name
    {Amount_SetBonusCount, L"SetBonusCount"},   // Item is the set bonus name
    {Amount_SLA, L"SLA"},                   // item is the SLA name
    {Amount_SpellInfo, L"SpellInfo"},      // item is spell name and then class
    {Amount_Dice, L"Dice"},
    {Amount_CriticialDice, L"CriticalDice"},
    {Amount_BAB, L"BAB"},
    {AmountType(0), NULL}
};

class Effect :
    public XmlLib::SaxContentElement
{
    public:
        Effect(void);
        Effect(EffectType t, const std::string& name, const std::string& group);
        Effect(EffectType t, const std::string& name, const std::string& bonusType, double amount);

        void SetBuild(const Build * pBuild);

        void Write(XmlLib::SaxWriter * writer) const;

        bool IsType(EffectType type) const;
        bool HasSpellPower(SpellPowerType type) const;
        bool HasSpellSchool(SpellSchoolType type, bool bSpecificDCOnly) const;
        bool HasPrimer(PrimerType type) const;
        bool HasTacticalType(TacticalType type) const;
        void SetDisplayName(const std::string displayName);
        bool VerifyObject(std::stringstream * ss) const;

        bool IsActive(const Character& c, InventorySlotType slot, WeaponType wtMain, WeaponType wtOffhand) const;
        double TotalAmount(bool allowTruncate) const;
        size_t EffectStacks() const;
        void AddEffectStack();
        bool RevokeEffectStack();
        std::string StacksAsString() const;
        void SetEffectStacks(size_t count);

        void SetPercentValue(double value) const;
        double GetPercentValue() const;

        bool UpdateAbilityEffects(AbilityType at);
        bool UpdateTreeEffects(const std::string& treeName);
        bool UpdateSliderEffects(const std::string& sliderName, int newValue);

        void SetType(EffectType t);
        void SetAType(AmountType t);
        void SetAmount(double value);
        void SetBonus(const std::string& bonus);
        void SetItem(const std::list<std::string>& items);
        void AddItem(const std::string& item);
        void AddValue(const std::string& str);

        void SetRequirements(const Requirements& req);
        void ReplaceLastItem(const std::string& newEntry);
        void SetIsItemSpecific();
        void SetApplyAsItemEffect();
        void SetWeapon1();
        void SetWeapon2();
        void SetStackSource(const std::string& source);

        bool operator==(const Effect & other) const;

    protected:
        XmlLib::SaxContentElementInterface * StartElement(
                const XmlLib::SaxString & name,
                const XmlLib::SaxAttributes & attributes);

        virtual void EndElement();

        // note that all the optional XML elements can be
        // used by a variety of feat effects as different interpretations
        #define Effect_PROPERTIES(_) \
                DL_OPTIONAL_STRING(_, DisplayName) \
                DL_ENUM_LIST(_, EffectType, Type, Effect_Unknown, effectTypeMap) \
                DL_STRING(_, Bonus) \
                DL_STRING_LIST(_, Item) \
                DL_OPTIONAL_ENUM(_, AmountType, AType, Amount_Unknown, amountTypeMap) \
                DL_OPTIONAL_VECTOR(_, double, Amount) \
                DL_OPTIONAL_STRING(_, Value) \
                DL_OPTIONAL_STRING(_, StackSource) \
                DL_FLAG(_, IsItemSpecific) \
                DL_FLAG(_, Percent) \
                DL_FLAG(_, Weapon1) \
                DL_FLAG(_, Weapon2) \
                DL_FLAG(_, ApplyAsItemEffect) \
                DL_FLAG(_, UpdateAutomaticEffects) \
                DL_FLAG(_, Rare) \
                DL_OPTIONAL_OBJECT(_, Requirements, RequirementsToBeActive) \
                DL_OPTIONAL_SIMPLE(_, size_t, Rank, 0) \
                DL_OPTIONAL_OBJECT(_, Dice, DamageDice) \
                DL_OPTIONAL_ENUM(_, DamageType, Damage, Damage_Unknown, damageTypeMap)

        DL_DECLARE_ACCESS(Effect_PROPERTIES)
        DL_DECLARE_VARIABLES(Effect_PROPERTIES)

    private:
        bool CheckAType(std::stringstream* ss, bool *bRequiresAmount, int *requiredAmountElements) const;

        size_t m_stacks;
        mutable double m_percentValue;
        const Build * m_pBuild;

        friend class CBreakdownsPane;
};
