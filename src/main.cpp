#include <windows.h>
#include <windowsx.h>
#include <commdlg.h>
#include <shellapi.h>

#include <fstream>
#include <string>
#include <vector>

#include "auth.h"
#include "config.h"
#include "menu.h"
#include "resource_ids.h"
#include "theme.h"

namespace {

constexpr const char* kWindowClass = "WndProc";
constexpr const char* kWindowTitle = "";

constexpr UINT SCI_SETTEXT = 2181;
constexpr UINT SCI_GETTEXT = 2182;
constexpr UINT SCI_GETTEXTLENGTH = 2183;
constexpr UINT SCI_SETLEXER = 4001;
constexpr UINT SCI_SETKEYWORDS = 4005;
constexpr UINT SCI_STYLECLEARALL = 2050;
constexpr UINT SCI_STYLESETFORE = 2051;
constexpr UINT SCI_STYLESETBACK = 2052;
constexpr UINT SCI_STYLESETBOLD = 2053;
constexpr UINT SCI_STYLESETITALIC = 2054;
constexpr UINT SCI_STYLESETSIZE = 2055;
constexpr UINT SCI_STYLESETFONT = 2056;
constexpr UINT SCI_SETCARETFORE = 2069;
constexpr UINT SCI_SETSELFORE = 2067;
constexpr UINT SCI_SETSELBACK = 2068;
constexpr UINT SCI_SETCARETLINEVISIBLE = 2096;
constexpr UINT SCI_SETCARETLINEBACK = 2098;
constexpr UINT SCI_SETMARGINTYPEN = 2240;
constexpr UINT SCI_SETMARGINWIDTHN = 2242;
constexpr UINT SCI_SETMARGINMASKN = 2244;
constexpr UINT SCI_SETMARGINSENSITIVEN = 2246;
constexpr UINT SCI_SETFOLDFLAGS = 2233;
constexpr UINT SCI_SETPROPERTY = 4004;

constexpr int SCLEX_LUA = 15;
constexpr int STYLE_DEFAULT = 32;
constexpr int STYLE_LINENUMBER = 33;
constexpr int STYLE_BRACELIGHT = 34;
constexpr int STYLE_BRACEBAD = 35;
constexpr int STYLE_INDENTGUIDE = 37;
constexpr int SCE_LUA_DEFAULT = 0;
constexpr int SCE_LUA_COMMENT = 1;
constexpr int SCE_LUA_COMMENTLINE = 2;
constexpr int SCE_LUA_COMMENTDOC = 3;
constexpr int SCE_LUA_NUMBER = 4;
constexpr int SCE_LUA_WORD = 5;
constexpr int SCE_LUA_STRING = 6;
constexpr int SCE_LUA_CHARACTER = 7;
constexpr int SCE_LUA_LITERALSTRING = 8;
constexpr int SCE_LUA_PREPROCESSOR = 9;
constexpr int SCE_LUA_OPERATOR = 10;
constexpr int SCE_LUA_IDENTIFIER = 11;
constexpr int SCE_LUA_STRINGEOL = 12;
constexpr int SCE_LUA_WORD2 = 13;
constexpr int SCE_LUA_WORD3 = 14;
constexpr int SCE_LUA_WORD4 = 15;
constexpr int SCE_LUA_WORD5 = 16;
constexpr int SCE_LUA_WORD6 = 17;
constexpr int SCE_LUA_WORD7 = 18;
constexpr int SCE_LUA_WORD8 = 19;
constexpr int SCE_LUA_LABEL = 20;

COLORREF RgbToBgr(COLORREF rgb) {
  return RGB(GetBValue(rgb), GetGValue(rgb), GetRValue(rgb));
}

HWND g_hwnd = nullptr;
HWND g_userEdit = nullptr;
HWND g_passEdit = nullptr;
HWND g_submitBtn = nullptr;

HWND g_tab = nullptr;
HWND g_editor = nullptr;
HWND g_commandBox = nullptr;
HWND g_executeBtn = nullptr;
HWND g_openBtn = nullptr;
HWND g_clearBtn = nullptr;

HWND g_sideSave = nullptr;
HWND g_sideWordWrap = nullptr;
HWND g_sideAuto = nullptr;
HWND g_sideGoogle = nullptr;
HWND g_sideKrystal = nullptr;
HWND g_sideWofly = nullptr;
HWND g_sideHide = nullptr;

HFONT g_loginFont = nullptr;
HFONT g_cmdFont = nullptr;
HFONT g_buttonFont = nullptr;

HMENU g_mainMenu = nullptr;
bool g_loggedIn = false;
rc7::UiConfig g_uiConfig{};
rc7::ThemeContext g_theme{};

std::string GetText(HWND h) {
  char buf[512]{};
  GetWindowTextA(h, buf, static_cast<int>(sizeof(buf)));
  return std::string(buf);
}

bool IsScintillaWindow(HWND h) {
  if (!h) return false;
  char cls[64]{};
  GetClassNameA(h, cls, static_cast<int>(sizeof(cls)));
  return std::string(cls) == "Scintilla";
}

std::string GetEditorText() {
  if (!g_editor || !IsWindow(g_editor)) return {};
  if (IsScintillaWindow(g_editor)) {
    const LRESULT len = SendMessageA(g_editor, SCI_GETTEXTLENGTH, 0, 0);
    if (len <= 0) return {};
    std::vector<char> buf(static_cast<size_t>(len) + 1, '\0');
    SendMessageA(g_editor, SCI_GETTEXT, static_cast<WPARAM>(buf.size()), reinterpret_cast<LPARAM>(buf.data()));
    return std::string(buf.data());
  }
  return GetText(g_editor);
}

void SetEditorText(const std::string& text) {
  if (!g_editor || !IsWindow(g_editor)) return;
  if (IsScintillaWindow(g_editor)) {
    SendMessageA(g_editor, SCI_SETTEXT, 0, reinterpret_cast<LPARAM>(text.c_str()));
  } else {
    SetWindowTextA(g_editor, text.c_str());
  }
}

void ConfigureScintillaLua() {
  if (!g_editor || !IsScintillaWindow(g_editor)) return;

  constexpr COLORREF kBack = RGB(30, 30, 30);
  constexpr COLORREF kGutterBack = RGB(37, 37, 38);
  constexpr COLORREF kGutterText = RGB(133, 133, 133);
  constexpr COLORREF kDefaultFore = RGB(212, 212, 212);
  constexpr COLORREF kCaret = RGB(220, 220, 220);
  constexpr COLORREF kSelectionBack = RGB(38, 79, 120);
  constexpr COLORREF kCaretLine = RGB(45, 45, 48);

  SendMessageA(g_editor, SCI_SETLEXER, SCLEX_LUA, 0);
  SendMessageA(
      g_editor,
      SCI_SETKEYWORDS,
      0,
      reinterpret_cast<LPARAM>(
          "and break do else elseif end false for function if in local nil not or repeat return then true until while"));
  SendMessageA(
      g_editor,
      SCI_SETKEYWORDS,
      1,
      reinterpret_cast<LPARAM>("self _G _VERSION pairs ipairs pcall xpcall getmetatable setmetatable rawget rawset require"));

  SendMessageA(g_editor, SCI_SETPROPERTY, reinterpret_cast<WPARAM>("fold"), reinterpret_cast<LPARAM>("1"));
  SendMessageA(g_editor, SCI_SETPROPERTY, reinterpret_cast<WPARAM>("fold.compact"), reinterpret_cast<LPARAM>("1"));

  SendMessageA(g_editor, SCI_STYLESETFONT, STYLE_DEFAULT, reinterpret_cast<LPARAM>("Verdana"));
  SendMessageA(g_editor, SCI_STYLESETSIZE, STYLE_DEFAULT, 10);
  SendMessageA(g_editor, SCI_STYLESETBOLD, STYLE_DEFAULT, 0);
  SendMessageA(g_editor, SCI_STYLESETITALIC, STYLE_DEFAULT, 0);
  SendMessageA(g_editor, SCI_STYLESETFORE, STYLE_DEFAULT, RgbToBgr(kDefaultFore));
  SendMessageA(g_editor, SCI_STYLESETBACK, STYLE_DEFAULT, RgbToBgr(kBack));
  SendMessageA(g_editor, SCI_STYLECLEARALL, 0, 0);

  SendMessageA(g_editor, SCI_STYLESETFORE, STYLE_LINENUMBER, RgbToBgr(kGutterText));
  SendMessageA(g_editor, SCI_STYLESETBACK, STYLE_LINENUMBER, RgbToBgr(kGutterBack));
  SendMessageA(g_editor, SCI_STYLESETFORE, STYLE_BRACELIGHT, RGB(0, 0, 0));
  SendMessageA(g_editor, SCI_STYLESETBACK, STYLE_BRACELIGHT, RGB(218, 165, 32));
  SendMessageA(g_editor, SCI_STYLESETFORE, STYLE_BRACEBAD, RGB(255, 255, 255));
  SendMessageA(g_editor, SCI_STYLESETBACK, STYLE_BRACEBAD, RGB(170, 0, 0));
  SendMessageA(g_editor, SCI_STYLESETFORE, STYLE_INDENTGUIDE, RGB(64, 64, 64));
  SendMessageA(g_editor, SCI_STYLESETBACK, STYLE_INDENTGUIDE, RgbToBgr(kBack));

  SendMessageA(g_editor, SCI_STYLESETFORE, SCE_LUA_DEFAULT, RgbToBgr(kDefaultFore));
  SendMessageA(g_editor, SCI_STYLESETBACK, SCE_LUA_DEFAULT, RgbToBgr(kBack));
  SendMessageA(g_editor, SCI_STYLESETFORE, SCE_LUA_COMMENT, RGB(106, 153, 85));
  SendMessageA(g_editor, SCI_STYLESETBACK, SCE_LUA_COMMENT, RgbToBgr(kBack));
  SendMessageA(g_editor, SCI_STYLESETFORE, SCE_LUA_COMMENTLINE, RGB(106, 153, 85));
  SendMessageA(g_editor, SCI_STYLESETBACK, SCE_LUA_COMMENTLINE, RgbToBgr(kBack));
  SendMessageA(g_editor, SCI_STYLESETFORE, SCE_LUA_COMMENTDOC, RGB(87, 166, 74));
  SendMessageA(g_editor, SCI_STYLESETBACK, SCE_LUA_COMMENTDOC, RgbToBgr(kBack));
  SendMessageA(g_editor, SCI_STYLESETFORE, SCE_LUA_NUMBER, RGB(181, 206, 168));
  SendMessageA(g_editor, SCI_STYLESETBACK, SCE_LUA_NUMBER, RgbToBgr(kBack));
  SendMessageA(g_editor, SCI_STYLESETFORE, SCE_LUA_WORD, RGB(86, 156, 214));
  SendMessageA(g_editor, SCI_STYLESETBOLD, SCE_LUA_WORD, 0);
  SendMessageA(g_editor, SCI_STYLESETBACK, SCE_LUA_WORD, RgbToBgr(kBack));
  SendMessageA(g_editor, SCI_STYLESETFORE, SCE_LUA_WORD2, RGB(78, 201, 176));
  SendMessageA(g_editor, SCI_STYLESETBOLD, SCE_LUA_WORD2, 0);
  SendMessageA(g_editor, SCI_STYLESETBACK, SCE_LUA_WORD2, RgbToBgr(kBack));
  SendMessageA(g_editor, SCI_STYLESETFORE, SCE_LUA_STRING, RGB(206, 145, 120));
  SendMessageA(g_editor, SCI_STYLESETBACK, SCE_LUA_STRING, RgbToBgr(kBack));
  SendMessageA(g_editor, SCI_STYLESETFORE, SCE_LUA_CHARACTER, RGB(206, 145, 120));
  SendMessageA(g_editor, SCI_STYLESETBACK, SCE_LUA_CHARACTER, RgbToBgr(kBack));
  SendMessageA(g_editor, SCI_STYLESETFORE, SCE_LUA_LITERALSTRING, RGB(214, 157, 133));
  SendMessageA(g_editor, SCI_STYLESETBACK, SCE_LUA_LITERALSTRING, RgbToBgr(kBack));
  SendMessageA(g_editor, SCI_STYLESETFORE, SCE_LUA_PREPROCESSOR, RGB(220, 220, 170));
  SendMessageA(g_editor, SCI_STYLESETBACK, SCE_LUA_PREPROCESSOR, RgbToBgr(kBack));
  SendMessageA(g_editor, SCI_STYLESETFORE, SCE_LUA_OPERATOR, RGB(212, 212, 212));
  SendMessageA(g_editor, SCI_STYLESETBACK, SCE_LUA_OPERATOR, RgbToBgr(kBack));
  SendMessageA(g_editor, SCI_STYLESETFORE, SCE_LUA_IDENTIFIER, RGB(156, 220, 254));
  SendMessageA(g_editor, SCI_STYLESETBACK, SCE_LUA_IDENTIFIER, RgbToBgr(kBack));
  SendMessageA(g_editor, SCI_STYLESETFORE, SCE_LUA_STRINGEOL, RGB(255, 0, 0));
  SendMessageA(g_editor, SCI_STYLESETBACK, SCE_LUA_STRINGEOL, RgbToBgr(kBack));
  SendMessageA(g_editor, SCI_STYLESETFORE, SCE_LUA_WORD3, RGB(197, 134, 192));
  SendMessageA(g_editor, SCI_STYLESETBACK, SCE_LUA_WORD3, RgbToBgr(kBack));
  SendMessageA(g_editor, SCI_STYLESETFORE, SCE_LUA_WORD4, RGB(79, 193, 255));
  SendMessageA(g_editor, SCI_STYLESETBACK, SCE_LUA_WORD4, RgbToBgr(kBack));
  SendMessageA(g_editor, SCI_STYLESETFORE, SCE_LUA_WORD5, RGB(220, 220, 170));
  SendMessageA(g_editor, SCI_STYLESETBACK, SCE_LUA_WORD5, RgbToBgr(kBack));
  SendMessageA(g_editor, SCI_STYLESETFORE, SCE_LUA_WORD6, RGB(156, 220, 254));
  SendMessageA(g_editor, SCI_STYLESETBACK, SCE_LUA_WORD6, RgbToBgr(kBack));
  SendMessageA(g_editor, SCI_STYLESETFORE, SCE_LUA_WORD7, RGB(255, 198, 109));
  SendMessageA(g_editor, SCI_STYLESETBACK, SCE_LUA_WORD7, RgbToBgr(kBack));
  SendMessageA(g_editor, SCI_STYLESETFORE, SCE_LUA_WORD8, RGB(87, 166, 74));
  SendMessageA(g_editor, SCI_STYLESETBACK, SCE_LUA_WORD8, RgbToBgr(kBack));
  SendMessageA(g_editor, SCI_STYLESETFORE, SCE_LUA_LABEL, RGB(212, 212, 212));
  SendMessageA(g_editor, SCI_STYLESETBACK, SCE_LUA_LABEL, RgbToBgr(kBack));

  SendMessageA(g_editor, SCI_SETCARETFORE, RgbToBgr(kCaret), 0);
  SendMessageA(g_editor, SCI_SETSELFORE, 1, RgbToBgr(RGB(255, 255, 255)));
  SendMessageA(g_editor, SCI_SETSELBACK, 1, RgbToBgr(kSelectionBack));
  SendMessageA(g_editor, SCI_SETCARETLINEVISIBLE, 1, 0);
  SendMessageA(g_editor, SCI_SETCARETLINEBACK, RgbToBgr(kCaretLine), 0);
  SendMessageA(g_editor, SCI_SETMARGINTYPEN, 0, 1);
  SendMessageA(g_editor, SCI_SETMARGINWIDTHN, 0, 40);
  SendMessageA(g_editor, SCI_SETMARGINMASKN, 2, static_cast<LPARAM>(-1));
  SendMessageA(g_editor, SCI_SETMARGINWIDTHN, 2, 16);
  SendMessageA(g_editor, SCI_SETMARGINSENSITIVEN, 2, 1);
  SendMessageA(g_editor, SCI_SETFOLDFLAGS, 16, 0);
}

void CenterWindow(HWND hwnd, int width, int height) {
  RECT r{};
  GetClientRect(GetDesktopWindow(), &r);
  const int x = (r.right / 2) - (width / 2);
  const int y = (r.bottom / 2) - (height / 2);
  SetWindowPos(hwnd, nullptr, x, y, width, height, SWP_NOZORDER);
}

void OpenUrl(const char* url) {
  ShellExecuteA(nullptr, "open", url, nullptr, nullptr, SW_SHOWNORMAL);
}

bool HitClientPoint(HWND ctrl, POINT ptClient) {
  if (!ctrl || !IsWindow(ctrl)) return false;
  RECT rc{};
  GetWindowRect(ctrl, &rc);
  MapWindowPoints(nullptr, g_hwnd, reinterpret_cast<LPPOINT>(&rc), 2);
  return ptClient.x > rc.left && ptClient.x < rc.right && ptClient.y > rc.top && ptClient.y < rc.bottom;
}

void FlashStatic(HWND ctrl) {
  if (!ctrl || !IsWindow(ctrl)) return;
  ShowWindow(ctrl, SW_SHOW);
  Sleep(60);
  ShowWindow(ctrl, SW_HIDE);
}

void UpdateMenuChecks() {
  if (!g_mainMenu) return;
  CheckMenuItem(g_mainMenu, rc7::MENU_WORD_WRAP, MF_BYCOMMAND | (g_uiConfig.wordWrap ? MF_CHECKED : MF_UNCHECKED));
  CheckMenuItem(g_mainMenu, rc7::MENU_AUTO_HOOK, MF_BYCOMMAND | (g_uiConfig.autoHook ? MF_CHECKED : MF_UNCHECKED));
}

void CreateControls() {
  g_passEdit = CreateWindowExA(0, "EDIT", "", 0x500100A1u, 74, 140, 156, 24, g_hwnd, nullptr, nullptr, nullptr);
  g_userEdit = CreateWindowExA(0, "EDIT", "", 0x50000081u, 74, 110, 156, 24, g_hwnd, nullptr, nullptr, nullptr);
  g_editor = CreateWindowExA(0x20000u, "Scintilla", "", 0x40311044u, 6, 27, 286, 216, g_hwnd, nullptr, nullptr, nullptr);
  if (!g_editor) {
    g_editor = CreateWindowExA(0x20000u, "EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | WS_VSCROLL, 6, 27, 286, 216, g_hwnd, nullptr, nullptr, nullptr);
  }
  g_commandBox = CreateWindowExA(0x200u, "EDIT", "", 0x40200854u, 6, 270, 287, 53, g_hwnd, nullptr, nullptr, nullptr);
  g_tab = CreateWindowExA(0, "SysTabControl32", "", 0x44002000u, 6, 1, 288, 25, g_hwnd, reinterpret_cast<HMENU>(0x20A), nullptr, nullptr);

  g_submitBtn = CreateWindowExA(0, "BUTTON", "", 0x5000000Bu, 102, 170, 100, 23, g_hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(rc7::CTRL_SUBMIT)), nullptr, nullptr);
  g_executeBtn = CreateWindowExA(0, "BUTTON", "", 0x4000000Bu, 102, 244, 95, 25, g_hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(rc7::CTRL_EXECUTE)), nullptr, nullptr);
  g_openBtn = CreateWindowExA(0, "BUTTON", "", 0x4000000Bu, 6, 244, 95, 25, g_hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(rc7::CTRL_OPEN)), nullptr, nullptr);
  g_clearBtn = CreateWindowExA(0, "BUTTON", "", 0x4000000Bu, 198, 244, 95, 25, g_hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(rc7::CTRL_CLEAR)), nullptr, nullptr);

  g_sideSave = CreateWindowExA(0, "STATIC", "", 0x40000000u, 304, 107, 30, 30, g_hwnd, nullptr, nullptr, nullptr);
  g_sideWordWrap = CreateWindowExA(0, "STATIC", "", 0x40000000u, 304, 144, 30, 30, g_hwnd, nullptr, nullptr, nullptr);
  g_sideAuto = CreateWindowExA(0, "STATIC", "", 0x40000000u, 304, 181, 30, 30, g_hwnd, nullptr, nullptr, nullptr);
  g_sideGoogle = CreateWindowExA(0, "STATIC", "", 0x40000000u, 304, 218, 30, 30, g_hwnd, nullptr, nullptr, nullptr);
  g_sideKrystal = CreateWindowExA(0, "STATIC", "", 0x40000000u, 304, 255, 30, 30, g_hwnd, nullptr, nullptr, nullptr);
  g_sideWofly = CreateWindowExA(0, "STATIC", "", 0x40000000u, 304, 292, 30, 30, g_hwnd, nullptr, nullptr, nullptr);
  g_sideHide = CreateWindowExA(0, "STATIC", "", 0x50000000u, 300, 0, 39, 328, g_hwnd, nullptr, nullptr, nullptr);

  g_cmdFont = CreateFontA(19, 7, 0, 0, 300, 0, 0, 0, 0, 0, 2u, 2u, 0x41u, "Sans Serif");
  if (g_cmdFont && g_commandBox) SendMessageA(g_commandBox, WM_SETFONT, reinterpret_cast<WPARAM>(g_cmdFont), TRUE);

  g_loginFont = CreateFontA(20, 7, 0, 0, 400, 0, 0, 0, 0, 0, 0, 0, 1u, "Verdana");
  if (g_loginFont && g_userEdit) SendMessageA(g_userEdit, WM_SETFONT, reinterpret_cast<WPARAM>(g_loginFont), TRUE);
  if (g_loginFont && g_passEdit) SendMessageA(g_passEdit, WM_SETFONT, reinterpret_cast<WPARAM>(g_loginFont), TRUE);
  g_buttonFont = CreateFontA(17, 7, 0, 0, 400, 0, 0, 0, 0, 0, 0, 0, 1u, "Verdana");

  if (g_cmdFont && g_editor && !IsScintillaWindow(g_editor)) {
    SendMessageA(g_editor, WM_SETFONT, reinterpret_cast<WPARAM>(g_cmdFont), TRUE);
  }
  ConfigureScintillaLua();

  ShowWindow(g_tab, SW_HIDE);
  ShowWindow(g_editor, SW_HIDE);
  ShowWindow(g_commandBox, SW_HIDE);
  ShowWindow(g_executeBtn, SW_HIDE);
  ShowWindow(g_openBtn, SW_HIDE);
  ShowWindow(g_clearBtn, SW_HIDE);
  ShowWindow(g_sideSave, SW_SHOW);
  ShowWindow(g_sideWordWrap, SW_SHOW);
  ShowWindow(g_sideAuto, SW_SHOW);
  ShowWindow(g_sideGoogle, SW_SHOW);
  ShowWindow(g_sideKrystal, SW_SHOW);
  ShowWindow(g_sideWofly, SW_SHOW);
  ShowWindow(g_sideHide, SW_SHOW);
}

