// ==========  This file is under  LGPL, the GNU Lesser General Public Licence
// ==========  Dialing Lemmatizer (www.aot.ru)
// ==========  Copyright by Alexey Sokirko

#pragma once

#include "agramtab_.h"

enum RussianPartOfSpeechEnum
{
    NOUN = 0,
    ADJ_FULL = 1,
    VERB = 2,
    PRONOUN = 3,
    PRONOUN_P = 4,
    PRONOUN_PREDK = 5,
    NUMERAL = 6,
    NUMERAL_P = 7,
    ADV = 8,
    PREDK = 9,
    PREP = 10,
    POSL = 11,
    CONJ = 12,
    INTERJ = 13,
    INP = 14,
    PHRASE = 15,
    PARTICLE = 16,
    ADJ_SHORT = 17,
    PARTICIPLE = 18,
    ADVERB_PARTICIPLE = 19,
    PARTICIPLE_SHORT = 20,
    INFINITIVE = 21,
    RUSSIAN_PART_OF_SPEECH_COUNT = 22
};



enum RussianGrammemsEnum {
    rPlural = 0,
    rSingular = 1,
    rAllNumbers = ((1 << rSingular) | (1 << rPlural)),

    rNominativ = 2,
    rGenitiv = 3,
    rDativ = 4,
    rAccusativ = 5,
    rInstrumentalis = 6,//тв,
    rLocativ = 7,	//пр,
    rVocativ = 8,	//зв,
    rAllCases = ((1 << rNominativ) | (1 << rGenitiv) | (1 << rDativ) | (1 << rAccusativ) | (1 << rInstrumentalis) | (1 << rVocativ) | (1 << rLocativ)),

    rMasculinum = 9,
    rFeminum = 10,
    rNeutrum = 11,
    //rMascFem = 12,
    rAllGenders = ((1 << rMasculinum) | (1 << rFeminum) | (1 << rNeutrum)),


    rPresentTense = 13,
    rFutureTense = 14,
    rPastTense = 15,
    rAllTimes = ((1 << rPresentTense) | (1 << rFutureTense) | (1 << rPastTense)),

    rFirstPerson = 16,
    rSecondPerson = 17,
    rThirdPerson = 18,
    rAllPersons = ((1 << rFirstPerson) | (1 << rSecondPerson) | (1 << rThirdPerson)),

    rImperative = 19,

    rAnimative = 20,
    rNonAnimative = 21,
    rAllAnimative = ((1 << rAnimative) | (1 << rNonAnimative)),

    rComparative = 22,

    rPerfective = 23,//св
    rNonPerfective = 24,//нс

    rNonTransitive = 25,//нп
    rTransitive = 26,//пе

    rActiveVoice = 27,//дст
    rPassiveVoice = 28,//стр


    rIndeclinable = 29, //"0"
    rInitialism = 30,	//"аббр"

    rPatronymic = 31, // "отч"

    rToponym = 32,//лок
    rOrganisation = 33,//орг

    rQualitative = 34,//кач
    rDeFactoSingTantum = 35,//дфст

    rInterrogative = 36,//вопр
    rDemonstrative = 37,//указат

    rName = 38,
    rSurName = 39,
    rImpersonal = 40,//безл
    rSlang = 41,//жарг
    rMisprint = 42,//опч
    rColloquial = 43,//разг
    rPossessive = 44,//притяж
    rArchaism = 45,//арх
    rSecondCase = 46,
    rPoetry = 47,
    rProfession = 48,
    rSuperlative = 49,
    rPositive = 50,
    RussianGrammemsCount = 51
};




enum RussianClauseTypeEnum  {
    VERB_PERS_T = 0,//ГЛ_ЛИЧН
    ADVERB_PARTICIPLE_T = 1, //ДПР
    PARTICIPLE_SHORT_T = 2,	//КР_ПРЧ
    ADJ_SHORT_T = 3,//КР_ПРИЛ
    PREDK_T = 4, //ПРЕДК
    PARTICIPLE_T = 5,	//ПРЧ
    INFINITIVE_T = 6,	//ИНФ
    INP_T = 7, //ВВОД
    DASH_T = 8,//ТИРЕ
    UNDETACHED_ADJ_PATIC = 9,//НСО
    COMPARATIVE_T = 10,//СРАВН
    COPUL_T = 11,
    RUSSIAN_CLAUSE_TYPE_COUNT = 12
};


struct CRusPopularGramCodes {
    std::string m_ProductiveNoun;
    std::string m_ProductiveSingNoun;
    std::string m_InanimIndeclNoun;
    std::string m_MasAbbrNoun;
    std::string m_GenderNumeral;
};


class CRusGramTab : public CAgramtab {
    const static size_t rStartUp = 0xC0E0; //Аа
    const static size_t rEndUp = 0x10000;  //яя
    const static size_t rMaxGrmCount = rEndUp - rStartUp; // // 16159  (16 Кб)

