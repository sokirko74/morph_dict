// ==========  This file is under  LGPL, the GNU Lesser General Public Licence
// ==========  Dialing Lemmatizer (www.aot.ru)
// ==========  Copyright by Alexey Sokirko

#pragma once

#include "agramtab_.h"


class CRusGramTab : public CAgramtab {
    const static size_t rStartUp = 0xC0E0; //Аа
    const static size_t rEndUp = 0x10000;  //яя
    const static size_t rMaxGrmCount = rEndUp - rStartUp; // // 16159  (16 Кб)
protected:

    grammems_mask_t DeduceGrammems(part_of_speech_t PartOfSpeech, grammems_mask_t grammems) const override;

public:
    CAgramtabLine *Lines[rMaxGrmCount];

    CRusGramTab();
    ~CRusGramTab() override;


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

    const char *GetRegistryString() const override;

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

    bool is_small_number(const char *lemma) const override;

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

    bool is_morph_article(part_of_speech_mask_t poses) const override;

    bool is_morph_personal_pronoun(part_of_speech_mask_t poses, grammems_mask_t grammems) const override;

    bool IsSimpleParticle(const char *lemma, part_of_speech_mask_t poses) const override;

    bool IsSynNoun(part_of_speech_mask_t poses, const char *Lemma) const override;

    bool IsStandardParamAbbr(const char *WordStrUpper) const override;

    bool FilterNounNumeral(std::string &gcNoun, const std::string &gcNum, grammems_mask_t &grammems) const override;

    grammems_mask_t
    ChangeGleicheAncode1(GrammemCompare CompareFunc, const std::string &wordGramCodes, std::string &groupGramCodes,
                         const grammems_mask_t wordGrammems) const override;

    bool PartOfSpeechIsProductive(part_of_speech_t p) const override;
};

extern bool GenderNumberCaseRussian(const CAgramtabLine *l1, const CAgramtabLine *l2);

extern bool FiniteFormCoordRussian(const CAgramtabLine *l1, const CAgramtabLine *l2);

extern bool CaseNumber(const CAgramtabLine *l1, const CAgramtabLine *l2);

extern bool CaseGender(const CAgramtabLine *l1, const CAgramtabLine *l2);

extern bool CaseNumberGender(const CAgramtabLine *l1, const CAgramtabLine *l2);

extern bool CaseNumberGender0(const CAgramtabLine *l1, const CAgramtabLine *l2); //with absent grammems check
extern bool GenderNumber0(const CAgramtabLine *l1, const CAgramtabLine *l2); //with absent grammems check

