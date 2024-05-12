#pragma once
#include <afxvisualmanageroffice2007.h>
class CMFCVisualManagerOffice2007DarkMode :
    public CMFCVisualManagerOffice2007
{
    DECLARE_DYNCREATE(CMFCVisualManagerOffice2007DarkMode)

    CMFCVisualManagerOffice2007DarkMode();
public:
    void UpdateColours();
};

