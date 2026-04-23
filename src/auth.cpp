#include "auth.h"

#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>

#include <cstring>
#include <sstream>

#pragma comment(lib, "ws2_32.lib")

namespace rc7 {

static std::string UrlEncodeSimple(const std::string& s) {
  std::ostringstream out;
  for (unsigned char c : s) {
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '-' || c == '_' || c == '.' || c == '~') {
      out << static_cast<char>(c);
    } else if (c == ' ') {
      out << '+';
    } else {
      const char* hex = "0123456789ABCDEF";
      out << '%';
      out << hex[(c >> 4) & 0xF];
      out << hex[c & 0xF];
    }
  }
  return out.str();
}

bool SaveCredentialsToRegistry(const Credentials& creds) {
  if (creds.username.size() <= 1 || creds.password.size() <= 1) {
    return false;
  }

  HKEY key = nullptr;
  LONG open = RegCreateKeyExA(HKEY_CURRENT_USER, "Software\\RC7", 0, nullptr, 0, KEY_ALL_ACCESS, nullptr, &key, nullptr);
  if (open != ERROR_SUCCESS || key == nullptr) {
    return false;
  }

  RegSetValueExA(key, "Username", 0, REG_SZ, reinterpret_cast<const BYTE*>(creds.username.c_str()), static_cast<DWORD>(creds.username.size()));
  RegSetValueExA(key, "Password", 0, REG_SZ, reinterpret_cast<const BYTE*>(creds.password.c_str()), static_cast<DWORD>(creds.password.size()));
  RegCloseKey(key);
  return true;
}

bool AuthenticateOriginal(const Credentials& creds, std::string* outVersion, std::string* outError) {
  if (outVersion) outVersion->clear();
  if (outError) outError->clear();

  WSADATA wsa{};
  if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
    if (outError) *outError = "WSAStartup failed";
    return false;
  }

  const char* hostName = "pob.pcriot.com";
  const char* hostIp = "107.180.58.50"; 
  const int port = 80;

  std::string body = "username=" + UrlEncodeSimple(creds.username) + "&password=" + UrlEncodeSimple(creds.password);
  std::ostringstream req;
  req << "POST /wl.php HTTP/1.1\r\n";
  req << "HOST: " << hostName << "\r\n";
  req << "Cache-Control: no-cache\r\n";
  req << "Content-Length: " << body.size() << "\r\n";
  req << "Content-Type: application/x-www-form-urlencoded\r\n\r\n";
  req << body;
  std::string packet = req.str();

  SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (s == INVALID_SOCKET) {
    if (outError) *outError = "socket failed";
    WSACleanup();
    return false;
  }

  sockaddr_in addr{};
  addr.sin_family = AF_INET;
  addr.sin_port = htons(static_cast<u_short>(port));
  addr.sin_addr.s_addr = inet_addr(hostIp);

  if (connect(s, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
    closesocket(s);
    WSACleanup();
    if (outError) *outError = "connect failed";
    return false;
  }

  if (send(s, packet.c_str(), static_cast<int>(packet.size()), 0) != static_cast<int>(packet.size())) {
    closesocket(s);
    WSACleanup();
    if (outError) *outError = "send failed";
    return false;
  }

  shutdown(s, SD_SEND);
  std::string response;
  char chunk[512]{};
  for (;;) {
    int r = recv(s, chunk, static_cast<int>(sizeof(chunk)), 0);
    if (r <= 0) break;
    response.append(chunk, chunk + r);
  }

  closesocket(s);
  WSACleanup();

  if (response.find("This user") != std::string::npos) {
    if (outError) *outError = "This user is already logged in";
    return false;
  }
  if (response.find("Failed to authenticate this user") != std::string::npos) {
    if (outError) *outError = "Your username or password is incorrect";
    return false;
  }

  std::size_t p = response.find("Version");
  if (p == std::string::npos) {
    if (outError) {
      *outError = "Authentication failed: host blocked/unavailable";
    }
    return false;
  }

  if (outVersion) {
    std::size_t start = p + 8;
    std::size_t end = start;
    while (end < response.size() && (response[end] == '.' || (response[end] >= '0' && response[end] <= '9'))) {
      ++end;
    }
    *outVersion = response.substr(start, end - start);
  }
  return true;
}

bool AuthenticateCrackedBypass() {
  return true;
}

} 
