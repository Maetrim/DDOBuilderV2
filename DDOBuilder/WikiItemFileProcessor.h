#pragma once
#include <string>
#include <map>
#include "Item.h"

class WikiItemFileProcessor :
    public XmlLib::SaxContentElement
{
public:
    WikiItemFileProcessor();
    ~WikiItemFileProcessor();

    void Start();
    void ProcessSpecificFile();

protected:
    XmlLib::SaxContentElementInterface* StartElement(
        const XmlLib::SaxString& name,
        const XmlLib::SaxAttributes& attributes);

    void EndElement();

private:
    void ProcessFile(int fileIndex, const std::string& filename);
    void ExtractFields(std::map<std::string, std::string>& itemFields, const std::string& fileData);
    std::string ExtractSingleField(const std::string& field, const std::string& startMarker, const std::string& endMarker, const std::string& fileData);
    void ReplaceRequiredCharacters(std::string& field);
    void RemoveLinks(std::string& field);
    void ExtractEnchantmentsText(std::map<std::string, std::string>& itemFields, const std::string& fileData);
    void CreateItem(const std::map<std::string, std::string>& itemFields, bool bMinorArtifact);
    bool SetItemSlot(const std::map<std::string, std::string>& itemFields);
    void AddArmorFields(const std::map<std::string, std::string>& itemFields);
    void AddAttackMods(const std::map<std::string, std::string>& itemFields);
    void AddDamageMods(const std::map<std::string, std::string>& itemFields);
    void AddDamageDice(const std::map<std::string, std::string>& itemFields);
    void AddCriticalThreatAndMultiplier(const std::map<std::string, std::string>& itemFields);
    void AddItemEffects(const std::map<std::string, std::string>& itemFields);
    bool ProcessEnchantmentLine(const std::string& line);
    bool AddAugmentSlots(const std::string& line);
    bool AddAugmentSlot(const std::string& line, const std::string& searchText);
    bool AddSetBonuses(const std::string& line);
    bool AddClassRequirements(const std::string& line);

    bool ProcessRomanNumeralTypes(const std::string& line, const std::string& bonus);
    bool ProcessRomanNumeralType(const std::string& line, const std::string& numeral, const std::string& bonus, size_t index);

    bool ProcessBonusType(const std::string& line, const std::string& bonus);
    bool ProcessAbility(const std::string& line, const std::string& bonus);
    bool ProcessFortification(const std::string& line, const std::string& bonus);
    bool ProcessPhysicalSheltering(const std::string& line, const std::string& bonus);
    bool ProcessMagicalSheltering(const std::string& line, const std::string& bonus);
    bool ProcessSheltering(const std::string& line, const std::string& bonus);
    bool ProcessCombatMastery(const std::string& line, const std::string& bonus);
    bool ProcessArmorMastery(const std::string& line, const std::string& bonus);
    bool ProcessSpellResistance(const std::string& line, const std::string& bonus);
    bool ProcessSpellPower(const std::string& line, const std::string& bonus);
    bool ProcessSpellLore(const std::string& line, const std::string& bonus);
    bool ProcessSkill(const std::string& line, const std::string& bonus);
    bool ProcessAlluringSkills(const std::string& line, const std::string& bonus);
    bool ProcessAstuteSkills(const std::string& line, const std::string& bonus);
    bool ProcessIncite(const std::string& line, const std::string& bonus);
    bool ProcessWizardryNumeric(const std::string& line, const std::string& bonus);
    bool ProcessWizardryRomanNumeral(const std::string& line, const std::string& numeral, const std::string& bonus, size_t value);
    bool ProcessSpellFocusNumeric(const std::string& line, const std::string& bonus);
    bool ProcessSpellFocusRomanNumeral(const std::string& line, const std::string& numeral, const std::string& bonus, size_t value);
    bool ProcessSeeker(const std::string& line, const std::string& bonus);
    bool ProcessDodge(const std::string& line, const std::string& bonus);
    bool ProcessAttackBonus(const std::string& line, const std::string& bonus);
    bool ProcessParrying(const std::string& line, const std::string& bonus);
    bool ProcessParryingRomanNumeral(const std::string& line, const std::string& numeral, const std::string& bonus, size_t value);
    bool ProcessPowerNumeral(const std::string& line, const std::string& numeral, const std::string& bonus, size_t value);
    bool ProcessSpellPenetrationNumeral(const std::string& line, const std::string& numeral, const std::string& bonus, size_t value);
    bool ProcessWillSave(const std::string& line, const std::string& bonus);
    bool ProcessFortitudeSave(const std::string& line, const std::string& bonus);
    bool ProcessReflexSave(const std::string& line, const std::string& bonus);
    bool ProcessSpellSave(const std::string& line, const std::string& bonus);
    bool ProcessCurseSave(const std::string& line, const std::string& bonus);
    bool ProcessResistance(const std::string& line, const std::string& bonus);
    bool ProcessAbsorption(const std::string& line, const std::string& bonus);
    bool ProcessDoublestrike(const std::string& line, const std::string& bonus);
    bool ProcessDoubleshot(const std::string& line, const std::string& bonus);
    bool ProcessSpellPenetration(const std::string& line, const std::string& bonus);
    bool ProcessArmorPiercing(const std::string& line, const std::string& bonus);
    bool ProcessAccuracy(const std::string& line, const std::string& bonus);
    bool ProcessDeadly(const std::string& line, const std::string& bonus);
    bool ProcessEnergyResistance(const std::string& line, const std::string& bonus);
    bool ProcessEnchantmentSave(const std::string& line, const std::string& bonus);
    bool ProcessNaturalArmor(const std::string& line, const std::string& bonus);
    bool ProcessFalseLife(const std::string& line, const std::string& bonus);
    bool ProcessDiversion(const std::string& line, const std::string& bonus);
    bool ProcessDistantDiversion(const std::string& line, const std::string& bonus);
    bool ProcessMysticDiversion(const std::string& line, const std::string& bonus);
    bool ProcessHealingAmplification(const std::string& line, const std::string& bonus);
    bool ProcessRepairAmplification(const std::string& line, const std::string& bonus);
    bool ProcessNegativeHealingAmplification(const std::string& line, const std::string& bonus);
    bool ProcessSpeed(const std::string& line);
    bool AddFavoredWeapon(const std::string& line);
    bool ProcessDeception(const std::string& line, const std::string& bonus);
    bool ProcessSneakAttack(const std::string& line, const std::string& bonus);
    bool ProcessCombatAction(const std::string& line, const std::string& bonus, const std::string& type);
    bool ProcessKeen(const std::string& line);

    bool AddCommonEffect(const std::string& line, const std::string& buffName, const std::string& searchText, const std::string& bonus, const std::string& item, size_t minStartPos = 5);

    std::string m_sourcePath;
    std::string m_destinationPath;
    Item m_item;
};

