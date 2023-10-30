#include "lemma_info.h"


CLemmaInfo::CLemmaInfo()
{
	m_FlexiaModelNo = UnknownParadigmNo;
	m_AccentModelNo = UnknownAccentModelNo;
	m_CommonAncode[0] = 0;
};


bool CLemmaInfo::operator ==(const CLemmaInfo& obj) const
{
	return ((m_FlexiaModelNo == obj.m_FlexiaModelNo)
		&& (m_AccentModelNo == obj.m_AccentModelNo)
		&& !strncmp(m_CommonAncode, obj.m_CommonAncode, CommonAncodeSize)
		);
}

bool CLemmaInfo::operator <(const CLemmaInfo& obj) const
{
	if (m_FlexiaModelNo != obj.m_FlexiaModelNo)
		return m_FlexiaModelNo < obj.m_FlexiaModelNo;

	int res = strncmp(m_CommonAncode, obj.m_CommonAncode, CommonAncodeSize);
	if (res != 0)
		return res < 0;

	return m_AccentModelNo < obj.m_AccentModelNo;
}

std::string CLemmaInfo::GetCommonAncodeIfCan()  const
{
	if (m_CommonAncode[0] == 0) return "";
	return std::string(m_CommonAncode, 2);
}

std::string CLemmaInfo::GetBase(const std::string lemma, const CFlexiaModel& m) const {
	size_t flex_size = m.get_first_flex().size();
	return lemma.substr(0, lemma.size() - flex_size);
}

