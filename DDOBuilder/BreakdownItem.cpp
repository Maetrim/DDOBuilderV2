// BreakdownItem.cpp
//
#include "stdafx.h"
#include "BreakdownItem.h"

#include "BreakdownsPane.h"
#include "Bonus.h"
#include "Character.h"
#include "GlobalSupportFunctions.h"
#include "TreeListCtrl.h"
#include "StancesPane.h"
#include "MainFrm.h"

BreakdownItem::BreakdownItem(
        BreakdownType type,
        MfcControls::CTreeListCtrl * treeList,
        HTREEITEM hItem) :
    m_pCharacter(NULL),
    m_pTreeList(treeList),
    m_hItem(hItem),
    m_type(type),
    m_bHasWeapon(false),
    m_weapon(Weapon_Unknown),
    m_weaponCriticalMultiplier(0),
    m_bAddEnergies(true),
    m_bHasNonStackingEffects(false)
{
}

BreakdownItem::~BreakdownItem()
{
}

void BreakdownItem::PopulateBreakdownControl(CListCtrl * pControl)
{
    pControl->LockWindowUpdate();
    pControl->DeleteAllItems();
    if (m_pCharacter != NULL
            && m_pCharacter->ActiveBuild() != NULL)
    {
        AddItems(pControl);
        AppendItems(pControl);      // virtual, standard does nothing
    }
    pControl->UnlockWindowUpdate();
}

void BreakdownItem::AddItems(CListCtrl * pControl)
{
    // add all the items
    AddActiveItems(m_otherEffects, pControl, false);
    AddActiveItems(m_effects, pControl, false);
    std::list<Effect> itemEffects = m_itemEffects;
    std::list<Effect> inactiveEffects;
    std::list<Effect> nonStackingEffects;
    std::list<Effect> temporaryEffects;
    RemoveInactive(&itemEffects, &inactiveEffects);
    RemoveNonStacking(&itemEffects, &nonStackingEffects);
    RemoveTemporary(&itemEffects, &temporaryEffects);
    AddActiveItems(itemEffects, pControl, true);

    // finally add the active percentage items
    AddActivePercentageItems(m_otherEffects, pControl);
    AddActivePercentageItems(m_effects, pControl);
    AddActivePercentageItems(itemEffects, pControl);
    AddActiveItems(temporaryEffects, pControl, false);

    int inactiveStart = pControl->GetItemCount();
    // also show inactive and non stack effects if we have any so user
    // knows which duplicate effects they have in place
    AddDeactiveItems(m_otherEffects, pControl, true);
    AddDeactiveItems(m_effects, pControl, true);
    AddDeactiveItems(m_itemEffects, pControl, true);
    if (inactiveStart != pControl->GetItemCount())
    {
        // blank item between active and inactive
        pControl->InsertItem(inactiveStart, "");
        pControl->InsertItem(inactiveStart + 1, "Inactive Items");
    }
    if (nonStackingEffects.size() > 0)
    {
        // separate these non-stacking effects from the main list
        size_t index = pControl->InsertItem(
                pControl->GetItemCount(),
                "",         // blank line entry
                0);
        index = pControl->InsertItem(
                pControl->GetItemCount(),
                "Non-Stacking Effects",
                0);
        AddActiveItems(nonStackingEffects, pControl, true);
        AddActivePercentageItems(nonStackingEffects, pControl);
    }
}

void BreakdownItem::BuildChanged(Character * pCharacter)
{
    m_pCharacter = pCharacter;
    m_mainAbility = m_baseAbility;
    // if the document has changed, we will
    // need to regenerate all the items that are used to
    // calculate the total for this breakdown
    m_otherEffects.clear();
    m_effects.clear();
    m_itemEffects.clear();
}

void BreakdownItem::BuildChangeComplete()
{
    if (m_pCharacter != NULL)
    {
        CreateOtherEffects();
        if (m_pCharacter->ActiveBuild() != NULL)
        {
            // now add the standard items
            Populate();
        }
    }
}

BreakdownType BreakdownItem::Type() const
{
    return m_type;
}

