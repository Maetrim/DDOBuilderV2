// CustomTabbedPane.h
//
#pragma once

class Character;
//---------------------------------------------------------------------------
class CCustomTabbedPane :
        public CTabbedPane
{
    public:
        CCustomTabbedPane();
        virtual ~CCustomTabbedPane();

        //MFC
        DECLARE_MESSAGE_MAP()
        DECLARE_SERIAL(CCustomTabbedPane)

        virtual void DrawCaption(CDC* pDC, CRect rectCaption);
    public:
        HICON m_hIcon;
};
