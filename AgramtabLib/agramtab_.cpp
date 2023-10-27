// ==========  This file is under  LGPL, the GNU Lesser General Public Licence
// ==========  Dialing Lemmatizer (www.aot.ru)
// ==========  Copyright by Alexey Sokirko

#include "../common/util_classes.h"
#include "agramtab_.h"
#include "rus_consts.h"

#include <fstream>
#include <string>


CAgramtab::CAgramtab()
{
    m_bInited = false;
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

int CAgramtab::AreEqualPartOfSpeech(const char* grm1, const char* grm2)
{
    if ((grm1 == 0) && (grm2 == 0)) return 1;
    if ((grm1 == 0) && (grm2 != 0)) return 0;
    if ((grm2 == 0) && (grm1 != 0)) return 0;
    if (((unsigned char)grm1[0] == '?') || ((unsigned char)grm2[0] == '?')) return 0;
    return GetLine(GramcodeToLineIndex(grm1))->m_PartOfSpeech == GetLine(GramcodeToLineIndex(grm2))->m_PartOfSpeech;
}



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

    if (L == NULL)
        return UnknownPartOfSpeech;

    return L->m_PartOfSpeech;
}

size_t CAgramtab::GetSourceLineNo(const char* gram_code) const
{
    if (gram_code == 0) return 0;

    if (!strcmp(gram_code, "??")) return 0;

    const CAgramtabLine* L = GetLine(GramcodeToLineIndex(gram_code));

    if (L == NULL)
        return 0;

    return L->m_SourceLineNo;
}


