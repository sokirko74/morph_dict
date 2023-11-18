// ==========  This file is under  LGPL, the GNU Lesser General Public Licence
// ==========  Dialing Lemmatizer (www.aot.ru)
// ==========  Copyright by Alexey Sokirko

#include "morph_dict/common/json.h"
#include "morph_dict/morph_wizard/paradigm_consts.h"
#include "MorphanHolder.h"

CMorphanHolder::CMorphanHolder()
{
	m_pLemmatizer = 0;
	m_pGramTab = 0;
    m_bUsePrediction = true;
};

CMorphanHolder::~CMorphanHolder()
{
	DeleteProcessors();
};

void CMorphanHolder::DeleteProcessors()
{
	if (m_pLemmatizer) 
	{
		delete m_pLemmatizer;
		m_pLemmatizer = 0;
	}
	if (m_pGramTab)
	{
		delete m_pGramTab;
		m_pGramTab = 0;
	}

};

void CMorphanHolder::CreateMorphDicts(MorphLanguageEnum langua) 
{
    if (langua == morphRussian)
    {
        m_pGramTab = new CRusGramTab;
        m_pLemmatizer = new CLemmatizerRussian;
    }
    else if (langua == morphFioDisclosures)
    {
        m_pGramTab = new CRusGramTab;
        m_pLemmatizer = new CLemmatizer(morphFioDisclosures);
    }
    else
        if (langua == morphGerman)
        {
            m_pGramTab = new CGerGramTab;
            m_pLemmatizer = new CLemmatizerGerman;
        }
        else
            if (langua == morphEnglish)
            {
                m_pGramTab = new CEngGramTab;
                m_pLemmatizer = new CLemmatizerEnglish;
            }
            else
            {
                throw CExpc("unsupported language");
            };
}

void CMorphanHolder::LoadLemmatizer(MorphLanguageEnum langua, std::string custom_folder)
{
        DeleteProcessors();
        CreateMorphDicts(langua);
        if (custom_folder.empty()) {
            m_pLemmatizer->LoadDictionariesRegistry();
            m_pGramTab->LoadFromRegistry();
        }
        else {
            m_pLemmatizer->LoadDictionariesFromPath(custom_folder);
            m_pGramTab->ReadFromFolder(custom_folder);
        }
		m_CurrentLanguage = langua;
}

DwordVector CMorphanHolder::_GetLemmaIds(bool bNorm, std::string& word_str, bool capital, bool bUsePrediction) const {
    DwordVector ids;

    std::vector<CFormInfo > ParadigmCollection;
    std::string s8 = convert_from_utf8(word_str.c_str(), m_CurrentLanguage);
    if (!m_pLemmatizer->CreateParadigmCollection(bNorm, word_str, capital, bUsePrediction, ParadigmCollection))
    {
        throw CExpc("cannot lemmatize %s", word_str.c_str());
    }
    for (auto& p : ParadigmCollection)
    {
        if (p.m_bFound || bUsePrediction) {
            ids.push_back(p.GetParadigmId());
        }
    }
    return ids;
}


DwordVector CMorphanHolder::GetLemmaIds(std::string lemma) const
{
    return _GetLemmaIds(true, lemma, true, false);
}

DwordVector CMorphanHolder::GetWordFormIds(std::string word_form) const
{
    return _GetLemmaIds(false, word_form, false, false);
}

std::string CMorphanHolder::id_to_string(long id) const
{
	
	CFormInfo Res;
	if (!m_pLemmatizer->CreateParadigmFromID(id, Res))
		throw CExpc (Format( "cannot get lemma  by id %i", id));
	return convert_to_utf8(Res.GetWordForm(0), m_CurrentLanguage);
}



std::string CMorphanHolder::GetGrammems(const char* tab_str) const {
	uint64_t G;
	m_pGramTab->GetGrammems(tab_str, G);
	std::string s = m_pGramTab->GrammemsToStr(G);
	if (!s.empty() && (s[s.length() - 1] == ','))
		s.erase(s.length() - 1);
	return s;
}