    CRusPopularGramCodes m_PopularGramCodes;

public:
    CAgramtabLine *Lines[rMaxGrmCount];

    CRusGramTab();
    ~CRusGramTab() override;
    void LoadFromRegistry() override;

    part_of_speech_t GetPartOfSpeechesCount() const override;

    const char *GetPartOfSpeechStr(part_of_speech_t i, NamingAlphabet na = naDefault) const override;

    const char* GetPartOfSpeechStrLong(part_of_speech_t i) const  override;

    grammem_t GetGrammemsCount() const override;

    const char *GetGrammemStr(size_t i, NamingAlphabet na=naDefault) const override;

    size_t GetMaxGrmCount() const override;

    CAgramtabLine *&GetLine(size_t LineNo) override;

    const CAgramtabLine *GetLine(size_t LineNo) const override;

    size_t GramcodeToLineIndex(const char *s) const override;

    std::string LineIndexToGramcode(uint16_t i) const override;

    bool ProcessPOSAndGrammems(const char *tab_str, part_of_speech_t &PartOfSpeech, grammems_mask_t &grammems) const override;

    bool GleicheCase(const char *gram_code_noun, const char *gram_code_adj) const override;

    bool GleicheCaseNumber(const char *gram_code1, const char *gram_code2) const override;

    grammems_mask_t GleicheGenderNumberCase(const char *common_gram_code_noun, const char *gram_code_noun,
                                            const char *gram_code_adj) const override;

    bool GleicheGenderNumber(const char *gram_code1, const char *gram_code2) const override;

    bool ConflictGenderNumber(const char *gram_code1,
                              const char *gram_code2) const;//with absent grammems check, less strict than GleicheGenderNumber
    bool ConflictGrammems(grammems_mask_t g1, grammems_mask_t g2,
                          grammems_mask_t breaks) const;//with absent grammems check, less strict than GleicheGenderNumber
    bool GleicheSubjectPredicate(const char *gram_code1, const char *gram_code2) const override;

    long GetClauseTypeByName(const char *TypeName) const override;

    const char *GetClauseNameByType(long type) const override;

    bool IsStrongClauseRoot(part_of_speech_mask_t poses) const override;

    bool is_month(const char *lemma) const override;

    bool IsMorphNoun(part_of_speech_mask_t poses) const override;

    bool is_morph_adj(part_of_speech_mask_t poses) const override;

    bool is_morph_participle(part_of_speech_mask_t poses) const override;

    bool is_morph_pronoun(part_of_speech_mask_t poses) const override;

    bool is_morph_pronoun_adjective(part_of_speech_mask_t poses) const override;

    bool is_left_noun_modifier(part_of_speech_mask_t poses, grammems_mask_t grammems) const override;

    bool is_numeral(part_of_speech_mask_t poses) const override;

    bool is_verb_form(part_of_speech_mask_t poses) const override;

    bool is_infinitive(part_of_speech_mask_t poses) const override;

    bool is_morph_predk(part_of_speech_mask_t poses) const override;

    bool is_morph_adv(part_of_speech_mask_t poses) const override;

    bool is_morph_personal_pronoun(part_of_speech_mask_t poses, grammems_mask_t grammems) const override;

    bool IsSimpleParticle(const char *lemma, part_of_speech_mask_t poses) const override;

    bool IsSynNoun(part_of_speech_mask_t poses, const char *Lemma) const override;

    bool IsStandardParamAbbr(const char *WordStrUpper) const override;

    bool FilterNounNumeral(std::string &gcNoun, const std::string &gcNum, grammems_mask_t &grammems) const override;

    grammems_mask_t
    ChangeGleicheAncode1(GrammemCompare CompareFunc, const std::string &wordGramCodes, std::string &groupGramCodes,
                         const grammems_mask_t wordGrammems) const override;

    bool PartOfSpeechIsProductive(part_of_speech_t p) const override;

    const CRusPopularGramCodes& GramCodes() const {return m_PopularGramCodes;};

    std::string FilterGramCodes1(const std::string& gram_codes, grammems_mask_t positive, grammems_mask_t negative) const;

};

extern bool GenderNumberCaseRussian(const CAgramtabLine *l1, const CAgramtabLine *l2);

extern bool FiniteFormCoordRussian(const CAgramtabLine *l1, const CAgramtabLine *l2);

extern bool CaseNumber(const CAgramtabLine *l1, const CAgramtabLine *l2);

extern bool CaseGender(const CAgramtabLine *l1, const CAgramtabLine *l2);

extern bool CaseNumberGender0(const CAgramtabLine *l1, const CAgramtabLine *l2); //with absent grammems check

extern bool GenderNumber0(const CAgramtabLine *l1, const CAgramtabLine *l2); //with absent grammems check

extern bool GrammemsInclusion(const CAgramtabLine *l1, const CAgramtabLine *l2);

extern bool GrammemsEqu(const CAgramtabLine *l1, const CAgramtabLine *l2);

extern bool AnyGender(const CAgramtabLine* l1, const CAgramtabLine* );



