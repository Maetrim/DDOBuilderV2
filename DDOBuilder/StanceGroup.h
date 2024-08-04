// StanceGroup.h
//
#pragma once
#include "StanceButton.h"
#include <list>

class Build;
class Character;

class StanceGroup
{
public:
    StanceGroup(const std::string& groupName, bool bSingleSelection);
    ~StanceGroup();

    const std::string& GroupName() const;
    CWnd* GroupLabel();
    size_t NumButtons() const;
    CWnd* StanceButton(size_t index);

    bool IsSingleSelection() const;

    void CreateWindows(CWnd* pParent, UINT& nextControlId);
    void AddStance(Character* pCharacter, const Stance& stance, const std::string& group, CWnd* pParent, UINT& nextControlId);
    bool RevokeStance(const Stance& stance, const std::string& group);
    void DeactivateOtherStancesExcept(const std::string& stanceName, Build* pBuild);
    void DestroyAll();
    CStanceButton* GetStance(const std::string& stanceName);
    CStanceButton* GetStance(size_t index);
    const CStanceButton* GetStance(size_t index) const;
    void AddActiveStances(std::vector<CStanceButton*>* stances);
    bool HasStance(CStanceButton* pButton);
private:
    std::string m_groupName;
    CStatic m_groupLabel;
    std::list<CStanceButton*> m_stanceButtons;
    bool m_bSingleSelection;
    static CFont sm_smallFont;
};
