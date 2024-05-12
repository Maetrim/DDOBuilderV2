#include "stdafx.h"
#include "MFCCheckBox.h"
#include "MFCPaintDC.h"
#include "WinSystem.h"
#include <Uxtheme.h>

CMFCCheckBox::CMFCCheckBox(
) noexcept
{
}

CMFCCheckBox::~CMFCCheckBox(
) noexcept
{
}

void CMFCCheckBox::SetColours(
    const CMFCControlColours& krobColours
) noexcept
{
    CMFCButtonEx::SetColours(krobColours);
}

void CMFCCheckBox::ClearColours(
) noexcept
{
    CMFCButtonEx::ClearColours();
}

//---------------------------------------------------------------------------
// ButtonPaint
//---------------------------------------------------------------------------
void CMFCCheckBox::ButtonPaint(CDC *pDC)
{
    CRect obClient;
    GetClientRect(obClient);
    HDC defSafeHDC = robDC.GetSafeHDC();
    HWND defHwnd = GetSafeHwnd();
    DrawThemeParentBackgroundEx(defHwnd, pDC->GetSafeHdc(),
                                               DTPB_USEERASEBKGND, obClient);
    CRect obRectTextFrame, obRectCBFrame;
    GetDrawSections(obRectCBFrame, obRectTextFrame);

    CPaintDC* pobDeviceContext = robDC.GetDeviceContext();
    UINT uDFCState = DFCS_BUTTONCHECK;
    BOOL bWindowEnabled = IsWindowEnabled();
    if (FALSE == bWindowEnabled)
    {
        uDFCState |= DFCS_INACTIVE;
        pobDeviceContext->SetTextColor(GetSysColor(COLOR_INACTIVECAPTIONTEXT));
    }

    int nChecked = GetCheck();
    if (0 != (nChecked & BST_CHECKED))
    {
        uDFCState |= DFCS_CHECKED;
    }

    int nState = GetState();
    if (0 != (nState & BST_PUSHED))
    {
        uDFCState |= DFCS_PUSHED;
    }
    BOOL bResult = ::DrawFrameControl(defSafeHDC, obRectCBFrame,
                                                      DFC_BUTTON, uDFCState);
    if (FALSE == bResult)
    {
        U_GENERAL_EXCEPTION_EXTRA(UNEXPECTED_RESULT, 
              _T("CMFCCheckBox::ButtonPaint"), _T("DrawFrameControl failed"))
    }

    DWORD dwControlStyle = GetStyle();
    DWORD dwStyle = DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS;
    if (dwControlStyle & BS_RIGHT)
    {
        dwStyle |= DT_RIGHT;
    }
    else if (dwControlStyle & BS_LEFT)
    {
        dwStyle |= DT_LEFT;
    }
    else
    {
        dwStyle |= DT_CENTER;
    }

    CString strText;
    GetWindowText(strText);

    BOOL bSuccess = pobDeviceContext->DrawText(strText, obRectTextFrame, 
                                                                    dwStyle);
    if (FALSE == bSuccess)
    {
        U_GENERAL_EXCEPTION_EXTRA(UNEXPECTED_RESULT,
                                     _T("CMFCCheckBox::ButtonPaint"), _T(""))
    }
}

#pragma warning(disable:6387) // defTheme can be NULL

//---------------------------------------------------------------------------
// GetDrawSections
//---------------------------------------------------------------------------
void CMFCCheckBox::GetDrawSections(
    CRect& robRectCBFrame, 
    CRect& robRectTextFrame
) throw(CU_GeneralException)
{
    GetClientRect(robRectTextFrame);
    robRectCBFrame = robRectTextFrame;

    HTHEME defTheme = ::OpenThemeData(m_hWnd, L"BUTTON");
    if (defTheme == NULL)
    {
        U_GENERAL_EXCEPTION_EXTRA(UNEXPECTED_RESULT,
             _T("CMFCCheckBox::GetDrawSections"), _T("OpenThemeData failed"))
    }

    SIZE defRBSize;
    HRESULT defResult = GetThemePartSize(defTheme, NULL, BP_CHECKBOX,
                        CBS_UNCHECKEDNORMAL, NULL, TS_TRUE, &defRBSize);
    if (defResult != S_OK)
    {
        CloseThemeData(defTheme);
        U_GENERAL_EXCEPTION_EXTRA(UNEXPECTED_RESULT,
          _T("CMFCCheckBox::GetDrawSections"), _T("GetThemePartSize failed"))
    }

    int nXEdge = 0;
    try
    {
        nXEdge = CWinSystem::GetSystemMetric(SM_CXEDGE);
    }
    catch(CU_GeneralException & robErr)
    {
        U_RETHROW(robErr, _T("CMFCCheckBox::GetDrawSections"), _T(""))
    }

    DWORD dwControlStyle = GetStyle();
    bool bLeftSideText = (dwControlStyle & BS_LEFTTEXT);
    if (bLeftSideText)
    {
        robRectCBFrame.left = robRectCBFrame.right - defRBSize.cx;
        robRectTextFrame.right = robRectCBFrame.left - nXEdge;
    }
    else
    {
        robRectCBFrame.right = robRectCBFrame.left + defRBSize.cx;
        robRectTextFrame.left = robRectCBFrame.right + nXEdge;
    }
    
    defResult = CloseThemeData(defTheme);
    if (defResult != S_OK)
    {
        U_GENERAL_EXCEPTION_EXTRA(UNEXPECTED_RESULT,
                                  _T("CMFCCheckBox::GetDrawSections"),
                                                 _T("CloseThemeData failed"))
    }
}

#pragma warning(default:6387)

//---------------------------------------------------------------------------
// re-enable warnings
//---------------------------------------------------------------------------
#pragma warning( default : 4290 )
