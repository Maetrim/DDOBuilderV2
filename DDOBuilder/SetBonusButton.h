// SetBonusButton.h
//

#pragma once

#include "Resource.h"
#include <string>
#include "SetBonus.h"

class Character;

// this window handles selection of stances and enhancement sub-option
class CSetBonusButton :
    public CStatic
{
    public:
        CSetBonusButton(Character * charData, const SetBonus& setBonus);

        const SetBonus& GetSetBonus() const;

        void AddStack();
        void RevokeStack();
        size_t NumStacks() const;
        bool IsYou(const SetBonus& setBonus);

    ///////////////////////////////////////////////////////////////////
    // MFC

    public:
        //{{AFX_DATA(CSetBonusButton)
        //}}AFX_DATA

    protected:
        //{{AFX_VIRTUAL(CSetBonusButton)
        //}}AFX_VIRTUAL

        //{{AFX_MSG(CSetBonusButton)
        afx_msg BOOL OnEraseBkgnd(CDC* pDC);
        afx_msg void OnPaint();
        //}}AFX_MSG
        DECLARE_MESSAGE_MAP()

    private:
        Character * m_pCharacter;
        SetBonus m_setBonus;
        CImage m_image;
        size_t m_stacks;
};

//{{AFX_INSERT_LOCATION}}
