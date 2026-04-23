#pragma once

#include <windows.h>

namespace rc7 {

struct UiConfig {
  COLORREF fontColor = RGB(255, 255, 255);
  bool wordWrap = false;
  bool autoHook = false;
};

UiConfig LoadUiConfig();
bool SaveUiConfig(const UiConfig& cfg);

} 
