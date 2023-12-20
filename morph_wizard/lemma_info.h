#pragma once

#include "../common/utilit.h"
#include "flexia_model.h"


const size_t CommonAncodeSize = 3; // ancode length is 2, plus null for null termination
const uint16_t UnknownFlexiaModelNo = 0xffff - 1;
const uint16_t UnknownAccentModelNo = 0xffff - 1;


class CLemmaInfo
{

	char	m_CommonAncode[CommonAncodeSize];

public:
	uint16_t	m_FlexiaModelNo;
	uint16_t	m_AccentModelNo;
	
	CLemmaInfo();

	bool operator ==(const CLemmaInfo& obj) const;
	bool operator <(const CLemmaInfo& obj) const;
	std::string GetCommonAncodeCopy()  const;
	std::string GetBase(const std::string lemma, const CFlexiaModel& m) const;
	void SetCommonAncode(const char* s);
	const char* GetCommonAncode() const;
	int CompareCommonAncode(const char* s) const;
};

