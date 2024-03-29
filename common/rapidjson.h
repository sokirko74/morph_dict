﻿#pragma once 

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
#include <vector>

class CJsonObject {
	rapidjson::Document& m_Doc;
	rapidjson::Value m_Value;
public: 
	CJsonObject(rapidjson::Document& d, rapidjson::Type t=rapidjson::kObjectType);
	CJsonObject(rapidjson::Value& v, rapidjson::Document& d);
	rapidjson::Value& get_value() { return m_Value;}
	rapidjson::Document& get_doc() { return m_Doc; }
	rapidjson::Document::AllocatorType& get_allocator() { return m_Doc.GetAllocator(); }

	void add_string(const char* key, const std::string& value);
	void add_string(const char* key, const char* value);
	void add_string_copy(const char* key, const std::string& value); // copies value to json
	void add_int(const char* key, uint32_t v);
	void add_int64(const char* key, uint64_t v);
	void add_double(const char* key, double v);
	void add_bool(const char* key, bool v);
	void move_to_member(const char* key, rapidjson::Value& value);
	void push_back(rapidjson::Value& value);
	void push_back(CJsonObject& o);


	std::string dump_rapidjson() const;
	std::string dump_rapidjson_pretty(int ident = 1) const;
	void dump_rapidjson_pretty(std::string filepath, int ident=1) const;

};
