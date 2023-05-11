// EnhancementsFile.cpp
//
#include "StdAfx.h"
#include "EnhancementsFile.h"
#include "XmlLib\SaxReader.h"
#include "LogPane.h"
#include "GlobalSupportFunctions.h"

namespace
{
    const XmlLib::SaxString f_saxElementName = L"Enhancements"; // root element name to look for
}

EnhancementsFile::EnhancementsFile(const std::string & path) :
    SaxContentElement(f_saxElementName),
    m_path(path),
    m_loadTotal(0)
{
}


EnhancementsFile::~EnhancementsFile(void)
{
}

void EnhancementsFile::ReadFiles()
{
    std::string fileFilter = m_path;
    fileFilter += "*.tree.xml";
    // read all the item files found in the Items sub-directory
    // first enumerate each file and load it
    WIN32_FIND_DATA findFileData;
    HANDLE hFind = FindFirstFile(fileFilter.c_str(), &findFileData);
    if (hFind != INVALID_HANDLE_VALUE)
    {
        std::string fullFilename = m_path + "\\";
        fullFilename += findFileData.cFileName;
        ReadFile(fullFilename);
        while (FindNextFile(hFind, &findFileData))
        {
            fullFilename = m_path + "\\";
            fullFilename += findFileData.cFileName;
            ReadFile(fullFilename);
        }
        FindClose(hFind);
    }
}

bool EnhancementsFile::ReadFile(const std::string & filename)
{
    // set up a reader with this as the expected root node
    XmlLib::SaxReader reader(this, f_saxElementName);
    // read in the xml from a file (fully qualified path)
    bool ok = reader.Open(filename);
    if (!ok)
    {
        std::string errorMessage = reader.ErrorMessage();
        // document has failed to load. Tell the user what we can about it
        CString text;
        text.Format("The document %s\n"
                "failed to load. The XML parser reported the following problem:\n"
                "\n", filename.c_str());
        text += errorMessage.c_str();
        AfxMessageBox(text, MB_ICONERROR);
    }
    return ok;
}

XmlLib::SaxContentElementInterface * EnhancementsFile::StartElement(
        const XmlLib::SaxString & name,
        const XmlLib::SaxAttributes & attributes)
{
    XmlLib::SaxContentElementInterface * subHandler =
            SaxContentElement::StartElement(name, attributes);
    if (subHandler == NULL)
    {
        EnhancementTree tree;
        if (tree.SaxElementIsSelf(name, attributes))
        {
            m_loadedTrees.push_back(tree);
            subHandler = &(m_loadedTrees.back());
            // update log during load action
            CString strTreeCount;
            strTreeCount.Format("Loading Enhancement Trees...%d", m_loadedTrees.size());
            GetLog().UpdateLastLogEntry(strTreeCount);
        }
    }

    return subHandler;
}

void EnhancementsFile::EndElement()
{
    SaxContentElement::EndElement();
}

const std::list<EnhancementTree> & EnhancementsFile::EnhancementTrees() const
{
    return m_loadedTrees;
}
