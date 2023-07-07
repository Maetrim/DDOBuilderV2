// DDOBuilderDoc.cpp : implementation of the CDDOBuilderDoc class
//

#include "stdafx.h"
#include "DDOBuilderDoc.h"
#include "XmlLib\SaxReader.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace
{
    const XmlLib::SaxString f_saxElementName = L"DDOBuilderCharacterData"; // root element name to look for
}

// CDDOBuilderDoc
IMPLEMENT_DYNCREATE(CDDOBuilderDoc, CDocument)

BEGIN_MESSAGE_MAP(CDDOBuilderDoc, CDocument)
END_MESSAGE_MAP()

CDDOBuilderDoc::CDDOBuilderDoc() :
    SaxContentElement(f_saxElementName),
    m_character(this)
{
}

CDDOBuilderDoc::~CDDOBuilderDoc()
{
}

BOOL CDDOBuilderDoc::OnNewDocument()
{
    if (!CDocument::OnNewDocument())
        return FALSE;
    // default to a new empty character
    m_character = Character(this);

    // (SDI documents will reuse this document)
    CWnd * pWnd = AfxGetApp()->m_pMainWnd;
    CMainFrame * pMF = dynamic_cast<CMainFrame*>(pWnd);
    pMF->NewDocument(this);

    return TRUE;
}

BOOL CDDOBuilderDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
    m_character.AboutToLoad();
    CWaitCursor longOperation;
    // set up a reader with this as the expected root node
    XmlLib::SaxReader reader(this, f_saxElementName);
    // read in the xml from a file (fully qualified path)
    bool ok = reader.Open(lpszPathName);
    if (ok)
    {
        //m_characterData.JustLoaded();
    }
    else
    {
        std::string errorMessage = reader.ErrorMessage();
        // document has failed to load. Tell the user what we can about it
        CString text;
        text.Format("The document %s\n"
                "failed to load. The XML parser reported the following problem:\n"
                "\n", lpszPathName);
        text += errorMessage.c_str();
        AfxMessageBox(text, MB_ICONERROR);
    }

    CWnd* pWnd = AfxGetApp()->m_pMainWnd;
    CMainFrame * pMF = dynamic_cast<CMainFrame*>(pWnd);
    pMF->NewDocument(this);

    return ok;
}

BOOL CDDOBuilderDoc::OnSaveDocument(LPCTSTR lpszPathName)
{
    bool ok = false;
    try
    {
        XmlLib::SaxWriter writer;
        writer.Open(lpszPathName);
        writer.StartDocument(f_saxElementName);
        m_character.Write(&writer);
        writer.EndDocument();
        ok = true;
    }
    catch (const std::exception & e)
    {
        std::string errorMessage = e.what();
        // document has failed to save. Tell the user what we can about it
        CString text;
        text.Format("The document %s\n"
                "failed to save. The XML parser reported the following problem:\n"
                "\n", lpszPathName);
        text += errorMessage.c_str();
        AfxMessageBox(text, MB_ICONERROR);
        ok = false;
    }
    SetModifiedFlag(FALSE);
    return ok;
}

// CDDOBuilderDoc serialization
void CDDOBuilderDoc::Serialize(CArchive &)
{
    // not used
    throw;
}

XmlLib::SaxContentElementInterface * CDDOBuilderDoc::StartElement(
        const XmlLib::SaxString & name,
        const XmlLib::SaxAttributes & attributes)
{
    XmlLib::SaxContentElementInterface * subHandler =
            SaxContentElement::StartElement(name, attributes);
    if (subHandler == NULL)
    {
        if (m_character.SaxElementIsSelf(name, attributes))
        {
            subHandler = &m_character;
        }
    }

    return subHandler;
}

void CDDOBuilderDoc::EndElement()
{
    SaxContentElement::EndElement();
}
