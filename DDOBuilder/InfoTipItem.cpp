// InfoTipItem.cpp
//

#include "stdafx.h"
#include "InfoTipItem.h"

#include "Build.h"
#include "EnhancementTreeItem.h"
#include "SetBonus.h"
#include "GlobalSupportFunctions.h"

bool InfoTipItem::s_bFontsCreated = false;
CFont InfoTipItem::s_standardFont;
CFont InfoTipItem::s_boldFont;
CFont InfoTipItem::s_smallFont;

// Base class
InfoTipItem::InfoTipItem() :
    m_requiredSize(0, 0)
{
    if (!s_bFontsCreated)
    {
        // create the fonts used
        LOGFONT lf;
        ZeroMemory((PVOID)&lf, sizeof(LOGFONT));
        NONCLIENTMETRICS nm;
        nm.cbSize = sizeof(NONCLIENTMETRICS);
        VERIFY(SystemParametersInfo(SPI_GETNONCLIENTMETRICS, nm.cbSize, &nm, 0));
        lf = nm.lfMenuFont;
        s_standardFont.CreateFontIndirect(&lf);
        lf.lfWeight = FW_BOLD;
        s_boldFont.CreateFontIndirect(&lf);

        LOGFONT lfs;
        ZeroMemory((PVOID)&lfs, sizeof(LOGFONT));
        strcpy_s(lfs.lfFaceName, "Consolas");
        lfs.lfHeight = 11;
        VERIFY(s_smallFont.CreateFontIndirect(&lfs) != 0);
        s_bFontsCreated = true;
    }
}

InfoTipItem::~InfoTipItem()
{
}

CSize InfoTipItem::Size() const
{
    return m_requiredSize;
}

// Header item
bool InfoTipItem_Header::LoadIcon(
        const std::string& path,
        const std::string& filename,
        bool bReportError)
{
    m_image.Destroy();
    HRESULT hr = LoadImageFile(path, filename, &m_image, CSize(32, 32), bReportError);
    if (hr == S_OK)
    {
        m_image.SetTransparentColor(c_transparentColour);
    }
    return (hr == S_OK);
}

void InfoTipItem_Header::IconFromImageList(CImageList& il, int index)
{
    m_image.Destroy();

    CImageList tmpList;
    tmpList.Create(&il);

    if (index != 0)
    {
        // Then swap the requested image to the first spot in the list 
        tmpList.Copy(0, index, ILCF_SWAP);
    }

    // Now we need to get some information about the image 
    IMAGEINFO lastImage;
    tmpList.GetImageInfo(0, &lastImage);

    // Create a Compatible Device Context using 
    // the valid DC of your calling window
    CDC screenDC;
    screenDC.Attach(::GetDC(NULL));
    CDC dcMem;
    dcMem.CreateCompatibleDC(&screenDC);

    // This rect simply stored the size of the image we need
    CRect rect(lastImage.rcImage);

    // Create a bitmap  compatible with the window DC
    CBitmap bmp;
    bmp.CreateCompatibleBitmap(&screenDC, rect.Width(), rect.Height());
    ::ReleaseDC(NULL, screenDC.Detach());

    // Select the new destination bitmap into the DC we created above
    CBitmap* pBmpOld = dcMem.SelectObject(&bmp);

    //This call apparently "draws" the bitmap from the list, 
    //onto the new destination bitmap
    tmpList.DrawIndirect(&dcMem, 0, CPoint(0, 0), CSize(rect.Width(), rect.Height()), CPoint(0, 0));

    //cleanup by reselecting the old bitmap object into the DC
    dcMem.SelectObject(pBmpOld);
    dcMem.DeleteDC();
    HBITMAP hBitmap = static_cast<HBITMAP>(bmp.Detach());
    m_image.Attach(hBitmap);
    m_image.SetTransparentColor(c_transparentColour);
}

void InfoTipItem_Header::SetTitle(const CString& strTitle)
{
    m_title = strTitle;
}

