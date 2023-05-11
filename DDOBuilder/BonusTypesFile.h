// BonusTypesFile.h
//
// A class that handles the loading of the BonusTypes file.
#pragma once
#include "Bonus.h"

class BonusTypesFile :
    public XmlLib::SaxContentElement
{
    public:
        BonusTypesFile(const std::string& filename);
        ~BonusTypesFile(void);

        void Read();
        const std::list<Bonus> & BonusTypes() const;

    protected:
        XmlLib::SaxContentElementInterface * StartElement(
                const XmlLib::SaxString & name,
                const XmlLib::SaxAttributes & attributes);

        void EndElement();

        std::string m_filename;
        std::list<Bonus> m_loadedBonusTypes;
        size_t m_loadTotal;
};
