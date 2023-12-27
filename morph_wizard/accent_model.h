#pragma once

#include "../common/utilit.h"
#include "../common/rapidjson.h"


struct CAccentModel
{
	std::vector<BYTE> m_Accents;

	bool operator == (const CAccentModel& X) const
	{
		return m_Accents == X.m_Accents;

	};
	CAccentModel& FromJson(const rapidjson::Value& inj);
	CAccentModel& FromString(const std::string& s);
};