void InfoTipItem_Header::SetTitle2(const CString& strTitle2)
{
    m_title2 = strTitle2;
}

void InfoTipItem_Header::SetCost(const CString& strCost)
{
    m_cost = strCost;
}

void InfoTipItem_Header::SetRank(const CString& strRank)
{
    m_ranks = strRank;
}

CSize InfoTipItem_Header::Measure(CDC* pDC)
{
    // +---------------------------------------------------+
    // | +----+ Title Line 1                        Cost x |
    // | |icon| Title Line 2                       Ranks n |
    // | +----+                                            |
    // +---------------------------------------------------+
    pDC->SaveDC();
    pDC->SelectObject(s_boldFont);
    CRect rctRequiredSize(0, 0, 0, 0);
    // border space to left/right first
    rctRequiredSize.InflateRect(c_controlSpacing, 0, c_controlSpacing, 0);
    // Calculate the area for the tip text
    CRect rctTitle;
    CRect rctTitle2;
    pDC->DrawText(m_title, &rctTitle, DT_CALCRECT | DT_LEFT | DT_EXPANDTABS | DT_NOPREFIX);
    pDC->DrawText(m_title2, &rctTitle2, DT_CALCRECT | DT_LEFT | DT_EXPANDTABS | DT_NOPREFIX);
    rctRequiredSize.bottom += max(rctTitle.Height() * 2, 32);      // 2 lines or icon height
    rctRequiredSize.bottom += c_controlSpacing;                    // also needs a border
    // cost and ranks could be empty (optional)
    CRect rctCost;
    CRect rctRanks;
    pDC->DrawText(m_cost, &rctCost, DT_CALCRECT | DT_LEFT | DT_EXPANDTABS | DT_NOPREFIX);
    pDC->DrawText(m_ranks, &rctRanks, DT_CALCRECT | DT_LEFT | DT_EXPANDTABS | DT_NOPREFIX);
    if (rctCost.Width() == 0 && rctRanks.Width() == 0)
    {
        // no text to right, just icon and title for the total width
        rctRequiredSize.right += 32 + c_controlSpacing + max(rctTitle.Width(), rctTitle2.Width());
    }
    else
    {
        // use largest of text to right + set gap and icon + title for total width
        rctRequiredSize.right += 32 + c_controlSpacing + max(rctTitle.Width(), rctTitle2.Width());
        rctRequiredSize.right += max(rctCost.Width(), rctRanks.Width());
        CRect rctGap;
        pDC->DrawText("Gap Size", &rctGap, DT_CALCRECT | DT_LEFT | DT_EXPANDTABS | DT_NOPREFIX);
        rctRequiredSize.right += rctGap.Width();
    }
    pDC->RestoreDC(-1);
    m_requiredSize = rctRequiredSize.Size();               // save actual required size in base object
    return m_requiredSize;
}

void InfoTipItem_Header::Draw(CDC* pDC, const CRect& rect)
{
    // note that Rect is the area this item needs to draw into
    // the background has already been erased
    // draw the icon
    m_image.TransparentBlt(
            pDC->GetSafeHdc(),
            CRect(rect.left + c_controlSpacing, rect.top + c_controlSpacing, rect.left + c_controlSpacing + 32, rect.top + c_controlSpacing + 32),
            CRect(0, 0, 32, 32));

    // render the title in bold
    pDC->SaveDC();
    pDC->SelectObject(s_boldFont);
    CRect rcTitle(rect.left + c_controlSpacing + 32 + c_controlSpacing, rect.top + c_controlSpacing, rect.right, rect.bottom);
    pDC->DrawText(m_title, rcTitle, DT_LEFT | DT_EXPANDTABS | DT_NOPREFIX);
    CSize sizeTitle = pDC->GetTextExtent(m_title);
    rcTitle += CPoint(0, sizeTitle.cy);
    pDC->DrawText(m_title2, rcTitle, DT_LEFT | DT_EXPANDTABS | DT_NOPREFIX);
    CSize sizeCost = pDC->GetTextExtent(m_cost);
    CSize sizeRanks = pDC->GetTextExtent(m_ranks);
    pDC->TextOut(rect.right - c_controlSpacing - sizeCost.cx, rect.top + c_controlSpacing, m_cost);
    COLORREF old = pDC->SetTextColor(RGB(0,128, 0));
    pDC->TextOut(rect.right - c_controlSpacing - sizeRanks.cx, rect.top + c_controlSpacing + sizeCost.cy, m_ranks);
    pDC->SetTextColor(old);
    pDC->RestoreDC(-1);
}

