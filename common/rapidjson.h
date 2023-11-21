#pragma once 

#define RAPIDJSON_HAS_STDSTRING 1
#define RAPIDJSON_HAS_CXX11_RVALUE_REFS 1 
#include "../contrib/rapidjson/document.h"
#include "../contrib/rapidjson/writer.h"
#include "../contrib/rapidjson/stringbuffer.h"
#include "../contrib/rapidjson/istreamwrapper.h"
#include "../contrib/rapidjson/ostreamwrapper.h"
#include "../contrib/rapidjson/pointer.h"
#include "../contrib/rapidjson/prettywriter.h"

#include <fstream>

class CJsonObject {
	rapidjson::Document& m_Doc;
	rapidjson::Value& m_Value;
	rapidjson::Value m_InnerValue;
public: 
	CJsonObject(rapidjson::Document& d, rapidjson::Value& v);
	CJsonObject(rapidjson::Document& d);
	rapidjson::Value& get_value() { return m_Value;}
	rapidjson::Document& get_doc() { return m_Doc; }
	rapidjson::Document::AllocatorType& get_allocator() { return m_Doc.GetAllocator(); }

	std::string dump_rapidjson() const;
	std::string dump_rapidjson_pretty() const;
	void dump_rapidjson_pretty(std::string filepath) const;
	void add_member(const char* key, const std::string& value);
	void add_member(const char* key, const char* value);
	void add_member(const char* key, rapidjson::Value& value);
	void add_member(const char* key, uint16_t v);
	void add_member(const char* key, bool v);
};

