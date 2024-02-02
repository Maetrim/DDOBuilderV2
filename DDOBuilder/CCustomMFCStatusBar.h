#pragma once
#include <afxstatusbar.h>
class CCustomMFCStatusBar:
    public CMFCStatusBar
{
public:
    CCustomMFCStatusBar();
    virtual BOOL SetPaneText(int nIndex, LPCTSTR lpszNewText, BOOL bUpdate = TRUE) override;
    void SetPaneTextLockState(bool bLocked);
private:
    bool m_bLockPaneText;
};