// SLA Header item
CSize InfoTipItem_SLAHeader::Measure(CDC* pDC)
{
    // +---------------------------------------------------+
    // | Caster Level: xx                                  |
    // | [         x/x Charges (Recharged/Day: y)         ]|
    // +---------------------------------------------------+
    pDC->SaveDC();
    pDC->SelectObject(s_boldFont);
    CRect rctRequiredSize(0, 0, 0, 0);
    // Calculate the area for the tip text
    CRect rctCLText;
    CString clText;
    if (m_nCasterLevel > 0)
    {
        clText.Format("Caster Level: %d", m_nCasterLevel);
        pDC->DrawText(clText, &rctCLText, DT_CALCRECT | DT_LEFT | DT_EXPANDTABS | DT_NOPREFIX);
        rctRequiredSize.bottom += rctCLText.Height();
    }

    CRect rctCharges;
    CString chargesText;
    if (m_nRecharge > 0)
    {
        chargesText.Format("%d / %d Charges (Recharged/Day: %d)", m_nCharges, m_nCharges, m_nRecharge);
    }
    else
    {
        chargesText.Format("%d / %d Charges", m_nCharges, m_nCharges);
    }
    pDC->DrawText(chargesText, &rctCharges, DT_CALCRECT | DT_LEFT | DT_EXPANDTABS | DT_NOPREFIX);
    rctRequiredSize.bottom += rctCharges.Height() + c_controlSpacing * 2;
    rctRequiredSize.right = max(rctCharges.Width(), rctCLText.Width());
    // border space to left/right
    rctRequiredSize.InflateRect(c_controlSpacing, 0, c_controlSpacing, 0);
    pDC->RestoreDC(-1);
    m_requiredSize = rctRequiredSize.Size();               // save actual required size in base object
    return m_requiredSize;
}

void InfoTipItem_SLAHeader::Draw(CDC* pDC, const CRect& rect)
{
    CRect rct(rect);
    rct.left += c_controlSpacing;
    rct.right -= c_controlSpacing;
    pDC->SaveDC();
    pDC->SelectObject(s_boldFont);
    CString clText;
    CSize sizeText(0, 0);
    CRect rctCharges(rct.left, rct.top, rct.right, rct.bottom);
    if (m_nCasterLevel > 0)
    {
        clText.Format("Caster Level: %d", m_nCasterLevel);
        sizeText = pDC->GetTextExtent(clText);
        pDC->DrawText(clText, rct, DT_LEFT | DT_EXPANDTABS | DT_NOPREFIX);
        rctCharges.top += sizeText.cy;
    }
    else
    {
        sizeText = pDC->GetTextExtent("Text Height");
    }

    rctCharges.bottom = rctCharges.top + sizeText.cy + c_controlSpacing * 2;
    CString chargesText;
    if (m_nRecharge > 0)
    {
        chargesText.Format("%d / %d Charges (Recharged/Day: %d)", m_nCharges, m_nCharges, m_nRecharge);
    }
    else
    {
        chargesText.Format("%d / %d Charges", m_nCharges, m_nCharges);
    }

    CBrush greenBrush(RGB(24, 74, 34));
    pDC->FillRect(rctCharges, &greenBrush);

    pDC->SetTextColor(RGB(185, 233, 175));
    rctCharges.top += c_controlSpacing;

    pDC->DrawText(chargesText, rctCharges, DT_CENTER | DT_VCENTER | DT_EXPANDTABS | DT_NOPREFIX);
    pDC->RestoreDC(-1);
}

