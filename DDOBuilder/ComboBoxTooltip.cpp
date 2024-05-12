// ComboBoxTooltip.cpp
//
// This specialised combo box has two functions:
// 1: It can display images to the left of items (set an image list on it)
// 2: When the drop list is displayed it notifies its parent which item
//    is selected so it can display a tooltip. This is done via a WM_MOUSEHOVER message
#include "stdafx.h"
#include "ComboBoxTooltip.h"
#include "GlobalSupportFunctions.h"

namespace
{
    COLORREF f_selectedColour = ::GetSysColor(COLOR_HIGHLIGHT);
    COLORREF f_backgroundColour = ::GetSysColor(COLOR_BTNFACE); // grey
    COLORREF f_backgroundColourDark = RGB(83, 83, 83);
    COLORREF f_white = RGB(255, 255, 255);                      // white
    COLORREF f_black = RGB(0, 0, 0);                            // black
}

// CComboBoxTooltip
IMPLEMENT_DYNAMIC(CComboBoxTooltip, CComboBox)

CComboBoxTooltip::CComboBoxTooltip() :
    m_bHasImageList(false),
    m_selection(CB_ERR),
    m_bSubclassedListbox(false),
    m_bCanRemoveItems(false)
{
}

CComboBoxTooltip::~CComboBoxTooltip()
{
}

void CComboBoxTooltip::SetCanRemoveItems()
{
    m_bCanRemoveItems = true;
}

BEGIN_MESSAGE_MAP(CComboBoxTooltip, CComboBox)
    //ON_WM_SETFOCUS()
    //ON_WM_RBUTTONDOWN()
END_MESSAGE_MAP()

void CComboBoxTooltip::SetImageList(CImageList* il)
{
    m_bHasImageList = false;
    m_imageList.DeleteImageList();  // clear any previous image list
    if (il != NULL)
    {
        // create from list passed in
        m_imageList.Create(il);
        IMAGEINFO imageInfo;
        m_imageList.GetImageInfo(0, &imageInfo);
        // ensure the combo edit control height is correct
        CRect iconRect(imageInfo.rcImage);
        SetItemHeight(-1, iconRect.Height());
        m_bHasImageList = true;
    }
}

void CComboBoxTooltip::DrawItem(LPDRAWITEMSTRUCT lpDis)
{
    bool bDarkMode = DarkModeEnabled();
    bool isDropped = (GetDroppedState() != 0);
    if (!m_bSubclassedListbox)
    {
        COMBOBOXINFO cbInfo;
        memset(&cbInfo, 0, sizeof(COMBOBOXINFO));
        cbInfo.cbSize = sizeof(COMBOBOXINFO);
        GetComboBoxInfo(&cbInfo);
        m_delayedListBox.SetOwner(this);
        m_delayedListBox.SubclassWindow(cbInfo.hwndList);
        m_bSubclassedListbox = true;
    }
    if (lpDis->itemID != CB_ERR)
    {
        CDC* pDC = CDC::FromHandle(lpDis->hDC);
        CRect rcItem = lpDis->rcItem;
        int nState = lpDis->itemState;
        int nItem = lpDis->itemID;
        int nIndexDC = pDC->SaveDC();

        // highlight the background if selected.
        if (nState & ODS_SELECTED)
        {
            pDC->FillSolidRect(rcItem, f_selectedColour);
            pDC->SetTextColor(f_white);
            if (isDropped
                && lpDis->itemID != static_cast<UINT>(m_selection)
                && lpDis->itemAction == ODA_SELECT
                && (nState & 0x1000) == 0)  // Cancel operation
            {
                m_selection = lpDis->itemID;
                // this is the item we need to notify a hover about
                ::SendMessage(
                    GetParent()->GetSafeHwnd(),
                    WM_MOUSEHOVER,
                    lpDis->itemID,
                    GetDlgCtrlID());
            }
        }
        else
        {
            // white background, black text
            pDC->FillSolidRect(rcItem, bDarkMode ? f_backgroundColourDark  : f_white);
            pDC->SetTextColor(bDarkMode ? f_white : f_black);
        }

        // draw image if it has one
        CRect rcText(lpDis->rcItem);
        if (m_bHasImageList)
        {
            IMAGEINFO imageInfo;
            m_imageList.GetImageInfo(0, &imageInfo);
            CRect iconRect(imageInfo.rcImage);
            // ensure text is drawn to the right of the image
            rcText.left += rcItem.Height() + 2;
            m_imageList.Draw(pDC, lpDis->itemData, rcItem.TopLeft(), ILD_NORMAL);
        }

        pDC->SetBkMode(TRANSPARENT);    // draw text in transparent mode
        // Draw the text item.
        CString strFontName;
        GetLBText(nItem, strFontName);
        pDC->DrawText(strFontName, rcText, DT_VCENTER | DT_SINGLELINE);

        // Restore the original device context.
        pDC->RestoreDC(nIndexDC);
    }
}

void CComboBoxTooltip::MeasureItem(LPMEASUREITEMSTRUCT lpMis)
{
    ASSERT(lpMis->CtlType == ODT_COMBOBOX);
    if (m_bHasImageList)
    {
        IMAGEINFO imageInfo;
        m_imageList.GetImageInfo(0, &imageInfo);
        CRect iconRect(imageInfo.rcImage);
        lpMis->itemHeight = max(::GetSystemMetrics(SM_CYVTHUMB), iconRect.Height());
        lpMis->itemWidth = 0;
    }
    else
    {
        // default control height
        lpMis->itemHeight = ::GetSystemMetrics(SM_CYVTHUMB);
        lpMis->itemWidth = 0;
    }
}

int CComboBoxTooltip::CompareItem(LPCOMPAREITEMSTRUCT lpCis)
{
    ASSERT(lpCis->CtlType == ODT_COMBOBOX);

    int iItem1 = (int)lpCis->itemID1;
    int iItem2 = (int)lpCis->itemID2;
    if (iItem1 != CB_ERR && iItem2 != CB_ERR)
    {
        CString strItem1;
        GetLBText(iItem1, strItem1);
        CString strItem2;
        GetLBText(iItem2, strItem2);
        return strItem1.Collate(strItem2);
    }
    return -1;
}

void CComboBoxTooltip::DeleteItem(LPDELETEITEMSTRUCT lpDis)
{
    UNREFERENCED_PARAMETER(lpDis);
}

void CComboBoxTooltip::OnSetFocus(CWnd* pWnd)
{
    m_delayedListBox.ResetTimer();
    CComboBox::OnSetFocus(pWnd);
}

void CComboBoxTooltip::OnRButtonDown(int selection)
{
    CPoint pt;
    GetCursorPos(&pt);
    CWnd* pWnd = WindowFromPoint(pt);
    if (pWnd == this
            || pWnd == &m_delayedListBox)
    {
        if (m_bCanRemoveItems)
        {
            if (selection != CB_ERR)
            {
                CString strItem;
                GetLBText(selection, strItem);
                GetParent()->SendMessage(UWM_TOGGLE_INCLUDED, selection, (LPARAM)(&strItem));
                // force a mouse move after click to get a new selection in the control
                // as an item may have been deleted
                CPoint p;
                GetCursorPos(&p);
                p.x += 1;
                SetCursorPos(p.x, p.y);
                if (selection >= GetCount())
                {
                    selection = GetCount() - 1;
                }
                // ensure any tooltip updates
                // this is the item we need to notify a hover about
                ::SendMessage(
                    GetParent()->GetSafeHwnd(),
                    WM_MOUSEHOVER,
                    selection,
                    GetDlgCtrlID());
            }
        }
    }
}
