// This is a part of the Microsoft Foundation Classes C++ library.
#pragma once

class COutlookControl : public CWnd
{
    DECLARE_SERIAL(COutlookControl)

// Construction
public:
	COutlookControl();
	virtual ~COutlookControl();

	virtual BOOL Create(
            LPCTSTR lpszCaption,
            CWnd* pParentWnd,
            const RECT& rect,
            UINT nID,
            DWORD dwStyle);

protected:
	//{{AFX_MSG(COutlookControl)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnPaint();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:
	CObList m_lstCustomPages;
	CString m_strBarName;
};
