// ChallengesFile.h
//
// A class that handles the loading of the race file.
#pragma once
#include "Challenge.h"

class ChallengesFile :
    public XmlLib::SaxContentElement
{
    public:
        ChallengesFile(const std::string& filename);
        ~ChallengesFile(void);

        void Read();
        const std::list<Challenge>& Challenges() const;

    protected:
        XmlLib::SaxContentElementInterface* StartElement(
                const XmlLib::SaxString& name,
                const XmlLib::SaxAttributes& attributes);

        void EndElement();

        std::string m_filename;
        std::list<Challenge> m_loadedChallenges;
        size_t m_loadTotal;
};
