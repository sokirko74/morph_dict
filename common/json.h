#pragma once
#include "../contrib/nlohmann/json.hpp"
#include "utilit.h"

extern nlohmann::json& ConvertToUtfRecursive(nlohmann::json& r, MorphLanguageEnum langua);
extern nlohmann::ordered_json& ConvertToUtfRecursive(nlohmann::ordered_json& r, MorphLanguageEnum langua);

extern nlohmann::json& ConvertFromUtfRecursive(nlohmann::json& r, MorphLanguageEnum langua);
extern nlohmann::ordered_json& ConvertFromUtfRecursive(nlohmann::ordered_json& r, MorphLanguageEnum langua);
