#pragma once

namespace rc7 {


enum MenuId : unsigned int {
  MENU_RESPAWN_PLAYERS = 1,
  MENU_DOWNLOAD_THEMES = 2,
  MENU_SCRIPT_GUEST = 3,
  MENU_WORD_WRAP = 4,
  MENU_FORUMS = 5,
  MENU_FONT_COLOR = 6,
  MENU_AUTO_RUN = 7,
  MENU_RESTART = 8,
  MENU_ROBLOX_WIKI = 9,
  MENU_GLOBAL_CHAT = 10,
  MENU_RC7_WEBSITE = 11,
  MENU_OPEN_THEME_DIR = 12,
  MENU_QUESTIONS = 13,
  MENU_CHANGELOG = 14,
  MENU_ENVIRONMENT = 15,
  MENU_FORCEFIELD = 16,
  MENU_CODE_EDITOR = 22,
  MENU_CLOSE_ROBLOX = 23,
  MENU_AUTO_HOOK = 24,

  
  MENU_CLOSE_TAB = 29,
};


constexpr int CTRL_SUBMIT = 599;
constexpr int CTRL_EXECUTE = 420;
constexpr int CTRL_OPEN = 424;
constexpr int CTRL_CLEAR = 434;
constexpr int CTRL_SAVE = 610;
constexpr int CTRL_HIDE_SIDE = 611;
constexpr int CTRL_WORDWRAP = 612;
constexpr int CTRL_AUTOHOOK = 613;
constexpr int CTRL_GOOGLE_DRIVE = 614;
constexpr int CTRL_KRYSTAL = 615;
constexpr int CTRL_WOFLY = 616;


constexpr int BMP_SAVE_IN = 0x7A;
constexpr int BMP_WORDWRAP_IN = 0x7B;
constexpr int BMP_AUTO_IN = 0x7C;
constexpr int BMP_GOOGLE_DRIVE_IN = 0x7D;
constexpr int BMP_WOFLY_IN = 0x7E;
constexpr int BMP_BUTTON_HOVER = 0x7F;
constexpr int BMP_BUTTON_CLICKED = 0x80;
constexpr int BMP_MAIN_UI = 0x82;
constexpr int BMP_BUTTON_IDLE = 0x83;
constexpr int BMP_TEXTBOX = 0x84;
constexpr int BMP_KRYSTAL_IN = 0x85;
constexpr int BMP_S_BUTTON_HOVER = 0x88;
constexpr int BMP_S_BUTTON_CLICKED = 0x89;
constexpr int BMP_S_BUTTON_IDLE = 0x8A;
constexpr int BMP_HIDE_SIDE = 0x8B;

} 
