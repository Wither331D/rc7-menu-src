#include <windows.h>

#include <algorithm>
#include <array>
#include <cstring>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace {

struct ResourceKey {
  bool isId = false;
  uint16_t id = 0;
  std::wstring name;
};

struct ResourceType {
  ResourceKey key;
  std::wstring label;
};

struct ResourceName {
  ResourceType type;
  ResourceKey key;
};

struct ResourceEntry {
  ResourceName name;
  uint16_t lang = 0;
};

std::wstring ToW(const char* s) {
  if (!s) return L"";
  const int len = MultiByteToWideChar(CP_UTF8, 0, s, -1, nullptr, 0);
  if (len <= 0) return L"";
  std::wstring out(static_cast<size_t>(len - 1), L'\0');
  MultiByteToWideChar(CP_UTF8, 0, s, -1, out.data(), len);
  return out;
}

std::string ToUtf8(const std::wstring& s) {
  if (s.empty()) return {};
  const int len = WideCharToMultiByte(CP_UTF8, 0, s.c_str(), -1, nullptr, 0, nullptr, nullptr);
  if (len <= 0) return {};
  std::string out(static_cast<size_t>(len - 1), '\0');
  WideCharToMultiByte(CP_UTF8, 0, s.c_str(), -1, out.data(), len, nullptr, nullptr);
  return out;
}

bool IsIntRes(LPCWSTR p) {
  return (reinterpret_cast<uintptr_t>(p) >> 16) == 0;
}

ResourceKey MakeKey(LPCWSTR p) {
  ResourceKey k{};
  if (IsIntRes(p)) {
    k.isId = true;
    k.id = static_cast<uint16_t>(reinterpret_cast<uintptr_t>(p));
  } else {
    k.name = p;
  }
  return k;
}

LPCWSTR KeyAsPtr(const ResourceKey& k, std::wstring* tmpStorage) {
  if (k.isId) {
    return MAKEINTRESOURCEW(k.id);
  }
  *tmpStorage = k.name;
  return tmpStorage->c_str();
}

std::wstring KeyString(const ResourceKey& k) {
  if (k.isId) {
    return std::to_wstring(k.id);
  }
  return k.name;
}

std::wstring SafeName(const std::wstring& s) {
  std::wstring out;
  out.reserve(s.size());
  for (wchar_t c : s) {
    const bool ok = (c >= L'0' && c <= L'9') || (c >= L'A' && c <= L'Z') || (c >= L'a' && c <= L'z') || c == L'_' || c == L'-' || c == L'.';
    out.push_back(ok ? c : L'_');
  }
  if (out.empty()) return L"unnamed";
  return out;
}

const std::map<std::wstring, std::wstring> kTypeMap = {
    {L"1", L"RT_CURSOR"},
    {L"2", L"RT_BITMAP"},
    {L"3", L"RT_ICON"},
    {L"4", L"RT_MENU"},
    {L"5", L"RT_DIALOG"},
    {L"6", L"RT_STRING"},
    {L"7", L"RT_FONTDIR"},
    {L"8", L"RT_FONT"},
    {L"9", L"RT_ACCELERATOR"},
    {L"10", L"RT_RCDATA"},
    {L"11", L"RT_MESSAGETABLE"},
    {L"12", L"RT_GROUP_CURSOR"},
    {L"14", L"RT_GROUP_ICON"},
    {L"16", L"RT_VERSION"},
    {L"24", L"RT_MANIFEST"},
    {L"WAVE", L"TYPE_WAVE"},
};

std::wstring TypeLabel(const ResourceKey& key) {
  const std::wstring raw = KeyString(key);
  const auto it = kTypeMap.find(raw);
  if (it != kTypeMap.end()) return it->second;
  if (key.isId) return L"TYPE_" + raw;
  return L"TYPE_" + SafeName(raw);
}

std::vector<uint8_t> GetResourceBytes(HMODULE mod, const ResourceEntry& e) {
  std::wstring tStore;
  std::wstring nStore;
  const LPCWSTR typePtr = KeyAsPtr(e.name.type.key, &tStore);
  const LPCWSTR namePtr = KeyAsPtr(e.name.key, &nStore);
  HRSRC res = FindResourceExW(mod, typePtr, namePtr, e.lang);
  if (!res) return {};
  const DWORD size = SizeofResource(mod, res);
  if (size == 0) return {};
  HGLOBAL loaded = LoadResource(mod, res);
  if (!loaded) return {};
  void* ptr = LockResource(loaded);
  if (!ptr) return {};
  std::vector<uint8_t> out(size);
  std::memcpy(out.data(), ptr, size);
  return out;
}

