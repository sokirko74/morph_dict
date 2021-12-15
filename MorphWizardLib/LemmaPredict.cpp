#include "FormInfo.h"
#include "wizard.h"
#include "LemmaPredict.h"

std::string CPredictSuffix::getCommonGrammemsUtf8() {
    return convert_to_utf8(m_pWizard->get_grammem_string(m_SourceCommonAncode), m_pWizard->m_Language);
}

bool CPredictSuffix::operator  < (const  CPredictSuffix& X) const
{
    if (m_FlexiaModelNo != X.m_FlexiaModelNo)
        return m_FlexiaModelNo < X.m_FlexiaModelNo;

    if (m_SourceLemmaAncode != X.m_SourceLemmaAncode)
        return m_SourceLemmaAncode < X.m_SourceLemmaAncode;

    return m_Suffix < X.m_Suffix;
};

bool CPredictSuffix::operator  == (const  CPredictSuffix& X) const
{
    return			(m_FlexiaModelNo == X.m_FlexiaModelNo)
        && (m_Suffix == X.m_Suffix)
        && (m_SourceLemmaAncode == X.m_SourceLemmaAncode);
};

std::string CPredictSuffix::getSLF_Utf8(std::string lemm) {
    CDumpParadigm P(m_pWizard);
    lemm = convert_from_utf8(lemm.c_str(), m_pWizard->m_Language);
    P.m_TypeGrammemsStr = m_pWizard->get_grammem_string(m_SourceCommonAncode);
    P.m_SlfStr = m_pWizard->create_slf_for_lemm(lemm, m_FlexiaModelNo, 80);
    return P.ToStringUtf8();
}

std::string CPredictSuffix::getWiktionaryTemplateRef() {
    return convert_to_utf8(m_pWizard->m_FlexiaModels[m_FlexiaModelNo].m_WiktionaryMorphTemplate, m_pWizard->m_Language);
}

bool IsLessByLemmaLength(const CPredictSuffix& _X1, const CPredictSuffix& _X2) {
    return _X1.m_SourceLemma.length() < _X2.m_SourceLemma.length();
};

std::string GetSuffix(const std::string& Lemma, int PrefferedLength) {
    int SuffLen = (int)Lemma.length() - PrefferedLength;
    if (SuffLen < 0) SuffLen = 0;
    std::string Suffix = Lemma.substr(SuffLen);
    return Suffix;

};

TLemmaPredictor::TLemmaPredictor(const MorphoWizard* wizard) {
    m_pWizard = wizard;
}

void TLemmaPredictor::CreateIndex() {

    for (size_t i = 0; i < MaxPredictSuffixLength - MinPredictSuffixLength + 1; i++) {
        m_PredictIndex[i].clear();
    }

    // go through all words
    std::vector<CPredictSuffix> AllLemmas;
    for (auto& [key, value] : m_pWizard->m_LemmaToParadigm) {

        const CFlexiaModel& p = m_pWizard->m_FlexiaModels[value.m_FlexiaModelNo];

        const char* lemma = key.c_str();

        // create predict suffix
        CPredictSuffix S;
        S.m_pWizard = m_pWizard;
        S.m_FlexiaModelNo = value.m_FlexiaModelNo;
        S.m_SourceLemmaAncode = p.get_first_code();
        S.m_SourceCommonAncode = value.GetCommonAncodeIfCan();
        S.m_SourceLemma = lemma;
        S.m_PrefixSetStr = m_pWizard->get_prefix_set(value);
        S.m_Frequence = 1;
        if (S.m_SourceLemma.length() < 3) continue;
        AllLemmas.push_back(S);
    };

    sort(AllLemmas.begin(), AllLemmas.end(), IsLessByLemmaLength);

    // going through all prefix suffixes
    for (size_t i = 0; i < AllLemmas.size(); i++) {
        CPredictSuffix& S = AllLemmas[i];
        for (size_t suff_len = MinPredictSuffixLength; suff_len <= MaxPredictSuffixLength; suff_len++) {
            predict_container_t& PredictIndex = m_PredictIndex[suff_len - MinPredictSuffixLength];
            S.m_Suffix = GetSuffix(S.m_SourceLemma, (int)suff_len);
            std::pair<predict_container_t::iterator, bool> bRes = PredictIndex.insert(S);
            if (!bRes.second) {
                bRes.first->m_Frequence++;
            }
        };
    }
};

