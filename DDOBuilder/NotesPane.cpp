// NotesPane.cpp
//
#include "stdafx.h"

#include "NotesPane.h"
#include "GlobalSupportFunctions.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COutputBar

CNotesPane::CNotesPane() :
    CFormView(CNotesPane::IDD),
    m_pCharacter(NULL),
    m_pDocument(NULL)
{
}

CNotesPane::~CNotesPane()
{
}

IMPLEMENT_DYNCREATE(CNotesPane, CFormView)
BEGIN_MESSAGE_MAP(CNotesPane, CFormView)
    ON_WM_CREATE()
    ON_WM_SIZE()
    ON_EN_CHANGE(IDC_EDIT_NOTES, OnNotesChanged)
    ON_REGISTERED_MESSAGE(UWM_NEW_DOCUMENT, OnNewDocument)
    ON_WM_ERASEBKGND()
    ON_COMMAND(ID_EDIT_CUT, OnEditCut)
    ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
    ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
    ON_COMMAND(ID_EDIT_REDO, OnEditRedo)
    ON_COMMAND(ID_EDIT_UNDO, OnEditUndo)
END_MESSAGE_MAP()

void CNotesPane::OnInitialUpdate()
{
    CFormView::OnInitialUpdate();
    UpdateFonts();
}

LRESULT CNotesPane::OnNewDocument(WPARAM wParam, LPARAM lParam)
{
    if (m_pCharacter != NULL)
    {
        m_pCharacter->DetachObserver(this);
    }

    // wParam is the document pointer
    CDocument* pDoc = (CDocument*)(wParam);
    m_pDocument = pDoc;
    // lParam is the character pointer
    Character* pCharacter = (Character*)(lParam);
    m_pCharacter = pCharacter;
    if (m_pCharacter != NULL)
    {
        m_pCharacter->AttachObserver(this);
    }
    return 0L;
}

void CNotesPane::DoDataExchange(CDataExchange* pDX)
{
    CFormView::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_EDIT_NOTES, m_editNotes);
}

void CNotesPane::OnSize(UINT nType, int cx, int cy)
{
    CFormView::OnSize(nType, cx, cy);
    if (IsWindow(m_editNotes.GetSafeHwnd()))
    {
        m_editNotes.MoveWindow(0, 0, cx, cy, TRUE);
    }
    SetScaleToFitSize(CSize(cx, cy));
    CWnd *pWnd = this;
    while (pWnd != NULL)
    {
        pWnd = pWnd->GetParent();
    }
}

void CNotesPane::UpdateFonts()
{
    m_editNotes.SetFont(&afxGlobalData.fontRegular);
}

void CNotesPane::OnNotesChanged()
{
    Build* pBuild = m_pCharacter->ActiveBuild();
    if (pBuild != NULL)
    {
        CString text;
        m_editNotes.GetWindowText(text);
        pBuild->SetNotes((LPCTSTR)text);
    }
}

void CNotesPane::UpdateActiveBuildChanged(Character* pChar)
{
    Build* pBuild = pChar->ActiveBuild();
    if (pBuild != NULL)
    {
        m_editNotes.SetWindowText(pBuild->Notes().c_str());
    }
}

BOOL CNotesPane::OnEraseBkgnd(CDC* pDC)
{
    static int controlsNotToBeErased[] =
    {
        IDC_EDIT_NOTES,
        0 // end marker
    };

    return OnEraseBackground(this, pDC, controlsNotToBeErased);
}

void CNotesPane::OnEditCut()
{
    if (GetFocus() == &m_editNotes)
    {
        m_editNotes.Cut();
    }
}

void CNotesPane::OnEditCopy()
{
    if (GetFocus() == &m_editNotes)
    {
        m_editNotes.Copy();
    }
}

void CNotesPane::OnEditPaste()
{
    if (GetFocus() == &m_editNotes)
    {
        m_editNotes.Paste();
    }
}

void CNotesPane::OnEditRedo()
{
    if (GetFocus() == &m_editNotes)
    {
        //m_editNotes.Redo();
    }
}

void CNotesPane::OnEditUndo()
{
    if (GetFocus() == &m_editNotes)
    {
        m_editNotes.Undo();
    }
}

