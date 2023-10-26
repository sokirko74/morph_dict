// ==========  This file is under  LGPL, the GNU Lesser General Public Licence
// ==========  Dialing Lemmatizer (www.aot.ru)
// ==========  Copyright by Alexey Sokirko

#include "Predict.h"
#include "MorphAutomBuilder.h"
#include "MorphDictBuilder.h"
#include "LemmaInfoSerialize.h"
#include "common_consts.h"



struct CPredictWord
{
	uint16_t				m_ItemNo;
	uint16_t				m_Freq;
	int						m_LemmaInfoNo;
	uint16_t				m_nps;



	CPredictWord(uint16_t Freq, int LemmaInfoNo, uint16_t nps, uint16_t ItemNo)
		: m_Freq(Freq), m_LemmaInfoNo(LemmaInfoNo), m_nps(nps), m_ItemNo(ItemNo)
	{	}

	void AddToAutomat(CMorphAutomatBuilder& R, std::string s) const {
		reverse(s.begin(), s.end());
		s += R.m_AnnotChar;
		s += R.EncodeIntToAlphabet(m_nps);
		s += R.m_AnnotChar;
		s += R.EncodeIntToAlphabet(m_LemmaInfoNo);
		s += R.m_AnnotChar;
		s += R.EncodeIntToAlphabet(m_ItemNo);
		R.AddStringDaciuk(s);
	}
};

struct CModelPostfix
{
	std::string m_Postfix;
	size_t m_ModelNo;
	CModelPostfix(std::string Postfix, size_t ModelNo)
	{
		m_Postfix = Postfix;
		m_ModelNo = ModelNo;
	};

	bool operator < (const CModelPostfix& X) const
	{
		if (m_ModelNo != X.m_ModelNo)
			return m_ModelNo < X.m_ModelNo;
		else
			return m_Postfix < X.m_Postfix;
	};

};


typedef std::map<std::string, std::vector<CPredictWord> > Flex2WordMap;
typedef std::map<CModelPostfix, size_t > Postfix2FreqMap;


/*
 this function add  a new item to svMapRaw.
 svMapRaw is  from a postfix to  a std::vector of possible predict words.

*/
void AddElem(Flex2WordMap& svMapRaw,
	const std::string& Postfix,
	int LemmaInfoNo,
	const uint16_t nps,
	const uint16_t ItemNo,
	Postfix2FreqMap& ModelFreq,
	const std::vector<CLemmaInfoAndLemma>& LemmaInfos
)
{

	// ============= determine the part of speech (nps)
	const CLemmaInfo& LemmaInfo = LemmaInfos[LemmaInfoNo].m_LemmaInfo;


	// ============= adding to  svMapRaw, if not exists, otherwise update frequence
	CPredictWord set2(1, LemmaInfoNo, nps, ItemNo);
	Flex2WordMap::iterator svMapIt = svMapRaw.find(Postfix);

	if (svMapIt == svMapRaw.end())
	{
		std::vector<CPredictWord> set2vec;
		set2vec.push_back(set2);
		svMapRaw[Postfix] = set2vec;
	}
	else
	{
		int i = 0;
		CModelPostfix new_p(Postfix, LemmaInfo.m_FlexiaModelNo);
		for (; i < svMapIt->second.size(); i++)
		{
			// if postfix, flexia and PartOfSpeech are equal then we should update frequence and exit
			if (svMapIt->second[i].m_nps == set2.m_nps)
			{
				svMapIt->second[i].m_Freq++;

				//  if  the new example is more frequent then we should predict using this example
				const CLemmaInfo& OldLemmaInfo = LemmaInfos[svMapIt->second[i].m_LemmaInfoNo].m_LemmaInfo;

				if (ModelFreq[new_p] > ModelFreq[CModelPostfix(Postfix, OldLemmaInfo.m_FlexiaModelNo)])
				{
					svMapIt->second[i].m_LemmaInfoNo = LemmaInfoNo;
					svMapIt->second[i].m_ItemNo = ItemNo;

				};

				break;
			}
		}
		// if no such part of speech for this postfix and flexia is found
		// then add new item
		if (i >= svMapIt->second.size())
			svMapIt->second.push_back(set2);
	}
}




struct IsLessByModelNoAndBase
{
	const MorphoWizard& m_Wizard;

	IsLessByModelNoAndBase(const MorphoWizard& wizard) : m_Wizard(wizard)
	{
	};

	bool operator() (const_lemma_iterator_t _X1, const_lemma_iterator_t _X2) const
	{
		if (_X1->second.m_FlexiaModelNo != _X2->second.m_FlexiaModelNo)
			return _X1->second.m_FlexiaModelNo < _X2->second.m_FlexiaModelNo;

		return m_Wizard.get_base_string(_X1) < m_Wizard.get_base_string(_X2);
	};
};


const size_t MinimalFlexiaModelFrequence = 10;