std::vector<uint8_t> DibToBmp(const std::vector<uint8_t>& dib) {
  if (dib.size() < 40) return dib;
  const auto rd_u16 = [&](size_t off) -> uint16_t { return *reinterpret_cast<const uint16_t*>(dib.data() + off); };
  const auto rd_u32 = [&](size_t off) -> uint32_t { return *reinterpret_cast<const uint32_t*>(dib.data() + off); };
  const uint32_t biSize = rd_u32(0);
  const uint16_t biBitCount = rd_u16(14);
  const uint32_t biClrUsed = rd_u32(32);
  uint32_t paletteEntries = 0;
  if (biClrUsed > 0) paletteEntries = biClrUsed;
  else if (biBitCount <= 8) paletteEntries = 1u << biBitCount;
  const uint32_t paletteBytes = paletteEntries * 4u;
  const uint32_t bfOffBits = 14u + biSize + paletteBytes;
  const uint32_t bfSize = static_cast<uint32_t>(dib.size()) + 14u;

  std::vector<uint8_t> out;
  out.resize(14 + dib.size());
  out[0] = 'B';
  out[1] = 'M';
  *reinterpret_cast<uint32_t*>(&out[2]) = bfSize;
  *reinterpret_cast<uint32_t*>(&out[6]) = 0;
  *reinterpret_cast<uint32_t*>(&out[10]) = bfOffBits;
  std::memcpy(out.data() + 14, dib.data(), dib.size());
  return out;
}

bool BuildIco(HMODULE mod, const ResourceEntry& groupEntry, const fs::path& outIcoPath) {
  const std::vector<uint8_t> grp = GetResourceBytes(mod, groupEntry);
  if (grp.size() < 6) return false;
  const auto u16 = [&](size_t off) -> uint16_t { return *reinterpret_cast<const uint16_t*>(grp.data() + off); };
  const auto u32 = [&](size_t off) -> uint32_t { return *reinterpret_cast<const uint32_t*>(grp.data() + off); };
  if (u16(0) != 0 || u16(2) != 1) return false;
  const uint16_t count = u16(4);
  if (count == 0 || grp.size() < 6 + static_cast<size_t>(count) * 14) return false;

  std::vector<std::vector<uint8_t>> images;
  images.reserve(count);
  std::vector<uint8_t> header(6 + static_cast<size_t>(count) * 16, 0);
  *reinterpret_cast<uint16_t*>(&header[0]) = 0;
  *reinterpret_cast<uint16_t*>(&header[2]) = 1;
  *reinterpret_cast<uint16_t*>(&header[4]) = count;
  uint32_t offset = static_cast<uint32_t>(header.size());

  for (uint16_t i = 0; i < count; ++i) {
    const size_t base = 6 + static_cast<size_t>(i) * 14;
    const uint16_t iconId = u16(base + 12);
    ResourceEntry icon{};
    icon.name.type = groupEntry.name.type;
    icon.name.type.key.isId = true;
    icon.name.type.key.id = 3;
    icon.name.type.label = L"RT_ICON";
    icon.name.key.isId = true;
    icon.name.key.id = iconId;
    icon.lang = groupEntry.lang;
    std::vector<uint8_t> image = GetResourceBytes(mod, icon);
    if (image.empty()) return false;
    images.push_back(std::move(image));

    const size_t e = 6 + static_cast<size_t>(i) * 16;
    header[e + 0] = grp[base + 0];
    header[e + 1] = grp[base + 1];
    header[e + 2] = grp[base + 2];
    header[e + 3] = grp[base + 3];
    *reinterpret_cast<uint16_t*>(&header[e + 4]) = u16(base + 4);
    *reinterpret_cast<uint16_t*>(&header[e + 6]) = u16(base + 6);
    *reinterpret_cast<uint32_t*>(&header[e + 8]) = static_cast<uint32_t>(images.back().size());
    *reinterpret_cast<uint32_t*>(&header[e + 12]) = offset;
    offset += static_cast<uint32_t>(images.back().size());
  }

  std::vector<uint8_t> ico = header;
  for (const auto& img : images) {
    ico.insert(ico.end(), img.begin(), img.end());
  }
  std::ofstream f(outIcoPath, std::ios::binary);
  if (!f) return false;
  f.write(reinterpret_cast<const char*>(ico.data()), static_cast<std::streamsize>(ico.size()));
  return static_cast<bool>(f);
}

