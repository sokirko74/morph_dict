#pragma once

#include "../common/json.h"


struct CAccentModel
{
	std::vector<BYTE> m_Accents;

	bool operator == (const CAccentModel& X) const
	{
		return m_Accents == X.m_Accents;

	};
	CAccentModel& FromJson(nlohmann::json inj);
	CAccentModel& FromString(const std::string& s);
};

