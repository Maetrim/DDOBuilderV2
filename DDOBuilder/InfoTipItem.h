// InfoTipItem.h
//
#pragma once
#include <vector>
#include "Spell.h"

class Build;
class EnhancementSelection;
class EnhancementTreeItem;
class Requirements;
class SetBonus;

class InfoTipItem
{
public:
    InfoTipItem();
    virtual ~InfoTipItem();

    virtual CSize Measure(CDC* pDC) = 0;
    virtual void Draw(CDC* pDC, const CRect& rect) = 0;
    CSize Size() const;
protected:
    CSize m_requiredSize;
    static bool s_bFontsCreated;
    static CFont s_standardFont;
    static CFont s_boldFont;
    static CFont s_smallFont;
};

class InfoTipItem_Header :
    public InfoTipItem
{
public:
    InfoTipItem_Header() {};
    virtual ~InfoTipItem_Header() {};

    bool LoadIcon(const std::string& path, const std::string& filename, bool bReportError);
    void IconFromImageList(CImageList& il, int index);
    void SetTitle(const CString& strTitle);
    void SetTitle2(const CString& strTitle2);
    void SetCost(const CString& strCost);
    void SetRank(const CString& strRank);

    virtual CSize Measure(CDC* pDC) override;
    virtual void Draw(CDC *pDC, const CRect& rect) override;
private:
    CImage m_image;
    CString m_title;
    CString m_title2;
    CString m_cost;
    CString m_ranks;
};

class InfoTipItem_SLAHeader :
    public InfoTipItem
{
public:
    InfoTipItem_SLAHeader(int nCasterLevel, int nCharges, int nRecharge) :
            m_nCasterLevel(nCasterLevel),
            m_nCharges(nCharges),
            m_nRecharge(nRecharge) {};
    virtual ~InfoTipItem_SLAHeader() {};

    virtual CSize Measure(CDC* pDC) override;
    virtual void Draw(CDC* pDC, const CRect& rect) override;
private:
    int m_nCasterLevel;
    int m_nCharges;
    int m_nRecharge;
};

class InfoTipItem_Requirements :
    public InfoTipItem
{
public:
    InfoTipItem_Requirements() {};
    virtual ~InfoTipItem_Requirements() {};

    void AddRequirement(const CString& text, bool bMet);
    void CreateRequirementsStrings(
            const Build& build,
            const EnhancementTreeItem* pItem,
            size_t spentInTree);

    void CreateRequirementsStrings(
            const Build& build,
            const EnhancementTreeItem* pItem,
            const EnhancementSelection* pSelection,
            size_t spentInTree);

    void CreateRequirementsStrings(const Build& build, const Requirements& req);

    void CreateSetBonusStrings(const SetBonus& setBonus, size_t numStacks);

    virtual CSize Measure(CDC* pDC) override;
    virtual void Draw(CDC *pDC, const CRect& rect) override;
private:
    std::vector<CString> m_requirements;
    std::vector<bool> m_bRequirementMet;
};

class InfoTipItem_MultilineText :
    public InfoTipItem
{
public:
    InfoTipItem_MultilineText() {};
    virtual ~InfoTipItem_MultilineText() {};

    void SetText(const CString& text);

    virtual CSize Measure(CDC* pDC) override;
    virtual void Draw(CDC *pDC, const CRect& rect) override;
private:
    CString m_text;
};

class InfoTipItem_BuffDescription :
    public InfoTipItem
{
public:
    InfoTipItem_BuffDescription(const CString& text) : m_text(text) {};
    virtual ~InfoTipItem_BuffDescription() {};

    virtual CSize Measure(CDC* pDC) override;
    virtual void Draw(CDC* pDC, const CRect& rect) override;
private:
    CString m_text;
};

class InfoTipItem_DC :
    public InfoTipItem
{
public:
    InfoTipItem_DC() {};
    virtual ~InfoTipItem_DC() {};

    void SetText(const CString& text);

    virtual CSize Measure(CDC* pDC) override;
    virtual void Draw(CDC *pDC, const CRect& rect) override;
private:
    CString m_text;
};

class InfoTipItem_Metamagics :
    public InfoTipItem
{
public:
    InfoTipItem_Metamagics() {};
    virtual ~InfoTipItem_Metamagics() {};

    void SetSpell(const Spell& spell);

    virtual CSize Measure(CDC* pDC) override;
    virtual void Draw(CDC *pDC, const CRect& rect) override;
private:
    void DrawIcon(CDC* pDC, const CRect& rect, const std::string& iconName);
    Spell m_spell;
};