void BreakdownItem::Populate()
{
    Total();    // ensure active percentage items have numbers
    NotifyTotalChanged();
    if (m_pTreeList != NULL)
    {
        CString title(Title());
        if (title != "")
        {
            m_pTreeList->SetItemText(m_hItem, 0, title);
        }
        m_pTreeList->SetItemText(m_hItem, 1, Value());
        m_pTreeList->SetItemColor(m_hItem, m_bHasNonStackingEffects
                ? COLORREF(RGB(255, 0, 0))
                : COLORREF(RGB(0, 0, 0)));

        if (m_pTreeList->GetSelectedItem() == m_hItem)
        {
            // force an update if the actively viewed item has changed
            m_pTreeList->SelectItem(m_pTreeList->GetSelectedItem());
        }
    }
}

double BreakdownItem::Total() const
{
    // to sum the total, just get the contributions of all the stacking effects
    double total = 0.0;
    if (m_pCharacter != NULL)
    {
        total += SumItems(m_otherEffects, false);
        total += SumItems(m_effects, false);

        std::list<Effect> itemEffects = m_itemEffects;
        std::list<Effect> inactiveEffects;
        std::list<Effect> nonStackingEffects;
        std::list<Effect> temporaryEffects;
        RemoveInactive(&itemEffects, &inactiveEffects);
        RemoveNonStacking(&itemEffects, &nonStackingEffects);
        RemoveTemporary(&itemEffects, &temporaryEffects);
        total += SumItems(itemEffects, true);
        double baseTotal = total;

        // TBD Future: Handle percentage effects for items that stack by multiplication
        // there should not currently be any cases for this

        // now apply percentage effects. Note percentage effects do not stack.
        // a test on live shows two percentage bonus's to hp adds two lots
        // of the base total (before percentages) to the total
        total += DoPercentageEffects(m_otherEffects, baseTotal);
        total += DoPercentageEffects(m_effects, baseTotal);
        // make sure we update listed items
        DoPercentageEffects(m_itemEffects, baseTotal);
        total += DoPercentageEffects(itemEffects, baseTotal);
        total += SumItems(temporaryEffects, false);
    }
    return total;
}

double BreakdownItem::CappedTotal() const
{
    // default implemented as Total()
    // override in inheriting classes to handle special capped totals
    return Total();
}

void BreakdownItem::AddActiveItems(
        const std::list<Effect> & effects,
        CListCtrl * pControl,
        bool bShowMultiplier)
{
    UNREFERENCED_PARAMETER(bShowMultiplier);
    // add all the breakdown items from this particular list
    for (auto&& it: effects)
    {
        // only add active items when it has an active stance flag
        if (it.IsActive(*m_pCharacter, m_weapon)
                && !it.HasPercent())
        {
            // only list it if its non-zero
            double total = it.TotalAmount(false);
            if (total != 0
                    || it.AType() == Amount_NotNeeded)
            {
                // put the effect name into the control
                CString effectName = it.DisplayName().c_str();
                size_t index = pControl->InsertItem(
                        pControl->GetItemCount(),
                        effectName,
                        0);

                // the number of stacks of it
                if (it.AType() != Amount_NotNeeded)
                {
                    CString stacks = it.StacksAsString().c_str();
                    pControl->SetItemText(index, CO_Stacks, stacks);
                }

                // and the total amount the number of stacks contribute
                CString amount;
                if (it.AType() == Amount_NotNeeded
                    && it.HasValue())
                {
                    amount = it.Value().c_str();
                }
                else if (it.AType() == Amount_NotNeeded && it.Item().size() > 0)
                {
                    amount = it.Item().front().c_str();
                }
                else if (it.HasValue())
                {
                    amount = it.Value().c_str();
                }
                else
                {
                    amount.Format("%.2f", total); //= it.AmountAsText(bShowMultiplier ? Multiplier() : 1.0);
                    if (it.HasPercent()) amount += "%";
                    amount.Replace(".00", "");
                }
                pControl->SetItemText(index, CO_Value, amount);

                // add the bonus type
                CString bonus = it.Bonus().c_str();
                pControl->SetItemText(index, CO_BonusType, bonus);
            }
        }
    }
}

