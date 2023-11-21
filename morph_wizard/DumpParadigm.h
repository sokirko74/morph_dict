#pragma once

#include <string>
#include "../common/rapidjson.h"

class MorphoWizard;

//----------------------------------------------------------------------------
struct CMorphSession
{
	std::string		m_UserName;
	std::string		m_SessionStart;
	std::string		m_LastSessionSave;

	bool operator  == (const  CMorphSession& X) const;
	void GetJson(CJsonObject& out) const;
	CMorphSession& FromJson(const rapidjson::Value& inj);
	void		SetEmpty();
	bool		IsEmpty() const;
	std::string GetJsonStr() const;
};


class CDumpParadigm
{
	bool	ReadFromFile(FILE* fp, int& line_no, bool& bError, std::string& Errors);

public:
	std::string			m_TypeGrammemsStr;
	std::string			m_PrefixesStr;
	std::string			m_SlfStr;
	std::string			m_AuthorStr;
	CMorphSession		m_Session;
	int					m_FirstSlfLineNo;
	const MorphoWizard* m_pWizard;

	CDumpParadigm(const MorphoWizard* wizard) : m_pWizard(wizard) {};
	void	SetEmpty();
	std::string	ToStringUtf8() const;
	bool	ReadNextParadigmFromFile(FILE* fp, int& line_no, bool& bError, std::string& Errors);

};
