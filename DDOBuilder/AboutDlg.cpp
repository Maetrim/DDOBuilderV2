// AboutDlg.cpp
//
#include "stdafx.h"
#include "AboutDlg.h"
#include "BuildInfo.h"
#include "GlobalSupportFunctions.h"

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_STATIC_VERSION, m_staticVersion);
    DDX_Control(pDX, IDC_BUG_REPORTERS, m_bugReporters);

    CString text;
    text.Format("DDOBuilder, Version %s", BUILDINFO_VERSION);
    m_staticVersion.SetWindowText(text);

    text.Format("Class Count: %d", Classes().size()-2); // ignore Epic/Legendary
    SetDlgItemText(IDC_STATIC_CLASSCOUNT, text);

    text.Format("Race Count: %d", Races().size());
    SetDlgItemText(IDC_STATIC_RACECOUNT, text);

    int count = StandardFeats().size()
                + HeroicPastLifeFeats().size()
                + RacialPastLifeFeats().size()
                + IconicPastLifeFeats().size()
                + EpicPastLifeFeats().size()
                + SpecialFeats().size()
                + UniversalTreeFeats().size()
                + DestinyTreeFeats().size()
                + FavorFeats().size();
    text.Format("Feat Count: %d", count);
    SetDlgItemText(IDC_STATIC_FEATCOUNT, text);

    count = Augments().size();
    text.Format("Augment Count: %d", count);
    SetDlgItemText(IDC_STATIC_AUGMENTCOUNT, text);

    count = Items().size();
    text.Format("Item Count: %d", count);
    SetDlgItemText(IDC_STATIC_ITEMCOUNT, text);

    count = Spells().size();
    text.Format("Spell Count: %d", count);
    SetDlgItemText(IDC_STATIC_SPELLCOUNT, text);

    count = EnhancementTrees().size();
    text.Format("Enhancement Tree Count: %d", count);
    SetDlgItemText(IDC_STATIC_TREECOUNT, text);

    count = OptionalBuffs().size();
    text.Format("Self and Party Buff Count: %d", count);
    SetDlgItemText(IDC_STATIC_PARTYBUFF_COUNT, text);

    m_bugReporters.SetWindowText(
        "In alphabetical order:\r\n"
        "\r\n"
        "aarondev42, "
        "Aaumini, "
        "AmericanCarioca, "
        "Anty, "
        "Arcanaverse, "
        "Alexmcdavid, "
        "Arthol, "
        "Attlanttizz, "
        "Awesome, "
        "Aymerick55, "
        "Baldur9, "
        "Belenian, "
        "diogo-ayres, "
        "Bizmarkie, "
        "Bjond, "
        "Bogs, "
        "Brakkart, "
        "Brasidas, "
        "BrentAuble, "
        "BuckGB, "
        "Callie033, "
        "Cardo, "
        "canicus, "
        "CBDunk, "
        "chaos_magus, "
        "ChickenMobile, "
        "ChilledMage, "
        "christhemisss, "
        "CottonEyeJim, "
        "Crolug, "
        "d3x-dt3, "
        "Daawsomeone, "
        "Daedricz-1, "
        "davidcysero, "
        "dbroccoliman, "
        "Deathbringer, "
        "Dielzen, "
        "DiQuintino, "
        "DODOCung, "
        "Dom, "
        "DougSK, "
        "dpm5d, "
        "drjoans, "
        "Droomar, "
        "Duncanthrax, "
        "Dvinesword, "
        "ericcater, "
        "EmmaHarper, "
        "Eunostos, "
        "Fisto Mk I, "
        "fitzabir, "
        "funifulv, "
        "Garix, "
        "Gemini-Dragon, "
        "ghouleater, "
        "GillianGroks, "
        "Graceana, "
        "Griglok, "
        "Guntharm, "
        "Gus, "
        "Habreno, "
        "Hakoist, "
        "heartmanpd, "
        "HighLordPudding, "
        "Hobgoblin, "
        "Howiedoohan, "
        "HunterRayder93, "
        "Huntxrd, "
        "Ibtaken, "
        "jacko, "
        "JelloDDO, "
        "Jennan, "
        "Jeronimo, "
        "joenuts1, "
        "Jokersir2, "
        "Kalibano, "
        "karlisher, "
        "keluynn, "
        "Kenaustin, "
        "Kerkos, "
        "KingOfBritains, "
        "knitesilver, "
        "KSnowGinger, "
        "Lanta_Nan, "
        "Larendell, "
        "Lilthiken, "
        "Lithol, "
        "mccoole, "
        "Melkizadek1, "
        "melynnn007, "
        "MIvanIsten, "
        "morrikan, "
        "MrEkshin, "
        "MuazAlhaidar, "
        "Mummykiller, "
        "nadia72295, "
        "nb756, "
        "neain, "
        "Nectmar, "
        "Nick-PC, "
        "NickJam, "
        "nonamenoredacted, "
        "Novatron, "
        "Ntoukis, "
        "Nyreb, "
        "Ogridium, "
        "opi-smccoole, "
        "onkei69, "
        "Orbalus, "
        "osake, "
        "Ozmandias42, "
        "Quijonn, "
        "PatDBunny, "
        "pevergreen, "
        "PurpleSerpent, "
        "rabidfox, "
        "Raydiante72, "
        "Refutor, "
        "Rehmlah, "
        "rosstracy, "
        "Rudebasilisk, "
        "Sarandra, "
        "Seppi Pearlsmith, "
        "shoka, "
        "SlaapKous, "
        "Sneakywink, "
        "SpartanKiller13, "
        "Spook,"
        "Strimtom, "
        "Stoner81, "
        "TandyFW, "
        "Terraism, "
        "tevwoods, "
        "Texturace, "
        "The Being, "
        "thomas-langworthy, "
        "Thumbed_Servant, "
        "trucnar, "
        "Tuxedomanwashere, "
        "Underflow, "
        "Videospirit, "
        "Viperswhip, "
        "vryxnr, "
        "VSversus, "
        "Vyder, "
        "warioman91, "
        "Warsaga, "
        "WestonChan, "
        "xDTungTungTungSahur, "
        "Xylyth, "
        "ydufraisse, "
        "Yndrofian, "
        "YoureNotDead, "
        "yugodealer, "
        "zenith90210, "
        "zilgabex, "
        "Zoe Kestral");
    m_bugReporters.SetSel((DWORD)-1);    // nothing starts selected
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


BOOL CAboutDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    GetDlgItem(IDOK)->SetFocus();

    return FALSE;
}