void BreakdownItem::AddActivePercentageItems(
        const std::list<Effect> & effects,
        CListCtrl * pControl)
{
    // add all the breakdown items from this particular list
    for (auto&& it : effects)
    {
        // only add active items when it has an active stance flag and is a percentage
        if (it.IsActive(*m_pCharacter, m_weapon)
                && it.HasPercent())
        {
            // only list it if its non-zero
            double total = it.TotalAmount(false);
            if (total != 0
                || it.AType() == Amount_NotNeeded)
            {
                // put the effect name into the control
                CString effectName = it.DisplayName().c_str();
                size_t index = pControl->InsertItem(
                        pControl->GetItemCount(),
                        effectName,
                        0);

                // the number of stacks of it
                if (it.AType() != Amount_NotNeeded)
                {
                    CString stacks = it.StacksAsString().c_str();
                    pControl->SetItemText(index, CO_Stacks, stacks);
                }

                // and the total amount the number of stacks contribute
                CString amount;
                if (it.AType() == Amount_NotNeeded)
                {
                    amount = it.Item().front().c_str();
                }
                else if (it.HasValue())
                {
                    amount = it.Value().c_str();
                }
                else
                {
                    amount.Format("%.2f(%.2f%%)", it.GetPercentValue(), total);
                    amount.Replace(".00", "");
                }
                pControl->SetItemText(index, CO_Value, amount);

                // add the bonus type
                CString bonus = it.Bonus().c_str();
                pControl->SetItemText(index, CO_BonusType, bonus);
            }
        }
    }
}

void BreakdownItem::AddDeactiveItems(
        const std::list<Effect> & effects,
        CListCtrl * pControl,
        bool bShowMultiplier)
{
    UNREFERENCED_PARAMETER(bShowMultiplier);
    // add all inactive breakdown items from this particular list
    for (auto&& it : effects)
    {
        // only add inactive items when it has a stance flag
        if (!it.IsActive(*m_pCharacter, m_weapon))
        {
            // only list it if its non-zero
            double total = it.TotalAmount(false);
            if (total != 0
                || it.AType() == Amount_NotNeeded)
            {
                // put the effect name into the control
                CString effectName = it.DisplayName().c_str();
                size_t index = pControl->InsertItem(
                        pControl->GetItemCount(),
                        effectName,
                        0);

                // the number of stacks of it
                if (it.AType() != Amount_NotNeeded)
                {
                    CString stacks = it.StacksAsString().c_str();
                    pControl->SetItemText(index, CO_Stacks, stacks);
                }

                // and the total amount the number of stacks contribute
                CString amount;
                if (it.AType() == Amount_NotNeeded
                    && it.HasValue())
                {
                    amount = it.Value().c_str();
                }
                else if (it.AType() == Amount_NotNeeded && it.Item().size() > 0)
                {
                    amount = it.Item().front().c_str();
                }
                else if (it.HasValue())
                {
                    amount = it.Value().c_str();
                }
                else
                {
                    amount.Format("%.2f", total); //= it.AmountAsText(bShowMultiplier ? Multiplier() : 1.0);
                    if (it.HasPercent()) amount += "%";
                    amount.Replace(".00", "");
                }
                pControl->SetItemText(index, CO_Value, amount);

                // add the bonus type
                CString bonus = it.Bonus().c_str();
                pControl->SetItemText(index, CO_BonusType, bonus);
            }
        }
    }
}

double BreakdownItem::SumItems(
        const std::list<Effect> & effects,
        bool bApplyMultiplier) const
{
    double total = StacksByMultiplication() ? 100.0 : 0.0;
    for (auto&& it : effects)
    {
        // only add active items when it has an active stance flag
        if (it.IsActive(*m_pCharacter, m_weapon))
        {
            if (!it.HasPercent())
            {
                double amount = it.TotalAmount(true);
                if (bApplyMultiplier)
                {
                    amount *= Multiplier();
                }
                if (StacksByMultiplication())
                {
                    // fractional multiplication
                    total *= ((100.0 - amount) / 100.0);
                }
                else
                {
                    // its straight addition
                    total +=  amount;
                }
            }
        }
    }
    return total;
}

double BreakdownItem::DoPercentageEffects(
        const std::list<Effect> & effects,
        double total) const
{
    double amountAdded = 0;
    for (auto&& it : effects)
    {
        // only count the active items in the total
        if (it.IsActive(*m_pCharacter, m_weapon))
        {
            if (it.HasPercent())
            {
                // the amount is a percentage of the current total that
                // needs to be added.
                double percent = it.TotalAmount(false);
                double amount = (total * percent / 100.0);
                // round it to a whole number
                if (amount > 0)
                {
                    amount = (double)(int)(amount + 0.5);   // round up
                }
                else
                {
                    amount = (double)(int)(amount - 0.5);   // round up
                }
                amountAdded += amount;
                it.SetPercentValue(amount);   // so it can display its amount
            }
        }
    }
    return amountAdded;
}

