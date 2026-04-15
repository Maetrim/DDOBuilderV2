// CMFCButtonMMRelay

#pragma once
#include "CMFCButtonEx.h"

class CMFCButtonMMRelay : public CMFCButtonEx
{
    DECLARE_DYNAMIC(CMFCButtonMMRelay)

public:
    CMFCButtonMMRelay();
    virtual ~CMFCButtonMMRelay();

protected:
    DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnMButtonDown(UINT nFlags, CPoint point) ;
    void SetTransparent();
};

