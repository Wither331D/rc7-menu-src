#include "config.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>
#include <string>

namespace rc7 {
namespace {

constexpr const char* kThemeConfigPath = "C:\\RC7_THEMES\\Config.txt";
constexpr const char* kLocalConfigPath = ".\\Config.txt";

std::string Trim(std::string s) {
  auto isSpace = [](unsigned char c) { return std::isspace(c) != 0; };
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), [&](char ch) { return !isSpace(static_cast<unsigned char>(ch)); }));
  s.erase(std::find_if(s.rbegin(), s.rend(), [&](char ch) { return !isSpace(static_cast<unsigned char>(ch)); }).base(), s.end());
  return s;
}

std::string ToLower(std::string s) {
  std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
  return s;
}

bool ParseBool(const std::string& value) {
  const std::string v = ToLower(Trim(value));
  return v == "1" || v == "true" || v == "yes" || v == "on";
}

bool ParseColor(const std::string& value, COLORREF* out) {
  std::stringstream ss(value);
  int r = 255;
  int g = 255;
  int b = 255;
  char comma1 = 0;
  char comma2 = 0;
  if (!(ss >> r >> comma1 >> g >> comma2 >> b)) {
    return false;
  }
  if (comma1 != ',' || comma2 != ',') {
    return false;
  }
  r = std::clamp(r, 0, 255);
  g = std::clamp(g, 0, 255);
  b = std::clamp(b, 0, 255);
  *out = RGB(r, g, b);
  return true;
}

bool LoadFromFile(const char* path, UiConfig* out) {
  std::ifstream in(path);
  if (!in.is_open()) {
    return false;
  }

  std::string line;
  while (std::getline(in, line)) {
    line = Trim(line);
    if (line.empty() || line[0] == '#' || line[0] == ';') {
      continue;
    }

    const std::size_t colon = line.find(':');
    if (colon != std::string::npos) {
      const std::string key = ToLower(Trim(line.substr(0, colon)));
      std::string value = Trim(line.substr(colon + 1));
      const std::size_t semi = value.find(';');
      if (semi != std::string::npos) {
        value = Trim(value.substr(0, semi));
      }
      if (key == "fontcolor" || key == "font_color") {
        COLORREF color = out->fontColor;
        if (ParseColor(value, &color)) {
          out->fontColor = color;
        }
      }
      continue;
    }

    const std::size_t eq = line.find('=');
    if (eq == std::string::npos) continue;

    const std::string key = ToLower(Trim(line.substr(0, eq)));
    const std::string value = Trim(line.substr(eq + 1));

    if (key == "fontcolor" || key == "font_color") {
      COLORREF color = out->fontColor;
      if (ParseColor(value, &color)) {
        out->fontColor = color;
      }
    } else if (key == "wordwrap" || key == "word_wrap") {
      out->wordWrap = ParseBool(value);
    } else if (key == "autohook" || key == "auto_hook") {
      out->autoHook = ParseBool(value);
    }
  }

  return true;
}

} 

UiConfig LoadUiConfig() {
  UiConfig cfg{};
  if (!LoadFromFile(kThemeConfigPath, &cfg)) {
    LoadFromFile(kLocalConfigPath, &cfg);
  }
  return cfg;
}

bool SaveUiConfig(const UiConfig& cfg) {
  std::ofstream out(kThemeConfigPath, std::ios::trunc);
  if (!out.is_open()) {
    out.open(kLocalConfigPath, std::ios::trunc);
    if (!out.is_open()) {
      return false;
    }
  }

  const int r = GetRValue(cfg.fontColor);
  const int g = GetGValue(cfg.fontColor);
  const int b = GetBValue(cfg.fontColor);
  out << "FontColor:" << r << "," << g << "," << b << ";\n";
  return out.good();
}

} 
