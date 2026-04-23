#pragma once

#include <string>

namespace rc7 {

struct Credentials {
  std::string username;
  std::string password;
};


bool SaveCredentialsToRegistry(const Credentials& creds);



bool AuthenticateOriginal(const Credentials& creds, std::string* outVersion, std::string* outError);


bool AuthenticateCrackedBypass();

} 
