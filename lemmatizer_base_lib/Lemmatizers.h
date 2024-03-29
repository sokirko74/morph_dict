// ==========  This file is under  LGPL, the GNU Lesser General Public License
// ==========  Dialing Lemmatizer (www.aot.ru)
// ==========  Copyright by Alexey Sokirko, Andrey Putrin

#pragma once

#pragma warning (disable : 4786)

#include "MorphDict.h"
#include "Paradigm.h"
#include "Statistic.h"
#include "Predict.h"
#include "common_consts.h"

#pragma warning (disable : 4250)

typedef enum {
    subjFinance = 1, subjComputer = 2, subjLiterature = 4
} SubjectEnum;


class CLemmatizer : public CMorphDict {
    bool m_bEnablePrediction;
    void predict_hyphen_word(std::string& wordform, bool capital, std::vector<CFormInfo>& Result) const;

    bool LemmatizeWord(std::string& InputWordStr, const bool cap, const bool predict,
        std::vector<CAutomAnnotationInner>& results, bool bGetLemmaInfos) const;
    void PredictByDataBase(std::string InputWordStr, std::vector<CAutomAnnotationInner>& results, bool is_cap) const;


protected:
    // Postfixes-particles, that do not change the meaning 
    std::set<std::string> m_HyphenPostfixes;
    // productive prefixes
    std::set<std::string> m_HyphenPrefixes;

    CStatistic m_Statistic;
    CPredictBase m_Predict;
    std::set<std::string> m_PrefixesSet;


    virtual void FilterSrc(std::string& src) const {};
    
    void ReadOptions(std::string FileName);

    // prediction by suffix
    bool
    CheckAbbreviation(std::string InputWordStr, std::vector<CAutomAnnotationInner> &FindResults, bool is_cap) const;
    CAutomAnnotationInner ConvertPredictTupleToAnnot(const CPredictTuple &input) const;
    bool IsPrefix(const std::string &Prefix) const;


public:
    bool m_bLoaded;
    bool m_bMaximalPrediction;
    bool m_bUseStatistic;
    bool m_bAllowRussianJo;

    CLemmatizer(MorphLanguageEnum Language);
    virtual ~CLemmatizer();

    MorphLanguageEnum GetLanguage() const { return m_pFormAutomat->m_Language; };
    const CStatistic &GetStatistic() const;
    bool CheckABC(const std::string &WordForm) const;
    bool IsHyphenPostfix(const std::string &Postfix) const;
    bool IsHyphenPrefix(const std::string &Prefix) const;
    void LoadDictionariesFromPath(std::string load_path);
    void LoadDictionariesRegistry();
    bool LoadStatisticRegistry(SubjectEnum subj);
    bool CreateParadigmCollection(bool bNorm, std::string &WordStr, bool capital, bool bUsePrediction,
                                  std::vector<CFormInfo> &Result) const;
    bool GetAllAncodesAndLemmasQuick(std::string &InputWordStr, bool capital, char *OutBuffer, size_t MaxBufferSize,
                                     bool bUsePrediction) const;
    bool CreateParadigmFromID(uint32_t id, CFormInfo &Result) const;
    bool IsInDictionary(std::string &word, const bool cap) const;
    std::vector<CFuzzyResult> CorrectMisspelledWord1(std::string InputWordStr,  size_t maxStrDistance=2) const;
};


class CLemmatizerRussian : public CLemmatizer {
public:
    CLemmatizerRussian();
    virtual ~CLemmatizerRussian() {};
    void FilterSrc(std::string &src) const;
};


class CLemmatizerEnglish : public CLemmatizer {
public:
    CLemmatizerEnglish();
    virtual ~CLemmatizerEnglish() {};
};

class CLemmatizerGerman : public CLemmatizer {
public:
    CLemmatizerGerman();
    virtual ~CLemmatizerGerman() {};
};

