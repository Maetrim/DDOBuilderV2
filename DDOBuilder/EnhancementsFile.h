// EnhancementsFile.h
//

#pragma once
#include "XmlLib\SaxContentElement.h"
#include "XmlLib\SaxWriter.h"
#include "EnhancementTree.h"

class EnhancementsFile :
    public XmlLib::SaxContentElement
{
    public:
        EnhancementsFile(const std::string & path);
        ~EnhancementsFile(void);

        void ReadFiles();
        const std::list<EnhancementTree> & EnhancementTrees() const;

    protected:
        XmlLib::SaxContentElementInterface * StartElement(
                const XmlLib::SaxString & name,
                const XmlLib::SaxAttributes & attributes);

        void EndElement();
        bool ReadFile(const std::string & filename);

        std::string m_path;
        std::list<EnhancementTree> m_loadedTrees;
        size_t m_loadTotal;
};

