#include "morph_dict/agramtab/agramtab_.h"
#include "flexia_model.h"
#include "accent_model.h"
#include "paradigm_consts.h"
#include "morph_dict/common/util_classes.h"

struct AncodeLess
{
    const CAgramtab* m_pGramTab;
    
    AncodeLess(const CAgramtab* pGramTab) {
        m_pGramTab = pGramTab;
    }

    bool operator()(const std::string& s1, const std::string& s2) const {
        return m_pGramTab->GetSourceLineNo(s1.c_str()) < m_pGramTab->GetSourceLineNo(s2.c_str());
    }


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

static BYTE get_accented_vowels_index(std::string& form, BYTE& aux_accent_offset) {
    BYTE result_offset = UnknownAccent;
    size_t accents_count = 0;
    size_t vowels_count = 0;
    auto wform = utf8_to_wstring(form);
    for (int k = (int)wform.length() - 1; k >= 0; k--) {
        if (IsUpperVowel(wform[k]))
            vowels_count++;

        if (wform[k] == '\'') {
            accents_count++;
            if (accents_count > 2)
                throw CExpc("Error! Too many stresses in %s!", form.c_str());
            if (k == 0) {
                throw CExpc("Error! Bad stress format in %s", form.c_str());
            }
            if (!IsUpperVowel(wform[k - 1])) {
                throw CExpc("A stress should be put on a vowel in %s", form.c_str());
            }
            //  we should determine the auxiliary accent, which is permanent for all word forms that's  why
            // we can read it from the first line, but it should differ from the main accent
            // So it should be something like this "aaaAaaaMaaa", where A is the auxiliary
            //  stress, and M  is the main stress.
            if (result_offset == UnknownAccent)
                result_offset = (BYTE)vowels_count;
            else {
                if (aux_accent_offset != UnknownAccent) {
                    if (aux_accent_offset != k - 1)
                        throw CExpc("Auxiliary stress should be on the same position: %s ", form.c_str());
                }
                else
                    aux_accent_offset = k - 1;
            };
            wform.erase(k, 1);
        };
    };
    if (wform.length() != form.length()) {
        form = wstring_to_utf8(wform);
    }
    return result_offset;
}


typedef std::set<CSlfLineByAncode, CSlfLineByAncodeLess> SlfLineSet;
typedef std::map<std::string, SlfLineSet, AncodeLess> Gramcode2SlfLineMap;

void parse_slf(const CAgramtab* gramtab, const std::string& s, std::string& lemm, CFlexiaModel& FlexiaModel, CAccentModel& AccentModel, BYTE& aux_accent_offset, int& line_no_err)  {
    AncodeLess ancode_less(gramtab);
    StringTokenizer lines(s.c_str(), "\r\n");

    std::string lemm_gramcode;
    Gramcode2SlfLineMap Paradigm(ancode_less);
    size_t CommonLeftPartSize = lemm.size();
    BYTE LemmaAccentByte;
    aux_accent_offset = UnknownAccent;

    StringSet dubllcates;

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

        if (dubllcates.find(line) == dubllcates.end())
            dubllcates.insert(line);
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

        std::string gramcode = gramtab->GetFirstAncodeByPattern(pos_and_grammems);
        if (gramcode.empty())
            throw CExpc("Error! Wrong morphological annotation(%s)", pos_and_grammems.c_str());

        MakeUpperUtf8(form);
        BYTE accent = get_accented_vowels_index(form, aux_accent_offset);

        std::string Prefix;
        size_t PrefixInd = form.find("|");
        if (PrefixInd != std::string::npos) {
            Prefix = form.substr(0, PrefixInd);
            form.erase(0, PrefixInd + 1);
        };


        if (line_no_err == 1) {
            lemm = form;
            lemm_gramcode = gramcode;
            LemmaAccentByte = accent;
            CommonLeftPartSize = form.length();
        }
        else {
            CSlfLineByAncode Line;
            Line.m_AccentByte = accent;
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
    for (auto& [gramcode, slf_set]: Paradigm) {
        for (auto& l : slf_set) {
            std::string flexia = l.m_Form.substr(CommonLeftPartSize);
            FlexiaModel.m_Flexia.push_back(CMorphForm(gramcode, flexia, l.m_Prefix));
            AccentModel.m_Accents.push_back(l.m_AccentByte);
        }
    };

}