grammems_mask_t CAgramtab::GetAllGrammems(const char* gram_code) const
{
    if (gram_code == 0) return 0;
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

std::string CAgramtab::ReadFromFolder(std::string folder) {
    BuildPartOfSpeechMap();

    auto path = std::filesystem::path(folder) / "gramtab.json";

    std::ifstream inp(path);
    if (!inp.good()) {
        throw CExpc("Cannot read gramtab for language %s path=%s", GetStringByLanguage(m_Language).c_str(), path.string().c_str());
    }
    nlohmann::json gramtab = nlohmann::json::parse(inp);
    inp.close();

    std::unordered_map<std::string, grammem_t> grammem_dict;
    for (part_of_speech_t i = 0; i < GetGrammemsCount(); i++) {
        grammem_dict.insert({ GetGrammemStr(i, naLatin), i });
    }

    for (size_t i = 0; i < GetMaxGrmCount(); i++)
        GetLine(i) = 0;

    size_t line_no = 0;
    auto all_gramcodes = gramtab["gramcodes"].items();
    for (auto& [key, val] : all_gramcodes) {
        std::string gramcode = convert_from_utf8(key.c_str(), m_Language);
        part_of_speech_t pos = UnknownPartOfSpeech;
        const auto pos_it = val.find("p");
        if (pos_it != val.end()) {
            const std::string& pos_str = pos_it.value();
            if (!pos_str.empty()) {
                pos = m_PartOfSpeechesHashMap.at(pos_str);
            }
        }
        grammems_mask_t grammems = 0;
        nlohmann::json grs = val["g"];
        for (auto it = grs.begin(); it != grs.end(); ++it) {
            grammems |= _QM(grammem_dict[*it]);
        }

        CAgramtabLine* pAgramtabLine = new CAgramtabLine(line_no);
        pAgramtabLine->m_Grammems = DeduceGrammems(pos, grammems);
        pAgramtabLine->m_PartOfSpeech = pos;
        size_t gram_index = GramcodeToLineIndex(gramcode.c_str());
        if (GetLine(gram_index)) {
            throw CExpc(Format("line %s in  %s contains a dublicate gramcode", path, key.c_str()));
        }
        GetLine(gram_index) = pAgramtabLine;
        line_no++;
    }
   
    std::string gramcode_utf8 = gramtab["plug_noun_gram_code"].template get<std::string>();
    m_PlugNoun.m_GramCode = convert_from_utf8(gramcode_utf8.c_str(), m_Language);
    assert(!m_PlugNoun.m_GramCode.empty());
    m_PlugNoun.m_Lemma  = convert_from_utf8(gramtab["gramcodes"][gramcode_utf8]["l"].template get<std::string>().c_str(), m_Language);
    assert(!m_PlugNoun.m_Lemma.empty());
    
    m_bInited = true;

    return path.string();
}

void CAgramtab::LoadFromRegistry()
{
    auto key = Format("Software\\Dialing\\Lemmatizer\\%s\\DictPath", GetStringByLanguage(m_Language).c_str());
    ReadFromFolder(::GetRegistryString(key));
};

part_of_speech_t CAgramtab::GetFirstPartOfSpeech(const part_of_speech_mask_t poses) const
{
    part_of_speech_t Count = GetPartOfSpeechesCount();
    for (part_of_speech_t i = 0; i < Count; i++)
        if ((poses & (1 << i)) != 0)
            return i;

    return Count;
};

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
std::string	CAgramtab::GetGramCodes(part_of_speech_t pos, grammems_mask_t grammems, GrammemCompare CompareFunc)const
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

//  uses gleiche to compare ancodes from gram_codes1 with  ancodes gram_codes2
//  returns all ancodes from gram_codes1, which satisfy CompareFunc
std::string CAgramtab::GleicheAncode1(GrammemCompare CompareFunc, const char* gram_codes1, const char* gram_codes2) const
{
    std::string EmptyString;
    return GleicheAncode1(CompareFunc, std::string(gram_codes1), std::string(gram_codes2), EmptyString);
}

std::string CAgramtab::GleicheAncode1(GrammemCompare CompareFunc, std::string gram_codes1, std::string gram_codes2) const
{
    std::string EmptyString;
    return GleicheAncode1(CompareFunc, gram_codes1, gram_codes2, EmptyString);
}

//changes GramCodes1pair according to satisfied GramCodes1 
//(if gramcode number N is good(bad) in GramCodes1 it must be good(bad) in GramCodes1pair)
std::string CAgramtab::GleicheAncode1(GrammemCompare CompareFunc, std::string GramCodes1, std::string GramCodes2, std::string& GramCodes1pair) const
{
    std::string Result;
    std::string pair;
    const char* gram_codes1 = GramCodes1.c_str();
    const char* gram_codes2 = GramCodes2.c_str();
    if (!gram_codes1) return "";
    if (!gram_codes2) return "";
    if (!strcmp(gram_codes1, "??")) return gram_codes2;
    if (!strcmp(gram_codes2, "??")) return gram_codes2;
    size_t len1 = strlen(gram_codes1);
    size_t len2 = strlen(gram_codes2);
    bool has_pair = GramCodes1pair.length() == len1;
    for (size_t l = 0; l < len1; l += 2)
        if (CompareFunc)
        {
            const CAgramtabLine* l1 = GetLine(GramcodeToLineIndex(gram_codes1 + l));
            for (size_t m = 0; m < len2; m += 2)
            {
                const CAgramtabLine* l2 = GetLine(GramcodeToLineIndex(gram_codes2 + m));
                if (CompareFunc(l1, l2))
                {
                    //printf ("%s[%i]=%c\n",gram_codes1,l,gram_codes1[l]);
                    Result.append(gram_codes1 + l, 2);
                    if (has_pair) pair.append(GramCodes1pair.substr(l, 2));
                    //Result += gram_codes1[l+1];
                    break;
                };
            };
        }
        else
        {
            for (size_t m = 0; m < len2; m += 2)
            {
                if (GramcodeToLineIndex(gram_codes1 + l) == GramcodeToLineIndex(gram_codes2 + m))
                {
                    Result.append(gram_codes1 + l, 2);
                    if (has_pair) pair.append(GramCodes1pair.substr(l, 2));
                    break;
                };
            };
        };
    if (has_pair) GramCodes1pair = pair;
    return Result;
};

std::string CAgramtab::UniqueGramCodes(std::string gram_codes) const
{
    std::string Result;
    for (size_t m = 0; m < gram_codes.length(); m += 2)
        if (Result.find(gram_codes.substr(m, 2)) == std::string::npos)
            Result.append(gram_codes.substr(m, 2));
    return Result;
}

std::string CAgramtab::FilterGramCodes(const std::string& gram_codes, grammems_mask_t grammems1, grammems_mask_t grammems2) const
{
    std::string result;
    if (gram_codes == "??") {
        return gram_codes;
    }
    for (size_t l = 0; l < gram_codes.length(); l += 2)
    {
        grammems_mask_t ancode_grammems = GetLine(GramcodeToLineIndex(gram_codes.c_str() + l))->m_Grammems;
        if (!(ancode_grammems & ~grammems1) || !(ancode_grammems & ~grammems2))
            result.append(gram_codes.c_str() + l, 2);
    }
    return result;
}

std::string CAgramtab::FilterGramCodes(grammems_mask_t breaks, std::string gram_codes, grammems_mask_t g1) const
{
    std::string Result;
    grammems_mask_t BR[] = { rAllCases, rAllNumbers, rAllGenders, rAllAnimative, rAllPersons, rAllTimes };
    const char* gram_codes1 = gram_codes.c_str();
    if (!strcmp(gram_codes1, "??")) return gram_codes1;
    size_t len1 = strlen(gram_codes1);
    for (size_t l = 0; l < len1; l += 2)
    {
        const CAgramtabLine* l1 = GetLine(GramcodeToLineIndex(gram_codes1 + l));
        bool R = true;
        for (int i = 0; i < (sizeof BR) / (sizeof BR[0]) && R; i++)
        {
            grammems_mask_t g2 = l1->m_Grammems;
            if (breaks & BR[i])
                R &= ((BR[i] & g1 & g2) > 0 || !(BR[i] & g1) || !(BR[i] & g2));
        }
        if (R)
            Result.append(gram_codes1 + l, 2);
    }
    return Result;
}

std::string CommonAncodeAssignFunction(const CAgramtab* pGramTab, const std::string& s1, const std::string& s2)
{
    std::string Result;
    size_t len1 = s1.length();
    size_t len2 = s2.length();
    for (size_t i = 0; i < len1; i += 2)
        for (size_t k = 0; k < len2; k += 2)
        {
            if ((s1[i] == s2[k])
                && (s1[i + 1] == s2[k + 1])
                )
            {
                Result += s1[i];
                Result += s1[i + 1];
                break;
            };

        };

    return Result.c_str();
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


