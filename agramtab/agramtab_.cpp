// ==========  This file is under  LGPL, the GNU Lesser General Public Licence
// ==========  Dialing Lemmatizer (www.aot.ru)
// ==========  Copyright by Alexey Sokirko

#include "../common/util_classes.h"
#include "agramtab_.h"
#include "RusGramTab.h"

#include <fstream>
#include <string>


CAgramtab::CAgramtab()
{
    m_bUseNationalConstants = true;
};


part_of_speech_t CAgramtab::GetPartOfSpeechByStr(const std::string& part_of_speech  ) const
{
    auto it = m_PartOfSpeechesHashMap.find(part_of_speech);
    if (it == m_PartOfSpeechesHashMap.end()) {
        return UnknownPartOfSpeech;
    }
    return it->second;
}


CAgramtabLine::CAgramtabLine(size_t SourceLineNo)
{
    m_SourceLineNo = SourceLineNo;
};


bool CAgramtab::GetGrammems(const char* gram_code, grammems_mask_t& grammems)  const
{
    grammems = 0;
    if (gram_code == 0) return false;
    if (!*gram_code) return false;
    if (gram_code[0] == '?') return false;

    const CAgramtabLine* L = GetLine(GramcodeToLineIndex(gram_code));

    if (L == NULL)
        return
        false;

    grammems = L->m_Grammems;
    return  true;
};

std::string   CAgramtab::GrammemsToStr(grammems_mask_t grammems, NamingAlphabet na) const
{
    char szGrammems[64 * 5];
    grammems_to_str(grammems, szGrammems, na);
    return szGrammems;
}

bool CAgramtab::ProcessPOSAndGrammems(const char* line_in_gramtab, part_of_speech_t& PartOfSpeech, grammems_mask_t& grammems)  const
{
    if (strlen(line_in_gramtab) > 300) return false;

    StringTokenizer tok(line_in_gramtab, " ,\t\r\n");
    const char* strPos = tok();
    if (!strPos)
    {
        //printf ("unknown pos");		
        return false;
    };


    //  getting the part of speech
    if (strcmp("*", strPos))
    {
        PartOfSpeech = GetPartOfSpeechByStr(strPos);
        if (PartOfSpeech == UnknownPartOfSpeech)
            return false;
    }
    else
        PartOfSpeech = UnknownPartOfSpeech;


    //  getting grammems
    grammems = 0;
    while (tok())
    {
        const char* grm = tok.val();
        auto it = m_GrammemHashMap.find(grm);
        if (it == m_GrammemHashMap.end()) {
            return false;
        }
        grammems |= _QM(it->second);
    };

    return true;
};

bool  CAgramtab::ProcessPOSAndGrammemsIfCan(const char* tab_str, part_of_speech_t* PartOfSpeech, grammems_mask_t* grammems) const
{
    return ProcessPOSAndGrammems(tab_str, *PartOfSpeech, *grammems);
};


void CAgramtab::BuildPartOfSpeechMap()
{
    for (part_of_speech_t i = 0; i < GetPartOfSpeechesCount(); i++) {
        m_PartOfSpeechesHashMap.insert({ GetPartOfSpeechStr(i, naNational), i });
        m_PartOfSpeechesHashMap.insert({ GetPartOfSpeechStr(i, naLatin), i });
    }
    for (grammem_t g = 0; g < GetGrammemsCount(); ++g) {
        m_GrammemHashMap[GetGrammemStr(g, naNational)] = g;
        m_GrammemHashMap[GetGrammemStr(g, naLatin)] = g;
    }
}

void CAgramtab::SetUseNationalConstants(bool value)
{
    m_bUseNationalConstants = value;
    BuildPartOfSpeechMap();
}


bool CAgramtab::GetPartOfSpeechAndGrammems(const BYTE* AnCodes, uint32_t& Poses, grammems_mask_t& Grammems) const
{
    size_t len = strlen((const char*)AnCodes);
    if (len == 0) return false;

    // grammems
    Grammems = 0;
    Poses = 0;
    for (size_t l = 0; l < len; l += 2)
    {
        const CAgramtabLine* L = GetLine(GramcodeToLineIndex((const char*)AnCodes + l));

        if (L == 0) return false;

        Poses |= (1 << L->m_PartOfSpeech);
        Grammems |= L->m_Grammems;
    };

    return true;
}

CAgramtab :: ~CAgramtab()
{
};


char* CAgramtab::grammems_to_str(grammems_mask_t grammems, char* out_buf, NamingAlphabet na ) const
{
    //may be it is wizer to use
    //https://lemire.me/blog/2018/02/21/iterating-over-set-bits-quickly/
    // but it must be crossplatform

    out_buf[0] = 0;
    auto GrammemsCount = GetGrammemsCount();
    for (int i = GrammemsCount - 1; i >= 0; i--)
        if (_QM(i) & grammems)
        {
            strcat(out_buf, GetGrammemStr(i, na));
            strcat(out_buf, ",");
        };
    return out_buf;
};


