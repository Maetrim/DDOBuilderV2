// CMFCButtonEx

#pragma once

class CMFCButtonEx : public CMFCButton
{
    DECLARE_DYNAMIC(CMFCButtonEx)

public:
    CMFCButtonEx();
    virtual ~CMFCButtonEx();

protected:
    DECLARE_MESSAGE_MAP()

    virtual void OnDraw(CDC* pDC, const CRect& rect, UINT uiState) override;
    bool m_bDoneResize;
};