void DrawOwnerButton(const DRAWITEMSTRUCT* dis) {
  const bool down = (dis->itemState & ODS_SELECTED) != 0;
  const bool hot = (dis->itemState & ODS_HOTLIGHT) != 0;

  HBITMAP bmp = nullptr;
  const char* text = nullptr;
  int tx = 0;
  int ty = 0;
  int len = 0;

  if (dis->CtlID == rc7::CTRL_SUBMIT) {
    bmp = down ? g_theme.bitmaps.buttonClicked : (hot ? g_theme.bitmaps.buttonHover : g_theme.bitmaps.buttonIdle);
    text = "Submit";
    tx = 21;
    ty = 3;
    len = 6;
  } else if (dis->CtlID == rc7::CTRL_EXECUTE) {
    bmp = down ? g_theme.bitmaps.smallButtonClicked : (hot ? g_theme.bitmaps.smallButtonHover : g_theme.bitmaps.smallButtonIdle);
    text = "Execute";
    tx = 17;
    ty = 2;
    len = 7;
  } else if (dis->CtlID == rc7::CTRL_OPEN) {
    bmp = down ? g_theme.bitmaps.smallButtonClicked : (hot ? g_theme.bitmaps.smallButtonHover : g_theme.bitmaps.smallButtonIdle);
    text = "Open";
    tx = 23;
    ty = 2;
    len = 4;
  } else if (dis->CtlID == rc7::CTRL_CLEAR) {
    bmp = down ? g_theme.bitmaps.smallButtonClicked : (hot ? g_theme.bitmaps.smallButtonHover : g_theme.bitmaps.smallButtonIdle);
    text = "Clear";
    tx = 23;
    ty = 2;
    len = 5;
  }

  RECT rc = dis->rcItem;
  if (!bmp) {
    FillRect(dis->hDC, &rc, static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH)));
  } else {
    BITMAP bm{};
    GetObjectA(bmp, sizeof(bm), &bm);
    HDC mem = CreateCompatibleDC(dis->hDC);
    HGDIOBJ oldBmp = SelectObject(mem, bmp);
    BitBlt(dis->hDC, rc.left, rc.top, bm.bmWidth, bm.bmHeight, mem, 0, 0, SRCCOPY);
    SelectObject(mem, oldBmp);
    DeleteDC(mem);
  }

  SetBkMode(dis->hDC, TRANSPARENT);
  SetTextColor(dis->hDC, g_uiConfig.fontColor);
  if (text) {
    HGDIOBJ oldFont = nullptr;
    if (g_buttonFont) {
      oldFont = SelectObject(dis->hDC, g_buttonFont);
    }
    TextOutA(dis->hDC, rc.left + tx, rc.top + ty, text, len);
    if (oldFont) {
      SelectObject(dis->hDC, oldFont);
    }
  }
}

