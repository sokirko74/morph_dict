#pragma once

struct CPredictSuffix
{
    uint16_t	m_FlexiaModelNo;
    std::string	m_Suffix;

    //  grammatical code of the lemma
    std::string	m_SourceLemmaAncode;

    //  common gramcode of the lemma
    std::string	m_SourceCommonAncode;

    std::string	m_SourceLemma;
    mutable size_t	m_Frequence;
    std::string  m_PrefixSetStr;

    bool operator  < (const  CPredictSuffix& X) const
    {
        if (m_FlexiaModelNo != X.m_FlexiaModelNo)
            return m_FlexiaModelNo < X.m_FlexiaModelNo;

        if (m_SourceLemmaAncode != X.m_SourceLemmaAncode)
            return m_SourceLemmaAncode < X.m_SourceLemmaAncode;

        return m_Suffix < X.m_Suffix;
    };
    bool operator  == (const  CPredictSuffix& X) const
    {
        return			(m_FlexiaModelNo == X.m_FlexiaModelNo)
            && (m_Suffix == X.m_Suffix)
            && (m_SourceLemmaAncode == X.m_SourceLemmaAncode);
    };
};

const int MinPredictSuffixLength = 2;
const int MaxPredictSuffixLength = 5;

typedef std::set<CPredictSuffix> predict_container_t;

class MorphWizard;

enum class TLemmPredictSortEnum {
    Grammems = 0,
    TypeDict = 1,
    Lemma = 2,
    Freq = 3,
    Index = 4,
    Paradigm = 5,
    LemmaPrefix = 6,
    FormsCount = 7
};

class IsLessForPredict
{
    const MorphoWizard* m_pWizard;
    TLemmPredictSortEnum m_SortBy;
public:
    IsLessForPredict(const MorphoWizard* pWizard, TLemmPredictSortEnum sortBy);
    bool  operator()(const CPredictSuffix* s1, const CPredictSuffix* s2) const;
};

class TLemmaPredictor {
public:
    const MorphoWizard* m_pWizard;
    predict_container_t	m_PredictIndex[MaxPredictSuffixLength - MinPredictSuffixLength + 1];
    std::string m_CurrentNewLemma;
    
    TLemmaPredictor(const MorphoWizard* wizard);
    void CreateIndex();
    std::vector<const CPredictSuffix*> predict_lemm(const std::string& lemm, const int preffer_suf_len, int minimal_frequence, bool bOnlyMainPartOfSpeeches, TLemmPredictSortEnum sortBy);
};
