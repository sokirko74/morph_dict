#include "json.h"


template <typename C>
void ConvertToUtfRecursiveT(C& r, MorphLanguageEnum langua) {
	if (r.is_array()) {
		for (auto& a : r) {
			ConvertToUtfRecursive(a, langua);
		}
	}
	else if (r.is_object()) {
		for (auto& a : r.items()) {
			ConvertToUtfRecursive(a.value(), langua);
		}

	}
	else if (r.is_string()) {
		r = convert_to_utf8(r, langua);
	}
}

nlohmann::ordered_json& ConvertToUtfRecursive(nlohmann::ordered_json& r, MorphLanguageEnum langua) {
	ConvertToUtfRecursiveT<nlohmann::ordered_json>(r, langua);
	return r;
}

nlohmann::json& ConvertToUtfRecursive(nlohmann::json& r, MorphLanguageEnum langua) {
	ConvertToUtfRecursiveT<nlohmann::json>(r, langua);
	return r;
}



template <typename C>
void ConvertFromUtfRecursiveT(C& r, MorphLanguageEnum langua) {
	if (r.is_array()) {
		for (auto& a : r) {
			ConvertFromUtfRecursiveT(a, langua);
		}
	}
	else if (r.is_object()) {
		for (auto& a : r.items()) {
			ConvertFromUtfRecursiveT(a.value(), langua);
		}

	}
	else if (r.is_string()) {
		r = convert_from_utf8(r.template get<std::string>().c_str(), langua);
	}
}

nlohmann::ordered_json& ConvertFromUtfRecursive(nlohmann::ordered_json& r, MorphLanguageEnum langua) {
	ConvertFromUtfRecursiveT<nlohmann::ordered_json>(r, langua);
	return r;
}

nlohmann::json& ConvertFromUtfRecursive(nlohmann::json& r, MorphLanguageEnum langua) {
	ConvertFromUtfRecursiveT<nlohmann::json>(r, langua);
	return r;
}
