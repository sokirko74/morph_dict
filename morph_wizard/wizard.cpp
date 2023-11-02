#include "wizard.h"


#include "morph_dict/common/util_classes.h"
#include "morph_dict/agramtab/EngGramTab.h"
#include "morph_dict/agramtab/RusGramTab.h"
#include "morph_dict/agramtab/GerGramTab.h"
#include "morph_dict/common/json.h"

#include <fstream>
#include <sstream>
#include <regex>
#include <filesystem>



const char* AnyCommonAncode = " ";

std::string GetCurrentDate() {
    time_t ltime;
    time(&ltime);
    struct tm* today = localtime(&ltime);
    char tmpbuf[255];
    strftime(tmpbuf, 255, "%H:%M, %d %B %Y", today);
    return tmpbuf;
}

//==============================================================================


MorphoWizard::MorphoWizard(): 
    m_bLoaded(false), 
    m_Predictor(this),
    m_bWasChanged(false) {
    m_ReadOnly = true;
    m_bFullTrace = true;
    m_pGramTab = 0;
    m_pMeter = 0;
    m_Language = morphUnknown;
}

MorphoWizard::~MorphoWizard() {
    if (m_pGramTab) delete m_pGramTab;
}


//MRD_FILE 	L:\MORPH.windows\SOURCE\RUS_SRC\morphs.json
//LANG	        RUSSIAN
//USERS       user1, user2, user3


const size_t MaxMrdLineLength = 10240;

void MorphoWizard::load_gramtab(bool useNationalConstants) {
    CAgramtab* pGramTab;
    switch (m_Language) {
    case morphFioDisclosures:
    case morphRussian:
        pGramTab = new CRusGramTab;
        break;
    case morphEnglish:
        pGramTab = new CEngGramTab;
        break;
    case morphGerman:
        pGramTab = new CGerGramTab;
        break;
    default:
        throw CExpc("Unknown language to load gramtab: " + GetStringByLanguage(m_Language));
    };

    m_GramtabPath = pGramTab->ReadFromFolder(m_MwzFolder.string());
    pGramTab->SetUseNationalConstants(useNationalConstants);

    m_pGramTab = pGramTab;

    // read all poses from  m_pGramTab
    m_PosesList.clear();

    for (int i = 0; i < m_pGramTab->GetPartOfSpeechesCount(); i++)
        m_PosesList.push_back(m_pGramTab->GetPartOfSpeechStr(i));
    sort(m_PosesList.begin(), m_PosesList.end());

    // read all grammems from  m_pGramTab
    m_GrammemsList.clear();
    for (int i = 0; i < m_pGramTab->GetGrammemsCount(); i++) {
        m_GrammemsList.push_back(m_pGramTab->GetGrammemStr(i));
    };
    sort(m_GrammemsList.begin(), m_GrammemsList.end());

    // read all type grammems from  m_pGramTab
    m_TypeGrammemsList.clear();
    std::string CommonAncodes = m_pGramTab->GetAllPossibleAncodes(UnknownPartOfSpeech, 0);
    for (size_t i = 0; i < CommonAncodes.length(); i += 2) {
        uint64_t G;
        m_pGramTab->GetGrammems(CommonAncodes.c_str() + i, G);
        std::string q = m_pGramTab->GrammemsToStr(G);
        m_TypeGrammemsList.push_back(q);
    };
    sort(m_TypeGrammemsList.begin(), m_TypeGrammemsList.end());

    ancode_less.init(m_pGramTab);

};

uint16_t MorphoWizard::GetCurrentSessionNo() const {
    assert(m_SessionNo < (uint16_t)m_Sessions.size());
    return (uint16_t)m_SessionNo;
}

bool MorphoWizard::StartSession(std::string user_name) {
    CMorphSession S;
    S.m_UserName = user_name;
    S.m_SessionStart = GetCurrentDate();
    S.m_LastSessionSave = "no";
    m_Sessions.push_back(S);
    m_SessionNo = m_Sessions.size() - 1;
    LOGI << "opened by " << user_name;
    return true;
};

void MorphoWizard::EndSession() {
    assert(m_SessionNo < m_Sessions.size());
    m_Sessions[m_SessionNo].m_LastSessionSave = GetCurrentDate();
};

void MorphoWizard::StartLastSessionOfUser(std::string user_name) {
    if (GetUserName() == user_name) return;
    EndSession();
    for (int i = (int)m_Sessions.size() - 1; i >= 0; i--)
        if (m_Sessions[i].m_UserName == user_name) {
            m_SessionNo = i;
            return;
        };
    StartSession(user_name);
};



void MorphoWizard::load_wizard(std::string mwz_path, std::string user_name, bool bCreatePrediction, bool useNationalConstants) {
    m_MwzFolder = std::filesystem::absolute(mwz_path).parent_path();

    std::ifstream mwzFile(mwz_path);
    if (!mwzFile.is_open())
        throw CExpc("Cannot open file " + std::string(mwz_path));
    if (m_pGramTab) delete m_pGramTab;
    nlohmann::json jf = nlohmann::json::parse(mwzFile);
    bool guest = user_name == "guest";
    for (auto& el : jf.items()) {
        if (el.key() == "MRD_FILE") {
            m_MrdPath = el.value();
        }
        else if (el.key() == "LANG") {
            std::string lang = el.value();
            if (!GetLanguageByString(lang, m_Language))
                throw CExpc("Unknown language: " + lang);
        }
        else if (el.key() == "USERS") {
            if (!guest) {
                bool login = false;
                for (auto e : el.value()) {
                    if (e == user_name) {
                        login = true;
                        break;
                    }
                }
                if (!login) {
                    throw CExpc("Incorrect login!");
                }
            }
        }
    }
    load_gramtab(useNationalConstants);
    load_mrd_json(guest, bCreatePrediction);
    StartSession(user_name);
    m_bLoaded = true;
}


void MorphoWizard::check_paradigm(long line_no) {
    CFlexiaModel& p = m_FlexiaModels[line_no];
    try {
        for (size_t i = 0; i < p.m_Flexia.size(); i++)
            if (get_pos_string(p.m_Flexia[i].m_Gramcode).empty())
                goto error_label;
        return;
    }
    catch (...) {
    };
error_label:
    std::vector<lemma_iterator_t> found_paradigms;
    find_lemm_by_prdno(line_no, found_paradigms);
    if (found_paradigms.size() > 0)
        ErrorMessage(Format("Flexiamodel No %i has invalid gramcodes", line_no));


};

static size_t getCount(std::ifstream& mrdFile, const char* sectionName) {
    std::string line;
    if (!getline(mrdFile, line)) {
        throw CExpc("Cannot get size of section  %s", sectionName);
    }
    return atoi(line.c_str());
}


void MorphoWizard::ReadOnePrefixSet(std::string in, std::set<std::string>& out) const {
    RmlMakeUpper(in, m_Language);
    out.clear();
    for (auto p : split_string(in, ' ')) {
        Trim(p);
        if (!p.empty()) {
            if (!CheckLanguage(p, m_Language)) {
                throw CExpc ("bad language in prefix %s", p.c_str());
            }
            out.insert(p);
        }
    }
};


void MorphoWizard::load_mrd_json(bool guest, bool bCreatePrediction) {
    m_ReadOnly = guest;

    auto path = m_MwzFolder / m_MrdPath;
    std::cerr << "Reading mrd-file: " << path << "\n";
    std::ifstream mrdFile(path);
    if (!mrdFile.is_open())
        throw CExpc("Wrong mrd file : " + m_MrdPath);
    auto mrd = nlohmann::json::parse(mrdFile);
    ConvertFromUtfRecursive(mrd, m_Language);
    m_FlexiaModels.clear();
    for (auto f : mrd["flexia_models"]) {
        m_FlexiaModels.push_back(CFlexiaModel().FromJson(f));
    }
    m_AccentModels.clear();
    for (auto f : mrd["accent_models"]) {
        m_AccentModels.push_back(CAccentModel().FromJson(f));
    }

    m_Sessions.clear();
    for (auto f : mrd["sessions"]) {
        m_Sessions.push_back(CMorphSession().FromJson(f));
    }

    m_PrefixSets.clear();
    for (auto f : mrd["prefix_sets"]) {
        std::set<std::string> prefix_set;
        for (auto p : f) {
            prefix_set.insert(std::string(p));
        }
        m_PrefixSets.push_back(prefix_set);
    }

    for (auto f : mrd["lemmas"]) {
        CParadigmInfo p;
        p.FromJson(f);
        m_LemmaToParadigm.insert(std::make_pair(p.m_Lemma, p));
    }   
    LOGV << "loading done";
}



