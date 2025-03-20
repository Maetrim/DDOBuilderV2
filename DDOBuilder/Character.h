// Character.h
//
#pragma once
#include "XmlLib\DLMacros.h"
#include "ObserverSubject.h"
#include "Life.h"

class CDDOBuilderDoc;
class Character;

class CharacterObserver :
    public Observer<Character>
{
    public:
        virtual void UpdateActiveLifeChanged(Character*) {};
        virtual void UpdateActiveBuildChanged(Character *) {};
        virtual void UpdateActiveBuildPositionChanged(Character*) {};
};

class Character :
    public XmlLib::SaxContentElement,
    public Subject<CharacterObserver>
{
    public:
        Character(CDDOBuilderDoc* pDoc);
        void Write(XmlLib::SaxWriter* writer) const;
        void AboutToLoad();
        void LoadComplete();

        bool ShowUnavailable() const;
        bool ShowEpicOnly() const;
        void ToggleShowEpicOnly();
        void ToggleShowUnavailable();

        void SetActiveBuild(size_t lifeIndex, size_t buildIndex, bool bOverride = false);
        Build* ActiveBuild();  // can be NULL
        const Build* ActiveBuild() const; // can be NULL
        void SetActiveBuildIndex(size_t buildIndex);

        // ability tomes
        void SetAbilityTome(AbilityType ability, size_t value);
        size_t AbilityTomeValue(AbilityType ability) const;
        int TomeAtLevel(AbilityType ability, size_t level) const;

        // special feats
        size_t GetSpecialFeatTrainedCount(const std::string& featName) const;
        void TrainSpecialFeat(const std::string& featName);
        bool RevokeSpecialFeat(const std::string& featName);

        // skill tomes
        void SetSkillTome(SkillType skill, size_t value);
        size_t SkillTomeValue(SkillType skill, size_t level) const;

        // life support
        Life* ActiveLife(); // can be NULL
        const Life* ActiveLife() const; // can be NULL
        const Life& GetLife(size_t lifeIndex) const;
        void SetLifeName(size_t lifeIndex, CString name);
        size_t AddLife();
        size_t AddImportLife();
        void DeleteLife(size_t lifeIndex);
        size_t AppendLife(const Life& life);

        // build support
        size_t AddBuild(size_t lifeIndex);
        void DeleteBuild(size_t lifeIndex, size_t buildIndex);
        size_t GetBuildLevel(size_t lifeIndex, size_t buildIndex) const;
        void SetBuildLevel(size_t lifeIndex, size_t buildIndex, size_t level);

        // guild buffs
        void ToggleApplyGuildBuffs();
        void SetGuildLevel(size_t level);

        void SetModifiedFlag(BOOL modified);

        void NotifyActiveBuildChanged();
        void NotifyActiveBuildPositionChanged();

        // quest favor tracking
        std::list<CompletedQuest> CompletedQuests() const;

        // upgrade support
        void AddSpecialFeats(const FeatsListObject& featsToAdd);
        void SetSkillTomes(const SkillTomes& tomes);
        void SetSpecialFeats(const FeatsListObject& feats);

protected:
        XmlLib::SaxContentElementInterface * StartElement(
                const XmlLib::SaxString & name,
                const XmlLib::SaxAttributes & attributes);

        virtual void EndElement();

        #define Character_PROPERTIES(_) \
                DL_SIMPLE(_, size_t, StrTome, 0) \
                DL_SIMPLE(_, size_t, DexTome, 0) \
                DL_SIMPLE(_, size_t, ConTome, 0) \
                DL_SIMPLE(_, size_t, IntTome, 0) \
                DL_SIMPLE(_, size_t, WisTome, 0) \
                DL_SIMPLE(_, size_t, ChaTome, 0) \
                DL_OBJECT(_, FeatsListObject, SpecialFeats) \
                DL_OBJECT(_, SkillTomes, Tomes) \
                DL_THIS_OBJECT_LIST(_, Life, Lives) \
                DL_OPTIONAL_SIMPLE(_, size_t, GuildLevel, 0) \
                DL_OPTIONAL_SIMPLE(_, bool, ApplyGuildBuffs, 0) \
                DL_SIMPLE(_, size_t, ActiveLifeIndex, 0) \
                DL_SIMPLE(_, size_t, ActiveBuildIndex, 0)

        DL_DECLARE_ACCESS(Character_PROPERTIES)
        DL_DECLARE_VARIABLES(Character_PROPERTIES)
    private:
        void NotifyActiveLifeChanged();

        CDDOBuilderDoc* m_pDocument;
        size_t m_uiActiveLifeIndex;
        size_t m_uiActiveBuildIndex;
        bool m_bShowEpicOnly;
        bool m_bShowUnavailableFeats;
        friend class CDDOBuilderApp;
};
