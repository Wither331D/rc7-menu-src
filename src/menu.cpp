#include "menu.h"

#include "resource_ids.h"

#include <shellapi.h>

namespace rc7 {

static void OpenUrl(const char* url) {
  ShellExecuteA(nullptr, "open", url, nullptr, nullptr, SW_SHOWNORMAL);
}

HMENU BuildMainMenu(HWND hwnd) {
  HMENU mainMenu = CreateMenu();
  HMENU helpMenu = CreateMenu();
  HMENU settingsMenu = CreateMenu();
  HMENU commandsMenu = CreateMenu();
  HMENU themeMenu = CreateMenu();
  HMENU globalChatMenu = CreateMenu();

  HMENU rc7Popup = CreatePopupMenu();
  HMENU scriptsPopup = CreatePopupMenu();
  HMENU mapsPopup = CreatePopupMenu();

  AppendMenuA(settingsMenu, MF_STRING, MENU_FONT_COLOR, "Font color");
  AppendMenuA(settingsMenu, MF_STRING, MENU_AUTO_HOOK, "Auto hook");

  AppendMenuA(commandsMenu, MF_POPUP, reinterpret_cast<UINT_PTR>(rc7Popup), "Rc7");
  AppendMenuA(commandsMenu, MF_POPUP, reinterpret_cast<UINT_PTR>(scriptsPopup), "Scripts");
  AppendMenuA(commandsMenu, MF_POPUP, reinterpret_cast<UINT_PTR>(mapsPopup), "Maps");

  AppendMenuA(rc7Popup, MF_STRING, MENU_CODE_EDITOR, "Code Editor");
  AppendMenuA(rc7Popup, MF_STRING, MENU_CLOSE_ROBLOX, "Close Roblox");
  AppendMenuA(rc7Popup, MF_STRING, MENU_RESTART, "Restart");

  AppendMenuA(mapsPopup, MF_STRING, 18, "Fenrier arena");
  AppendMenuA(mapsPopup, MF_STRING, 19, "Castle warfare");

  
  AppendMenuA(scriptsPopup, MF_STRING, 28, "Aimbot/ESP");
  AppendMenuA(scriptsPopup, MF_STRING, 20, "Kohl's admin V2");
  AppendMenuA(scriptsPopup, MF_STRING, 21, "Person299's Admin Cmds");
  AppendMenuA(scriptsPopup, MF_STRING, 26, "Phantom Speed/Jump");
  AppendMenuA(scriptsPopup, MF_STRING, MENU_RESPAWN_PLAYERS, "Respawn Players");
  AppendMenuA(scriptsPopup, MF_STRING, 25, "Dex Explorer");
  AppendMenuA(scriptsPopup, MF_STRING, MENU_FORCEFIELD, "ForceField");
  AppendMenuA(scriptsPopup, MF_STRING, 27, "Check FE");

  AppendMenuA(helpMenu, MF_STRING, MENU_RC7_WEBSITE, "RC7 Website");
  AppendMenuA(helpMenu, MF_STRING, MENU_ROBLOX_WIKI, "Roblox Wiki");
  AppendMenuA(helpMenu, MF_STRING, MENU_CHANGELOG, "Change log");
  AppendMenuA(helpMenu, MF_STRING, MENU_ENVIRONMENT, "Environment");
  AppendMenuA(helpMenu, MF_STRING, MENU_QUESTIONS, "Questions");
  AppendMenuA(helpMenu, MF_STRING, MENU_FORUMS, "Forums");

  AppendMenuA(themeMenu, MF_STRING, MENU_OPEN_THEME_DIR, "Open Directory");
  AppendMenuA(themeMenu, MF_STRING, MENU_DOWNLOAD_THEMES, "Download Themes");
  AppendMenuA(globalChatMenu, MF_STRING, MENU_GLOBAL_CHAT, "Coming Soon");

  AppendMenuA(mainMenu, MF_POPUP, reinterpret_cast<UINT_PTR>(settingsMenu), "Settings");
  AppendMenuA(mainMenu, MF_POPUP, reinterpret_cast<UINT_PTR>(commandsMenu), "Commands");
  AppendMenuA(mainMenu, MF_POPUP, reinterpret_cast<UINT_PTR>(helpMenu), "Help");
  AppendMenuA(mainMenu, MF_POPUP, reinterpret_cast<UINT_PTR>(globalChatMenu), "Global Chat");
  AppendMenuA(mainMenu, MF_POPUP, reinterpret_cast<UINT_PTR>(themeMenu), "Edit Theme");

  SetMenu(hwnd, mainMenu);
  return mainMenu;
}

void HandleMenuCommand(HWND hwnd, unsigned int commandId, bool* wordWrapEnabled, bool* autoHookEnabled) {
  HMENU menu = GetMenu(hwnd);
  switch (commandId) {
    case MENU_FONT_COLOR:
      
      break;
    case MENU_AUTO_HOOK:
      if (autoHookEnabled) {
        *autoHookEnabled = !*autoHookEnabled;
        CheckMenuItem(menu, MENU_AUTO_HOOK, MF_BYCOMMAND | (*autoHookEnabled ? MF_CHECKED : MF_UNCHECKED));
      }
      break;
    case MENU_WORD_WRAP:
      if (wordWrapEnabled) {
        *wordWrapEnabled = !*wordWrapEnabled;
        CheckMenuItem(menu, MENU_WORD_WRAP, MF_BYCOMMAND | (*wordWrapEnabled ? MF_CHECKED : MF_UNCHECKED));
      }
      break;
    case MENU_DOWNLOAD_THEMES:
      OpenUrl("https://goo.gl/ircaoJ");
      break;
    case MENU_OPEN_THEME_DIR:
      OpenUrl("C:\\RC7_THEMES\\");
      break;
    case MENU_RC7_WEBSITE:
      OpenUrl("https://crunchman.info/");
      break;
    case MENU_ROBLOX_WIKI:
      OpenUrl("http://wiki.roblox.com/?title=Scripting");
      break;
    case MENU_QUESTIONS:
      OpenUrl("https://crunchman.info/faq.html");
      break;
    case MENU_CHANGELOG:
      OpenUrl("https://crunchman.info/changelog.html");
      break;
    case MENU_ENVIRONMENT:
      OpenUrl("https://crunchman.info/api.html");
      break;
    case MENU_FORUMS:
      OpenUrl("http://rc7forums.pcriot.com");
      break;
    default:
      break;
  }
}

} 
