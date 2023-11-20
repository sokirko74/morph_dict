#include "../contrib/rapidjson/document.h"
#include "../contrib/rapidjson/writer.h"
#include "../contrib/rapidjson/stringbuffer.h"
#include "../contrib/rapidjson/istreamwrapper.h"


inline const rapidjson::Value& GetArrayFromJson(rapidjson::Document& doc, const char* key) {
	if (doc.HasMember(key)) {
		return doc[key];
	}
	return rapidjson::Value(rapidjson::kArrayType);
}

inline const bool& GetBoolFromJson(rapidjson::Document& doc, const char* key) {
	if (doc.HasMember(key)) {
		return doc[key].GetBool();
	}
	return false;
}

inline const char* GetStringFromJson(rapidjson::Document& doc, const char* key) {
	if (doc.HasMember(key)) {
		return doc[key].GetString();
	}
	return "";
}