bool CAgramtab::FindGrammems(const char* gram_codes, grammems_mask_t grammems) const
{
    for (size_t l = 0; l < strlen(gram_codes); l += 2)
        if ((GetLine(GramcodeToLineIndex(gram_codes + l))->m_Grammems & grammems) == grammems)
            return true;

    return false;
};

bool CAgramtab::GetGramCodeByGrammemsAndPartofSpeechIfCan(part_of_speech_t Pos, grammems_mask_t grammems, std::string& gramcodes) const
{
    for (uint16_t i = 0; i < GetMaxGrmCount(); i++) {
        if (GetLine(i) != NULL)
        {
            if ((GetLine(i)->m_Grammems == grammems) && (GetLine(i)->m_PartOfSpeech == Pos))
            {
                gramcodes = LineIndexToGramcode(i);
                return true;
            }
        }
    }
    return false;
};

std::string CAgramtab::GetFirstAncodeByPattern(const std::string& slf) const{
    // todo optimize me to make morphwizard faster
    part_of_speech_t pos;
    grammems_mask_t gra;
    std::string gramcode;
    if (   ProcessPOSAndGrammemsIfCan(slf.c_str(), &pos, &gra)
        && GetGramCodeByGrammemsAndPartofSpeechIfCan(pos, gra, gramcode)
        )
        return gramcode;
    return "";

}

bool CAgramtab::CheckGramCode(const char* gram_code) const
{
    if (gram_code == 0) return true;
    if (*gram_code == 0) return true;
    if (*gram_code == '?') return true;
    size_t line_no = GramcodeToLineIndex(gram_code);
    if (line_no >= GetMaxGrmCount()) return false;
    return   GetLine(line_no) != NULL;
}


part_of_speech_t CAgramtab::GetPartOfSpeech(const char* gram_code) const
{
    if (gram_code == 0) return UnknownPartOfSpeech;
    if (*gram_code == 0) return UnknownPartOfSpeech;
    if (*gram_code == '?') return UnknownPartOfSpeech;

    const CAgramtabLine* L = GetLine(GramcodeToLineIndex(gram_code));

    if (L == 0)
        return UnknownPartOfSpeech;

    return L->m_PartOfSpeech;
}

size_t CAgramtab::GetSourceLineNo(const char* gram_code) const
{
    if (gram_code == nullptr) return 0;

    if (!strcmp(gram_code, "??")) return 0;

    const CAgramtabLine* L = GetLine(GramcodeToLineIndex(gram_code));

    if (L == nullptr)
        return 0;

    return L->m_SourceLineNo;
}


grammems_mask_t CAgramtab::GetAllGrammems(const char* gram_code) const
{
    if (gram_code == nullptr) return 0;
    if (!strcmp(gram_code, "??")) return 0;

    size_t len = strlen(gram_code);

    grammems_mask_t grammems = 0;

    for (size_t l = 0; l < len; l += 2)
    {
        grammems_mask_t G = GetLine(GramcodeToLineIndex(gram_code + l))->m_Grammems;
        grammems |= G;
    };

    return grammems;
}

void CAgramtab::ReadFromFolder(std::string folder) {
    BuildPartOfSpeechMap();

    m_InputJsonPath = (std::filesystem::path(folder) / "gramtab.json").string();

    std::ifstream inp(m_InputJsonPath);
    if (!inp.good()) {
        throw CExpc("Cannot read gramtab for language %s path=%s", GetStringByLanguage(m_Language).c_str(), m_InputJsonPath.c_str());
    }
    
    rapidjson::Document doc;
    rapidjson::IStreamWrapper isw(inp);
    doc.ParseStream(isw);
    inp.close();

    std::unordered_map<std::string, grammem_t> grammem_dict;
    for (part_of_speech_t i = 0; i < GetGrammemsCount(); i++) {
        grammem_dict.insert({ GetGrammemStr(i, naLatin), i });
    }

    for (size_t i = 0; i < GetMaxGrmCount(); i++)
        GetLine(i) = 0;

    size_t line_no = 0;
    for (auto& item : doc["gramcodes"].GetObject()) {
        std::string gramcode = item.name.GetString();
        auto& val = item.value;
        part_of_speech_t pos = UnknownPartOfSpeech;
        auto pos_it = rapidjson::Pointer("/p").Get(val);
        if (pos_it != nullptr) {
            const std::string& pos_str = pos_it->GetString();
            if (!pos_str.empty()) {
                pos = m_PartOfSpeechesHashMap.at(pos_str);
            }
        }
        grammems_mask_t grammems = 0;
        auto& grs = val["g"];
        for (auto& s: val["g"].GetArray()) {
            grammems |= _QM(grammem_dict[s.GetString()]);
        }

        CAgramtabLine* pAgramtabLine = new CAgramtabLine(line_no);
        pAgramtabLine->m_Grammems = grammems;
        pAgramtabLine->m_PartOfSpeech = pos;
        size_t gram_index = GramcodeToLineIndex(gramcode.c_str());
        if (GetLine(gram_index)) {
            throw CExpc(Format("line %s in  %s contains a dublicate gramcode", gramcode.c_str(), m_InputJsonPath));
        }
        GetLine(gram_index) = pAgramtabLine;
        line_no++;
    }
   
    std::string gramcode = doc["plug_noun_gram_code"].GetString();
    m_PlugNoun.m_GramCode = gramcode;
    assert(!m_PlugNoun.m_GramCode.empty());
    m_PlugNoun.m_Lemma  = doc["gramcodes"][gramcode]["l"].GetString();
    assert(!m_PlugNoun.m_Lemma.empty());
    InitLanguageSpecific(doc);
}