nlohmann::json MorphoWizard::GetSessionsJson() const {
    auto r = nlohmann::json::array();
    for (auto s : m_Sessions) {
        r.push_back(s.GetJson());
    }
    return r;
}


nlohmann::json MorphoWizard::GetPrefixSetsJson() const {
    auto r = nlohmann::json::array();
    for (auto s : m_PrefixSets) {
        r.push_back(s);
    }
    return r;
}

nlohmann::json MorphoWizard::GetLemmsJson() const {
    auto lemms = nlohmann::json::array();
    for (auto& [lemma, lemm_info] : m_LemmaToParadigm) {
        lemms.push_back(lemm_info.GetJson(lemma));
    }
    return lemms;
}


void MorphoWizard::save_mrd() {
    assert(m_bLoaded);
    auto path = m_MwzFolder / m_MrdPath;
    EndSession();
    std::ofstream outp(path, std::ios::binary);
    if (!outp.is_open()) {
        throw CExpc("Error while saving to file. It may be corrupted");
    }
    nlohmann::ordered_json mrd;
    mrd["flexia_models"] = GetFlexiaModelsJson();
    mrd["accent_models"] = GetAccentModelsJson();
    mrd["sessions"] = GetSessionsJson();
    mrd["prefix_sets"] = GetPrefixSetsJson();
    mrd["lemmas"] = GetLemmsJson();
    ConvertToUtfRecursive(mrd, m_Language);
    outp << mrd.dump(1);
    outp.close();
    m_bWasChanged = false;
    LOGI << "Saved by " << GetUserName();
};


void MorphoWizard::find_lemm_by_regex(std::string pattern, bool bCheckLemmaPrefix, std::vector<lemma_iterator_t>& res) {
    std::regex word_regex(pattern);
    for (lemma_iterator_t it = m_LemmaToParadigm.begin(); it != m_LemmaToParadigm.end(); it++) {
        if (std::regex_search(it->first, word_regex)) {
            res.push_back(it);
        }
        if (!!m_pMeter) m_pMeter->AddPos();
    }
}

void MorphoWizard::find_lemm(std::string lemm, bool bCheckLemmaPrefix, std::vector<lemma_iterator_t>& res) {
    if (!!m_pMeter) {
        m_pMeter->SetMaxPos((uint32_t)m_LemmaToParadigm.size());
        m_pMeter->SetInfo("Finding lemmas...");
    }

    // search a regular expression
    if ((lemm.length() > 2) && (lemm[0] == '/') && (lemm[lemm.length() - 1] == '/')) {
        try {
            find_lemm_by_regex(lemm.substr(1, lemm.length() - 2), bCheckLemmaPrefix, res);
        }
        catch (std::regex_error e) {
            ErrorMessage(e.what());
        }
        return;
    }	

    size_t pos_acc = lemm.rfind('\'');
    if (pos_acc != std::string::npos && pos_acc > 0) {
        lemm.erase(pos_acc, 1);
        --pos_acc;
    }

    size_t pos_ast = lemm.find("*");
    if (pos_ast == std::string::npos) {
        std::string Lemma = lemm;
        //  the user can specify a prefix with '|" for example the input can be "auf|machen",
        // where "auf" is a prefix
        size_t prefix_pos = lemm.find("|");
        std::string Prefix;
        if (prefix_pos != std::string::npos) {
            Prefix = Lemma.substr(0, prefix_pos);
            RmlMakeUpper(Prefix, m_Language);
            Lemma.erase(0, prefix_pos + 1);
        };
        std::pair<lemma_iterator_t, lemma_iterator_t> range = m_LemmaToParadigm.equal_range(Lemma);

        if (!!m_pMeter)
            m_pMeter->SetMaxPos(distance(range.first, range.second));

        for (lemma_iterator_t it = range.first; it != range.second; ++it) {
            if (pos_acc == std::string::npos || GetLemmaAccent(it) == pos_acc)  // РїСЂРѕРІРµСЂРєР° СѓРґР°СЂРµРЅРёСЏ [17/Dec/2003]
                if ((!Prefix.empty()
                    && (it->second.m_PrefixSetNo != UnknownPrefixSetNo)
                    && (m_PrefixSets[it->second.m_PrefixSetNo].find(Prefix) !=
                        m_PrefixSets[it->second.m_PrefixSetNo].end())
                    )
                    || (Prefix.empty()
                        && (!bCheckLemmaPrefix
                            || (it->second.m_PrefixSetNo == UnknownPrefixSetNo)
                            )
                        )
                    )
                    res.push_back(it);

            if (!!m_pMeter) m_pMeter->AddPos();
        }
        return;
    }
    // search with right truncation
    else if (pos_ast == lemm.size() - 1) {
        std::string s = lemm.substr(0, lemm.size() - 1);
        for (lemma_iterator_t it = m_LemmaToParadigm.begin(); it != m_LemmaToParadigm.end(); it++) {
            size_t fnd_pos = it->first.find(s);
            if (fnd_pos == 0)
                res.push_back(it);

            if (!!m_pMeter) m_pMeter->AddPos();
        }
        return;
    }
    // search with left truncation
    else if (pos_ast == 0) {
        std::string s = lemm.substr(1, lemm.size() - 1);
        for (lemma_iterator_t it = m_LemmaToParadigm.begin(); it != m_LemmaToParadigm.end(); it++) {
            size_t fnd_pos = it->first.rfind(s);
            if (fnd_pos != std::string::npos && fnd_pos == it->first.size() - s.size())
                res.push_back(it);

            if (!!m_pMeter) m_pMeter->AddPos();
        }
        return;
    }
    else {
        std::string s1 = lemm.substr(0, pos_ast);
        std::string s2 = lemm.substr(pos_ast + 1, lemm.size() - pos_ast + 1);
        for (lemma_iterator_t it = m_LemmaToParadigm.begin(); it != m_LemmaToParadigm.end(); it++) {
            size_t fnd_pos1 = it->first.find(s1);
            size_t fnd_pos2 = it->first.rfind(s2);
            if (fnd_pos1 == 0 && fnd_pos2 != std::string::npos && fnd_pos2 == it->first.size() - s2.size())
                res.push_back(it);

            if (!!m_pMeter) m_pMeter->AddPos();
        }
        return;
    }
}


bool IsLessBySession(const lemma_iterator_t& it1, const lemma_iterator_t& it2) {
    return it1->second.m_SessionNo < it2->second.m_SessionNo;
};

void MorphoWizard::find_lemm_by_user(std::string username, std::vector<lemma_iterator_t>& res) {
    res.clear();

    std::set<size_t> Sessions;

    for (size_t i = 0; i < m_Sessions.size(); i++)
        if (m_Sessions[i].m_UserName == username)
            Sessions.insert(i);

    for (lemma_iterator_t it = m_LemmaToParadigm.begin(); it != m_LemmaToParadigm.end(); it++)
        if (Sessions.find(it->second.m_SessionNo) != Sessions.end()) {
            res.push_back(it);
        };
    sort(res.begin(), res.end(), IsLessBySession);
};


//----------------------------------------------------------------------------
bool simple_match(const std::string& pattern, const std::string& word) {
    size_t l = pattern.length();
    if (l == 0) return false;

    if (l == 1)
        if (pattern[0] == '*')
            return false;


    if (pattern[0] == '*')
        return word.length() >= l - 1
        && !strcmp(word.c_str() + word.length() - l + 1, pattern.c_str() + 1);

    if (pattern[l - 1] == '*')
        return word.length() >= l - 1
        && !strncmp(word.c_str(), pattern.c_str(), l - 1);

    return pattern == word;

};