std::string CMorphanHolder::PrintMorphInfoUtf8(std::string Form, bool printIds, bool printForms, bool sortParadigms) const
{
    std::string word_s8 = convert_from_utf8(Form.c_str(), m_CurrentLanguage);
	bool bCapital = is_upper_alpha((BYTE)word_s8[0], m_CurrentLanguage);

	std::vector<CFormInfo> Paradigms;
	m_pLemmatizer->CreateParadigmCollection(false, word_s8, bCapital, true, Paradigms);

	std::vector<std::string> Results;
	for (auto& f : Paradigms) {
        std::ostringstream ss;
		ss << (f.m_bFound ? "+" : "-") << " ";
		ss <<  f.GetWordFormUtf8(0) << " ";

		{
			std::string GramCodes = f.GetSrcAncode();
			BYTE PartOfSpeech = m_pGramTab->GetPartOfSpeech(GramCodes.c_str());
			ss << m_pGramTab->GetPartOfSpeechStr(PartOfSpeech) << " ";

			auto type = f.GetCommonAncode();
			ss << (type.empty() ? "" : GetGrammems(type.c_str())) << " ";

			for (long i = 0; i < GramCodes.length(); i += 2) {
				if (i > 0)
					ss << ";";
				ss << GetGrammems(GramCodes.c_str() + i);
			}

		}

		if (printIds)
			ss << " " << f.GetParadigmId();

		BYTE Accent = f.GetSrcAccentedVowel();
        if (Accent != UnknownAccent) {
            auto af = word_s8.substr(0, Accent + 1) + "'" + word_s8.substr(Accent + 1);
            ss << " " << convert_to_utf8(af, m_CurrentLanguage);
        }

		if (printForms) {
			ss << " ";
			for (int k = 0; k < f.GetCount(); k++) {
				if (k > 0)
					ss << ",";
				ss << f.GetWordFormUtf8(k);
			};
		};
		Results.push_back(ss.str());
	};

	if (sortParadigms) {
		std::sort(Results.begin(), Results.end());
	};
	std::string result;
	for (int i = 0; i < Results.size(); i++) {
		if (i > 0)
			result += "\t";
		result += Results[i] + "\n";
	}
	return result;
};

bool CMorphanHolder::_GetParadigmCollection(std::string WordForm, std::vector<CFormInfo>&	Paradigms) const {
	if (WordForm.empty())	{
		return false;
	};

	try
	{
		if (m_pLemmatizer == nullptr) return false;

		m_pLemmatizer->CreateParadigmCollection(false,WordForm,
                                    is_upper_alpha((unsigned char)WordForm[0], m_CurrentLanguage),
                                                          m_bUsePrediction,
                                                          Paradigms);
	}
	catch (...)
	{
		return false;;
	};
	return true;
};

bool CMorphanHolder::IsInDictionaryUtf8(std::string w) const {
    std::string s = convert_from_utf8(w.c_str(), m_pLemmatizer->GetLanguage());
    return m_pLemmatizer->IsInDictionary(s, true);
}