double BreakdownItem::Multiplier() const
{
    // by default all items have a multiplier of 1
    return 1.0;
}

void BreakdownItem::AddAbility(
        AbilityType ability)
{
    AbilityStance as;
    as.ability = ability;
    m_mainAbility.push_back(as);  // duplicates are fine
    // auto observe this ability
    BreakdownItem * pBI = FindBreakdown(StatToBreakdown(ability));
    pBI->AttachObserver(this);
}

void BreakdownItem::RemoveFirstAbility(
        AbilityType ability)
{
    AbilityStance as;
    as.ability = ability;
    for (size_t i = 0; i < m_mainAbility.size(); ++i)
    {
        if (m_mainAbility[i] == as)
        {
            m_mainAbility.erase(m_mainAbility.begin() + i);
            break;      // done
        }
    }
}

void BreakdownItem::AddAbility(
        AbilityType ability,
        const Requirements& requirements,
        WeaponType wt)
{
    AbilityStance as;
    as.ability = ability;
    as.requirements = requirements;
    as.weapon = wt;
    m_mainAbility.push_back(as);  // duplicates are fine
    // auto observe this ability
    BreakdownItem * pBI = FindBreakdown(StatToBreakdown(ability));
    pBI->AttachObserver(this);
}

void BreakdownItem::RemoveFirstAbility(
        AbilityType ability,
        const Requirements& requirements,
        WeaponType wt)
{
    AbilityStance as;
    as.ability = ability;
    as.requirements = requirements;
    as.weapon = wt;
    for (size_t i = 0; i < m_mainAbility.size(); ++i)
    {
        if (m_mainAbility[i] == as)
        {
            m_mainAbility.erase(m_mainAbility.begin() + i);
            break;      // done
        }
    }
}

AbilityType BreakdownItem::LargestStatBonus()
{
    AbilityType ability = Ability_Unknown;

    size_t bestIndex = 0;       // assume first is best
    int largest = -999;         // arbitrarily small
    for (size_t i = 0; i < m_mainAbility.size(); ++i)
    {
        // must be active
        bool active = m_mainAbility[i].requirements.Met(
                *m_pCharacter->ActiveBuild(),
                m_pCharacter->ActiveBuild()->Level()-1,
                true,
                Weapon(),
                Weapon_Unknown);
        if (active)
        {
            BreakdownItem * pBI = FindBreakdown(StatToBreakdown(m_mainAbility[i].ability));
            ASSERT(pBI != NULL);
            pBI->AttachObserver(this);  // need to know about changes to this stat
            int bonus = BaseStatToBonus(pBI->Total());
            if (bonus > largest)
            {
                largest = bonus;
                bestIndex = i;
            }
        }
    }
    if (largest != -999)
    {
        // we now have the best option
        ability = m_mainAbility[bestIndex].ability;
    }
    return ability;
}

void BreakdownItem::AddOtherEffect(const Effect & effect)
{
    if (effect.HasApplyAsItemEffect())
    {
        AddEffect(&m_itemEffects, effect);
    }
    else
    {
        AddEffect(&m_otherEffects, effect);
    }
}

void BreakdownItem::AddFeatEffect(const Effect & effect)
{
    if (effect.HasApplyAsItemEffect())
    {
        AddEffect(&m_itemEffects, effect);
    }
    else
    {
        AddEffect(&m_effects, effect);
    }
}

void BreakdownItem::AddEnhancementEffect(const Effect & effect)
{
    if (effect.HasApplyAsItemEffect())
    {
        AddEffect(&m_itemEffects, effect);
    }
    else
    {
        AddEffect(&m_effects, effect);
    }
}

void BreakdownItem::AddItemEffect(const Effect & effect)
{
    AddEffect(&m_itemEffects, effect);
}

void BreakdownItem::RevokeOtherEffect(const Effect & effect)
{
    if (effect.HasApplyAsItemEffect())
    {
        RevokeEffect(&m_itemEffects, effect);
    }
    else
    {
        RevokeEffect(&m_otherEffects, effect);
    }
}

void BreakdownItem::RevokeFeatEffect(const Effect & effect)
{
    if (effect.HasApplyAsItemEffect())
    {
        RevokeEffect(&m_itemEffects, effect);
    }
    else
    {
        RevokeEffect(&m_effects, effect);
    }
}