//----------------------------------------------------------------------------
// search a word form in all paradigms
//----------------------------------------------------------------------------
void MorphoWizard::find_wordforms(std::string wordform, std::vector<lemma_iterator_t>& res) {
    if (!!m_pMeter) {
        m_pMeter->SetMaxPos((uint32_t)m_LemmaToParadigm.size());
        m_pMeter->SetInfo("Finding wordforms...");
    }
    Trim(wordform);

    if (wordform.empty()) return;

    //  if a pure wordfom was given then transform it to the regular  expression syntax
    std::string pattern;
    if ((wordform[0] != '/')
        || (wordform[wordform.length() - 1] != '/')
        || (wordform.length() < 3)
        )
        pattern = std::string("^") + wordform + std::string("$");
    else
        pattern = wordform.substr(1, wordform.length() - 2);

    try {
        std::regex word_regex(pattern);

        StringVector wordforms;
        for (lemma_iterator_t it = m_LemmaToParadigm.begin(); it != m_LemmaToParadigm.end(); it++) {
            get_wordforms(it, wordforms);
            for (int i = 0; i < wordforms.size(); i++) {
                if (std::regex_search(wordforms[i], word_regex)) {
                    res.push_back(it);
                    break;
                }
            }
            if (!!m_pMeter) m_pMeter->AddPos();
        }
    }
    catch (std::regex_error r) {
        ErrorMessage(r.what());
    }
}


//----------------------------------------------------------------------------
// search an ancode std::string in all paradigms (an ancode std::string can contain more than one ancode)
//----------------------------------------------------------------------------
void MorphoWizard::find_ancodes(const std::string& ancodes, std::vector<lemma_iterator_t>& res) {
    if (!!m_pMeter) {
        m_pMeter->SetMaxPos((uint32_t)m_LemmaToParadigm.size());
        m_pMeter->SetInfo("Finding ancodes...");
    }

    std::vector<uint16_t> prdno;

    for (size_t i = 0; i < m_FlexiaModels.size(); i++)
        for (size_t l = 0; l < ancodes.size(); l += 2)
            if (m_FlexiaModels[i].has_ancode(ancodes.substr(l, 2)))
                prdno.push_back((uint16_t)i);

    sort(prdno.begin(), prdno.end());

    for (lemma_iterator_t i2 = m_LemmaToParadigm.begin(); i2 != m_LemmaToParadigm.end(); i2++) {
        uint16_t pno = i2->second.m_FlexiaModelNo;
        if (binary_search(prdno.begin(), prdno.end(), pno))
            res.push_back(i2);

        if (!!m_pMeter) m_pMeter->AddPos();
    }
}

//----------------------------------------------------------------------------
void MorphoWizard::find_lemm_by_grammem(const std::string& pos_and_grammems, std::vector<lemma_iterator_t>& res) {
    BYTE pos;
    uint64_t gra;
    /*{	// processing type grammems for example "РЎ Р»РѕРє | РѕРґ,
        int u = pos_and_grammems.find("|");
        if (u != std::string::npos)
        {
            std::string q = pos_and_grammems.substr(0, u-1);
            Trim(q);
            u = q.find(" ");
            if (u != std::string::npos)
                q.erase(0, u);
            q = "*" + q;
            if (m_pGramTab->ProcessPOSAndGrammemsIfCan(q.c_str(), &pos, &gra))
            {
                throw CExpc("Wrong type grammem");
            };
        };
    };
    StringTokenizer R (pos_and_grammems.c_str(), "|");*/

    if (!m_pGramTab->ProcessPOSAndGrammemsIfCan(pos_and_grammems.c_str(), &pos, &gra)
        ) {
        throw CExpc("Wrong grammem");
    }

    std::string _codes = m_pGramTab->GetAllPossibleAncodes(pos, gra);

    if (_codes.empty()) throw CExpc("Cannot find ancode by this morphological pattern");

    find_ancodes(_codes, res);
}

//----------------------------------------------------------------------------
void MorphoWizard::find_lemm_by_accent_model(int AccentModelNo, std::vector<lemma_iterator_t>& res) {
    if (!!m_pMeter) {
        m_pMeter->SetMaxPos((uint32_t)m_LemmaToParadigm.size());
        m_pMeter->SetInfo("Finding lemmas...");
    }
    std::set<uint16_t> Models;
    if (AccentModelNo == -1) {
        for (size_t k = 0; k < m_AccentModels.size(); k++)
            if (find(m_AccentModels[k].m_Accents.begin(), m_AccentModels[k].m_Accents.end(), UnknownAccent) !=
                m_AccentModels[k].m_Accents.end())
                Models.insert((uint16_t)k);
    }
    else
        Models.insert(AccentModelNo);

    for (lemma_iterator_t it = m_LemmaToParadigm.begin(); it != m_LemmaToParadigm.end(); it++) {
        if (Models.find(it->second.m_AccentModelNo) != Models.end())
            res.push_back(it);

        if (!!m_pMeter) m_pMeter->AddPos();
    }
}


//----------------------------------------------------------------------------
void MorphoWizard::find_lemm_by_prdno(uint16_t prdno, std::vector<lemma_iterator_t>& res) {
    if (!!m_pMeter) {
        m_pMeter->SetMaxPos((uint32_t)m_LemmaToParadigm.size());
        m_pMeter->SetInfo("Finding lemmas...");
    }

    for (lemma_iterator_t it = m_LemmaToParadigm.begin(); it != m_LemmaToParadigm.end(); it++) {
        if (it->second.m_FlexiaModelNo == prdno)
            res.push_back(it);

        if (!!m_pMeter) m_pMeter->AddPos();
    }
}

//----------------------------------------------------------------------------
void MorphoWizard::find_lemm_by_prd_info(const CParadigmInfo& info, std::vector<lemma_iterator_t>& res) {
    if (!!m_pMeter) {
        m_pMeter->SetMaxPos((uint32_t)m_LemmaToParadigm.size());
        m_pMeter->SetInfo("Finding lemmas...");
    }

    for (lemma_iterator_t it = m_LemmaToParadigm.begin(); it != m_LemmaToParadigm.end(); it++) {
        if (info.IsAnyEqual(it->second))
            res.push_back(it);

        if (!!m_pMeter) m_pMeter->AddPos();
    }
}

//----------------------------------------------------------------------------
std::string MorphoWizard::get_pos_string(const std::string& code) const {
    return m_pGramTab->GetPartOfSpeechStr(m_pGramTab->GetPartOfSpeech(code.c_str()));
}

//----------------------------------------------------------------------------
std::string MorphoWizard::get_pos_string(const lemma_iterator_t it) const {
    return get_pos_string(m_FlexiaModels[it->second.m_FlexiaModelNo].get_first_code());
}

//----------------------------------------------------------------------------
std::string MorphoWizard::get_grammem_string(const std::string& code) const {
    std::string res;
    for (int i = 0; i < code.size(); i += 2) {
        if (i) res += ";";
        uint64_t grams;
        m_pGramTab->GetGrammems(code.substr(i, 2).c_str(), grams);
        res += m_pGramTab->GrammemsToStr(grams);

    }
    return res;
}

//----------------------------------------------------------------------------
std::string MorphoWizard::get_pos_string_and_grammems(const std::string& code) const {
    return get_pos_string(code) + " " + get_grammem_string(code);
}

//----------------------------------------------------------------------------
std::string MorphoWizard::get_common_grammems_string(const_lemma_iterator_t it) const {
    std::string s = it->second.GetCommonAncodeIfCan();
    if (s.empty()) return "";

    uint64_t grams;
    m_pGramTab->GetGrammems(s.c_str(), grams);
    return m_pGramTab->GrammemsToStr(grams);
}

//----------------------------------------------------------------------------
//  get union of type and form grammems of the input lemma
uint64_t MorphoWizard::get_all_lemma_grammems(const_lemma_iterator_t it) const {
    uint64_t grams = 0;
    std::string s = it->second.GetCommonAncodeIfCan();
    if (!s.empty())
        grams = m_pGramTab->GetAllGrammems(s.c_str());


    s = m_FlexiaModels[it->second.m_FlexiaModelNo].get_first_code();
    if (!s.empty())
        grams |= m_pGramTab->GetAllGrammems(s.c_str());

    return grams;
}


//----------------------------------------------------------------------------
std::string MorphoWizard::get_grammem_string(lemma_iterator_t it) const {
    return get_grammem_string(m_FlexiaModels[it->second.m_FlexiaModelNo].get_first_code());

}

//----------------------------------------------------------------------------
std::string MorphoWizard::get_lemm_string(const_lemma_iterator_t it) const {
    return it->first;
}

//----------------------------------------------------------------------------
std::string MorphoWizard::get_lemm_string_with_accents(const_lemma_iterator_t it) const {
    std::string form = it->first;
    RmlMakeLower(form, m_Language);
    SetAccent(it->second.m_AccentModelNo, it->second.m_AuxAccent, 0, form);
    return form;
}

