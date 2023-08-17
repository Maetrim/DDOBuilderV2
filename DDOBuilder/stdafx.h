
// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently

#pragma once
#pragma warning( disable : 4481 ) //nonstandard extension used: override specifier 'override'
#pragma warning( disable : 4407 ) // cast between different pointer to member representations, compiler may generate incorrect code
#pragma warning( disable : 26812 ) //the enum type "name" is unscoped. Prefer 'enum class' over 'enum'

#ifndef _SECURE_ATL
#define _SECURE_ATL 1
#endif

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // Exclude rarely-used stuff from Windows headers
#endif

#include "targetver.h"

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // some CString constructors will be explicit

// turns off MFC's hiding of some common and often safely ignored warning messages
#define _AFX_ALL_WARNINGS

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdisp.h>        // MFC Automation classes

#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>           // MFC support for Internet Explorer 4 Common Controls
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>             // MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <afxcontrolbars.h>     // MFC support for ribbons and control bars

#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif

const int c_controlSpacing = 3;

#define MAX_ACTION_POINTS       80
#define MAX_CLASS_LEVEL         20
#define MAX_EPIC_LEVEL          10
#define MAX_BUILDER_LEVEL       40
#define MAX_GAME_LEVEL          32
#define MAX_SPELL_LEVEL         9
#define MAX_ENHANCEMENT_TREES   7
#define MAX_DESTINY_TREES       3
#define MAX_REAPER_TREES        3
#define MAX_FILIGREE            20
#define MAX_ARTIFACT_FILIGREE   10
#define MAX_SKILL_TOME          5
#define MAX_FAVOR_ITEMS         25

#include <string>
const UINT UWM_NEW_DOCUMENT = ::RegisterWindowMessage(_T("NewActiveDocument"));
const UINT UWM_LOAD_COMPLETE = ::RegisterWindowMessage(_T("LoadComplete"));
const UINT UWM_UPDATE_TREES = ::RegisterWindowMessage(_T("UpdateTrees"));
const UINT UWM_UPDATE = ::RegisterWindowMessage(_T("Update"));
const UINT UWM_TOGGLE_INCLUDED = ::RegisterWindowMessage(_T("ToggleIgnoreItem"));
const COLORREF c_transparentColour = RGB(255, 128, 255);
const std::string Class_Unknown = "Unknown";
const std::string Class_Epic = "Epic";
const std::string Class_Legendary = "Legendary";
const std::string Feat_Automatic = "Automatic";
const std::string Feat_Granted = "Granted Feat";
const std::string Race_All = "All";
extern bool g_bShowIgnoredItems;

enum TreeType
{
    TT_enhancement = 0,
    TT_racial,
    TT_universal,
    TT_epicDestiny,
    TT_reaper,
    TT_preview,
    TT_allEnhancement,
    TT_allEnhancementMinusSpent
};
