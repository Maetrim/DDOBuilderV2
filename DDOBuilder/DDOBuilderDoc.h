// DDOBuilderDoc.h : interface of the CDDOBuilderDoc class
//
#pragma once
#include "Character.h"

class CDDOBuilderDoc :
    public CDocument,
    public XmlLib::SaxContentElement
{
public:
    explicit CDDOBuilderDoc();      // needed for life export to file
    virtual BOOL OnNewDocument() override;
    virtual BOOL OnOpenDocument(LPCTSTR lpszPathName) override;
    virtual BOOL OnSaveDocument(LPCTSTR lpszPathName) override;
    virtual void Serialize(CArchive& ar) override;
    virtual ~CDDOBuilderDoc();
    afx_msg void OnUpdateRevertToBackup(CCmdUI* pCmdUI);
    afx_msg void OnRevertToBackup();

    BOOL LoadIndirect(LPCTSTR lpszPathName);

    Character* GetCharacter() { return &m_character;}

protected: // create from serialization only
    DECLARE_DYNCREATE(CDDOBuilderDoc)
    DECLARE_MESSAGE_MAP()

    // XML support
    XmlLib::SaxContentElementInterface * StartElement(
            const XmlLib::SaxString & name,
            const XmlLib::SaxAttributes & attributes);
    void EndElement();
private:
    Character m_character;
    CString m_strBackupFilename;
};
