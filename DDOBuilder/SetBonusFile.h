// SetBonusFile.h
//
// A class that handles the loading of the race file.
#pragma once
#include "SetBonus.h"

class SetBonusFile :
    public XmlLib::SaxContentElement
{
    public:
        SetBonusFile(const std::string& filename);
        ~SetBonusFile(void);

        void Read();
        const std::list<SetBonus> & SetBonuses() const;

    protected:
        XmlLib::SaxContentElementInterface * StartElement(
                const XmlLib::SaxString & name,
                const XmlLib::SaxAttributes & attributes);

        void EndElement();

        std::string m_filename;
        std::list<SetBonus> m_loadedSetBonuses;
        size_t m_loadTotal;
};
