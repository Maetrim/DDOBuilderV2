// DDOBuilderDoc.cpp : implementation of the CDDOBuilderDoc class
//

#include "stdafx.h"
#include "DDOBuilderDoc.h"
#include "XmlLib\SaxReader.h"
#include "MainFrm.h"
#include "LogPane.h"
#include "GlobalSupportFunctions.h"

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
    ON_UPDATE_COMMAND_UI(ID_FILE_REVERTTOBACKUP, &CDDOBuilderDoc::OnUpdateRevertToBackup)
    ON_COMMAND(ID_FILE_REVERTTOBACKUP, &CDDOBuilderDoc::OnRevertToBackup)
END_MESSAGE_MAP()

CDDOBuilderDoc::CDDOBuilderDoc() :
    SaxContentElement(f_saxElementName),
    m_character(this)
{
}

CDDOBuilderDoc::~CDDOBuilderDoc()
{
    // if we have a backup file when the doc goes away, delete it
    if (m_strBackupFilename != "")
    {
        ::DeleteFile((LPCTSTR)m_strBackupFilename);
        m_strBackupFilename = "";   // no backup anymore
    }
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
    if (m_strBackupFilename != "")
    {
        ::DeleteFile((LPCTSTR)m_strBackupFilename);
        m_strBackupFilename = "";   // no backup anymore
    }

    return TRUE;
}

BOOL CDDOBuilderDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
    SpendInTree::ResetUpgradeSelection();
    // default to a new empty character
    m_character = Character(this);
    m_character.AboutToLoad();
    CWaitCursor longOperation;
    // set up a reader with this as the expected root node
    XmlLib::SaxReader reader(this, f_saxElementName);
    // read in the xml from a file (fully qualified path)
    bool ok = reader.Open(lpszPathName);
    if (ok)
    {
        std::stringstream ss;
        ss << "Document \"" << lpszPathName << "\" loaded.";
        GetLog().AddLogEntry(ss.str().c_str());
        // create a backup of the file to allow a revert option
        m_strBackupFilename = lpszPathName;
        m_strBackupFilename += ".backup";
        ::CopyFile(lpszPathName, (LPCTSTR)m_strBackupFilename, FALSE);
        CString strLog;
        strLog.Format("Backup document \"%s\" created", (LPCTSTR)m_strBackupFilename);
        GetLog().AddLogEntry(strLog);
    }
    else
    {
        std::string errorMessage = reader.ErrorMessage();
        // document has failed to load. Tell the user what we can about it
        CString text;
        text.Format("The document \"%s\"\n"
                "failed to load. The XML parser reported the following problem:\n"
                "\n", lpszPathName);
        text += errorMessage.c_str();
        AfxMessageBox(text, MB_ICONERROR);
    }
    m_character.SetSupportLegacyTrees(SpendInTree::SupportLegacyTrees());
    m_character.LoadComplete(); // does file format upgrades etc
    CWnd* pWnd = AfxGetApp()->m_pMainWnd;
    CMainFrame * pMF = dynamic_cast<CMainFrame*>(pWnd);
    pMF->NewDocument(this);
    // just opened the file, its unmodified!
    SetModifiedFlag(FALSE);
    return ok;
}

BOOL CDDOBuilderDoc::LoadIndirect(LPCTSTR lpszPathName)
{
    m_character.AboutToLoad();
    CWaitCursor longOperation;
    // set up a reader with this as the expected root node
    XmlLib::SaxReader reader(this, f_saxElementName);
    // read in the xml from a file (fully qualified path)
    bool ok = reader.Open(lpszPathName);
    if (ok)
    {
        std::stringstream ss;
        ss << "Document \"" << lpszPathName << "\" loaded.";
        GetLog().AddLogEntry(ss.str().c_str());
    }
    else
    {
        std::string errorMessage = reader.ErrorMessage();
        // document has failed to load. Tell the user what we can about it
        CString text;
        text.Format("The document \"%s\"\n"
            "failed to load. The XML parser reported the following problem:\n"
            "\n", lpszPathName);
        text += errorMessage.c_str();
        AfxMessageBox(text, MB_ICONERROR);
    }
    return ok;
}