//----------------------------------------------------------------------------
std::string MorphoWizard::get_base_string(const_lemma_iterator_t it) const {
    const CFlexiaModel& p = m_FlexiaModels[it->second.m_FlexiaModelNo];
    std::string flex = p.get_first_flex();
    std::string lemm = it->first;
    std::string base = lemm.substr(0, lemm.size() - flex.size());
    return base;
}

const CMorphSession& MorphoWizard::get_session(int SessionNo) const {
    return m_Sessions[SessionNo];
}


//----------------------------------------------------------------------------
void MorphoWizard::remove_lemm(lemma_iterator_t it) {
    uint16_t paradigm_num = it->second.m_FlexiaModelNo;
    CFlexiaModel& p = m_FlexiaModels[paradigm_num];
    log_lemma(it->first, p, false);
    m_LemmaToParadigm.erase(it);
}

std::string MorphoWizard::get_prefix_set_str(uint16_t PrefixSetNo) const {
    std::string Result;
    const std::set<std::string>& PS = m_PrefixSets[PrefixSetNo];
    assert(!PS.empty());
    if (PS.empty()) return "";

    for (std::set<std::string>::const_iterator it = PS.begin(); it != PS.end(); it++) {
        Result += *it;
        Result += ",";
    };
    Result.erase(Result.length() - 1);
    return Result;
};

std::string MorphoWizard::get_prefix_set(CParadigmInfo i) const {
    return (i.m_PrefixSetNo == UnknownPrefixSetNo) ? "" : get_prefix_set_str(i.m_PrefixSetNo);
}

std::string MorphoWizard::get_prefix_set(const_lemma_iterator_t it) const {
    return get_prefix_set(it->second);
}


std::string MorphoWizard::get_slf_string(lemma_iterator_t it, std::string& common_grammems, std::string& Prefixes, int line_size) {
    const CParadigmInfo& I = it->second;
    const CFlexiaModel& P = m_FlexiaModels[I.m_FlexiaModelNo];
    Prefixes = get_prefix_set(it);
    common_grammems = get_grammem_string(I.GetCommonAncodeIfCan());
    return mrd_to_slf(it->first, P, I.m_AccentModelNo, I.m_AuxAccent, line_size);
}


BYTE TransferReverseVowelNoToCharNo(const std::string& form, BYTE AccentCharNo, MorphLanguageEnum Language) {
    if (AccentCharNo == UnknownAccent) return UnknownAccent;

    assert(AccentCharNo < form.length());
    int CountOfVowels = -1;
    int i = (int)form.length() - 1;
    assert(i < UnknownAccent);
    for (; i >= 0; i--) {
        if (is_lower_vowel((BYTE)form[i], Language)
            || is_upper_vowel((BYTE)form[i], Language)
            )
            CountOfVowels++;

        if (CountOfVowels == AccentCharNo) {
            return i;
        };
    }
    //	assert (false);
    return UnknownAccent;
};


void MorphoWizard::SetAccent(uint16_t AccentModelNo, BYTE AuxAccent, int FormNo, std::string& form) const {
    if (AccentModelNo == UnknownAccentModelNo) return;
    assert(FormNo < m_AccentModels[AccentModelNo].m_Accents.size());
    int u = TransferReverseVowelNoToCharNo(form, m_AccentModels[AccentModelNo].m_Accents[FormNo], m_Language);
    if (u != UnknownAccent) {
        form.insert(u + 1, "'");
    };
    if (AuxAccent != UnknownAccent) {
        assert(AccentModelNo != UnknownAccentModelNo);
        // in  some forms auxiliary and main accents can be the same
        if (form[AuxAccent + 1] != '\'')
            form.insert(AuxAccent + 1, "'");

    };
};


std::string MorphoWizard::mrd_to_slf(const std::string& lemm, const CFlexiaModel& p, uint16_t AccentModelNo, BYTE AuxAccent,
    int line_size) const {
    std::string res;
    std::string base;
    std::string lem_code;
    for (size_t n = 0; n < p.m_Flexia.size(); n++) {
        std::string prefix = p.m_Flexia[n].m_PrefixStr;
        if (!prefix.empty()) prefix += "|";
        std::string flex = p.m_Flexia[n].m_FlexiaStr;
        std::string code = p.m_Flexia[n].m_Gramcode;
        if (!n) base = lemm.substr(0, lemm.size() - flex.size());
        if (code.size() % 2 != 0) throw CExpc("Wrong gramm code");
        std::string form = prefix + base + flex;

        RmlMakeLower(form, m_Language);
        SetAccent(AccentModelNo, AuxAccent, (int)n, form);


        for (int i = 0; i < code.size(); i += 2) {
            std::string gramcode = code.substr(i, 2);
            std::string grammems = get_pos_string_and_grammems(gramcode);

            //  adding word form
            res += form;
            res += " ";
            int n_spaces = (int)(line_size - form.size() - grammems.size() - 1);
            while (n_spaces-- >= 0) res += " ";

            //  adding morphological information

            res += grammems;
            res += "\r\n";
        };
    }

    return res;
}

void MorphoWizard::get_wordforms(const_lemma_iterator_t it, StringVector& forms) const {
    const CFlexiaModel& p = m_FlexiaModels[it->second.m_FlexiaModelNo];
    const std::string& lemm = it->first;
    std::string base;
    forms.clear();
    for (size_t n = 0; n < p.m_Flexia.size(); n++) {
        std::string flex = p.m_Flexia[n].m_FlexiaStr;
        if (!n)
            base = lemm.substr(0, lemm.size() - flex.size());
        forms.push_back(base + flex);
    }
}


bool MorphoWizard::slf2ancode(const std::string slf_line, std::string& gramcode) const {
    BYTE pos;
    uint64_t gra;

    if (!m_pGramTab->ProcessPOSAndGrammemsIfCan(slf_line.c_str(), &pos, &gra)
        || !m_pGramTab->GetGramCodeByGrammemsAndPartofSpeechIfCan(pos, gra, gramcode)
        )
        return false;
    return true;
};


struct CSlfLineByAncode {
    std::string m_Form;
    BYTE m_AccentByte;
    std::string m_Prefix;
};


//----------------------------------------------------------------------------
// при добавлении парадигмы производится сортировка сначала по граммемам, 
//		при совпадающих граммемах - по формам,
//		при совпадающих формах - по ударениям
// изменено Кецарисом [12/Apr/2004]
//----------------------------------------------------------------------------
struct CSlfLineByAncodeLess {
    bool operator()(const CSlfLineByAncode& s1, const CSlfLineByAncode& s2) const {
        int c = s1.m_Form.compare(s2.m_Form);
        if (c != 0) return c < 0;
        c = s1.m_Prefix.compare(s2.m_Prefix);
        if (c != 0) return c < 0;
        return s1.m_AccentByte < s2.m_AccentByte;
    }
};

