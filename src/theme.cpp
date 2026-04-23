#include "theme.h"

#include "resource_ids.h"

namespace rc7 {
namespace {

HBITMAP LoadThemeBitmapOrResource(HINSTANCE hInstance, const char* filePath, int width, int height, int resId) {
  HBITMAP bmp = static_cast<HBITMAP>(LoadImageA(hInstance, filePath, IMAGE_BITMAP, width, height, LR_LOADFROMFILE));
  if (!bmp) {
    bmp = LoadBitmapA(hInstance, MAKEINTRESOURCEA(resId));
  }
  return bmp;
}

void DestroyBitmap(HBITMAP* bmp) {
  if (*bmp) {
    DeleteObject(*bmp);
    *bmp = nullptr;
  }
}

} 

ThemeContext LoadThemeContext(HINSTANCE hInstance) {
  ThemeContext theme{};
  ThemeBitmaps& bmp = theme.bitmaps;

  bmp.mainUi = LoadThemeBitmapOrResource(hInstance, "C:\\RC7_THEMES\\MainUi.bmp", 339, 328, BMP_MAIN_UI);
  bmp.buttonIdle = LoadThemeBitmapOrResource(hInstance, "C:\\RC7_THEMES\\Button_Idle.bmp", 100, 23, BMP_BUTTON_IDLE);
  bmp.buttonClicked = LoadThemeBitmapOrResource(hInstance, "C:\\RC7_THEMES\\Button_Clicked.bmp", 100, 23, BMP_BUTTON_CLICKED);
  bmp.buttonHover = LoadThemeBitmapOrResource(hInstance, "C:\\RC7_THEMES\\Button_Hover.bmp", 100, 23, BMP_BUTTON_HOVER);

  bmp.smallButtonIdle = LoadThemeBitmapOrResource(hInstance, "C:\\RC7_THEMES\\S_Button_Idle.bmp", 95, 25, BMP_S_BUTTON_IDLE);
  bmp.smallButtonClicked = LoadThemeBitmapOrResource(hInstance, "C:\\RC7_THEMES\\S_Button_Clicked.bmp", 95, 25, BMP_S_BUTTON_CLICKED);
  bmp.smallButtonHover = LoadThemeBitmapOrResource(hInstance, "C:\\RC7_THEMES\\S_Button_Hover.bmp", 95, 25, BMP_S_BUTTON_HOVER);

  bmp.textBox = LoadThemeBitmapOrResource(hInstance, "C:\\RC7_THEMES\\Textbox.bmp", 156, 24, BMP_TEXTBOX);
  if (!bmp.textBox) {
    bmp.textBox = LoadThemeBitmapOrResource(hInstance, "C:\\RC7_THEMES\\TextBox.bmp", 156, 24, BMP_TEXTBOX);
  }

  bmp.saveIn = LoadThemeBitmapOrResource(hInstance, "C:\\RC7_THEMES\\Save_In.bmp", 30, 30, BMP_SAVE_IN);
  bmp.hideSide = LoadThemeBitmapOrResource(hInstance, "C:\\RC7_THEMES\\Hide_Side.bmp", 39, 328, BMP_HIDE_SIDE);
  bmp.wordWrapIn = LoadThemeBitmapOrResource(hInstance, "C:\\RC7_THEMES\\WordWrap_In.bmp", 30, 30, BMP_WORDWRAP_IN);
  bmp.autoIn = LoadThemeBitmapOrResource(hInstance, "C:\\RC7_THEMES\\Auto_In.bmp", 30, 30, BMP_AUTO_IN);
  bmp.googleDriveIn = LoadThemeBitmapOrResource(hInstance, "C:\\RC7_THEMES\\Google_Drive_In.bmp", 30, 30, BMP_GOOGLE_DRIVE_IN);
  bmp.krystalIn = LoadThemeBitmapOrResource(hInstance, "C:\\RC7_THEMES\\Krystal_In.bmp", 30, 30, BMP_KRYSTAL_IN);
  bmp.woflyIn = LoadThemeBitmapOrResource(hInstance, "C:\\RC7_THEMES\\Wofly_In.bmp", 30, 30, BMP_WOFLY_IN);

  if (!bmp.woflyIn) {
    bmp.woflyIn = static_cast<HBITMAP>(LoadImageA(hInstance, "C:\\RC7_THEMES\\Wolfy_In.bmp", IMAGE_BITMAP, 30, 30, LR_LOADFROMFILE));
    if (!bmp.woflyIn) {
      bmp.woflyIn = LoadBitmapA(hInstance, MAKEINTRESOURCEA(BMP_WOFLY_IN));
    }
  }

  if (bmp.mainUi) {
    theme.backgroundBrush = CreatePatternBrush(bmp.mainUi);
  }
  if (bmp.textBox) {
    theme.textBoxBrush = CreatePatternBrush(bmp.textBox);
  }
  if (bmp.saveIn) {
    theme.saveBrush = CreatePatternBrush(bmp.saveIn);
  }
  if (bmp.hideSide) {
    theme.hideSideBrush = CreatePatternBrush(bmp.hideSide);
  }
  if (bmp.wordWrapIn) {
    theme.wordWrapBrush = CreatePatternBrush(bmp.wordWrapIn);
  }
  if (bmp.autoIn) {
    theme.autoBrush = CreatePatternBrush(bmp.autoIn);
  }
  if (bmp.googleDriveIn) {
    theme.googleBrush = CreatePatternBrush(bmp.googleDriveIn);
  }
  if (bmp.krystalIn) {
    theme.krystalBrush = CreatePatternBrush(bmp.krystalIn);
  }
  if (bmp.woflyIn) {
    theme.woflyBrush = CreatePatternBrush(bmp.woflyIn);
  }

  return theme;
}

void DestroyThemeContext(ThemeContext* theme) {
  if (!theme) {
    return;
  }

  if (theme->backgroundBrush) {
    DeleteObject(theme->backgroundBrush);
    theme->backgroundBrush = nullptr;
  }
  if (theme->textBoxBrush) {
    DeleteObject(theme->textBoxBrush);
    theme->textBoxBrush = nullptr;
  }
  if (theme->saveBrush) {
    DeleteObject(theme->saveBrush);
    theme->saveBrush = nullptr;
  }
  if (theme->hideSideBrush) {
    DeleteObject(theme->hideSideBrush);
    theme->hideSideBrush = nullptr;
  }
  if (theme->wordWrapBrush) {
    DeleteObject(theme->wordWrapBrush);
    theme->wordWrapBrush = nullptr;
  }
  if (theme->autoBrush) {
    DeleteObject(theme->autoBrush);
    theme->autoBrush = nullptr;
  }
  if (theme->googleBrush) {
    DeleteObject(theme->googleBrush);
    theme->googleBrush = nullptr;
  }
  if (theme->krystalBrush) {
    DeleteObject(theme->krystalBrush);
    theme->krystalBrush = nullptr;
  }
  if (theme->woflyBrush) {
    DeleteObject(theme->woflyBrush);
    theme->woflyBrush = nullptr;
  }

  ThemeBitmaps& bmp = theme->bitmaps;
  DestroyBitmap(&bmp.mainUi);
  DestroyBitmap(&bmp.buttonIdle);
  DestroyBitmap(&bmp.buttonClicked);
  DestroyBitmap(&bmp.buttonHover);
  DestroyBitmap(&bmp.smallButtonIdle);
  DestroyBitmap(&bmp.smallButtonClicked);
  DestroyBitmap(&bmp.smallButtonHover);
  DestroyBitmap(&bmp.textBox);
  DestroyBitmap(&bmp.saveIn);
  DestroyBitmap(&bmp.hideSide);
  DestroyBitmap(&bmp.wordWrapIn);
  DestroyBitmap(&bmp.autoIn);
  DestroyBitmap(&bmp.googleDriveIn);
  DestroyBitmap(&bmp.krystalIn);
  DestroyBitmap(&bmp.woflyIn);
}

} 
