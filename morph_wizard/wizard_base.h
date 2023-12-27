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

	void SaveFlexiaModelsToJson(CJsonObject& d) const;
	void SaveAccentModelsToJson(CJsonObject& d) const;

	void  SerializeFlexiaModelsToAnnotFile(std::ostream& outp) const;
	void  SerializeAccentModelsToAnnotFile(std::ostream& outp) const;
};