void MorphoWizard::slf_to_mrd(const std::string& s, std::string& lemm, CFlexiaModel& FlexiaModel, CAccentModel& AccentModel,
    BYTE& AuxAccent, int& line_no_err) const {

    StringTokenizer lines(s.c_str(), "\r\n");

    std::string lemm_gramcode;
    typedef std::set<CSlfLineByAncode, CSlfLineByAncodeLess> SlfLineSet;
    typedef std::map<std::string, SlfLineSet, AncodeLess> Gramcode2SlfLineMap;
    Gramcode2SlfLineMap Paradigm(ancode_less);
    size_t CommonLeftPartSize = lemm.size();
    BYTE LemmaAccentByte;
    AuxAccent = UnknownAccent;

    StringSet TestForFullDublicates;

    //  going through all lines of slf-representation,
    // building all pairs <wordform, gramcode>
    line_no_err = 0;
    int start = 0;
    do {
        line_no_err++;

        std::string line;
        {
            size_t end = s.find("\n", start);
            if (end == std::string::npos) {
                // no last eoln
                line = s.substr(start);
                start = (int)s.length();
            }
            else {
                line = s.substr(start, end - start);
                start = (int)end + 1;
            }
        }

        Trim(line);
        if (line.empty()) continue;

        if (TestForFullDublicates.find(line) == TestForFullDublicates.end())
            TestForFullDublicates.insert(line);
        else
            continue;

        std::string form;
        StringTokenizer tok(line.c_str(), "\t \r");
        if (!tok()) throw CExpc("Error! Cannot find a word form");
        form = tok.val();
        if (form.empty()) throw CExpc("Error! Empty word form");

        std::string pos_and_grammems;
        if (!tok()) throw CExpc("Error! Cannot find part of speech");
        pos_and_grammems = tok.val();
        if (tok()) pos_and_grammems += std::string(" ") + tok.val();

        if (pos_and_grammems.empty())
            throw CExpc("Error! No morphological annotation");

        if (tok())
            throw CExpc("Error! Unparsed chars at the end of the line");

        std::string gramcode;
        if (!slf2ancode(pos_and_grammems, gramcode))
            throw CExpc("Error! Wrong morphological annotation(%s)", pos_and_grammems.c_str());


        BYTE AccentByte = UnknownAccent;
        size_t CountOfAccents = 0;
        size_t CountOfVowels = 0;

        for (int k = (int)form.length() - 1; k >= 0; k--) {
            if (is_lower_vowel((BYTE)form[k], m_Language))
                CountOfVowels++;

            if (form[k] == '\'') {
                CountOfAccents++;
                if (CountOfAccents > 2)
                    throw CExpc("Error! Too many stresses!");
                if ((k == 0)
                    || !is_lower_vowel((BYTE)form[k - 1], m_Language)
                    )
                    throw CExpc("A stress should be put on a vowel ");
                //  we should determine the auxiliary accent, which is permanent for all word forms that's  why
                // we can read it from the first line, but it should differ from the main accent
                // So it should be something like this "aaaAaaaMaaa", where A is the auxiliary
                //  stress, and M  is the main stress.
                if (AccentByte == UnknownAccent)
                    AccentByte = (BYTE)CountOfVowels;
                else {
                    if (AuxAccent != UnknownAccent) {
                        if (AuxAccent != k - 1)
                            throw CExpc("Auxiliary stress should be on the same position");
                    }
                    else
                        AuxAccent = k - 1;
                };
                form.erase(k, 1);
            };

        };


        RmlMakeUpper(form, m_Language);
        std::string Prefix;
        size_t PrefixInd = form.find("|");
        if (PrefixInd != std::string::npos) {
            Prefix = form.substr(0, PrefixInd);
            form.erase(0, PrefixInd + 1);
        };


        if (line_no_err == 1) {
            lemm = form;
            lemm_gramcode = gramcode;
            LemmaAccentByte = AccentByte;
            CommonLeftPartSize = form.length();
        }
        else {
            CSlfLineByAncode Line;
            Line.m_AccentByte = AccentByte;
            Line.m_Form = form;
            Line.m_Prefix = Prefix;

            SlfLineSet slfset;
            slfset.insert(Line);
            std::pair<Gramcode2SlfLineMap::iterator, bool> p = Paradigm.insert(std::make_pair(gramcode, slfset));
            if (!p.second) {
                p.first->second.insert(Line);
            }

            //  calculating the common left part  of  all wordforms
            size_t i = 0;
            for (; i < std::min(CommonLeftPartSize, form.length()); i++)
                if (form[i] != lemm[i])
                    break;

            CommonLeftPartSize = i;
        };
    } while (start < s.length());


    if (lemm.empty())
        throw CExpc("Error! Empty paradigm");

    FlexiaModel.m_Flexia.clear();
    AccentModel.m_Accents.clear();

    //  adding lemma, it should be always at the first position
    FlexiaModel.m_Flexia.push_back(CMorphForm(lemm_gramcode, lemm.substr(CommonLeftPartSize), ""));
    AccentModel.m_Accents.push_back(LemmaAccentByte);

    //  adding the rest paradigm ordered by ancode
    for (Gramcode2SlfLineMap::const_iterator pit = Paradigm.begin(); pit != Paradigm.end(); pit++) {
        const SlfLineSet& slf_set = pit->second;
        std::string Gramcode = pit->first;
        for (SlfLineSet::const_iterator it = slf_set.begin(); it != slf_set.end(); it++) {
            std::string Flexia = it->m_Form.substr(CommonLeftPartSize);
            BYTE AccentByte = it->m_AccentByte;
            std::string Prefix = it->m_Prefix;
            FlexiaModel.m_Flexia.push_back(CMorphForm(Gramcode, Flexia, Prefix));
            AccentModel.m_Accents.push_back(AccentByte);
        }
    };

}

void MorphoWizard::AncodeLess::init(const CAgramtab* pGramTab) {
    m_pGramTab = pGramTab;
}

bool MorphoWizard::AncodeLess::operator()(const std::string& s1, const std::string& s2) const {
    return m_pGramTab->GetSourceLineNo(s1.c_str()) < m_pGramTab->GetSourceLineNo(s2.c_str());
}

std::string MorphoWizard::GetUserName() const {
    if (m_Sessions.empty())
        return "guest";
    else
        return m_Sessions.back().m_UserName;
};


void MorphoWizard::log_lemma(const std::string& lemm, const CFlexiaModel& p, bool is_added) const {
    if (!m_bFullTrace) return;
    LOGI << (is_added ? "+" : "-")  << " " << lemm << " " << p.ToJson().dump();
}

uint16_t AddAccentModel(MorphoWizard& C, const CAccentModel& AccentModel) {
    uint16_t AccentModelNo = UnknownAccentModelNo;
    if (!AccentModel.m_Accents.empty()) {
        std::vector<CAccentModel>::iterator accent_it = find(C.m_AccentModels.begin(), C.m_AccentModels.end(), AccentModel);
        if (accent_it == C.m_AccentModels.end()) {
            //  a new accent model should be added
            AccentModelNo = (uint16_t)C.m_AccentModels.size();
            if (AccentModelNo == UnknownAccentModelNo)
                throw CExpc("Too many accent models");

            C.m_AccentModels.push_back(AccentModel);
        }
        else {
            AccentModelNo = accent_it - C.m_AccentModels.begin();
        }
    };
    return AccentModelNo;
};

uint16_t AddFlexiaModel(MorphoWizard& C, const CFlexiaModel& FlexiaModel) {
    uint16_t ParadigmNo;
    // finding Paradigm No
    std::vector<CFlexiaModel>::iterator pit = find(C.m_FlexiaModels.begin(), C.m_FlexiaModels.end(), FlexiaModel);

    if (pit == C.m_FlexiaModels.end()) {
        //  a new paradigm should be added
        ParadigmNo = (uint16_t)C.m_FlexiaModels.size();
        if (ParadigmNo == 0xffff)
            throw CExpc("Too many paradigms");

        C.m_FlexiaModels.push_back(FlexiaModel);
    }
    else {
        ParadigmNo = pit - C.m_FlexiaModels.begin();
    }
    return ParadigmNo;
};


uint16_t MorphoWizard::AddPrefixSet(std::string PrefixSetStr) {
    Trim(PrefixSetStr);

    if (PrefixSetStr.empty())
        return UnknownPrefixSetNo;

    std::set<std::string> PrefixSet;
    ReadOnePrefixSet(PrefixSetStr, PrefixSet);

    if (PrefixSet.empty())
        throw CExpc("Cannot add empty prefix set");


    uint16_t Result;
    std::vector<std::set<std::string> >::iterator pit = find(m_PrefixSets.begin(), m_PrefixSets.end(), PrefixSet);
    if (pit == m_PrefixSets.end()) {
        //  a new prefix std::set should be added
        Result = (uint16_t)m_PrefixSets.size();
        if (Result == 0xffff)
            throw CExpc("Too many prefix sets");

        m_PrefixSets.push_back(PrefixSet);
    }
    else {
        Result = pit - m_PrefixSets.begin();
    }

    return Result;
}

CParadigmInfo
MorphoWizard::add_lemma(const std::string& slf, std::string common_grammems, const std::string& prefixes, int& line_no_err,
    uint16_t SessionNo) {
    std::string lemm;
    CFlexiaModel FlexiaModel;
    CAccentModel AccentModel;
    BYTE AuxAccent;
    slf_to_mrd(slf, lemm, FlexiaModel, AccentModel, AuxAccent, line_no_err);

    std::string common_gramcode;
    if (!common_grammems.empty())
        if (!slf2ancode("* " + common_grammems, common_gramcode))
            throw CExpc(Format("Wrong common grammems  \"%s\"", common_grammems.c_str()));


    uint16_t ParadigmNo = AddFlexiaModel(*this, FlexiaModel);
    uint16_t AccentModelNo = AddAccentModel(*this, AccentModel);
    uint16_t PrefixSetNo = AddPrefixSet(prefixes);

    if (SessionNo == UnknownSessionNo)
        SessionNo = GetCurrentSessionNo();

    CParadigmInfo NewInfo(ParadigmNo, AccentModelNo, SessionNo, AuxAccent, common_gramcode.c_str(), PrefixSetNo);
    m_LemmaToParadigm.insert(std::make_pair(lemm, NewInfo));
    log_lemma(lemm, FlexiaModel, true);
    m_bWasChanged = true;
    return NewInfo;
}

