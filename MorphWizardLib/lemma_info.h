#pragma once

#include "../common/utilit.h"
#include "flexia_model.h"


const size_t CommonAncodeSize = 2;
const uint16_t UnknownParadigmNo = 0xffff - 1;
const uint16_t UnknownAccentModelNo = 0xffff - 1;


struct CLemmaInfo
{
	uint16_t	m_FlexiaModelNo;
	uint16_t	m_AccentModelNo;
	char	m_CommonAncode[CommonAncodeSize];

	CLemmaInfo();

	bool operator ==(const CLemmaInfo& obj) const;
	bool operator <(const CLemmaInfo& obj) const;
	std::string GetCommonAncodeIfCan()  const;
	std::string GetBase(const std::string lemma, const CFlexiaModel& m) const;
};

