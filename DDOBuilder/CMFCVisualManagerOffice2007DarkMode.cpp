#include "stdafx.h"
#include "CMFCVisualManagerOffice2007DarkMode.h"

IMPLEMENT_DYNCREATE(CMFCVisualManagerOffice2007DarkMode, CMFCVisualManagerOffice2007)

CMFCVisualManagerOffice2007DarkMode::CMFCVisualManagerOffice2007DarkMode()
{
    // now override the colours used in the base class

    m_clrEditBorder = RGB(255, 255, 255);
    m_clrEditSelection = RGB(128, 128, 128);

    m_clrMainClientArea = RGB(0, 0, 0);
    m_clrMenuText = RGB(255, 255, 255);
}

void CMFCVisualManagerOffice2007DarkMode::UpdateColours()
{
    m_ToolTipParams.m_clrText = RGB(255, 255, 255);
    m_ToolTipParams.m_clrFill = RGB(83, 83, 83);

    m_clrMenuLight = RGB(222, 222, 222);
    m_clrHighlight = RGB(255, 0, 0);
    m_clrHighlightDn = RGB(255, 0, 0);
    m_clrMenuBorder = RGB(255, 255, 255);
    m_clrMenuItemBorder = RGB(255, 255, 255);

    // menu background color
    m_brMenuLight.DeleteObject();
    m_brMenuLight.CreateSolidBrush(RGB(83, 83, 83));

    m_brMainClientArea.DeleteObject();
    m_brMainClientArea.CreateSolidBrush(RGB(83, 83, 83));

    //m_brMenuRarelyUsed.DeleteObject();
    //m_brMenuRarelyUsed.CreateSolidBrush(RGB(83, 83, 83));

    //m_brHighlight.DeleteObject();
    //m_brHighlight.CreateSolidBrush(RGB(83, 83, 83));

    // toolbar background colors
    m_clrToolBarGradientDark = RGB(83, 83, 83);
    m_clrToolBarGradientLight = RGB(83, 83, 83);

    //m_clrToolBarGradientVertLight = RGB(255, 0, 0);
    //m_clrToolBarGradientVertDark = RGB(255, 0, 0);

    //m_clrHighlightMenuItem = RGB(255, 0, 0);
    //m_clrHighlightGradientLight = RGB(255, 0, 0);
    //m_clrHighlightGradientDark = RGB(255, 0, 0);

    //m_colorToolBarCornerTop = RGB(255, 0, 0);
    //m_colorToolBarCornerBottom = RGB(255, 0, 0);

    //m_clrHighlightGradientLight = RGB(255, 0, 0);
    //m_clrHighlightGradientDark = RGB(255, 0, 0);
    //m_clrAppCaptionActiveStart = RGB(255, 0, 0);
    //m_clrAppCaptionActiveFinish = RGB(255, 0, 0);
    //m_clrAppCaptionInactiveStart = RGB(255, 0, 0);
    //m_clrAppCaptionInactiveFinish = RGB(255, 0, 0);
    m_clrAppCaptionActiveText = RGB(255, 255, 255);
    m_clrAppCaptionInactiveText = RGB(222, 222, 222);
    //m_clrAppCaptionActiveTitleText = RGB(255, 0, 0);
    //m_clrAppCaptionInactiveTitleText = RGB(255, 0, 0);
    m_clrMainClientArea = RGB(83, 83, 83);
    //m_clrMenuBarGradientLight = RGB(255, 0, 0);
    //m_clrMenuBarGradientDark = RGB(255, 0, 0);
    //m_clrMenuBarGradientVertLight = RGB(255, 0, 0);
    //m_clrMenuBarGradientVertDark = RGB(255, 0, 0);
    m_clrMenuBarBtnText = RGB(222, 222, 222);
    m_clrMenuBarBtnTextHighlighted = RGB(255, 255, 255);
    m_clrMenuBarBtnTextDisabled = RGB(128, 128, 128);
    m_clrToolBarBtnText = RGB(255, 0, 0);
    m_clrToolBarBtnTextHighlighted = RGB(255, 0, 0);
    m_clrToolBarBtnTextDisabled = RGB(255, 0, 0);
    m_clrMenuText = RGB(222, 222, 222);
    m_clrMenuTextHighlighted = RGB(255, 255, 255);
    m_clrMenuTextDisabled = RGB(128, 128, 128);
    //m_clrStatusBarText = RGB(255, 0, 0);
    //m_clrStatusBarTextDisabled = RGB(255, 0, 0);
    //m_clrExtenedStatusBarTextDisabled = RGB(255, 0, 0);
    //m_clrEditBorder = RGB(255, 0, 0);
    //m_clrEditBorderDisabled = RGB(255, 0, 0);
    //m_clrEditBorderHighlighted = RGB(255, 0, 0);
    //m_clrEditSelection = RGB(255, 0, 0);
    m_clrComboBorder = RGB(255, 0, 0);
    m_clrComboBorderDisabled = RGB(255, 0, 0);
    m_clrComboBorderPressed = RGB(255, 0, 0);
    m_clrComboBorderHighlighted = RGB(255, 0, 0);
    m_clrComboBtnStart = RGB(255, 0, 0);
    m_clrComboBtnFinish = RGB(255, 0, 0);
    m_clrComboBtnBorder = RGB(255, 0, 0);
    m_clrComboBtnDisabledStart = RGB(255, 0, 0);
    m_clrComboBtnDisabledFinish = RGB(255, 0, 0);
    m_clrComboBtnBorderDisabled = RGB(255, 0, 0);
    m_clrComboBtnPressedStart = RGB(255, 0, 0);
    m_clrComboBtnPressedFinish = RGB(255, 0, 0);
    m_clrComboBtnBorderPressed = RGB(255, 0, 0);
    m_clrComboBtnHighlightedStart = RGB(255, 0, 0);
    m_clrComboBtnHighlightedFinish = RGB(255, 0, 0);
    m_clrComboBtnBorderHighlighted = RGB(255, 0, 0);
    m_clrComboSelection = RGB(255, 0, 0);
    //m_clrHeaderNormalStart = RGB(255, 0, 0);
    //m_clrHeaderNormalFinish = RGB(255, 0, 0);
    //m_clrHeaderNormalBorder = RGB(255, 0, 0);
    //m_clrHeaderHighlightedStart = RGB(255, 0, 0);
    //m_clrHeaderHighlightedFinish = RGB(255, 0, 0);
    //m_clrHeaderHighlightedBorder = RGB(255, 0, 0);
    //m_clrHeaderPressedStart = RGB(255, 0, 0);
    //m_clrHeaderPressedFinish = RGB(255, 0, 0);
    //m_clrHeaderPressedBorder = RGB(255, 0, 0);
    //m_clrBarCaption = RGB(255, 0, 0);
    //m_clrMiniFrameCaption = RGB(255, 0, 0);
    //m_clrSeparator1 = RGB(255, 0, 0);
    //m_clrSeparator2 = RGB(255, 0, 0);
    //m_clrGroupText = RGB(255, 255, 0);
    //m_clrCaptionBarText = RGB(255, 0, 0);
    m_clrTaskPaneGroupCaptionHighDark = RGB(255, 0, 0);
    m_clrTaskPaneGroupCaptionHighLight = RGB(255, 0, 0);
    m_clrTaskPaneGroupCaptionHighSpecDark = RGB(255, 0, 0);
    m_clrTaskPaneGroupCaptionHighSpecLight = RGB(255, 0, 0);
    m_clrTaskPaneGroupCaptionTextSpec = RGB(255, 0, 0);
    m_clrTaskPaneGroupCaptionTextHighSpec = RGB(255, 0, 0);
    m_clrTaskPaneGroupCaptionText = RGB(255, 0, 0);
    m_clrTaskPaneGroupCaptionTextHigh = RGB(255, 0, 0);
    m_clrTabFlatBlack = RGB(255, 0, 0);
    m_clrTabFlatHighlight = RGB(255, 0, 0);
    m_clrTabTextActive = RGB(255, 0, 0);
    m_clrTabTextInactive = RGB(255, 0, 0);
    //m_clrOutlookPageTextNormal = RGB(255, 0, 0);
    //m_clrOutlookPageTextHighlighted = RGB(255, 0, 0);
    //m_clrOutlookPageTextPressed = RGB(255, 0, 0);
    //m_clrOutlookCaptionTextNormal = RGB(255, 0, 0);
    //m_clrPlannerTodayCaption[4] = RGB(255, 0, 0);
    //m_clrPlannerTodayBorder = RGB(255, 0, 0);
    //m_clrPlannerNcArea = RGB(255, 0, 0);
    //m_clrPlannerNcLine = RGB(255, 0, 0);
    //m_clrPlannerNcText = RGB(255, 0, 0);
    m_clrPopupGradientLight = RGB(83, 83, 83);
    m_clrPopupGradientDark = RGB(83, 83, 83);
    //m_clrRibbonHyperlinkInactive = RGB(255, 0, 0);
    //m_clrRibbonHyperlinkActive = RGB(255, 0, 0);
    //m_clrRibbonStatusbarHyperlinkInactive = RGB(255, 0, 0);
    //m_clrRibbonStatusbarHyperlinkActive = RGB(255, 0, 0);

    // Ribbon Bar properties
    //pDarkVisMan->SetClrRibbonBarBackground(RGB(60, 60, 60));
    //pDarkVisMan->SetClrRibbonBarTextPB(RGB(255, 255, 255));
    //pDarkVisMan->SetClrRibbonBarTextHighlightedPB(RGB(255, 255, 255));

    // Ribbon Category Properties
    //pDarkVisMan->SetClrRibbonCategoryBkTop(RGB(90, 90, 90));
    //pDarkVisMan->SetClrRibbonCategoryBkBottom(RGB(30, 30, 30));

    // Ribbon Panel Properties
    //pDarkVisMan->SetClrRibbonPanelBkTop(RGB(0, 0, 0));
    //pDarkVisMan->SetClrRibbonPanelBkBottom(RGB(90, 90, 90));
    //pDarkVisMan->SetClrRibbonPanelCaptionBk(RGB(0, 0, 0));
    //pDarkVisMan->SetClrRibbonPanelCaptionTextPB(RGB(255, 255, 255));
    //pDarkVisMan->SetClrRibbonPanelCaptionTextHighlightedPB(
     //m_clrRibbonPanelCaptionTextHighlighted = RGB(255, 255, 255);
    //pDarkVisMan->SetClrRibbonPanelTextPB(RGB(255, 255, 255));

    // Ribbon Panel outline colour
    //pDarkVisMan->SetClrRibbonPanelOutline(RGB(255, 255, 255));
    m_clrRibbonCategoryText= RGB(255, 0, 0);
    m_clrRibbonCategoryTextHighlighted= RGB(255, 0, 0);
    m_clrRibbonCategoryTextDisabled= RGB(255, 0, 0);
    m_clrRibbonPanelText= RGB(255, 0, 0);
    m_clrRibbonPanelTextHighlighted= RGB(255, 0, 0);
    m_clrRibbonPanelCaptionText= RGB(255, 0, 0);
    m_clrRibbonPanelCaptionTextHighlighted= RGB(255, 0, 0);
    m_clrRibbonKeyTipTextNormal= RGB(255, 0, 0);
    m_clrRibbonKeyTipTextDisabled= RGB(255, 0, 0);
    m_clrRibbonEdit= RGB(255, 0, 0);
    m_clrRibbonEditDisabled= RGB(255, 0, 0);
    m_clrRibbonEditHighlighted= RGB(255, 0, 0);
    m_clrRibbonEditPressed= RGB(255, 0, 0);
    m_clrRibbonEditBorder= RGB(255, 0, 0);
    m_clrRibbonEditBorderDisabled= RGB(255, 0, 0);
    m_clrRibbonEditBorderHighlighted= RGB(255, 0, 0);
    m_clrRibbonEditBorderPressed= RGB(255, 0, 0);
    m_clrRibbonEditSelection= RGB(255, 0, 0);
    m_clrRibbonComboBtnStart= RGB(255, 0, 0);
    m_clrRibbonComboBtnFinish= RGB(255, 0, 0);
    m_clrRibbonComboBtnBorder= RGB(255, 0, 0);
    m_clrRibbonComboBtnDisabledStart= RGB(255, 0, 0);
    m_clrRibbonComboBtnDisabledFinish= RGB(255, 0, 0);
    m_clrRibbonComboBtnBorderDisabled= RGB(255, 0, 0);
    m_clrRibbonComboBtnPressedStart= RGB(255, 0, 0);
    m_clrRibbonComboBtnPressedFinish= RGB(255, 0, 0);
    m_clrRibbonComboBtnBorderPressed= RGB(255, 0, 0);
    m_clrRibbonComboBtnHighlightedStart= RGB(255, 0, 0);
    m_clrRibbonComboBtnHighlightedFinish= RGB(255, 0, 0);
    m_clrRibbonComboBtnBorderHighlighted= RGB(255, 0, 0);
    m_clrRibbonContextPanelText= RGB(255, 0, 0);
    m_clrRibbonContextPanelTextHighlighted= RGB(255, 0, 0);
    m_clrRibbonContextPanelCaptionText= RGB(255, 0, 0);
    m_clrRibbonContextPanelCaptionTextHighlighted= RGB(255, 0, 0);


    m_clrTabFlatBlack = RGB(255, 0, 0);
    m_clrTabFlatHighlight = RGB(255, 0, 0);
    m_clrTabTextActive = RGB(255, 0, 0);
    m_clrTabTextInactive = RGB(255, 0, 0);
    m_clrOutlookPageTextNormal = RGB(255, 0, 0);
    m_clrOutlookPageTextHighlighted = RGB(255, 0, 0);
    m_clrOutlookPageTextPressed = RGB(255, 0, 0);
    m_clrOutlookCaptionTextNormal = RGB(255, 0, 0);
}
