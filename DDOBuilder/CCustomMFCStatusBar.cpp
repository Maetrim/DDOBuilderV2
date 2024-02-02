#include "stdafx.h"
#include "CCustomMFCStatusBar.h"

CCustomMFCStatusBar::CCustomMFCStatusBar() :
    m_bLockPaneText(false)
{
}

BOOL CCustomMFCStatusBar::SetPaneText(
    int nIndex,
    LPCTSTR lpszNewText,
    BOOL bUpdate)
{
    BOOL bRet = TRUE;
    if (!m_bLockPaneText)
    {
        bRet = CMFCStatusBar::SetPaneText(nIndex, lpszNewText, bUpdate);
    }
    return bRet;
}

void CCustomMFCStatusBar::SetPaneTextLockState(bool bLocked)
{
    m_bLockPaneText = bLocked;
}
