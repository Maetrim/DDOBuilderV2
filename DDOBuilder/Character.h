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
        virtual void UpdateNumBuildsChanged(Character *) {};
        virtual void UpdateActiveLifeChanged(Character*) {};
        virtual void UpdateActiveBuildChanged(Character *) {};
};

class Character :
    public XmlLib::SaxContentElement,
    public Subject<CharacterObserver>
{
    public:
        Character(CDDOBuilderDoc * pDoc);
        void Write(XmlLib::SaxWriter * writer) const;
        void AboutToLoad();

        void SetActiveBuild(size_t lifeIndex, size_t buildIndex);
        Build * ActiveBuild();  // can be NULL
        const Build* ActiveBuild() const; // can be NULL

        // life support
        Life* ActiveLife(); // can be NULL
        const Life* ActiveLife() const; // can be NULL
        const Life & GetLife(size_t lifeIndex) const;
        void SetLifeName(size_t lifeIndex, CString name);
        size_t AddLife();
        void DeleteLife(size_t lifeIndex);

        // build support
        size_t AddBuild(size_t lifeIndex);
        void DeleteBuild(size_t lifeIndex, size_t buildIndex);
        size_t GetBuildLevel(size_t lifeIndex, size_t buildIndex) const;
        void SetBuildLevel(size_t lifeIndex, size_t buildIndex, size_t level);

        void SetModifiedFlag(BOOL modified);

        void NotifyActiveBuildChanged();
    protected:
        XmlLib::SaxContentElementInterface * StartElement(
                const XmlLib::SaxString & name,
                const XmlLib::SaxAttributes & attributes);

        virtual void EndElement();

        #define Character_PROPERTIES(_) \
                DL_THIS_OBJECT_LIST(_, Life, Lives)

        DL_DECLARE_ACCESS(Character_PROPERTIES)
        DL_DECLARE_VARIABLES(Character_PROPERTIES)
    private:
        void NotifyNumBuildsChanged();
        void NotifyActiveLifeChanged();

        CDDOBuilderDoc * m_pDocument;
        size_t m_activeLifeIndex;
        size_t m_activeBuildIndex;
};