void BreakdownItem::RevokeEnhancementEffect(const Effect & effect)
{
    if (effect.HasApplyAsItemEffect())
    {
        RevokeEffect(&m_itemEffects, effect);
    }
    else
    {
        RevokeEffect(&m_effects, effect);
    }
}

void BreakdownItem::RevokeItemEffect(const Effect & effect)
{
    RevokeEffect(&m_itemEffects, effect);
}

void BreakdownItem::RemoveInactive(
        std::list<Effect> * effects,
        std::list<Effect> * inactiveEffects) const
{
    // add all inactive breakdown items from this particular list
    inactiveEffects->clear(); // ensure empty

    const Build* pBuild = m_pCharacter->ActiveBuild();
    std::list<Effect>::iterator it = effects->begin();
    while (it != effects->end())
    {
        // only add inactive items when it has a stance flag
        if ((*it).HasRequirementsToBeActive()
                && !(*it).RequirementsToBeActive().Met(*pBuild, pBuild->Level()-1, true, Weapon(), Weapon_Unknown))
        {
            // add the item to be removed into the inactive list
            inactiveEffects->push_back((*it));
            it = effects->erase(it);      // remove from source list
        }
        else
        {
            ++it;
        }
    }
}

void BreakdownItem::RemoveNonStacking(
        std::list<Effect> * effects,
        std::list<Effect> * nonStackingEffects) const
{
    m_bHasNonStackingEffects = false;
    // the same effect type can come from multiple sources in the form of
    // buffs or equipment effects. To avoid these giving increased values
    // we remove all the duplicate and lesser powered versions of any effects
    // from the list provided and push them onto a separate list
    nonStackingEffects->clear();        // ensure empty
    // look at each item in the list and see if it is a lesser or duplicate
    // of another effect
    std::list<Effect>::iterator sit = effects->begin();
    while (sit != effects->end())
    {
        bool removeIt = false;
        // now compare it to all other items in the same list
        std::list<Effect>::iterator tit = effects->begin();
        while (!removeIt && tit != effects->end())
        {
            // don't compare the item against itself
            if (sit != tit)
            {
                // not the same item, gets removed if it is a duplicate
                const Bonus& bonus = FindBonus((*sit).Bonus());
                if (bonus.Stacking() == StackingType_HighestOnly)
                {
                    if ((*sit).Bonus() == (*tit).Bonus())
                    {
                        // its the same bonus type, so it may be affected by stacking rules
                        // look up the bonus type
                        removeIt |= ((*sit).TotalAmount(false) <= (*tit).TotalAmount(false));
                    }
                }
            }
            ++tit;
        }
        if (removeIt)
        {
            // add the item to be removed into the non stacking list
            nonStackingEffects->push_back((*sit));
            sit = effects->erase(sit);      // remove from source list
            m_bHasNonStackingEffects = true;
        }
        else
        {
            // not removed, this is the top level kosher item for this effect
            ++sit;
        }
    }
}

void BreakdownItem::RemoveTemporary(
        std::list<Effect> * effects,
        std::list<Effect> * temporaryEffects) const
{
    UNREFERENCED_PARAMETER(effects);
    UNREFERENCED_PARAMETER(temporaryEffects);
    // any bonus type of "Temporary" is added after percentage multipliers
    temporaryEffects->clear();        // ensure empty
    std::list<Effect>::iterator sit = effects->begin();
    while (sit != effects->end())
    {
        if ((*sit).Bonus() == "Temporary")
        {
            // move it to the other list
            temporaryEffects->push_back((*sit));
            sit = effects->erase(sit);      // remove from source list
        }
        else
        {
            // not removed
            ++sit;
        }
    }
}

