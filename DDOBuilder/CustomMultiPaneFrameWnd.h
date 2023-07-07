// CustomMultiPaneFrameWnd.h
//
#pragma once

class Character;
//---------------------------------------------------------------------------
class CCustomMultiPaneFrameWnd :
        public CMultiPaneFrameWnd
{
    public:
        CCustomMultiPaneFrameWnd();
        virtual ~CCustomMultiPaneFrameWnd();

        //MFC
        afx_msg void OnNcPaint();
        afx_msg BOOL OnNcActivate(BOOL);
        DECLARE_MESSAGE_MAP()
        DECLARE_SERIAL(CCustomMultiPaneFrameWnd)
};
