// NotesPane.h
//
#pragma once
#include "Resource.h"
#include "Character.h"

class CNotesPane :
    public CFormView,
    public CharacterObserver
{
// Construction
public:
    CNotesPane();
    virtual ~CNotesPane();
    enum { IDD = IDD_NOTES_PANE };

    void UpdateFonts();

protected:
    virtual void OnInitialUpdate() override;
    virtual void DoDataExchange(CDataExchange* pDX) override;
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnNotesChanged();
    afx_msg LRESULT OnNewDocument(WPARAM wParam, LPARAM lParam);
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg void OnEditCut();
    afx_msg void OnEditCopy();
    afx_msg void OnEditPaste();
    afx_msg void OnEditRedo();
    afx_msg void OnEditUndo();

    DECLARE_DYNCREATE(CNotesPane)
    DECLARE_MESSAGE_MAP()

private:
    // CharacterObserver
    virtual void UpdateActiveBuildChanged(Character* pCharacter) override;

    CEdit m_editNotes;
    CDocument* m_pDocument;
    Character* m_pCharacter;
};