std::wstring GetExt(const ResourceEntry& e) {
  const std::wstring t = KeyString(e.name.type.key);
  if (t == L"2") return L".bmp";
  if (t == L"24") return L".manifest";
  if (t == L"23") return L".html";
  if (t == L"WAVE" || e.name.type.label.find(L"WAVE") != std::wstring::npos) return L".wav";
  return L".bin";
}

bool WriteBinary(const fs::path& p, const std::vector<uint8_t>& bytes) {
  std::ofstream f(p, std::ios::binary);
  if (!f) return false;
  f.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
  return static_cast<bool>(f);
}

bool ReadBinary(const fs::path& p, std::vector<uint8_t>* out) {
  std::ifstream f(p, std::ios::binary);
  if (!f) return false;
  f.seekg(0, std::ios::end);
  const auto n = f.tellg();
  if (n < 0) return false;
  f.seekg(0, std::ios::beg);
  out->resize(static_cast<size_t>(n));
  if (!out->empty()) {
    f.read(reinterpret_cast<char*>(out->data()), static_cast<std::streamsize>(out->size()));
  }
  return static_cast<bool>(f);
}

void WriteStringTable(const std::vector<uint8_t>& bytes, int blockId, const fs::path& outPath) {
  std::ofstream out(outPath, std::ios::binary);
  size_t off = 0;
  for (int i = 0; i < 16 && off + 2 <= bytes.size(); ++i) {
    uint16_t chars = *reinterpret_cast<const uint16_t*>(bytes.data() + off);
    off += 2;
    const int strId = (blockId - 1) * 16 + i;
    out << std::setw(4) << std::setfill('0') << strId << ": ";
    if (chars == 0) {
      out << "<empty>\n";
      continue;
    }
    const size_t byteCount = static_cast<size_t>(chars) * 2;
    if (off + byteCount > bytes.size()) break;
    std::wstring s(reinterpret_cast<const wchar_t*>(bytes.data() + off), chars);
    off += byteCount;
    out << ToUtf8(s) << "\n";
  }
}

BOOL CALLBACK EnumLangCb(HMODULE, LPCWSTR, LPCWSTR, WORD wIDLanguage, LONG_PTR lParam) {
  auto* vec = reinterpret_cast<std::vector<uint16_t>*>(lParam);
  vec->push_back(wIDLanguage);
  return TRUE;
}

BOOL CALLBACK EnumNameCb(HMODULE mod, LPCWSTR type, LPWSTR name, LONG_PTR lParam) {
  auto* out = reinterpret_cast<std::vector<ResourceName>*>(lParam);
  ResourceName n{};
  n.type.key = MakeKey(type);
  n.type.label = TypeLabel(n.type.key);
  n.key = MakeKey(name);
  out->push_back(std::move(n));
  return TRUE;
}

BOOL CALLBACK EnumTypeCb(HMODULE, LPWSTR type, LONG_PTR lParam) {
  auto* out = reinterpret_cast<std::vector<ResourceType>*>(lParam);
  ResourceType t{};
  t.key = MakeKey(type);
  t.label = TypeLabel(t.key);
  out->push_back(std::move(t));
  return TRUE;
}

void CopyNamedRc7Bitmaps(const fs::path& outDir) {
  const fs::path namedDir = outDir / "named";
  fs::create_directories(namedDir);
  const std::array<std::pair<int, const wchar_t*>, 17> idName = {{
      {130, L"MainUi.bmp"},
      {131, L"Button_Idle.bmp"},
      {128, L"Button_Clicked.bmp"},
      {127, L"Button_Hover.bmp"},
      {138, L"S_Button_Idle.bmp"},
      {137, L"S_Button_Clicked.bmp"},
      {136, L"S_Button_Hover.bmp"},
      {132, L"TextBox.bmp"},
      {132, L"Textbox.bmp"},
      {122, L"Save_In.bmp"},
      {139, L"Hide_Side.bmp"},
      {123, L"WordWrap_In.bmp"},
      {124, L"Auto_In.bmp"},
      {125, L"Google_Drive_In.bmp"},
      {133, L"Krystal_In.bmp"},
      {126, L"Wofly_In.bmp"},
      {126, L"Wolfy_In.bmp"},
  }};

  for (const auto& [id, fileName] : idName) {
    std::wstringstream srcName;
    srcName << L"RT_BITMAP__" << id << L"__lang1033.bmp";
    const fs::path src = outDir / srcName.str();
    if (fs::exists(src)) {
      std::error_code ec;
      fs::copy_file(src, namedDir / fileName, fs::copy_options::overwrite_existing, ec);
    }
  }
}

}  