void BreakdownItem::AddEffect(
        std::list<Effect> * effectList,
        const Effect & effect)
{
    // if an identical effect is already in the list, increase its stacking
    // else add a new item
    bool found = false;
    std::list<Effect>::iterator it = effectList->begin();
    while (!found && it != effectList->end())
    {
        if (effect == (*it))
        {
            // it is an existing effect, add another stack
            found = true;
            (*it).AddEffectStack();
        }
        ++it;
    }
    if (!found)
    {
        // it is a new effect, just add it to the list
        if (effect.AType() == Amount_SliderValue)
        {
            Effect effectCopy(effect);
            // make sure we start with the correct number of starting stacks
            CWnd* pWnd = AfxGetMainWnd();
            CMainFrame* pMainWnd = dynamic_cast<CMainFrame*>(pWnd);
            const CStancesPane* pStancesPane = dynamic_cast<const CStancesPane*>(pMainWnd->GetPaneView(RUNTIME_CLASS(CStancesPane)));
            if (pStancesPane != NULL)
            {
                const SliderItem* pSlider = pStancesPane->GetSlider(effect.StackSource());
                effectCopy.SetEffectStacks(pSlider->m_position);
            }
            effectList->push_back(effectCopy);
        }
        else
        {
            effectList->push_back(effect);
        }
        effectList->back().SetBuild(m_pCharacter->ActiveBuild());
    }
    if (effect.HasUpdateAutomaticEffects())
    {
        // this effect also required the item in question to also update its manually added effects
        CreateOtherEffects();
    }
    Populate();
}

void BreakdownItem::RevokeEffect(
        std::list<Effect> * effectList,
        const Effect & effect)
{
    // if an identical effect is already in the list, decrease its stacking
    // and remove if all stacks gone
    bool found = false;
    std::list<Effect>::iterator it = effectList->begin();
    while (!found && it != effectList->end())
    {
        if (effect == (*it))
        {
            // it an existing effect, clear a stack
            found = true;
            bool deleteIt = (*it).RevokeEffectStack();        // true if no stacks left
            if (deleteIt)
            {
                it = effectList->erase(it);
            }
            break;
        }
        ++it;
    }
    Populate();
}

// DocumentObserver overrides
void BreakdownItem::StanceActivated(
        Build*,
        const std::string&)
{
    // just repopulate
    Populate();
}

void BreakdownItem::StanceDeactivated(
    Build*,
    const std::string&)
{
    // just repopulate
    Populate();
}

void BreakdownItem::SliderChanged(
    Build*,
    const std::string& sliderName,
    int newValue)
{
    UpdateSliderEffects(sliderName, newValue);
}

void BreakdownItem::GearChanged(Build* pBuild, InventorySlotType slot)
{
    UNREFERENCED_PARAMETER(pBuild);
    UNREFERENCED_PARAMETER(slot);
}

//void BreakdownItem::UpdateFeatTrained(Character * charData, const std::string& featName)
//{
//    // just repopulate
//    Populate();
//}
//
//void BreakdownItem::UpdateFeatRevoked(Character * charData, const std::string& featName)
//{
//    // just repopulate
//    Populate();
//}

void BreakdownItem::NotifyTotalChanged()
{
    NotifyAll(&BreakdownObserver::UpdateTotalChanged, this, m_type);
}

bool BreakdownItem::UpdateTreeItemTotals()
{
    // check all items that are dependent on AP spent in a tree
    bool itemChanged = false;
    itemChanged |= UpdateTreeItemTotals(&m_otherEffects);
    itemChanged |= UpdateTreeItemTotals(&m_effects);
    itemChanged |= UpdateTreeItemTotals(&m_itemEffects);

    if (itemChanged)
    {
        Populate();
    }

    return itemChanged;
}

bool BreakdownItem::UpdateTreeItemTotals(std::list<Effect> * list)
{
    UNREFERENCED_PARAMETER(list);
    //bool itemChanged = false;
    //std::list<Effect>::iterator it = list->begin();
    //while (it != list->end())
    //{
    //    if ((*it).IsAmountPerAP())
    //    {
    //        size_t spent = m_pCharacter->APSpentInTree((*it).Tree());
    //        if (spent != (*it).NumStacks())
    //        {
    //            (*it).SetStacks(spent);
    //            itemChanged = true;
    //        }
    //    }
    //    ++it;
    //}
    //return itemChanged;
    return false;
}

bool BreakdownItem::UpdateEffectAmounts(
        std::list<Effect> * list,
        BreakdownType bt)
{
    UNREFERENCED_PARAMETER(list);
    UNREFERENCED_PARAMETER(bt);
    bool itemChanged = false;
    //std::list<Effect>::iterator it = list->begin();
    //while (it != list->end())
    //{
    //    if ((*it).HasBreakdownDependency(bt))
    //    {
    //        BreakdownItem * pBI = FindBreakdown(bt);
    //        ASSERT(pBI != NULL);
    //        (*it).SetAmount(pBI->Total());
    //        itemChanged = true;
    //    }
    //    ++it;
    //}
    return itemChanged;
}