std::string CAgramtab::GetGramtabPath() const {
    return m_InputJsonPath;
}

std::string CAgramtab::GetDefaultPath() const {
    auto key = Format("Software\\Dialing\\Lemmatizer\\%s\\DictPath", GetStringByLanguage(m_Language).c_str());
    return ::GetRegistryString(key);
}

std::string	CAgramtab::GetAllPossibleAncodes(part_of_speech_t pos, grammems_mask_t grammems)const
{
    std::string Result;
    for (uint16_t i = 0; i < GetMaxGrmCount(); i++)
        if (GetLine(i) != 0)
        {
            const CAgramtabLine* L = GetLine(i);
            if ((L->m_PartOfSpeech == pos)
                && ((grammems & L->m_Grammems) == grammems)
                )
                Result += LineIndexToGramcode(i);
        };
    return Result;
};

//Generate GramCodes for grammems with CompareFunc
std::string	CAgramtab::GetAllGramCodes(part_of_speech_t pos, grammems_mask_t grammems, GrammemCompare CompareFunc)const
{
    std::string Result;
    CAgramtabLine L0(0);
    L0.m_PartOfSpeech = pos;
    L0.m_Grammems = grammems;
    for (uint16_t i = 0; i < GetMaxGrmCount(); i++)
        if (GetLine(i) != 0)
        {
            const CAgramtabLine* L = GetLine(i);
            if ((L->m_PartOfSpeech == pos)
                && (CompareFunc ? CompareFunc(L, &L0) : (L->m_Grammems & grammems) == L->m_Grammems && !(pos == NOUN && (L->m_Grammems & rAllGenders) == rAllGenders)) 
                )
                Result += LineIndexToGramcode(i);
        };
    return Result;
};

grammems_mask_t CAgramtab::Gleiche(GrammemCompare CompareFunc, const char* gram_codes1, const char* gram_codes2) const
{
    grammems_mask_t grammems = 0;
    if (!gram_codes1) return false;
    if (!gram_codes2) return false;
    if (!strcmp(gram_codes1, "??")) return false;
    if (!strcmp(gram_codes2, "??")) return false;
    size_t len1 = strlen(gram_codes1);
    size_t len2 = strlen(gram_codes2);
    for (size_t l = 0; l < len1; l += 2)
        for (size_t m = 0; m < len2; m += 2)
        {
            const CAgramtabLine* l1 = GetLine(GramcodeToLineIndex(gram_codes1 + l));
            const CAgramtabLine* l2 = GetLine(GramcodeToLineIndex(gram_codes2 + m));
            if (CompareFunc(l1, l2))
                grammems |= (l1->m_Grammems & l2->m_Grammems);
        };

    return grammems;
};

bool EqualAncodes (const CAgramtabLine* l1, const CAgramtabLine* l2)
{
    return l1 == l2;
};


std::string CAgramtab::GleicheAncode1(GrammemCompare CompareFunc, std::string gram_codes1, std::string gram_codes2) const
{
    std::string result;
    if (gram_codes1.empty() || gram_codes2.empty()) return "";
    if (gram_codes1 == "??") return gram_codes2;
    if (gram_codes2 == "??") return gram_codes2;
    if (!CompareFunc) {
        CompareFunc = EqualAncodes;
    }
    for (size_t l = 0; l < gram_codes1.length(); l += 2) {
        const CAgramtabLine* l1 = GetLine(GramcodeToLineIndex(gram_codes1.c_str() + l));
        for (size_t m = 0; m < gram_codes2.length(); m += 2)
        {
            const CAgramtabLine* l2 = GetLine(GramcodeToLineIndex(gram_codes2.c_str() + m));
            if (CompareFunc(l1, l2))
            {
                result.append(gram_codes1.c_str() + l, 2);
                break;
            };
        };
    };
    return result;
}


std::string CAgramtab::UniqueGramCodes(std::string gram_codes) const
{
    std::string Result;
    for (size_t m = 0; m < gram_codes.length(); m += 2)
        if (Result.find(gram_codes.substr(m, 2)) == std::string::npos)
            Result.append(gram_codes.substr(m, 2));
    return Result;
}

std::string  CAgramtab::GetTabStringByGramCode(const char* gram_code) const
{
    if (!gram_code || gram_code[0] == '?')
        return "";
    part_of_speech_t POS = GetPartOfSpeech(gram_code);
    grammems_mask_t Grammems;
    GetGrammems(gram_code, Grammems);
    char buffer[256];
    grammems_to_str(Grammems, buffer);
    std::string POSstr = (POS == UnknownPartOfSpeech) ? "*" : GetPartOfSpeechStr(POS);
    return POSstr + std::string(" ") + buffer;
}