bool CMorphDictBuilder::GenPredictIdx(const MorphoWizard& wizard, int PostfixLength, int MinFreq, std::string path, nlohmann::json& output_opts)
{
	LOGI << "CMorphDictBuilder::GenPredictIdx";
	DwordVector ModelFreq(wizard.m_FlexiaModels.size(), 0);
	//  building frequences of flexia models
	for (const_lemma_iterator_t lnMapIt = wizard.m_LemmaToParadigm.begin(); lnMapIt != wizard.m_LemmaToParadigm.end(); lnMapIt++) {
		ModelFreq[lnMapIt->second.m_FlexiaModelNo]++;
	}

	bool bSparsedDictionary;
	{
		int Count = 0;
		for (size_t ModelNo = 0; ModelNo < ModelFreq.size(); ModelNo++)
			if (ModelFreq[ModelNo] >= MinimalFlexiaModelFrequence)
				Count++;
		bSparsedDictionary = 2 * Count < ModelFreq.size();
		if (bSparsedDictionary)
			LOGI << "Flexia models are too sparsed";
	};

	//  going through all words
	Postfix2FreqMap Postfix2Freq;
	for (size_t lin = 0; lin < m_LemmaInfos.size(); lin++)
	{
		if (!(lin % 1000))
			LOGD << "Prepare data... " << lin;
		const CLemmaInfo& LemmaInfo = m_LemmaInfos[lin].m_LemmaInfo;
		size_t ModelNo = LemmaInfo.m_FlexiaModelNo;
		const CFlexiaModel& paradigm = m_FlexiaModels[ModelNo];
		std::string	base = m_Bases[m_LemmaInfos[lin].m_LemmaStrNo].GetString();
		const std::vector <bool>& Infos = m_ModelInfo[ModelNo];
		for (size_t i = 0; i < paradigm.m_Flexia.size(); i++)
			if (Infos[i])
			{
				std::string flexia = paradigm.m_Flexia[i].m_FlexiaStr;
				std::string wordform = base + flexia;
				if (wordform.length() < PostfixLength) continue;
				CModelPostfix p(wordform.substr(wordform.length() - PostfixLength), ModelNo);
				Postfix2FreqMap::iterator it = Postfix2Freq.find(p);
				if (it != Postfix2Freq.end())
					it->second++;
				else
					Postfix2Freq[p] = 1;
			};

	};

	LOGI << "finish prepare";

	std::string plug_noun = convert_to_utf8(wizard.m_pGramTab->GetPlugNouInfo().m_Lemma, m_Language);
	
	int PlugLemmaInfoNo = -1;

	Flex2WordMap svMapRaw;
	//  going through all words
	for (size_t lin = 0; lin < m_LemmaInfos.size(); lin++)
	{

		if (!(lin % 1000))
			LOGD << "Pick up data... " << lin;

		const CLemmaInfo& LemmaInfo = m_LemmaInfos[lin].m_LemmaInfo;
		size_t ModelNo = LemmaInfo.m_FlexiaModelNo;
		const CFlexiaModel& paradigm = m_FlexiaModels[ModelNo];
		std::string	base = m_Bases[m_LemmaInfos[lin].m_LemmaStrNo].GetString();

		if (base + paradigm.get_first_flex() == plug_noun)
		{
			PlugLemmaInfoNo = (int)lin;
			output_opts["PlugNounGramCode"] = convert_to_utf8(paradigm.get_first_code(), m_Language);
			continue;
		};

		if (!bSparsedDictionary)
			if (ModelFreq[ModelNo] < MinimalFlexiaModelFrequence)
				continue;

		auto pos = wizard.m_pGramTab->GetPartOfSpeech(paradigm.get_first_code().c_str());
		if (!wizard.m_pGramTab->PartOfSpeechIsProductive(pos))
			continue;


		const std::vector <bool>& Infos = m_ModelInfo[ModelNo];
		for (size_t i = 0; i < paradigm.m_Flexia.size(); i++)
			if (Infos[i])
			{
				std::string flexia = paradigm.m_Flexia[i].m_FlexiaStr;
				std::string wordform = base + flexia;
				if (wordform.length() < PostfixLength) continue;
				std::string Postfix = wordform.substr(wordform.length() - PostfixLength);
				AddElem(svMapRaw, Postfix, (int)lin, pos, (uint16_t)i, Postfix2Freq, m_LemmaInfos);
			}

	}
	LOGI << "finish picking up data";

	if (PlugLemmaInfoNo == -1)
	{
		LOGE << "Cannot find a word for the default noun prediction (\"" << plug_noun <<
			"\") while  generating "<< GetStringByLanguage(wizard.m_Language)  <<" prediction base";
		return false;
	};

	LOGI << "Saving...";

	CMorphAutomatBuilder R(wizard.m_Language, MorphAnnotChar);
	R.InitTrie();

	// adding crtitical noun
	{
		std::string s = R.GetCriticalNounLetterPack();
		CPredictWord(0, PlugLemmaInfoNo, 0, 0).AddToAutomat(R, s);
		// 0 is noun
	};

	for (auto& it: svMapRaw)
	{
		for (auto& w : it.second)
		{
			if (w.m_Freq >= MinFreq) {
				w.AddToAutomat(R, it.first);
			}
		}

	};

	R.ConvertBuildRelationsToRelations();
	auto out_path = std::filesystem::path(path) / PREDICT_BIN_PATH;
	R.Save(out_path.string());


	svMapRaw.clear();
	return true;

}