bool BreakdownItem::UpdateEffectAmounts(std::list<Effect> * list, const std::string& type)
{
    UNREFERENCED_PARAMETER(list);
    UNREFERENCED_PARAMETER(type);
    bool itemChanged = false;
    //std::list<Effect>::iterator it = list->begin();
    //while (it != list->end())
    //{
    //    if ((*it).BasedOnClassLevel(type))
    //    {
    //        std::vector<size_t> classLevels = m_pCharacter->ClassLevels(MAX_LEVEL);
    //        (*it).SetClassLevel(classLevels[type]);
    //        itemChanged = true;
    //    }
    //    ++it;
    //}
    return itemChanged;
}

void BreakdownItem::AbilityTotalChanged(Build*, AbilityType at)
{
    bool bChangeMade = false;
    for (auto&& it : m_otherEffects)
    {
        bChangeMade |= it.UpdateAbilityEffects(at);
    }
    for (auto&& it : m_effects)
    {
        bChangeMade |= it.UpdateAbilityEffects(at);
    }
    for (auto&& it : m_itemEffects)
    {
        bChangeMade |= it.UpdateAbilityEffects(at);
    }
    if (bChangeMade)
    {
        Populate();
    }
}

void BreakdownItem::BuildLevelChanged(Build*)
{
    // level change can cause some effect values to change
    Populate();
}

void BreakdownItem::FeatTrained(Build*, const std::string&)
{
    // feat changes can affect some enhancement items being enabled/disabled
    Populate();
}

void BreakdownItem::FeatRevoked(Build*, const std::string&)
{
    // feat changes can affect some enhancement items being enabled/disabled
    Populate();
}

void BreakdownItem::FeatEffectApplied(
        Build*,
        const Effect & effect)
{
    // see if this feat effect applies to us, if so add it
    if (AffectsUs(effect))
    {
        AddFeatEffect(effect);
    }
}

void BreakdownItem::FeatEffectRevoked(
        Build*,
        const Effect & effect)
{
    // see if this feat effect applies to us, if so revoke it
    if (AffectsUs(effect))
    {
        RevokeFeatEffect(effect);
    }
}

void BreakdownItem::ItemEffectApplied(
        Build*,
        const Effect& effect)
{
    // see if this feat effect applies to us, if so add it
    if (AffectsUs(effect))
    {
        AddEffect(&m_itemEffects, effect);
    }
}

void BreakdownItem::ItemEffectRevoked(
        Build*,
        const Effect& effect)
{
    if (AffectsUs(effect))
    {
        RevokeEffect(&m_itemEffects, effect);
    }
}

void BreakdownItem::ItemEffectApplied(
    Build*,
    const Effect& effect,
    WeaponType wt)
{
    UNREFERENCED_PARAMETER(wt);
    // see if this feat effect applies to us, if so add it
    if (AffectsUs(effect))
    {
        AddEffect(&m_itemEffects, effect);
    }
}

void BreakdownItem::ItemEffectRevoked(
    Build*,
    const Effect& effect,
    WeaponType wt)
{
    UNREFERENCED_PARAMETER(wt);
    if (AffectsUs(effect))
    {
        RevokeEffect(&m_itemEffects, effect);
    }
}

void BreakdownItem::EnhancementTrained(
    Build*,
    const EnhancementItemParams& item)
{
    UpdateEnhancementTreeEffects(item.tree);
}

void BreakdownItem::EnhancementRevoked(
    Build*,
    const EnhancementItemParams& item)
{
    UpdateEnhancementTreeEffects(item.tree);
}

void BreakdownItem::EnhancementEffectApplied(
        Build*,
        const Effect & effect)
{
    // see if this enhancement effect applies to us, if so add it
    if (AffectsUs(effect))
    {
        AddEnhancementEffect(effect);
    }
}

void BreakdownItem::EnhancementEffectRevoked(
        Build*,
        const Effect & effect)
{
    // see if this feat effect applies to us, if so revoke it
    if (AffectsUs(effect))
    {
        RevokeEnhancementEffect(effect);
    }
}

//void BreakdownItem::UpdateAPSpentInTreeChanged(
//        Character * charData,
//        const std::string& treeName)
//{
//    UpdateTreeItemTotals();
//}

