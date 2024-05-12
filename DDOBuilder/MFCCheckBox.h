// MFCCheckBox.h
#pragma once

//---------------------------------------------------------------------------
// includes
//---------------------------------------------------------------------------
#include "MFCButtonEx.h"

class CMFCCheckBox : public CMFCButtonEx
{
public:
    CMFCCheckBox(void) noexcept;
    ~CMFCCheckBox(void) noexcept;

    void SetColours(const CMFCControlColours& krobColours) noexcept;
    void ClearColours(void) noexcept;
private:
    virtual void ButtonPaint(CDC *pDC);
};

//---------------------------------------------------------------------------
// re-enable warnings
//---------------------------------------------------------------------------
#pragma warning( default : 4290 )
