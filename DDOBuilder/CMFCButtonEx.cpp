// CMFCButtonEx.cpp : implementation file
//

#include "stdafx.h"
#include "CMFCButtonEx.h"
#include "GlobalSupportFunctions.h"

static const COLORREF clrDefault = (COLORREF) -1;
// CMFCButtonEx
IMPLEMENT_DYNAMIC(CMFCButtonEx, CMFCButton)

CMFCButtonEx::CMFCButtonEx() :
    m_bDoneResize(false)
{
}

CMFCButtonEx::~CMFCButtonEx()
{
}

BEGIN_MESSAGE_MAP(CMFCButtonEx, CMFCButton)
END_MESSAGE_MAP()

void CMFCButtonEx::OnDraw(CDC* pDC, const CRect& rect, UINT uiState)
{
    double dScaleFactor = GetDPIMultiplier(GetSafeHwnd(), true);
    CRect rectText = rect;
    CRect rectImage = rect;

    CString strText;
    GetWindowText(strText);

    CSize sizeImage(m_sizeImage);
    sizeImage.cx = static_cast<LONG>(sizeImage.cx * dScaleFactor);
    sizeImage.cy = static_cast<LONG>(sizeImage.cy * dScaleFactor);

    if (sizeImage.cx != 0)
    {
        if (!strText.IsEmpty())
        {
            if (m_bTopImage)
            {
                rectImage.bottom = rectImage.top + sizeImage.cy + GetVertMargin();
                rectText.top = rectImage.bottom;
                rectText.bottom -= GetVertMargin();
            }
            else if (m_bRightImage)
            {
                rectText.right -= sizeImage.cx + GetImageHorzMargin() / 2;
                rectImage.left = rectText.right;
                rectImage.right -= GetImageHorzMargin() / 2;
            }
            else
            {
                rectText.left +=  sizeImage.cx + GetImageHorzMargin() / 2;
                rectImage.left += GetImageHorzMargin() / 2;
                rectImage.right = rectText.left;
            }
        }

        // Center image:
        rectImage.DeflateRect((rectImage.Width() - sizeImage.cx) / 2, max(0, (rectImage.Height() - sizeImage.cy) / 2));
    }
    else
    {
        rectImage.SetRectEmpty();
    }

    // Draw text:
    CFont* pOldFont = SelectFont(pDC);
    ENSURE(pOldFont != NULL);

    pDC->SetBkMode(TRANSPARENT);
    COLORREF clrText = m_clrRegular == clrDefault ? GetGlobalData()->clrBtnText : m_clrRegular;

    if (m_bHighlighted && m_clrHover != clrDefault)
    {
        clrText = m_clrHover;
    }

    UINT uiDTFlags = DT_END_ELLIPSIS;
    BOOL bIsSingleLine = FALSE;

    if (strText.Find(_T('\n')) < 0)
    {
        uiDTFlags |= DT_VCENTER | DT_SINGLELINE;
        bIsSingleLine = TRUE;
    }
    else
    {
        rectText.DeflateRect(0, GetVertMargin() / 2);
    }

    switch (m_nAlignStyle)
    {
    case ALIGN_LEFT:
        uiDTFlags |= DT_LEFT;
        rectText.left += GetImageHorzMargin() / 2;
        break;

    case ALIGN_RIGHT:
        uiDTFlags |= DT_RIGHT;
        rectText.right -= GetImageHorzMargin() / 2;
        break;

    case ALIGN_CENTER:
        uiDTFlags |= DT_CENTER;
    }

    if (GetExStyle() & WS_EX_LAYOUTRTL)
    {
        uiDTFlags |= DT_RTLREADING;
    }

    if ((uiState & ODS_DISABLED) && m_bGrayDisabled)
    {
        pDC->SetTextColor(GetGlobalData()->clrBtnHilite);

        CRect rectShft = rectText;
        rectShft.OffsetRect(1, 1);
        OnDrawText(pDC, rectShft, strText, uiDTFlags, uiState);

        clrText = GetGlobalData()->clrGrayedText;
    }

    pDC->SetTextColor(clrText);

    if (m_bDelayFullTextTooltipSet)
    {
        BOOL bIsFullText = pDC->GetTextExtent(strText).cx <= rectText.Width();
        SetTooltip(bIsFullText || !bIsSingleLine ? NULL :(LPCTSTR) strText);
        m_bDelayFullTextTooltipSet = FALSE;
    }

    OnDrawText(pDC, rectText, strText, uiDTFlags, uiState);

    // Draw image:
    if (!rectImage.IsRectEmpty())
    {
        if (m_nStdImageId != (CMenuImages::IMAGES_IDS) -1)
        {
            CMenuImages::IMAGES_IDS id = m_nStdImageId;

            if ((uiState & ODS_DISABLED) && m_bGrayDisabled && m_nStdImageDisabledId != 0)
            {
                id = m_nStdImageDisabledId;
            }

            CMenuImages::Draw(pDC, id, rectImage.TopLeft(), m_StdImageState);
        }
        else
        {
            BOOL bIsDisabled = (uiState & ODS_DISABLED) && m_bGrayDisabled;

            CMFCToolBarImages& imageChecked = (bIsDisabled && m_ImageCheckedDisabled.GetCount() != 0) ? m_ImageCheckedDisabled :
                (m_bHighlighted && m_ImageCheckedHot.GetCount() != 0) ? m_ImageCheckedHot : m_ImageChecked;

            CMFCToolBarImages& image = (bIsDisabled && m_ImageDisabled.GetCount() != 0) ? m_ImageDisabled :
                (m_bHighlighted && m_ImageHot.GetCount() != 0) ? m_ImageHot : m_Image;

            if (!m_bDoneResize && dScaleFactor != 1.0)
            {
                imageChecked.SmoothResize(dScaleFactor);
                image.SmoothResize(dScaleFactor);
                m_bDoneResize = true;
            }
            if (m_bChecked && imageChecked.GetCount() != 0)
            {
                CAfxDrawState ds;
                imageChecked.PrepareDrawImage(ds);
                imageChecked.Draw(pDC, rectImage.left, rectImage.top, 0, FALSE, bIsDisabled && m_ImageCheckedDisabled.GetCount() == 0);
                imageChecked.EndDrawImage(ds);
            }
            else if (image.GetCount() != 0)
            {
                CAfxDrawState ds;

                image.PrepareDrawImage(ds);
                image.Draw(pDC, rectImage.left, rectImage.top, 0, FALSE, bIsDisabled && m_ImageDisabled.GetCount() == 0);
                image.EndDrawImage(ds);
            }
        }
    }

    pDC->SelectObject(pOldFont);
}