// BreakdownObserver overrides (may be specialised in inheriting classes)
void BreakdownItem::UpdateTotalChanged(BreakdownItem * item, BreakdownType type)
{
    UNREFERENCED_PARAMETER(item);
    // see if the breakdown item applies to any of our effects
    // if it does, update it and repopulate
    // check all items that are dependent on AP spent in a tree
    bool itemChanged = false;
    itemChanged |= UpdateEffectAmounts(&m_otherEffects, type);
    itemChanged |= UpdateEffectAmounts(&m_effects, type);
    itemChanged |= UpdateEffectAmounts(&m_itemEffects, type);

    if (itemChanged)
    {
        // an item was updated, re-populate
        Populate();
    }
}

//void BreakdownItem::UpdateClassChanged(
//        Character * charData,
//        const std::string& classFrom,
//        const std::string& classTo,
//        size_t level)
//{
//    bool itemChanged = false;
//    itemChanged |= UpdateEffectAmounts(&m_otherEffects, classFrom);
//    itemChanged |= UpdateEffectAmounts(&m_effects, classFrom);
//    itemChanged |= UpdateEffectAmounts(&m_itemEffects, classFrom);
//
//    itemChanged |= UpdateEffectAmounts(&m_otherEffects, classTo);
//    itemChanged |= UpdateEffectAmounts(&m_effects, classTo);
//    itemChanged |= UpdateEffectAmounts(&m_itemEffects, classTo);
//
//    if (itemChanged)
//    {
//        // an item was updated, re-populate
//        Populate();
//    }
//}

void BreakdownItem::SetHTreeItem(HTREEITEM hItem)
{
    m_hItem = hItem;
}

void BreakdownItem::SetWeapon(WeaponType wt, size_t weaponCriticalMultiplier)
{
    m_bHasWeapon = true;
    m_weapon = wt;
    m_weaponCriticalMultiplier = weaponCriticalMultiplier;
}

WeaponType BreakdownItem::Weapon() const
{
    return m_weapon;
}

double BreakdownItem::GetEffectValue(const std::string& bonus) const
{
    // now we have the list look for and sum all items with the given effect type
    double total = 0.0;
    std::list<Effect> allActiveEffects = AllActiveEffects();
    std::list<Effect>::iterator it = allActiveEffects.begin();
    while (it != allActiveEffects.end())
    {
        if ((*it).Bonus() == bonus)
        {
            total += (*it).TotalAmount(false);
        }
        ++it;
    }
    return total;
}

std::list<Effect> BreakdownItem::AllActiveEffects() const
{
    // build a list of all the current active effects
    std::list<Effect> allActiveEffects;
    allActiveEffects = m_otherEffects;
    allActiveEffects.insert(allActiveEffects.end(), m_effects.begin(), m_effects.end());
    std::list<Effect> itemEffects = m_itemEffects;
    std::list<Effect> inactiveEffects;
    std::list<Effect> nonStackingEffects;
    RemoveInactive(&itemEffects, &inactiveEffects);
    RemoveNonStacking(&itemEffects, &nonStackingEffects);
    allActiveEffects.insert(allActiveEffects.end(), itemEffects.begin(), itemEffects.end());
    return allActiveEffects;
}

void BreakdownItem::UpdateEnhancementTreeEffects(const std::string& treeName)
{
    bool bChangeMade = false;
    for (auto&& it : m_otherEffects)
    {
        bChangeMade |= it.UpdateTreeEffects(treeName);
    }
    for (auto&& it : m_effects)
    {
        bChangeMade |= it.UpdateTreeEffects(treeName);
    }
    for (auto&& it : m_itemEffects)
    {
        bChangeMade |= it.UpdateTreeEffects(treeName);
    }
    if (bChangeMade)
    {
        Populate();
    }
}

void BreakdownItem::UpdateSliderEffects(const std::string& sliderName, int newValue)
{
    bool bChangeMade = false;
    for (auto&& it : m_otherEffects)
    {
        bChangeMade |= it.UpdateSliderEffects(sliderName, newValue);
    }
    for (auto&& it : m_effects)
    {
        bChangeMade |= it.UpdateSliderEffects(sliderName, newValue);
    }
    for (auto&& it : m_itemEffects)
    {
        bChangeMade |= it.UpdateSliderEffects(sliderName, newValue);
    }
    if (bChangeMade)
    {
        Populate();
    }
}
