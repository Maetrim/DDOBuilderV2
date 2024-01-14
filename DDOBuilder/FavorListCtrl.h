#pragma once
#include <afxcmn.h>
class CFavorListCtrl:
    public CListCtrl
{
    DECLARE_DYNAMIC(CFavorListCtrl)

public:
    CFavorListCtrl();
    virtual ~CFavorListCtrl();

protected:
    DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
};