void EnterPostLoginMode() {
  CenterWindow(g_hwnd, 355, 382);

  ShowWindow(g_tab, SW_SHOW);
  ShowWindow(g_editor, SW_SHOW);
  ShowWindow(g_commandBox, SW_SHOW);
  ShowWindow(g_openBtn, SW_SHOW);
  ShowWindow(g_executeBtn, SW_SHOW);
  ShowWindow(g_clearBtn, SW_SHOW);
  ShowWindow(g_sideSave, SW_HIDE);
  ShowWindow(g_sideWordWrap, SW_HIDE);
  ShowWindow(g_sideAuto, SW_HIDE);
  ShowWindow(g_sideGoogle, SW_HIDE);
  ShowWindow(g_sideKrystal, SW_HIDE);
  ShowWindow(g_sideWofly, SW_HIDE);
  ShowWindow(g_sideHide, SW_HIDE);

  if (g_submitBtn) {
    DestroyWindow(g_submitBtn);
    g_submitBtn = nullptr;
  }
  if (g_userEdit) {
    DestroyWindow(g_userEdit);
    g_userEdit = nullptr;
  }
  if (g_passEdit) {
    DestroyWindow(g_passEdit);
    g_passEdit = nullptr;
  }

  g_mainMenu = rc7::BuildMainMenu(g_hwnd);
  UpdateMenuChecks();
  g_loggedIn = true;
  InvalidateRect(g_hwnd, nullptr, TRUE);
}