const int ParagigmGroupsCount = 45;
const std::string ParagigmGroups[ParagigmGroupsCount] = {
        "П ед,мр",
        "П ед,жр",
        "П ед,ср",
        "П мн",
        "П сравн",
        "С ед",
        "С мн",
        "Г нст",
        "Г прш",
        "Г буд",
        "Г пвл",
        "ПРИЧАСТИЕ нст,мр,стр",
        "ПРИЧАСТИЕ нст,жр,стр",
        "ПРИЧАСТИЕ нст,ср,стр",
        "ПРИЧАСТИЕ нст,мн,стр",
        "ПРИЧАСТИЕ прш,мр,стр",
        "ПРИЧАСТИЕ прш,жр,стр",
        "ПРИЧАСТИЕ прш,ср,стр",
        "ПРИЧАСТИЕ прш,мн,стр",
        "ПРИЧАСТИЕ нст,мр,дст",
        "ПРИЧАСТИЕ нст,жр,дст",
        "ПРИЧАСТИЕ нст,ср,дст",
        "ПРИЧАСТИЕ нст,мн,дст",
        "ПРИЧАСТИЕ прш,мр,дст",
        "ПРИЧАСТИЕ прш,жр,дст",
        "ПРИЧАСТИЕ прш,ср,дст",
        "ПРИЧАСТИЕ прш,мн,дст",
        "ЧИСЛ мр",
        "ЧИСЛ жр",
        "ЧИСЛ ср",
        "ЧИСЛ-П ед,мр",
        "ЧИСЛ-П ед,жр",
        "ЧИСЛ-П ед,ср",
        "ЧИСЛ-П мн",
        "МС-П ед,мр",
        "МС-П ед,жр",
        "МС-П ед,ср",
        "МС-П мн",
        "МС ед",
        "МС мн",
        "МС ед,мр",
        "МС ед,жр",
        "МС ед,ср",
        "VBE sg",
        "VBE pl"
};

struct CFormAndGrammems
{
    MorphLanguageEnum m_Langua;
    std::string m_Form;
    part_of_speech_t m_PartOfSpeech;
    grammems_mask_t m_Grammems;

    int get_sort_key() const {
        if (m_Langua == morphRussian && m_PartOfSpeech == INFINITIVE) {
            return -1;
        }
        return m_PartOfSpeech;
    }
    bool operator<(const CFormAndGrammems &X) const
    {
        return get_sort_key() < X.get_sort_key();
    };
};

struct CFormGroup
{
    grammems_mask_t m_IntersectGrammems;
    std::vector<int> m_FormNos;
};


std::string &TrimCommaRight(std::string &str)
{
    if (str.size() == 0)
        return str;
    size_t i = str.find_last_not_of(",");
    str.erase(i + 1);
    return str;
};

std::string GetGramInfoStr(std::string GramInfo, const CMorphanHolder *Holder)
{
    const CAgramtab *pGramtab = Holder->m_pGramTab;
    assert(!GramInfo.empty());
    int POS = pGramtab->GetPartOfSpeech(GramInfo.c_str());
    std::string Result;
    Result += pGramtab->GetPartOfSpeechStr(POS);
    Result += " ";
    grammems_mask_t grammems = pGramtab->GetAllGrammems(GramInfo.c_str());
    Result += pGramtab->GrammemsToStr(grammems);
    TrimCommaRight(Result);
    return Result;
};

std::vector<CFormGroup> GetParadigmGroupedLikeInTextbook(const std::vector<CFormAndGrammems> &Forms, const CMorphanHolder *Holder, grammems_mask_t &CommonGrammems)
{
    std::vector<CFormGroup> Results;
    std::vector<bool> IncludedVector;
    int i = 0;
    const CAgramtab *pGramtab = Holder->m_pGramTab;

    for (; i < Forms.size(); i++)
        IncludedVector.push_back(false);

    for (long GroupNo = 0; GroupNo < ParagigmGroupsCount; GroupNo++)
    {
        part_of_speech_t POS;
        grammems_mask_t Grammems = 0;
        if (!pGramtab->ProcessPOSAndGrammems(ParagigmGroups[GroupNo].c_str(), POS, Grammems))
            continue;
        ;
        CFormGroup F;
        F.m_IntersectGrammems = GetMaxQWORD();
        for (long i = 0; i < Forms.size(); i++)
            if (!IncludedVector[i])
                if (Forms[i].m_PartOfSpeech == POS)
                    if ((Grammems & Forms[i].m_Grammems) == Grammems)
                    {
                        int k = 0;
                        for (; k < F.m_FormNos.size(); k++)
                            if (Forms[F.m_FormNos[k]].m_Grammems >= Forms[i].m_Grammems)
                                break;
                        F.m_FormNos.insert(F.m_FormNos.begin() + k, i);
                        IncludedVector[i] = true;
                        F.m_IntersectGrammems &= Forms[i].m_Grammems;
                    };

        if (F.m_FormNos.size() > 0)
            Results.push_back(F);
    };

    CFormGroup LastGroup;
    LastGroup.m_IntersectGrammems = 0;
    for (size_t i = 0; i < Forms.size(); i++)
        if (!IncludedVector[i])
            LastGroup.m_FormNos.push_back(i);

    if (LastGroup.m_FormNos.size() > 0)
        Results.push_back(LastGroup);

    CommonGrammems = GetMaxQWORD();
    for (const auto& f : Forms) {
        CommonGrammems &= f.m_Grammems;
    }

    return Results;
};

