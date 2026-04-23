#pragma once

#include <windows.h>

namespace rc7 {

struct ThemeBitmaps {
  HBITMAP mainUi = nullptr;
  HBITMAP buttonIdle = nullptr;
  HBITMAP buttonClicked = nullptr;
  HBITMAP buttonHover = nullptr;
  HBITMAP smallButtonIdle = nullptr;
  HBITMAP smallButtonClicked = nullptr;
  HBITMAP smallButtonHover = nullptr;
  HBITMAP textBox = nullptr;
  HBITMAP saveIn = nullptr;
  HBITMAP hideSide = nullptr;
  HBITMAP wordWrapIn = nullptr;
  HBITMAP autoIn = nullptr;
  HBITMAP googleDriveIn = nullptr;
  HBITMAP krystalIn = nullptr;
  HBITMAP woflyIn = nullptr;
};

struct ThemeContext {
  ThemeBitmaps bitmaps{};
  HBRUSH backgroundBrush = nullptr;
  HBRUSH textBoxBrush = nullptr;
  HBRUSH saveBrush = nullptr;
  HBRUSH hideSideBrush = nullptr;
  HBRUSH wordWrapBrush = nullptr;
  HBRUSH autoBrush = nullptr;
  HBRUSH googleBrush = nullptr;
  HBRUSH krystalBrush = nullptr;
  HBRUSH woflyBrush = nullptr;
};

ThemeContext LoadThemeContext(HINSTANCE hInstance);
void DestroyThemeContext(ThemeContext* theme);

} 