void HandleSubmit() {
  rc7::Credentials creds{GetText(g_userEdit), GetText(g_passEdit)};
  rc7::SaveCredentialsToRegistry(creds);

  bool ok = rc7::AuthenticateCrackedBypass();
  std::string version;
  std::string error;
  if (!ok) {
    ok = rc7::AuthenticateOriginal(creds, &version, &error);
  }
  if (ok) {
    EnterPostLoginMode();
    return;
  }

  const char* msg = error.empty() ? "Authentication failed." : error.c_str();
  MessageBoxA(g_hwnd, msg, "Authentication failed.", MB_OK | MB_ICONERROR);
}

void HandleSideClick(POINT ptClient) {
  if (!g_loggedIn) return;

  if (HitClientPoint(g_sideSave, ptClient)) {
    FlashStatic(g_sideSave);
    OPENFILENAMEA ofn{};
    char path[MAX_PATH]{};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = g_hwnd;
    ofn.lpstrFilter = "Lua Script (*.lua)\0*.lua\0Text File (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
    ofn.lpstrFile = path;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrDefExt = "lua";
    ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;
    if (GetSaveFileNameA(&ofn)) {
      std::ofstream out(path, std::ios::binary | std::ios::trunc);
      const std::string text = GetEditorText();
      out.write(text.data(), static_cast<std::streamsize>(text.size()));
    }
    return;
  }
  if (HitClientPoint(g_sideAuto, ptClient)) {
    FlashStatic(g_sideAuto);
    g_uiConfig.autoHook = !g_uiConfig.autoHook;
    if (g_uiConfig.autoHook) {
      MessageBoxA(g_hwnd, "Auto hook results in frequent crashes.\nIt is recommended you leave this option off.", "RC7 - Warning", MB_OK | MB_ICONWARNING);
    }
    UpdateMenuChecks();
    return;
  }
  if (HitClientPoint(g_sideGoogle, ptClient)) {
    FlashStatic(g_sideGoogle);
    OpenUrl("https://goo.gl/C0aoBm");
    return;
  }
  if (HitClientPoint(g_sideKrystal, ptClient)) {
    FlashStatic(g_sideKrystal);
    OpenUrl("https://crunchman.info/");
    return;
  }
  if (HitClientPoint(g_sideWofly, ptClient)) {
    FlashStatic(g_sideWofly);
    OpenUrl("http://rc7forums.pcriot.com");
    return;
  }
  if (HitClientPoint(g_sideWordWrap, ptClient)) {
    FlashStatic(g_sideWordWrap);
    g_uiConfig.wordWrap = !g_uiConfig.wordWrap;
    UpdateMenuChecks();
    return;
  }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  switch (msg) {
    case WM_CREATE:
      g_hwnd = hwnd;
      CreateControls();
      CenterWindow(hwnd, 355, 367);
      return 0;
    case WM_DRAWITEM: {
      const auto* dis = reinterpret_cast<const DRAWITEMSTRUCT*>(lParam);
      if (!dis || dis->CtlType != ODT_BUTTON) return FALSE;
      DrawOwnerButton(dis);
      return TRUE;
    }
    case WM_CTLCOLOREDIT: {
      HDC dc = reinterpret_cast<HDC>(wParam);
      HWND ctl = reinterpret_cast<HWND>(lParam);
      if (ctl == g_userEdit || ctl == g_passEdit) {
        SetBkMode(dc, TRANSPARENT);
        SetTextColor(dc, g_uiConfig.fontColor);
        return reinterpret_cast<INT_PTR>(g_theme.textBoxBrush ? g_theme.textBoxBrush : GetSysColorBrush(COLOR_WINDOW));
      }
      return reinterpret_cast<INT_PTR>(GetSysColorBrush(COLOR_WINDOW));
    }
    case WM_CTLCOLORSTATIC: {
      HDC dc = reinterpret_cast<HDC>(wParam);
      HWND ctl = reinterpret_cast<HWND>(lParam);
      SetBkMode(dc, TRANSPARENT);
      SetTextColor(dc, g_uiConfig.fontColor);

      if (ctl == g_sideHide && g_theme.hideSideBrush) return reinterpret_cast<INT_PTR>(g_theme.hideSideBrush);
      if (ctl == g_sideSave && g_theme.saveBrush) return reinterpret_cast<INT_PTR>(g_theme.saveBrush);
      if (ctl == g_sideWordWrap && g_theme.wordWrapBrush) return reinterpret_cast<INT_PTR>(g_theme.wordWrapBrush);
      if (ctl == g_sideAuto && g_theme.autoBrush) return reinterpret_cast<INT_PTR>(g_theme.autoBrush);
      if (ctl == g_sideGoogle && g_theme.googleBrush) return reinterpret_cast<INT_PTR>(g_theme.googleBrush);
      if (ctl == g_sideKrystal && g_theme.krystalBrush) return reinterpret_cast<INT_PTR>(g_theme.krystalBrush);
      if (ctl == g_sideWofly && g_theme.woflyBrush) return reinterpret_cast<INT_PTR>(g_theme.woflyBrush);
      return reinterpret_cast<INT_PTR>(GetStockObject(WHITE_BRUSH));
    }
    case WM_LBUTTONDOWN: {
      POINT pt{GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
      HandleSideClick(pt);
      return 0;
    }
    case WM_COMMAND: {
      const unsigned int id = LOWORD(wParam);
      if (id == rc7::CTRL_SUBMIT && reinterpret_cast<HWND>(lParam) == g_submitBtn) {
        HandleSubmit();
        return 0;
      }
      if (id == rc7::CTRL_OPEN && reinterpret_cast<HWND>(lParam) == g_openBtn) {
        OPENFILENAMEA ofn{};
        char path[MAX_PATH]{};
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = hwnd;
        ofn.lpstrFilter = "Lua Script (*.lua)\0*.lua\0Text File (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
        ofn.lpstrFile = path;
        ofn.nMaxFile = MAX_PATH;
        ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
        if (GetOpenFileNameA(&ofn)) {
          std::ifstream in(path, std::ios::binary);
          std::string content((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
          SetEditorText(content);
        }
        return 0;
      }
      if (id == rc7::CTRL_CLEAR && reinterpret_cast<HWND>(lParam) == g_clearBtn) {
        SetEditorText("");
        if (g_commandBox && IsWindow(g_commandBox)) SetWindowTextA(g_commandBox, "");
        return 0;
      }
      if (id == rc7::CTRL_EXECUTE && reinterpret_cast<HWND>(lParam) == g_executeBtn) {
        return 0;
      }
      if (!g_loggedIn) return 0;

      if (id == rc7::MENU_FONT_COLOR) {
        CHOOSECOLORA cc{};
        COLORREF custom[16]{};
        cc.lStructSize = sizeof(cc);
        cc.hwndOwner = hwnd;
        cc.lpCustColors = custom;
        cc.rgbResult = g_uiConfig.fontColor;
        cc.Flags = CC_RGBINIT | CC_FULLOPEN;
        if (ChooseColorA(&cc)) {
          g_uiConfig.fontColor = cc.rgbResult;
          rc7::SaveUiConfig(g_uiConfig);
          ConfigureScintillaLua();
          InvalidateRect(hwnd, nullptr, TRUE);
        }
        return 0;
      }

      rc7::HandleMenuCommand(hwnd, id, &g_uiConfig.wordWrap, &g_uiConfig.autoHook);
      UpdateMenuChecks();
      return 0;
    }
    case WM_DESTROY:
      rc7::SaveUiConfig(g_uiConfig);
      if (g_loginFont) DeleteObject(g_loginFont);
      if (g_cmdFont) DeleteObject(g_cmdFont);
      if (g_buttonFont) DeleteObject(g_buttonFont);
      rc7::DestroyThemeContext(&g_theme);
      PostQuitMessage(0);
      return 0;
    default:
      return DefWindowProcA(hwnd, msg, wParam, lParam);
  }
}

}  

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nShowCmd) {
  g_uiConfig = rc7::LoadUiConfig();
  g_theme = rc7::LoadThemeContext(hInstance);

  WNDCLASSEXA wc{};
  wc.cbSize = sizeof(wc);
  wc.style = CS_HREDRAW | CS_VREDRAW;
  wc.lpfnWndProc = WndProc;
  wc.hInstance = hInstance;
  wc.hIcon = static_cast<HICON>(LoadImageA(hInstance, MAKEINTRESOURCEA(0x81), IMAGE_ICON, 32, 32, 0));
  if (!wc.hIcon) {
    wc.hIcon = LoadIconA(nullptr, IDI_APPLICATION);
  }
  wc.hIconSm = wc.hIcon;
  wc.hCursor = LoadCursorA(nullptr, IDC_ARROW);
  wc.hbrBackground = g_theme.backgroundBrush ? g_theme.backgroundBrush : GetSysColorBrush(COLOR_WINDOW);
  wc.lpszClassName = kWindowClass;

  if (!RegisterClassExA(&wc)) {
    MessageBoxA(nullptr, "RegisterClassEx failed!", "Runtime Error", MB_OK | MB_ICONERROR);
    return 1;
  }

  HWND hwnd = CreateWindowExA(
      8u,
      kWindowClass,
      kWindowTitle,
      0xA0000u,
      CW_USEDEFAULT,
      CW_USEDEFAULT,
      CW_USEDEFAULT,
      CW_USEDEFAULT,
      nullptr,
      nullptr,
      hInstance,
      nullptr);

  if (!hwnd) {
    MessageBoxA(nullptr, "CreateWindow failed!", "Runtime Error", MB_OK | MB_ICONERROR);
    return -1;
  }

  ShowWindow(hwnd, nShowCmd);
  UpdateWindow(hwnd);

  MSG msg{};
  while (GetMessageA(&msg, nullptr, 0, 0) > 0) {
    TranslateMessage(&msg);
    DispatchMessageA(&msg);
  }
  return static_cast<int>(msg.wParam);
}