std::vector<CFormGroup> BuildInterfaceParadigmPart(const CMorphanHolder *Holder, const std::vector<CFormAndGrammems> FormAndGrammems, int &FormNo, grammems_mask_t &commonGrammems)
{
    int EndFormNo = FormNo + 1;
    for (; EndFormNo < FormAndGrammems.size(); EndFormNo++)
        if (FormAndGrammems[FormNo].m_PartOfSpeech != FormAndGrammems[EndFormNo].m_PartOfSpeech)
            break;
    std::vector<CFormAndGrammems> FormAndGrammemsPart;
    FormAndGrammemsPart.insert(FormAndGrammemsPart.begin(), FormAndGrammems.begin() + FormNo, FormAndGrammems.begin() + EndFormNo);
    FormNo = EndFormNo;
    return GetParadigmGroupedLikeInTextbook(FormAndGrammemsPart, Holder, commonGrammems);
};

std::vector<CFormAndGrammems> BuildFormAndGrammems(const CMorphanHolder *Holder, const CFormInfo *piParadigm)
{
    const CAgramtab *pGramtab = Holder->m_pGramTab;
    // получаем все формы и граммемы в массив FormAndGrammems
    std::vector<CFormAndGrammems> FormAndGrammems;
    for (int j = 0; j < piParadigm->GetCount(); j++)
    {
        std::string GramInfo = piParadigm->GetAncode(j);
        for (long i = 0; i < GramInfo.length(); i += 2)
        {
            CFormAndGrammems F;
            F.m_Langua = Holder->m_CurrentLanguage;
            F.m_Form = piParadigm->GetWordForm(j);
            BYTE AccentedCharNo = piParadigm->GetAccentedVowel(j);
            if (AccentedCharNo != UnknownAccent)
                F.m_Form.insert(AccentedCharNo + 1, "'");
            F.m_Grammems = pGramtab->GetAllGrammems(GramInfo.substr(i, 2).c_str());
            F.m_PartOfSpeech = pGramtab->GetPartOfSpeech(GramInfo.substr(i, 2).c_str());
            FormAndGrammems.push_back(F);
        };
    };
    sort(FormAndGrammems.begin(), FormAndGrammems.end());
    return FormAndGrammems;
}


nlohmann::json GetParadigmFromDictionary(const CFormInfo *piParadigm, const CMorphanHolder *Holder, bool sortForms)
{
    const CAgramtab *pGramtab = Holder->m_pGramTab;
    const std::vector<CFormAndGrammems> FormAndGrammems = BuildFormAndGrammems(Holder, piParadigm);
    nlohmann::json result = nlohmann::json::array();
    int FormNo = 0;
    while (FormNo < FormAndGrammems.size())
    {
        int saveFormNo = FormNo;
        grammems_mask_t commonGrammems;
        const std::vector<CFormGroup> FormGroups = BuildInterfaceParadigmPart(Holder, FormAndGrammems, FormNo, commonGrammems);
        assert(FormNo > saveFormNo);
        auto prdPart = nlohmann::json::object();
        std::string pos = pGramtab->GetPartOfSpeechStrLong(FormAndGrammems[saveFormNo].m_PartOfSpeech);
        if (commonGrammems > 0)
            pos += std::string(" ") + pGramtab->GrammemsToStr(commonGrammems);
        prdPart["pos"] = TrimCommaRight(pos);

        prdPart["formsGroups"] = nlohmann::json::array();
        for (auto fg : FormGroups)
        {
            auto subg = nlohmann::json::object();
            std::string grm = pGramtab->GrammemsToStr(fg.m_IntersectGrammems & ~commonGrammems);
            subg["grm"] = TrimCommaRight(grm);
            subg["forms"] = nlohmann::json::array();
            for (auto formNo : fg.m_FormNos)
            {
                auto &f = FormAndGrammems[formNo + saveFormNo];
                std::string grm = pGramtab->GrammemsToStr(f.m_Grammems & ~(fg.m_IntersectGrammems | commonGrammems));
                subg["forms"].push_back(
                        {{"f", convert_to_utf8(f.m_Form, Holder->m_CurrentLanguage)},
                         {"grm", TrimCommaRight(grm)}});
            };
            if (sortForms)
            {
                sort(subg["forms"].begin(), subg["forms"].end());
            }
            prdPart["formsGroups"].push_back(subg);
        };
        result.push_back(prdPart);
    };
    return result;
};