// Requirements Item
void InfoTipItem_Requirements::AddRequirement(
        const CString& text,
        bool bMet)
{
    m_requirements.push_back(text);
    m_bRequirementMet.push_back(bMet);
}

void InfoTipItem_Requirements::CreateRequirementsStrings(
        const Build& build,
        const EnhancementTreeItem* pItem,
        size_t spentInTree)
{
    // add the required spent in tree to requirements list
    if (pItem->MinSpent() > 0)
    {
        CString text;
        text.Format("Requires: %d AP spent in tree", pItem->MinSpent());
        AddRequirement(text, spentInTree >= pItem->MinSpent());
    }
    pItem->CreateRequirementStrings(
            build,
            &m_requirements,
            &m_bRequirementMet,
            build.Level()-1);
}

void InfoTipItem_Requirements::CreateRequirementsStrings(
    const Build& build,
    const EnhancementTreeItem* pItem,
    const EnhancementSelection* pSelection,
    size_t spentInTree)
{
    // add the required spent in tree to requirements list
    if (pItem->MinSpent(pSelection->Name()) > 0)
    {
        CString text;
        text.Format("Requires: %d AP spent in tree", pItem->MinSpent(pSelection->Name()));
        AddRequirement(text, spentInTree >= pItem->MinSpent(pSelection->Name()));
    }
    if (pSelection->HasRequirementsToTrain())
    {
        pSelection->CreateRequirementStrings(
                build,
                &m_requirements,
                &m_bRequirementMet,
                build.Level()-1);
    }
    else
    {
        pItem->CreateRequirementStrings(
                build,
                &m_requirements,
                &m_bRequirementMet,
                build.Level() - 1);
    }
}

void InfoTipItem_Requirements::CreateRequirementsStrings(
        const Build& build,
        const Requirements& req)
{
    req.CreateRequirementStrings(
            build,
            build.Level()-1,
            true,
            &m_requirements,
            &m_bRequirementMet);
}

void InfoTipItem_Requirements::SetColour(COLORREF inactive, COLORREF active)
{
    m_inactiveColour = inactive;
    m_activeColour = active;
}

void InfoTipItem_Requirements::CreateSetBonusStrings(
        const SetBonus& setBonus,
        size_t numStacks)
{
    for (auto&& bit : setBonus.Buffs())
    {
        CString entry;
        entry.Format("    %d: %s", bit.EquippedCount(), bit.Description().c_str());
        AddRequirement(entry, bit.EquippedCount() <= numStacks);
    }
}

CSize InfoTipItem_Requirements::Measure(CDC* pDC)
{
    pDC->SaveDC();
    pDC->SelectObject(s_standardFont);
    int width = 0;
    int height = 0;
    ASSERT(m_requirements.size() == m_bRequirementMet.size());
    for (size_t ri = 0; ri < m_requirements.size(); ++ri)
    {
        CString text = m_requirements[ri];
        if (text == "")
        {
            // need to measure correct text height for a blank line
            text = "A";
        }
        CRect rctRequirement;
        pDC->DrawText(
                text,
                &rctRequirement,
                DT_CALCRECT | DT_LEFT | DT_EXPANDTABS | DT_NOPREFIX);
        width = max(width, rctRequirement.Width());
        height += rctRequirement.Height();
    }
    pDC->RestoreDC(-1);
    m_requiredSize = CSize(width + c_controlSpacing * 2, height);
    return m_requiredSize;
}

