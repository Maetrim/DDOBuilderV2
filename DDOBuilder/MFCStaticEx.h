// MFCStaticEx.h

#pragma once

#include <afxwin.h>

class CMFCStaticEx : public CStatic
{
    DECLARE_MESSAGE_MAP()

public:
    CMFCStaticEx() noexcept;
    virtual ~CMFCStaticEx(void) noexcept;

    void SetTextColour(COLORREF clrText);

private:
    afx_msg void OnPaint(void) noexcept;
    afx_msg BOOL OnEraseBkgnd(CDC* pobDC) noexcept;
    COLORREF m_obTextColour;
};
