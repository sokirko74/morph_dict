#include "rapidjson.h"

CJsonObject::CJsonObject(rapidjson::Document& d, rapidjson::Value& v) : m_Doc(d), m_Value(v) 
{

}

CJsonObject::CJsonObject(rapidjson::Document& d) : m_Doc(d), m_InnerValue(rapidjson::kObjectType), m_Value(m_InnerValue)
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

void CJsonObject::add_member(const char* key, const std::string& value) {
	m_Value.AddMember(rapidjson::StringRef(key), rapidjson::StringRef(value.c_str(), value.length()), m_Doc.GetAllocator());
}


void CJsonObject::add_member(const char* key, const char* value) {
	m_Value.AddMember(rapidjson::StringRef(key), rapidjson::StringRef(value), m_Doc.GetAllocator());
}

void CJsonObject::add_member(const char* key, rapidjson::Value& value) {
	m_Value.AddMember(rapidjson::StringRef(key), value.Move(), m_Doc.GetAllocator());
}

void CJsonObject::add_member(const char* key, uint16_t v) {
	m_Value.AddMember(rapidjson::StringRef(key), v, m_Doc.GetAllocator());
}

void CJsonObject::add_member(const char* key, bool v) {
	rapidjson::Value v1;
	v1.SetBool(v);
	m_Value.AddMember(rapidjson::StringRef(key), v, m_Doc.GetAllocator());

}