void InfoTipItem_Requirements::Draw(CDC* pDC, const CRect& rect)
{
    pDC->SaveDC();
    pDC->SelectObject(s_standardFont);
    COLORREF old = pDC->GetTextColor();
    int top = rect.top;
    ASSERT(m_requirements.size() == m_bRequirementMet.size());
    for (size_t ri = 0; ri < m_requirements.size(); ++ri)
    {
        CString text = m_requirements[ri];
        // text drawn in green or red depending on whether the requirement is met or not
        pDC->SetTextColor(m_bRequirementMet[ri] ? m_activeColour : m_inactiveColour);
        CString copy(text);
        copy += "A";            // ensure blank lines are correct height
        CRect rctRequirement;
        pDC->DrawText(
            copy,
            &rctRequirement,
            DT_CALCRECT | DT_LEFT | DT_EXPANDTABS | DT_NOPREFIX);
        rctRequirement += CPoint(c_controlSpacing, top);
        pDC->DrawText(
                text,
                &rctRequirement,
                DT_LEFT | DT_EXPANDTABS | DT_NOPREFIX);
        top += rctRequirement.Height();
    }
    pDC->SetTextColor(old);
    pDC->RestoreDC(-1);
}

// multiline text item
void InfoTipItem_MultilineText::SetText(const CString& text)
{
    m_text = text;
}

CSize InfoTipItem_MultilineText::Measure(CDC* pDC)
{
    pDC->SaveDC();
    pDC->SelectObject(s_standardFont);
    CRect rctText;
    pDC->DrawText(m_text, &rctText, DT_CALCRECT | DT_LEFT | DT_EXPANDTABS | DT_NOPREFIX);
    pDC->RestoreDC(-1);
    int width = c_controlSpacing * 2 + rctText.Width();
    int height = rctText.Height();
    m_requiredSize = CSize(width, height);
    return m_requiredSize;
}

void InfoTipItem_MultilineText::Draw(CDC* pDC, const CRect& rect)
{
    pDC->SaveDC();
    pDC->SelectObject(s_standardFont);
    CRect rctText(rect.left + c_controlSpacing, rect.top, rect.right - c_controlSpacing, rect.bottom);
    pDC->DrawText(m_text, rctText, DT_LEFT | DT_EXPANDTABS | DT_NOPREFIX);
    pDC->RestoreDC(-1);
}

// buff description text item
CSize InfoTipItem_BuffDescription::Measure(CDC* pDC)
{
    pDC->SaveDC();
    pDC->SelectObject(s_standardFont);
    CRect rctText;
    pDC->DrawText(m_text, &rctText, DT_CALCRECT | DT_LEFT | DT_EXPANDTABS | DT_NOPREFIX);
    pDC->RestoreDC(-1);
    int width = c_controlSpacing * 2 + rctText.Width();
    int height = rctText.Height();
    m_requiredSize = CSize(width, height);
    return m_requiredSize;
}

void InfoTipItem_BuffDescription::Draw(CDC* pDC, const CRect& rect)
{
    pDC->SaveDC();
    pDC->SelectObject(s_standardFont);
    CRect rctText(rect.left + c_controlSpacing, rect.top, rect.right - c_controlSpacing, rect.bottom);
    pDC->DrawText(m_text, rctText, DT_LEFT | DT_EXPANDTABS | DT_NOPREFIX);
    // we bold the text up to the ':' character
    if (m_text.Find(':', 0) >= 0)
    {
        CString txt;
        txt = m_text.Left(m_text.Find(':', 0));
        rctText.left += 1;
        pDC->DrawText(txt, rctText, DT_LEFT | DT_EXPANDTABS | DT_NOPREFIX);
    }
    pDC->RestoreDC(-1);
}

// DC
void InfoTipItem_DC::SetText(const CString& text)
{
    m_text = text;
}

CSize InfoTipItem_DC::Measure(CDC* pDC)
{
    pDC->SaveDC();
    pDC->SelectObject(s_boldFont);
    CRect rctText;
    pDC->DrawText(m_text, &rctText, DT_CALCRECT | DT_LEFT | DT_EXPANDTABS | DT_NOPREFIX);
    pDC->RestoreDC(-1);
    int width = c_controlSpacing * 2 + rctText.Width();
    int height = rctText.Height();
    m_requiredSize = CSize(width, height);
    return m_requiredSize;
}

