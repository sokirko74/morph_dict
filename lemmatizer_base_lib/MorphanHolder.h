#pragma once

#include "morph_dict/common/utilit.h"
#include "morph_dict/agramtab/RusGramTab.h"
#include "morph_dict/agramtab/GerGramTab.h"
#include "morph_dict/agramtab/EngGramTab.h"
#include "Paradigm.h"
#include "Lemmatizers.h"


// input and output strings in utf8
class CMorphanHolder
{
	DwordVector _GetLemmaIds(bool bNorm, std::string& word_str, bool capital, bool bUsePrediction) const;
protected:
	std::string GetGrammems(const char* tab_str) const;
	bool _GetParadigmCollection(std::string WordForm, std::vector<CFormInfo>& Paradigms) const;
public:
	MorphLanguageEnum				m_CurrentLanguage;
	CAgramtab*						m_pGramTab;
    CLemmatizer* 			        m_pLemmatizer;
	bool                            m_bUsePrediction;

	CMorphanHolder();
	~CMorphanHolder();

    void LoadMorphology(MorphLanguageEnum langua, std::string custom_folder="");
	void LoadOnlyLemmatizer(MorphLanguageEnum langua, std::string custom_folder="");
	void LoadOnlyGramtab(MorphLanguageEnum langua, std::string custom_folder = "");

	void DeleteProcessors();
	DwordVector GetLemmaIds(std::string lemma) const;
	DwordVector GetWordFormIds(std::string word_form) const;
	std::string		id_to_string(long id) const;
	std::string PrintMorphInfoUtf8(std::string Form, bool printIds, bool printForms, bool sortParadigms) const;
	std::string LemmatizeJson(std::string word_utf8, bool withParadigms, bool prettyJson=false, bool sortForms=false) const;
	bool IsInDictionaryUtf8(std::string WordForm) const;
	std::vector<CFuzzyResult> CorrectMisspelledWordUtf8(std::string word_utf8) const;
	
};