//----------------------------------------------------------------------------
void MorphoWizard::set_to_delete_false() {
    for (lemma_iterator_t i1 = m_LemmaToParadigm.begin(); i1 != m_LemmaToParadigm.end(); ++i1) {
        i1->second.m_bToDelete = false;
    }
}

//----------------------------------------------------------------------------
void MorphoWizard::delete_checked_lemms() {
    lemma_iterator_t i1 = m_LemmaToParadigm.begin();
    while (i1 != m_LemmaToParadigm.end())
        if (i1->second.m_bToDelete) {
            m_LemmaToParadigm.erase(i1);
            i1 = m_LemmaToParadigm.begin();
        }
        else
            i1++;

    m_bWasChanged = true;
};

//----------------------------------------------------------------------------
// del_dup_lemm deletes all equal lemmas with the same flexia and accent model
//----------------------------------------------------------------------------
size_t MorphoWizard::del_dup_lemm() {
    size_t num = 0;
    lemma_iterator_t i1, i2;
    i1 = m_LemmaToParadigm.begin();

AGAIN:
    while (i1 != m_LemmaToParadigm.end()) {
        i2 = i1;
        i2++;
        while (i2 != m_LemmaToParadigm.end()) {
            if (i1->first != i2->first)
                break;
            if (i1->second == i2->second) {
                std::string dbg_str = i2->first;
                uint16_t dbg_num = i2->second.m_FlexiaModelNo;
                m_LemmaToParadigm.erase(i2);
                num++;
                i1 = m_LemmaToParadigm.begin();
                goto AGAIN;
            }
            i2++;
        }
        i1++;
    }

    if (num)
        m_bWasChanged = true;

    return num;
}



void MorphoWizard::pack() {
    std::map<int, int> OldFlexiaModelsToNewFlexiaModels;
    std::map<int, int> OldAccentModelsToNewAccentModels;
    std::map<int, int> OldPrefixSetsToNewPrefixSets;

    if (HasMeter()) {
        GetMeter()->SetInfo("Packing paradigms...");
        GetMeter()->SetMaxPos((uint32_t)(m_LemmaToParadigm.size() + m_LemmaToParadigm.size()) / 4 * 2);
    }

    {
        LOGD << "finding all used flexia and accent modleys";
        std::set<uint16_t> UsedFlexiaModels;
        std::set<uint16_t> UsedAccentModels;
        std::set<uint16_t> UsedPrefixSets;
        for (lemma_iterator_t lemm_it = m_LemmaToParadigm.begin(); lemm_it != m_LemmaToParadigm.end(); lemm_it++) {
            UsedFlexiaModels.insert(lemm_it->second.m_FlexiaModelNo);
            UsedAccentModels.insert(lemm_it->second.m_AccentModelNo);
            UsedPrefixSets.insert(lemm_it->second.m_PrefixSetNo);
        };


        LOGD << "creating new flexia models without unused items";
        std::vector<CFlexiaModel> NewParadigms;
        for (size_t i = 0; i < m_FlexiaModels.size(); i++)
            if (UsedFlexiaModels.find((uint16_t)i) != UsedFlexiaModels.end()) {

                size_t j = 0;

                for (; j < NewParadigms.size(); j++)
                    if (m_FlexiaModels[i] == NewParadigms[j])
                        break;

                if (j == NewParadigms.size()) {
                    NewParadigms.push_back(m_FlexiaModels[i]);
                    OldFlexiaModelsToNewFlexiaModels[(int)i] = (int)NewParadigms.size() - 1;
                }
                else // equal paradigm is already in the list
                {
                    OldFlexiaModelsToNewFlexiaModels[(int)i] = (int)j;
                };
            }
        m_FlexiaModels = NewParadigms;

        LOGD << "creating new accent models without unused items";
        std::vector<CAccentModel> NewAccentModels;
        for (size_t k = 0; k < m_AccentModels.size(); k++)
            if (UsedAccentModels.find((uint16_t)k) != UsedAccentModels.end()) {
                NewAccentModels.push_back(m_AccentModels[k]);
                OldAccentModelsToNewAccentModels[(int)k] = (int)NewAccentModels.size() - 1;
            }
        m_AccentModels = NewAccentModels;

        LOGD << "creating new prefix sets";
        std::vector<std::set<std::string> > NewPrefixSets;
        for (size_t i = 0; i < m_PrefixSets.size(); i++)
            if (UsedPrefixSets.find((uint16_t)i) != UsedPrefixSets.end()) {
                NewPrefixSets.push_back(m_PrefixSets[i]);
                OldPrefixSetsToNewPrefixSets[(int)i] = (int)NewPrefixSets.size() - 1;
            }
        m_PrefixSets = NewPrefixSets;

        if (HasMeter()) GetMeter()->SetPos((uint32_t)m_LemmaToParadigm.size() / 4);
    }

    LOGD << "fixing index from lemmas to paradigms";
    LemmaMap NewLemmaToParadigm;
    for (lemma_iterator_t lemm_it = m_LemmaToParadigm.begin(); lemm_it != m_LemmaToParadigm.end(); lemm_it++) {
        std::map<int, int>::const_iterator flex_it = OldFlexiaModelsToNewFlexiaModels.find(lemm_it->second.m_FlexiaModelNo);
        assert(flex_it != OldFlexiaModelsToNewFlexiaModels.end());

        uint16_t AccentModelNo = lemm_it->second.m_AccentModelNo;
        if (AccentModelNo != UnknownAccentModelNo) {
            std::map<int, int>::const_iterator accent_it = OldAccentModelsToNewAccentModels.find(
                lemm_it->second.m_AccentModelNo);
            assert(accent_it != OldAccentModelsToNewAccentModels.end());
            AccentModelNo = accent_it->second;
        };

        uint16_t PrefixSetNo = lemm_it->second.m_PrefixSetNo;
        if (PrefixSetNo != UnknownPrefixSetNo) {
            std::map<int, int>::const_iterator prefix_set_it = OldPrefixSetsToNewPrefixSets.find(PrefixSetNo);
            assert(prefix_set_it != OldPrefixSetsToNewPrefixSets.end());
            PrefixSetNo = prefix_set_it->second;
        }


        CParadigmInfo NewInfo(flex_it->second,
            AccentModelNo,
            lemm_it->second.m_SessionNo,
            lemm_it->second.m_AuxAccent,
            lemm_it->second.m_CommonAncode,
            PrefixSetNo);

        NewLemmaToParadigm.insert(std::make_pair(lemm_it->first, NewInfo));

        if (HasMeter()) GetMeter()->AddPos();
    }
    m_LemmaToParadigm = NewLemmaToParadigm;


    del_dup_lemm();

    if (HasMeter())
        if (m_LemmaToParadigm.size() > 4)
            GetMeter()->AddPos((uint32_t)m_LemmaToParadigm.size() / 4);


    m_Predictor.CreateIndex();


    m_bWasChanged = true;

};