BOOL CDDOBuilderDoc::OnSaveDocument(LPCTSTR lpszPathName)
{
    bool ok = false;
    try
    {
        // we do a 2 stage save to avoid file corruption
        char pathdrive[_MAX_DRIVE];
        char pathfolder[_MAX_PATH];
        _splitpath_s(lpszPathName,
            pathdrive, _MAX_DRIVE,
            pathfolder, _MAX_PATH,
            NULL, 0,        // filename
            NULL, 0);       // extension
        std::string filePath = pathdrive;
        filePath += pathfolder;
        char tempFilename[_MAX_PATH];
        GetTempFileName(filePath.c_str(), "DDO", 0, tempFilename);

        XmlLib::SaxWriter writer;
        writer.Open(tempFilename);
        writer.StartDocument(f_saxElementName);
        m_character.Write(&writer);
        writer.EndDocument();

        // saved, now rename and delete
        MoveFileEx(tempFilename, lpszPathName, MOVEFILE_REPLACE_EXISTING);
        ok = true;
        CString newBackupFilename;
        newBackupFilename = lpszPathName;
        newBackupFilename += ".backup";
        if (newBackupFilename != m_strBackupFilename)
        {
            CString strLog;
            strLog.Format("Old backup document \"%s\" deleted", (LPCTSTR)m_strBackupFilename);
            GetLog().AddLogEntry(strLog);
            // been saved under a new name, the old backup file is now obsolete
            ::DeleteFile((LPCTSTR)m_strBackupFilename);
            // now create the new backup file
            m_strBackupFilename = newBackupFilename;
            ::CopyFile(lpszPathName, (LPCTSTR)m_strBackupFilename, FALSE);
            strLog.Format("New backup document \"%s\" created", (LPCTSTR)m_strBackupFilename);
            GetLog().AddLogEntry(strLog);
        }
    }
    catch (const std::exception & e)
    {
        std::string errorMessage = e.what();
        // document has failed to save. Tell the user what we can about it
        CString text;
        text.Format("The document \"%s\"\n"
                "failed to save. The XML parser reported the following problem:\n"
                "\n", lpszPathName);
        text += errorMessage.c_str();
        AfxMessageBox(text, MB_ICONERROR);
        ok = false;
    }
    SetPathName(lpszPathName);
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

void CDDOBuilderDoc::OnUpdateRevertToBackup(CCmdUI* pCmdUI)
{
    pCmdUI->Enable(m_strBackupFilename != "");
}

void CDDOBuilderDoc::OnRevertToBackup()
{
    CString text;
    text.Format("Are you sure you want to revert all changes made since this document "
            "was first opened, or saved under a new name?");
    int ret = AfxMessageBox(text, MB_ICONQUESTION | MB_YESNO);
    if (ret == IDYES)
    {
        CString oldFilename = GetPathName();
        // default to a new empty character
        m_character = Character(this);
        m_character.AboutToLoad();
        CWaitCursor longOperation;
        // set up a reader with this as the expected root node
        XmlLib::SaxReader reader(this, f_saxElementName);
        // read in the xml from backup file (fully qualified path)
        bool ok = reader.Open((LPCTSTR)m_strBackupFilename);
        if (ok)
        {
            std::stringstream ss;
            ss << "Backup Document \"" << (LPCTSTR)m_strBackupFilename << "\" loaded.";
            GetLog().AddLogEntry(ss.str().c_str());
        }
        else
        {
            std::string errorMessage = reader.ErrorMessage();
            // document has failed to load. Tell the user what we can about it
            text.Format("The backup document %s\n"
                    "failed to load. The XML parser reported the following problem:\n"
                    "\n", (LPCTSTR)m_strBackupFilename);
            text += errorMessage.c_str();
            AfxMessageBox(text, MB_ICONERROR);
        }
        m_character.LoadComplete(); // does file format upgrades etc
        CWnd* pWnd = AfxGetApp()->m_pMainWnd;
        CMainFrame * pMF = dynamic_cast<CMainFrame*>(pWnd);
        pMF->NewDocument(this);
        // just opened the file, its unmodified!
        SetPathName(oldFilename); // ensure we are not using the backup filename
        SetModifiedFlag(FALSE);
    }
}