std::vector<CPredictSuffix> TLemmaPredictor::predict_lemm(const std::string& lemm, const int preffer_suf_len, const int minimal_frequence,
    bool bOnlyMainPartOfSpeeches, TLemmPredictSortEnum sortBy) {

    std::vector<CPredictSuffix>	predicts;
    if (preffer_suf_len < MinPredictSuffixLength) return predicts;
    if (preffer_suf_len > MaxPredictSuffixLength) return predicts;

    const predict_container_t& PredictIndex = m_PredictIndex[preffer_suf_len - MinPredictSuffixLength];

    std::string Suffix = GetSuffix(lemm, preffer_suf_len);

    for (auto& s : PredictIndex) {
        if (s.m_Suffix != Suffix) continue;
        if (s.m_Frequence < minimal_frequence)
            continue;

        if (lemm.find("|") != std::string::npos)
            if (s.m_PrefixSetStr.empty())
                continue;

        const CFlexiaModel& P = m_pWizard->m_FlexiaModels[s.m_FlexiaModelNo];
        std::string flex = P.get_first_flex();
        if (flex.size() > Suffix.size()) {
            if (flex.size() >= lemm.size()) continue;
            if (flex != lemm.substr(lemm.length() - flex.size())) continue;
        };

        
        if (bOnlyMainPartOfSpeeches) {
            part_of_speech_t pos = m_pWizard->m_pGramTab->GetPartOfSpeech(s.m_SourceLemmaAncode.c_str());
            if (!m_pWizard->m_pGramTab->PartOfSpeechIsProductive(pos))
                continue;
        }
        predicts.push_back(s);
    }

    std::sort(predicts.begin(), predicts.end(), IsLessForPredict(m_pWizard, sortBy));

    return predicts;
}


IsLessForPredict::IsLessForPredict(const MorphoWizard* pWizard, TLemmPredictSortEnum sortBy)
{
    m_pWizard = pWizard;
    m_SortBy = sortBy;
}

bool  IsLessForPredict::operator()(const CPredictSuffix& s1, const CPredictSuffix& s2) const
{
    switch (m_SortBy)
    {
    case TLemmPredictSortEnum::Grammems:
    {
        std::string g1 = m_pWizard->get_pos_string_and_grammems(s1.m_SourceLemmaAncode);
        std::string g2 = m_pWizard->get_pos_string_and_grammems(s2.m_SourceLemmaAncode);
        if (g1 == g2) 
            return s1.m_Frequence > s2.m_Frequence;
        else
            return g1 < g2;
    };
    case TLemmPredictSortEnum::TypeDict:
    {
        std::string g1 = m_pWizard->get_grammem_string(s1.m_SourceCommonAncode);
        std::string g2 = m_pWizard->get_grammem_string(s2.m_SourceCommonAncode);
        if (g1 == g2) 
            return s1.m_Frequence > s2.m_Frequence;
        else
            return g1 < g2;
    };
    case TLemmPredictSortEnum::Lemma:
    {
        if (s1.m_SourceLemma == s2.m_SourceLemma)
            return s1.m_Frequence > s2.m_Frequence;
        else
            return s1.m_SourceLemma < s2.m_SourceLemma;
    };
    case TLemmPredictSortEnum::Freq: return s1.m_Frequence > s2.m_Frequence;
    case TLemmPredictSortEnum::Paradigm: return s1.m_FlexiaModelNo < s2.m_FlexiaModelNo;
    case TLemmPredictSortEnum::LemmaPrefix:
    {
        if (s1.m_PrefixSetStr == s2.m_PrefixSetStr)
            return s1.m_Frequence > s2.m_Frequence;
        else
            return s1.m_PrefixSetStr < s2.m_PrefixSetStr;
    };
    case TLemmPredictSortEnum::FormsCount:
            return m_pWizard->m_FlexiaModels[s1.m_FlexiaModelNo].m_Flexia.size() < m_pWizard->m_FlexiaModels[s2.m_FlexiaModelNo].m_Flexia.size();
    default: return s1 < s2;
    };
};
