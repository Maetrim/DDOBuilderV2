// SpecialFeatPane.h
//
#pragma once
#include "Resource.h"
#include <vector>
#include <list>

class CDDOBuilderDoc;
class Character;
class Feat;
class CFeatSelectionDialog;

class CSpecialFeatPane :
    public CFormView
{
public:
    CSpecialFeatPane();           // protected constructor used by dynamic creation
    virtual ~CSpecialFeatPane();
    enum { IDD = IDD_SPECIAL_FEATS_PANE };

    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual void OnInitialUpdate();
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg LRESULT OnNewDocument(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnLoadComplete(WPARAM, LPARAM);
    afx_msg void OnHeroic();
    afx_msg void OnRacial();
    afx_msg void OnIconic();
    afx_msg void OnEpic();
    afx_msg void OnSpecial();
    afx_msg void OnFavor();
    afx_msg LRESULT OnRevokeFeats(WPARAM, LPARAM);

    DECLARE_DYNCREATE(CSpecialFeatPane)
    DECLARE_MESSAGE_MAP()

private:
    void CreateFeatWindows(
            CStatic * groupWindow,
            const std::list<Feat> & featList,
            std::vector<CFeatSelectionDialog *> * dialogs);
    size_t PositionWindows(
            CStatic * groupWindow,
            size_t startIndex,
            const std::vector<CFeatSelectionDialog *> & dialogs,
            int * maxX,
            int * yPos);
    void UpdateDocumentPointers(std::vector<CFeatSelectionDialog*> & vWindows);
    void CleanUp(std::vector<CFeatSelectionDialog*> & vWindows);
    void TrainAllFeats(std::vector<CFeatSelectionDialog*>& feats);
    void RevokeAllFeats(std::vector<CFeatSelectionDialog*>& feats);

    std::vector<CFeatSelectionDialog *> m_heroicSelectionViews;
    std::vector<CFeatSelectionDialog *> m_racialSelectionViews;
    std::vector<CFeatSelectionDialog *> m_iconicSelectionViews;
    std::vector<CFeatSelectionDialog *> m_epicSelectionViews;
    std::vector<CFeatSelectionDialog *> m_specialSelectionViews;
    std::vector<CFeatSelectionDialog *> m_favorSelectionViews;

    CStatic m_staticHeroic;
    CStatic m_staticRacial;
    CStatic m_staticIconic;
    CStatic m_staticEpic;
    CStatic m_staticSpecial;
    CStatic m_staticFavor;
    CFont m_staticFont;

    CDDOBuilderDoc * m_pDoc;
    Character * m_pCharacter;
};
