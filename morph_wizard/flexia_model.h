#pragma once

#include "../common/utilit.h"
#include "../common/json.h"


struct CMorphForm
{
	std::string m_Gramcode;
	std::string m_FlexiaStr;
	std::string m_PrefixStr;

	CMorphForm(std::string Gramcode, std::string FlexiaStr, std::string PrefixStr);
	CMorphForm(nlohmann::json j);
	CMorphForm(const std::string& s);
	
	bool		operator == (const CMorphForm& X) const;
	nlohmann::json		ToJson() const;

	std::string ToString() const;
	CMorphForm& FromString(const std::string& s);

};


struct  CFlexiaModel
{
	std::vector<CMorphForm>	m_Flexia;
	std::string				m_WiktionaryMorphTemplate;
	std::string				m_Comments;

	bool		operator == (const CFlexiaModel& X) const
	{
		return	m_Flexia == X.m_Flexia;
	};
	nlohmann::json		ToJson() const;
	CFlexiaModel&	FromJson(nlohmann::json inj) ;

	std::string		ToString() const;
	CFlexiaModel& FromString(const std::string& s);

	std::string		get_first_flex() const;
	std::string		get_first_code() const;
	bool		has_ancode(const std::string& search_ancode) const;

};
