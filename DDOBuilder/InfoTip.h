// InfoTip.h
//
#pragma once
#include <vector>

class Augment;
class Build;
class DC;
class EnhancementSelection;
class EnhancementTreeItem;
class Feat;
class Filigree;
class Gem;
class Item;
class ItemAugment;
class InfoTipItem;
class Patron;
class Requirements;
class SetBonus;
class Stance;
class Spell;
class SpellDamage;
class SpellDC;
class SubItem;
class LevelTraining;

class CInfoTip :
        public CWnd
{
    public:
        CInfoTip();
        virtual ~CInfoTip();

        BOOL Create(CWnd* parent);

        void SetEnhancementTreeItem(
                const Build& build,
                const EnhancementTreeItem* pItem,
                const std::string& selection,
                size_t spentInTree);
        void SetEnhancementSelectionItem(
                const Build& build,
                const EnhancementTreeItem* pItem,
                const EnhancementSelection* pSelection,
                size_t spentInTree,
                size_t ranks);
        void SetFeatItem(
                const Build& build,
                const Feat* pItem,
                bool featSwapWarning,
                size_t level,
                bool grantedFeat);
        void SetStanceItem(
                const Stance& item);
        void SetSetBonusItem(
                const SetBonus& item,
                size_t numStacks,
                bool bSuppressed,
                const Build* pBuild);
        void SetItem(
                const Item* pItem,
                const Build* pBuild);
        void SetSelfBuff(
            const std::string& name);
        void SetFiligree(
                const Filigree* pFiligree,
                const Build& build);
        void SetAugment(
                const Augment* pAugment,
                const Build& build);
        void SetSentientGem(
                const Gem* pGem);
        void SetLevelItem(
                const Build& build,
                size_t level,
                const LevelTraining* levelData,
                const std::string&  expectedClass);
        void SetSpell(
                const Build& build,
                const Spell* pSpell);
        void SetFavorItem(
                const Patron& patron,
                int favorTier);
        //void SetSelfBuff(const std::string& name);
        void SetDCItem(
                const Build& build,
                const DC* pDC);

        void SetOrigin(CPoint origin, CPoint alternate, bool rightAlign);
        void Show();
        void Hide();

    protected:
        BOOL GetWindowSize(CDC* pDC, CSize* size);
        //{{AFX_MSG(CInfoTip)
        afx_msg void OnPaint();
        //}}AFX_MSG
        DECLARE_MESSAGE_MAP()

    private:
        void ClearOldTipItems();
        void AppendSubItem(const SubItem& item);
        void AppendFeatInfo(const Build& build, const std::string& feat, const Requirements& req);
        void AppendSetBonusDescription(const std::string& setBonusName, size_t numStacks, bool bSuppressed, const Build* pBuild);
        void AppendDCInfo(const Build& build, const DC& dc);
        void AppendDCInfo(const Build& build, const Spell& spell, const SpellDC& dc);
        void AppendSpellItem(const Build& build, const Spell& spell);
        void AppendSpellDamageEffect(const Build& build, const Spell& spell, const SpellDamage& sd);
        void AppendSLA(const Spell& spell, int nSpellLevel, int nCharges, int nRecharge);
        void AppendAugment(const Augment* pAugment, const Build& build);
        void AppendFilledAugment(int itemLevel, const ItemAugment& slot, const Augment* pAugment, const Build& build);
        CPoint m_origin;
        CPoint m_alternate;
        bool m_bRightAlign;
        std::vector<InfoTipItem*> m_tipItems;
};