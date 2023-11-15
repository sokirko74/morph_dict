#pragma once

#include "morph_dict/common/utilit.h"
#include "morph_dict/agramtab/RusGramTab.h"
#include "morph_dict/agramtab/GerGramTab.h"
#include "morph_dict/agramtab/EngGramTab.h"
#include "Paradigm.h"
#include "Lemmatizers.h"

class CMorphanHolder
{
protected:
	std::string GetGrammems(const char* tab_str) const;
	void CreateMorphDicts(MorphLanguageEnum langua);
public:
	MorphLanguageEnum				m_CurrentLanguage;
	CAgramtab*						m_pGramTab;
    CLemmatizer* 			        m_pLemmatizer;
	bool                            m_bUsePrediction;

	CMorphanHolder();
	~CMorphanHolder();

    void LoadLemmatizer(MorphLanguageEnum langua, std::string custom_folder="");
	void DeleteProcessors();
	DwordVector GetLemmaIds(std::string lemma) const;
	std::string		id_to_string(long id) const;
	CFormInfo		id_to_paradigm(long id) const;
	std::string PrintMorphInfoUtf8(std::string Form, bool printIds, bool printForms, bool sortParadigms) const;
	std::string LemmatizeJson(std::string WordForm, bool withParadigms, bool prettyJson=false, bool sortForms=false) const;
	bool IsInDictionary(std::string WordForm) const;
	std::vector<CFuzzyResult> CorrectMisspelledWordUtf8(std::string word_utf8) const;
	bool GetParadigmCollection(std::string WordForm, std::vector<CFormInfo>& Paradigms) const;
};

