#pragma once

#include "../common/json.h"


struct CAccentModel
{
	std::vector<BYTE> m_Accents;

	bool operator == (const CAccentModel& X) const
	{
		return m_Accents == X.m_Accents;

	};
	std::string		ToString() const;
	CAccentModel& FromJson(nlohmann::json inj);
};