//----------------------------------------------------------------------------
bool MorphoWizard::change_prd_info(CParadigmInfo& I, const std::string& Lemma,
    uint16_t NewFlexiaModelNo, uint16_t newAccentModelNo, bool keepOldAccents) {
    if (NewFlexiaModelNo >= m_FlexiaModels.size()
        || (newAccentModelNo >= m_AccentModels.size()
            && newAccentModelNo != UnknownAccentModelNo
            )
        || (NewFlexiaModelNo == I.m_FlexiaModelNo
            && newAccentModelNo == I.m_AccentModelNo
            )
        )
        return false;


    if ((newAccentModelNo == UnknownAccentModelNo && !keepOldAccents)
        || (I.m_FlexiaModelNo == UnknownParadigmNo)
        )
        I.m_AccentModelNo = UnknownAccentModelNo;
    else {
        /*
        if there is an old accent model, we can build a new accent model from the old one.
        We go through the new flexia model and for each flexia and gramcode we
        search for the same word form and gramcode in the old flexia model. If the search
        is a success, then we transfer accent from the old wordform to the new one.
        */
        std::string OldBase = Lemma;
        const std::vector<CMorphForm>& OldFlexia = m_FlexiaModels[I.m_FlexiaModelNo].m_Flexia;
        OldBase.erase(OldBase.length() - OldFlexia[0].m_FlexiaStr.length());


        std::string NewBase = Lemma;
        const std::vector<CMorphForm>& NewFlexia = m_FlexiaModels[NewFlexiaModelNo].m_Flexia;
        NewBase.erase(NewBase.length() - NewFlexia[0].m_FlexiaStr.length());

        CAccentModel NewAccents;

        for (size_t i = 0; i < NewFlexia.size(); i++) {
            std::string NewWordForm = NewBase + NewFlexia[i].m_FlexiaStr;
            size_t k = 0;
            for (; k < OldFlexia.size(); k++)
                if ((OldBase + OldFlexia[k].m_FlexiaStr == NewWordForm)
                    && (OldFlexia[k].m_Gramcode == NewFlexia[i].m_Gramcode)
                    )
                    break;


            int accOld = _GetReverseVowelNo(NewWordForm, I.m_AccentModelNo, (uint16_t)k);
            int accNew = _GetReverseVowelNo(NewWordForm, newAccentModelNo, (uint16_t)i);
            int acc;
            if (keepOldAccents)
                acc = (accOld == UnknownAccent ? accNew : accOld);
            else
                acc = (accNew == UnknownAccent ? accOld : accNew);

            NewAccents.m_Accents.push_back(acc);
        }

        I.m_AccentModelNo = AddAccentModel(*this, NewAccents);
    }

    I.m_FlexiaModelNo = NewFlexiaModelNo;
    I.m_SessionNo = GetCurrentSessionNo();
    return true;
}



std::string MorphoWizard::show_differences_in_two_paradigms(uint16_t FlexiaModelNo1, uint16_t FlexiaModelNo2) const {
    std::string s1 = mrd_to_slf("-", m_FlexiaModels[FlexiaModelNo1], UnknownAccentModelNo, UnknownAccent, 79);
    std::string s2 = mrd_to_slf("-", m_FlexiaModels[FlexiaModelNo2], UnknownAccentModelNo, UnknownAccent, 79);

    StringVector V1, V2;

    StringTokenizer t1(s1.c_str(), "\n");
    while (t1()) V1.push_back(t1.val());
    sort(V1.begin(), V1.end());

    StringTokenizer t2(s2.c_str(), "\n");
    while (t2()) V2.push_back(t2.val());
    sort(V2.begin(), V2.end());

    StringVector Missing1(V1.size());
    StringVector::iterator end = set_difference(V1.begin(), V1.end(), V2.begin(), V2.end(), Missing1.begin());
    Missing1.resize(end - Missing1.begin());

    StringVector Missing2(V2.size());
    end = set_difference(V2.begin(), V2.end(), V1.begin(), V1.end(), Missing2.begin());
    Missing2.resize(end - Missing2.begin());

    std::string Result;
    if (!Missing1.empty()) {
        Result += Format("missing word forms in %i:\r\n", FlexiaModelNo2);
        for (size_t i = 0; i < Missing1.size(); i++)
            Result += Missing1[i] + "\r\n";
    }

    if (!Missing2.empty()) {
        Result += Format("\r\nmissing word forms in %i:\r\n", FlexiaModelNo1);
        for (size_t i = 0; i < Missing2.size(); i++)
            Result += Missing2[i] + "\r\n";
    };

    if (Missing2.empty() && Missing1.empty()) {
        Result = "No differences";
    };

    return Result;
};

//----------------------------------------------------------------------------
bool MorphoWizard::check_common_grammems(std::string common_grammems) const {
    Trim(common_grammems);
    if (common_grammems.empty()) return true;
    std::string common_ancode;
    return slf2ancode("* " + common_grammems, common_ancode);
}

//----------------------------------------------------------------------------
bool MorphoWizard::check_prefixes(std::string prefixes) const {
    Trim(prefixes);
    StringTokenizer tok(prefixes.c_str(), ",");
    while (tok()) {
        if (strlen(tok.val()) == 0)
            return false;
        if (!CheckLanguage(tok.val(), m_Language))
            return false;
    };

    return true;
}

//----------------------------------------------------------------------------
//  This function converts all paradigms with prefixes which are ascribed to particular forms
//  to paradigms without prefixes.  
//  1. We find a CMorphForm, which has at least one  prefix.
//  2. We go through the list of lemmas of  this CMorphForm.
//  3. Let A be a slf-representation of one paradigm with prefixes
//  3. Delete  A in the dictionary.
//  4. Delete all '|' (prefix delimiter)   in A.
//  5. Insert A to the dictionary.
// For example, if se have  a paradigm
// aaa NOUN
// bb|aaa NOUN
// cc|aaa NOUN
// will be converted to 
// aaa NOUN
// bbaaa NOUN
// ccaaa NOUN
//----------------------------------------------------------------------------
bool MorphoWizard::attach_form_prefixes_to_bases() {
    bool bFound = false;
    fprintf(stderr, "   processing.... \n");
    DwordVector ModelsWithPrefixes;

    //  finding all models with prefixes
    for (int ModelNo = 0; ModelNo < m_FlexiaModels.size(); ModelNo++) {
        for (size_t k = 0; k < m_FlexiaModels[ModelNo].m_Flexia.size(); k++)
            if (!m_FlexiaModels[ModelNo].m_Flexia[k].m_PrefixStr.empty()) {
                ModelsWithPrefixes.push_back(ModelNo);
                break;
            };
    };

    if (ModelsWithPrefixes.empty())
        return true;

    size_t Count = 0;
    size_t Size = m_LemmaToParadigm.size();
    for (lemma_iterator_t it = m_LemmaToParadigm.begin(); it != m_LemmaToParadigm.end(); it++) {
        Count++;
        if (!(Count % 10000))
            std::cout <<  Count << "/" << Size << "    \r";

        if (binary_search(ModelsWithPrefixes.begin(), ModelsWithPrefixes.end(), it->second.m_FlexiaModelNo)) {
            bFound = true;

            std::string type_grm, Prefixes;
            std::string slf = get_slf_string(it, type_grm, Prefixes);

            {

                assert(slf.find("|") != std::string::npos);
                std::string new_slf;
                for (size_t i = 0; i < slf.length(); i++)
                    if (slf[i] != '|')
                        new_slf += slf[i];
                slf = new_slf;
            }

            lemma_iterator_t to_delete = it;
            it--;
            remove_lemm(to_delete);


            try {
                int line_no_err;
                add_lemma(slf, type_grm, Prefixes, line_no_err);

            }
            catch (...) {
                fprintf(stderr, "cannot add lemma \"%s\"!\n", it->first.c_str());
                fprintf(stderr, "Stopping the process!\n");
                return false;
            };

        }
    }

    std::cout << Count << "/" << Size << " \n";

    if (!bFound) return true;


    fprintf(stderr, "   packing.... \n");
    pack();


    fprintf(stderr, "   checking.... \n");

    for (int ModelNo = 0; ModelNo < m_FlexiaModels.size(); ModelNo++) {
        for (size_t k = 0; k < m_FlexiaModels[ModelNo].m_Flexia.size(); k++)
            if (!m_FlexiaModels[ModelNo].m_Flexia[k].m_PrefixStr.empty()) {
                fprintf(stderr, "FlexModelNo=%i still has prefixes  !\n", ModelNo);
                fprintf(stderr, "We cannot go further!\n");
                return false;
            };
    };
    return true;
}