void InfoTipItem_DC::Draw(CDC* pDC, const CRect& rect)
{
    pDC->SaveDC();
    pDC->SelectObject(s_boldFont);
    CRect rctText(rect.left + c_controlSpacing, rect.top, rect.right - c_controlSpacing, rect.bottom);
    pDC->DrawText(m_text, rctText, DT_LEFT | DT_EXPANDTABS | DT_NOPREFIX);
    pDC->RestoreDC(-1);
}

// Metamagics

void InfoTipItem_Metamagics::SetSpell(const Spell& spell)
{
    m_spell = spell;
}

CSize InfoTipItem_Metamagics::Measure(CDC* pDC)
{
    // +--------------------+
    // |     Metamagics     |
    // | [][][][][][][][][] | each [] is a metamgic icon which is 32*32
    // +--------------------+
    pDC->SaveDC();
    pDC->SelectObject(s_smallFont);
    CSize text = pDC->GetTextExtent("Metamagics");
    pDC->RestoreDC(-1);

    CSize size(0, 0);
    size.cy = text.cy + 36;     // 2 pixels between items vertically
    size_t metaCount = m_spell.MetamagicCount();
    size.cx = (32 * metaCount) + (2 * (metaCount + 1));
    m_requiredSize = size;
    return m_requiredSize;
}

void InfoTipItem_Metamagics::Draw(CDC* pDC, const CRect& rect)
{
    pDC->SaveDC();
    pDC->SelectObject(s_smallFont);
    CSize text = pDC->GetTextExtent("Metamagics");
    pDC->TextOut(rect.left + 2, rect.top, "Metamagics");
    // rect for each icon
    CRect iconRect(rect.left + 2, rect.top + text.cy + 2, rect.left + 34, rect.top + text.cy + 34);
    if (m_spell.HasAccelerate())
    {
        DrawIcon(pDC, iconRect, "AccelerateSpell");
        iconRect += CSize(34, 0);
    }
    if (m_spell.HasEmbolden())
    {
        DrawIcon(pDC, iconRect, "EmboldenSpell");
        iconRect += CSize(34, 0);
    }
    if (m_spell.HasEmpower())
    {
        DrawIcon(pDC, iconRect, "EmpowerSpell");
        iconRect += CSize(34, 0);
    }
    if (m_spell.HasEmpowerHealing())
    {
        DrawIcon(pDC, iconRect, "EmpowerHealingSpell");
        iconRect += CSize(34, 0);
    }
    if (m_spell.HasEnlarge())
    {
        DrawIcon(pDC, iconRect, "EnlargeSpell");
        iconRect += CSize(34, 0);
    }
    if (m_spell.HasExtend())
    {
        DrawIcon(pDC, iconRect, "ExtendSpell");
        iconRect += CSize(34, 0);
    }
    if (m_spell.HasHeighten())
    {
        DrawIcon(pDC, iconRect, "HeightenSpell");
        iconRect += CSize(34, 0);
    }
    if (m_spell.HasIntensify())
    {
        DrawIcon(pDC, iconRect, "IntensifySpell");
        iconRect += CSize(34, 0);
    }
    if (m_spell.HasMaximize())
    {
        DrawIcon(pDC, iconRect, "MaximizeSpell");
        iconRect += CSize(34, 0);
    }
    if (m_spell.HasQuicken())
    {
        DrawIcon(pDC, iconRect, "QuickenSpell");
        iconRect += CSize(34, 0);
    }

    pDC->RestoreDC(-1);
}

void InfoTipItem_Metamagics::DrawIcon(
        CDC* pDC,
        const CRect& rect,
        const std::string& iconName)
{
    CImage image;
    HRESULT hr = LoadImageFile("DataFiles\\FeatImages\\", iconName, &image, CSize(32, 32), true);
    if (hr == S_OK)
    {
        image.SetTransparentColor(c_transparentColour);
    }
    image.TransparentBlt(
            pDC->GetSafeHdc(),
            rect,
            CRect(0, 0, 32, 32));
}
