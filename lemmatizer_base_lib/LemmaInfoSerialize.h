#pragma once

#include  "morph_dict/common/bserialize.h"
#include  "morph_dict/morph_wizard/lemma_info.h"

inline size_t get_size_in_bytes (const CLemmaInfo& t)
{
	return
		get_size_in_bytes(t.m_FlexiaModelNo)
		+ get_size_in_bytes(t.m_AccentModelNo)
		+ CommonAncodeSize;
};

inline size_t save_to_bytes(const CLemmaInfo& i, BYTE* buf)
{
	buf += save_to_bytes(i.m_FlexiaModelNo, buf);
	buf += save_to_bytes(i.m_AccentModelNo, buf);
	memcpy(buf, i.GetCommonAncode(), CommonAncodeSize);
	buf += CommonAncodeSize;
	return get_size_in_bytes(i);
}


inline size_t restore_from_bytes(CLemmaInfo& i, const BYTE* buf)
{
	buf += restore_from_bytes(i.m_FlexiaModelNo, buf);
	buf += restore_from_bytes(i.m_AccentModelNo, buf);
	i.SetCommonAncode((const char*)buf);
	buf += CommonAncodeSize;
	return get_size_in_bytes(i);
}