nlohmann::json GetStringByParadigmJson(const CFormInfo *piParadigm, const CMorphanHolder *Holder, bool withParadigm, bool sortForms)
{
    auto result = nlohmann::json::object();
    result["found"] = piParadigm->m_bFound;

    std::string typeAncode = piParadigm->GetCommonAncode();
    std::string commonGrammems;
    if (!typeAncode.empty())
    {
        const CAgramtab *pGramtab = Holder->m_pGramTab;
        try
        {
            commonGrammems = pGramtab->GrammemsToStr(pGramtab->GetAllGrammems(typeAncode.c_str()));
        }
        catch (...)
        {
            throw CExpc(" an exception occurred while getting Common Ancode");
        };
    };
    result["commonGrammems"] = TrimCommaRight(commonGrammems);
    result["wordForm"] = piParadigm->GetWordFormUtf8(0);
    if (!piParadigm->m_bFound)
    {
        result["srcNorm"] = piParadigm->GetSrcNormUtf8();
    }
    std::string GramInfo;
    try
    {
        GramInfo = piParadigm->GetSrcAncode();
    }
    catch (...)
    {
        GramInfo = piParadigm->GetAncode(0);
    }
    result["morphInfo"] = GetGramInfoStr(GramInfo, Holder);

    if (withParadigm)
    {
        result["paradigm"] = GetParadigmFromDictionary(piParadigm, Holder, sortForms);
    }
    result["wordWeight"] = piParadigm->GetWordWeight();
    result["homonymWeight"] = piParadigm->GetHomonymWeight();
    return result;
}

std::string CMorphanHolder::LemmatizeJson(std::string word_utf8, bool withParadigm, bool prettyJson, bool sortForms) const
{
    auto word = convert_from_utf8(word_utf8.c_str(), m_pLemmatizer->GetLanguage());
    std::vector<CFormInfo> Paradigms;
    if (!_GetParadigmCollection(word, Paradigms))
    {
        return "[]";
    };

    nlohmann::json result = nlohmann::json::array();
    std::string strResult = "[";
    for (auto& p : Paradigms)
    {
        result.push_back(GetStringByParadigmJson(&(p), this, withParadigm, sortForms));
    };
    return result.dump(prettyJson ? 1 : -1);
}

std::vector<CFuzzyResult> CMorphanHolder::CorrectMisspelledWordUtf8(std::string word_utf8) const {
    std::vector<CFuzzyResult> r;
    if (IsInDictionaryUtf8(word_utf8)) {
        r.push_back({ word_utf8, 0});
    }
    else {
        auto word_s8 = convert_from_utf8(word_utf8.c_str(), m_CurrentLanguage);
        r = m_pLemmatizer->CorrectMisspelledWord1(word_s8);
    }
    for (auto& a: r) {
        a.CorrectedString = convert_to_utf8(a.CorrectedString, m_CurrentLanguage);
    }
    return r;
}
