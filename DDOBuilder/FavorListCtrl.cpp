#include "stdafx.h"
#include "FavorListCtrl.h"

IMPLEMENT_DYNAMIC(CFavorListCtrl, CListCtrl)

CFavorListCtrl::CFavorListCtrl()
{
}

CFavorListCtrl::~CFavorListCtrl()
{
}

BEGIN_MESSAGE_MAP(CFavorListCtrl, CListCtrl)
    ON_WM_RBUTTONUP()
END_MESSAGE_MAP()

void CFavorListCtrl::OnRButtonUp(UINT nFlags, CPoint point) 
{
    UNREFERENCED_PARAMETER(nFlags);
    UNREFERENCED_PARAMETER(point);
    NMHDR hdr;
    hdr.code = NM_RCLICK;
    hdr.hwndFrom = this->GetSafeHwnd();
    hdr.idFrom = GetDlgCtrlID();
    GetParent()->SendMessage(WM_NOTIFY, (WPARAM)hdr.idFrom, (LPARAM)&hdr);
}