//----------------------------------------------------------------------------
// СЌС‚Р° С„СѓРЅРєС†РёСЏ РїСЂРёРІРѕРґРёС‚ СЂСѓСЃСЃРєСѓСЋ РјРѕСЂС„РѕР»РѕРіРёСЋ Рє РІРёРґСѓ, РєРѕС‚РѕСЂС‹Р№ РѕРЅР° РґРѕР»Р¶РЅР°  РёРјРµС‚СЊ РІ Р”РёР°Р»РёРЅРіРµ:
// 1. СѓРґР°Р»РµРЅРёРµ "С‘"
// 2. ...
//----------------------------------------------------------------------------
bool MorphoWizard::prepare_for_RML() {
    if (m_Language != morphRussian) return true;

    // РїРµСЂРµРІРѕРґ "С‘"  РІ "Рµ"
    for (int ModelNo = 0; ModelNo < m_FlexiaModels.size(); ModelNo++)
        for (size_t k = 0; k < m_FlexiaModels[ModelNo].m_Flexia.size(); k++) {
            ConvertJO2Je(m_FlexiaModels[ModelNo].m_Flexia[k].m_PrefixStr);
            ConvertJO2Je(m_FlexiaModels[ModelNo].m_Flexia[k].m_FlexiaStr);
        };

    for (lemma_iterator_t lemm_it = m_LemmaToParadigm.begin(); lemm_it != m_LemmaToParadigm.end();) {
        std::string Lemma = lemm_it->first;
        ConvertJO2Je(Lemma);
        lemma_iterator_t next_lemm_it = lemm_it;
        next_lemm_it++;
        if (Lemma != lemm_it->first) {
            CParadigmInfo P = lemm_it->second;
            m_LemmaToParadigm.erase(lemm_it);
            m_LemmaToParadigm.insert(std::make_pair(Lemma, P));
        };
        lemm_it = next_lemm_it;
    };

    // checking
    for (lemma_iterator_t lemm_it = m_LemmaToParadigm.begin(); lemm_it != m_LemmaToParadigm.end(); lemm_it++) {
        std::string Lemma = lemm_it->first;
        ConvertJO2Je(Lemma);
        if (Lemma != lemm_it->first)
            return false;
    };
    return true;
}

bool MorphoWizard::prepare_for_RML2() {
    if (m_Language != morphRussian) return true;


    int sz = (int)m_FlexiaModels.size();
    std::map<int, int> joFlex;
    typedef std::pair<int, int> Int_Pair;
    for (int ModelNo = 0; ModelNo < sz; ModelNo++) {
        bool hasjo = false;
        for (size_t k = 0; k < m_FlexiaModels[ModelNo].m_Flexia.size(); k++)
            if (HasJO(m_FlexiaModels[ModelNo].m_Flexia[k].m_PrefixStr +
                m_FlexiaModels[ModelNo].m_Flexia[k].m_FlexiaStr)) {
                if (!hasjo) {
                    m_FlexiaModels.push_back(m_FlexiaModels[ModelNo]);
                    joFlex.insert(Int_Pair(ModelNo, (int)m_FlexiaModels.size() - 1));
                }
                hasjo = true;
                ConvertJO2Je(m_FlexiaModels[ModelNo].m_Flexia[k].m_PrefixStr); 
                ConvertJO2Je(m_FlexiaModels[ModelNo].m_Flexia[k].m_FlexiaStr);
            };

    }
    LemmaMap nojo;
    for (lemma_iterator_t lemm_it = m_LemmaToParadigm.begin(); lemm_it != m_LemmaToParadigm.end();) {
        std::string Lemma(lemm_it->first.length(), ' ');
        copy(lemm_it->first.begin(), lemm_it->first.end(), Lemma.begin());
        ConvertJO2Je(Lemma);
        lemma_iterator_t next_lemm_it = lemm_it;
        next_lemm_it++;
        CParadigmInfo P = lemm_it->second;
        if (Lemma != lemm_it->first || joFlex.find(P.m_FlexiaModelNo) != joFlex.end()) {
            //m_LemmaToParadigm.erase(lemm_it);
            nojo.insert(std::make_pair(Lemma, P)); // no jo
        };
        if (joFlex.find(P.m_FlexiaModelNo) != joFlex.end())
            lemm_it->second.m_FlexiaModelNo = joFlex.find(P.m_FlexiaModelNo)->second;
        lemm_it = next_lemm_it;
    };
    m_LemmaToParadigm.insert(nojo.begin(), nojo.end());
    return true;
}

//----------------------------------------------------------------------------
bool MorphoWizard::HasUnknownAccents(lemma_iterator_t it) const {
    if (it->second.m_AccentModelNo == UnknownAccentModelNo)
        return true;

    CAccentModel accModel = m_AccentModels[it->second.m_AccentModelNo];
    for (int i = 0; i < accModel.m_Accents.size(); ++i) {
        if (accModel.m_Accents[i] == UnknownAccent) return true;
    }
    return false;
}

//----------------------------------------------------------------------------
bool MorphoWizard::IsPartialAccented(lemma_iterator_t it) const {
    if (it->second.m_AccentModelNo == UnknownAccentModelNo)
        return false;

    CAccentModel accModel = m_AccentModels[it->second.m_AccentModelNo];
    int count = 0;
    for (int i = 0; i < accModel.m_Accents.size(); ++i) {
        if (accModel.m_Accents[i] == UnknownAccent)
            ++count;
    }
    return count < accModel.m_Accents.size();
}

//----------------------------------------------------------------------------
BYTE MorphoWizard::GetLemmaAccent(const_lemma_iterator_t it) const {
    if (it->second.m_AccentModelNo == UnknownAccentModelNo)
        return UnknownAccent;

    return TransferReverseVowelNoToCharNo(it->first,
        m_AccentModels[it->second.m_AccentModelNo].m_Accents[0], m_Language);
}

//----------------------------------------------------------------------------
BYTE MorphoWizard::_GetReverseVowelNo(const std::string& form, uint16_t accentModelNo, uint16_t formInd) const {
    if (accentModelNo == UnknownAccentModelNo || accentModelNo >= m_AccentModels.size()
        || formInd >= m_AccentModels[accentModelNo].m_Accents.size())
        return UnknownAccent;

    BYTE vowelNo = m_AccentModels[accentModelNo].m_Accents[formInd];
    return TransferReverseVowelNoToCharNo(form, vowelNo, m_Language) == UnknownAccent
        ? UnknownAccent : vowelNo;
}

//----------------------------------------------------------------------------
bool MorphoWizard::Filter(std::string flt_str, std::vector<lemma_iterator_t>& found_paradigms) const {
    BYTE pos;
    uint64_t grm;
    if (!m_pGramTab->ProcessPOSAndGrammemsIfCan(flt_str.c_str(), &pos, &grm)
        && !m_pGramTab->ProcessPOSAndGrammemsIfCan(std::string("* " + flt_str).c_str(), &pos, &grm)
        ) {
        return false;
    }
    else {
        std::string flt_pos;
        if (pos != UnknownPartOfSpeech)
            flt_pos = m_pGramTab->GetPartOfSpeechStr(pos);
        std::vector<lemma_iterator_t> filter_paradigms;
        for (int i = 0; i < found_paradigms.size(); i++) {
            std::string str_pos = get_pos_string(found_paradigms[i]);

            if (!flt_pos.empty())
                if (flt_pos != str_pos)
                    continue;

            if ((get_all_lemma_grammems(found_paradigms[i]) & grm) != grm)
                continue;

            filter_paradigms.push_back(found_paradigms[i]);
        }
        filter_paradigms.swap(found_paradigms);
    }
    return true;
}

std::string MorphoWizard::ToRMLEncoding(std::wstring strText) const
{
    return convert_from_utf8(utf16_to_utf8(strText).c_str(), m_Language);
}

std::wstring MorphoWizard::FromRMLEncoding(std::string s) const
{
    return utf8_to_utf16(convert_to_utf8(s, m_Language));
}

uint16_t MorphoWizard::RegisterSession(const CMorphSession& S) {
    if (S.IsEmpty()) return UnknownSessionNo;

    std::vector<CMorphSession>::const_iterator it = find(m_Sessions.begin(), m_Sessions.end(), S);
    if (it == m_Sessions.end()) {
        m_Sessions.push_back(S);
        return (uint16_t)m_Sessions.size() - 1;

    }
    else
        return it - m_Sessions.begin();
};

std::string MorphoWizard::create_slf_for_lemm(std::string lemm, size_t flexiaModelNo, int line_size) const {

    const CFlexiaModel& P = m_FlexiaModels[flexiaModelNo];
    std::string flex = P.get_first_flex();
    std::string NewLemma = lemm.substr(0, lemm.length() - flex.size()) + flex;
    if (NewLemma.find("|"))
        NewLemma.erase(0, NewLemma.find("|") + 1);
    return mrd_to_slf(NewLemma, P, UnknownAccentModelNo, UnknownAccent, line_size);
}