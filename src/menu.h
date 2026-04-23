#pragma once

#include <windows.h>

namespace rc7 {

HMENU BuildMainMenu(HWND hwnd);
void HandleMenuCommand(HWND hwnd, unsigned int commandId, bool* wordWrapEnabled, bool* autoHookEnabled);

} 
