
#pragma once
#include "Resource.h"

/////////////////////////////////////////////////////////////////////////////
// COutputList window

class COutputList : public CListBox
{
// Construction
public:
    COutputList();

// Implementation
public:
    virtual ~COutputList();

    afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
    afx_msg void OnEditCopy();
    afx_msg void OnEditClear();

    DECLARE_MESSAGE_MAP()
};

enum LogLevel
{
    Log_Always = 0,
    Log_Developer,
};

class CLogPane :
    public CFormView
{
// Construction
public:
    CLogPane();
    virtual ~CLogPane();
    enum { IDD = IDD_LOG_PANE };

    void UpdateFonts();

    void AddLogEntry(const CString & strText);
    void UpdateLastLogEntry(const CString & strText);

protected:
    virtual void DoDataExchange(CDataExchange* pDX) override;
    virtual void OnInitialUpdate() override;
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnCopyLogToClipboard();
    afx_msg void OnClearLog();

    DECLARE_DYNCREATE(CLogPane)
    DECLARE_MESSAGE_MAP()

private:
    COutputList m_wndOutputLog;

};

