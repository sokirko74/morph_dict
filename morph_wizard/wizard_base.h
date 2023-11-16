#pragma once
#include "../common/utilit.h"
#include "lemma_info.h"
#include "flexia_model.h"
#include "accent_model.h"



class CMorphWizardBase {
public:
	MorphLanguageEnum		m_Language;

	//  a vector of all Paradigms 
	std::vector<CFlexiaModel>	m_FlexiaModels;

	// all accent models
	std::vector<CAccentModel>	m_AccentModels;

	bool read_utf8_line(std::ifstream& inp, std::string& line) const;
	std::string str_to_utf8(const std::string& line) const;

	nlohmann::json GetFlexiaModelsJson() const;
	nlohmann::json GetAccentModelsJson() const;

	void  SerializeFlexiaModelsToAnnotFile(std::ostream& outp) const;
	void  SerializeAccentModelsToAnnotFile(std::ostream& outp) const;
};
