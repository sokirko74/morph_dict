#include "lemma_info.h"


CLemmaInfo::CLemmaInfo()
{
	m_FlexiaModelNo = UnknownFlexiaModelNo;
	m_AccentModelNo = UnknownAccentModelNo;
	m_CommonAncode[0] = 0;
};


bool CLemmaInfo::operator ==(const CLemmaInfo& obj) const
{
	return ((m_FlexiaModelNo == obj.m_FlexiaModelNo)
		&& (m_AccentModelNo == obj.m_AccentModelNo)
		&& !CompareCommonAncode(obj.m_CommonAncode)
		);
}

bool CLemmaInfo::operator <(const CLemmaInfo& obj) const
{
	if (m_FlexiaModelNo != obj.m_FlexiaModelNo)
		return m_FlexiaModelNo < obj.m_FlexiaModelNo;

	int res = CompareCommonAncode(obj.m_CommonAncode);
	if (res != 0)
		return res < 0;

	return m_AccentModelNo < obj.m_AccentModelNo;
}

std::string CLemmaInfo::GetCommonAncodeCopy()  const
{
	if (m_CommonAncode[0] == 0) return "";
	return std::string(m_CommonAncode, 2);
}

std::string CLemmaInfo::GetBase(const std::string lemma, const CFlexiaModel& m) const {
	size_t flex_size = m.get_first_flex().size();
	return lemma.substr(0, lemma.size() - flex_size);
}

void CLemmaInfo::SetCommonAncode(const char* s) {
    if (*s == 0) {
        m_CommonAncode[0] = 0;
    } else {
        memcpy(m_CommonAncode, s, CommonAncodeSize - 1);
        m_CommonAncode[CommonAncodeSize - 1] = 0;
    }
}

const char* CLemmaInfo::GetCommonAncode() const {
	return m_CommonAncode;
}

int CLemmaInfo::CompareCommonAncode(const char* s) const {
	return strncmp(m_CommonAncode, s, CommonAncodeSize - 1);
}