int wmain(int argc, wchar_t** argv) {
  const fs::path exePath = (argc >= 2) ? fs::path(argv[1]) : fs::path(L"..\\rc7_cracked.exe");
  const fs::path outDir = (argc >= 3) ? fs::path(argv[2]) : fs::path(L"..\\rc7_src\\assets\\full_dump_cpp");

  HMODULE mod = LoadLibraryExW(exePath.c_str(), nullptr, LOAD_LIBRARY_AS_DATAFILE);
  if (!mod) {
    std::wcerr << L"LoadLibraryExW failed: " << exePath << L"\n";
    return 1;
  }

  std::error_code ec;
  fs::remove_all(outDir, ec);
  fs::create_directories(outDir, ec);
  fs::create_directories(outDir / "icons_built", ec);

  std::vector<ResourceType> types;
  EnumResourceTypesW(mod, EnumTypeCb, reinterpret_cast<LONG_PTR>(&types));
  std::sort(types.begin(), types.end(), [](const ResourceType& a, const ResourceType& b) {
    const std::wstring ak = KeyString(a.key);
    const std::wstring bk = KeyString(b.key);
    return ak < bk;
  });

  std::ofstream manifest(outDir / "manifest.txt", std::ios::binary);
  manifest << "Source: " << ToUtf8(exePath.wstring()) << "\n\n";

  for (const auto& t : types) {
    std::wstring tStore;
    const LPCWSTR tPtr = KeyAsPtr(t.key, &tStore);
    std::vector<ResourceName> names;
    EnumResourceNamesW(mod, tPtr, EnumNameCb, reinterpret_cast<LONG_PTR>(&names));
    std::sort(names.begin(), names.end(), [](const ResourceName& a, const ResourceName& b) {
      return KeyString(a.key) < KeyString(b.key);
    });

    manifest << "=== " << ToUtf8(t.label) << " (" << ToUtf8(KeyString(t.key)) << ") ===\n";
    for (const auto& n : names) {
      std::wstring nStore;
      const LPCWSTR nPtr = KeyAsPtr(n.key, &nStore);
      std::vector<uint16_t> langs;
      EnumResourceLanguagesW(mod, tPtr, nPtr, EnumLangCb, reinterpret_cast<LONG_PTR>(&langs));
      std::sort(langs.begin(), langs.end());
      langs.erase(std::unique(langs.begin(), langs.end()), langs.end());

      for (uint16_t lang : langs) {
        ResourceEntry e{};
        e.name = n;
        e.lang = lang;
        std::vector<uint8_t> bytes = GetResourceBytes(mod, e);
        if (bytes.empty()) continue;

        std::wstringstream base;
        base << SafeName(t.label) << L"__" << SafeName(KeyString(n.key)) << L"__lang" << lang;
        fs::path outPath = outDir / (base.str() + GetExt(e));

        if (KeyString(e.name.type.key) == L"2") {
          bytes = DibToBmp(bytes);
          outPath = outDir / (base.str() + L".bmp");
        } else if (KeyString(e.name.type.key) == L"6" && n.key.isId) {
          outPath = outDir / (base.str() + L".txt");
          WriteStringTable(bytes, n.key.id, outPath);
        }

        if (!WriteBinary(outPath, bytes)) continue;
        manifest << "name=" << ToUtf8(KeyString(n.key)) << " lang=" << lang << " size=" << bytes.size()
                 << " -> " << ToUtf8(outPath.filename().wstring()) << "\n";

        if (KeyString(e.name.type.key) == L"14") {
          const fs::path icoPath = outDir / "icons_built" / (L"group_" + SafeName(KeyString(n.key)) + L"_lang" + std::to_wstring(lang) + L".ico");
          if (BuildIco(mod, e, icoPath)) {
            manifest << "  + built ico: " << ToUtf8(icoPath.filename().wstring()) << "\n";
          }
        }
      }
    }
    manifest << "\n";
  }

  CopyNamedRc7Bitmaps(outDir);
  FreeLibrary(mod);
  std::wcout << L"Done: " << outDir << L"\n";
  return 0;
}
