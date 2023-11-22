#include "rapidjson.h"

CJsonObject::CJsonObject(rapidjson::Document& d, rapidjson::Type t) : 
	m_Doc(d), m_Value(t)
{

}


std::string CJsonObject::dump_rapidjson() const {
	rapidjson::StringBuffer buffer;
	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
	m_Value.Accept(writer);
	return buffer.GetString();
}

std::string CJsonObject::dump_rapidjson_pretty() const {
	rapidjson::StringBuffer buffer;
	rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
	writer.SetIndent(' ', 1);
	m_Value.Accept(writer);
	return buffer.GetString();
}

void CJsonObject::dump_rapidjson_pretty(std::string filepath) const {
	std::ofstream ofs(filepath);
	rapidjson::OStreamWrapper osw(ofs);
	rapidjson::PrettyWriter<rapidjson::OStreamWrapper> writer(osw);
	writer.SetIndent(' ', 1);
	m_Value.Accept(writer);
	ofs.close();
}

void CJsonObject::add_string(const char* key, const std::string& value) {
	m_Value.AddMember(rapidjson::StringRef(key), rapidjson::StringRef(value.c_str(), value.length()), m_Doc.GetAllocator());
}

void CJsonObject::add_string_copy(const char* key, const std::string& value) {
	m_Value.AddMember(rapidjson::StringRef(key), value, m_Doc.GetAllocator());
}

void CJsonObject::add_string(const char* key, const char* value) {
	m_Value.AddMember(rapidjson::StringRef(key), rapidjson::StringRef(value), m_Doc.GetAllocator());
}

void CJsonObject::add_member(const char* key, rapidjson::Value& value) {
	m_Value.AddMember(rapidjson::StringRef(key), value.Move(), m_Doc.GetAllocator());
}

void CJsonObject::add_int(const char* key, uint32_t v) {
	m_Value.AddMember(rapidjson::StringRef(key), v, m_Doc.GetAllocator());
}

void CJsonObject::add_double(const char* key, double v) {
	m_Value.AddMember(rapidjson::StringRef(key), v, m_Doc.GetAllocator());
}

void CJsonObject::add_bool(const char* key, bool v) {
	rapidjson::Value v1;
	v1.SetBool(v);
	m_Value.AddMember(rapidjson::StringRef(key), v, m_Doc.GetAllocator());

}

void CJsonObject::push_back(rapidjson::Value& value) {
	m_Value.PushBack(value.Move(), m_Doc.GetAllocator());
}
