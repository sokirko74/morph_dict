#include "lemma_info.h"
#include "accent_model.h"

#include "../common/utilit.h"
#include "paradigm_consts.h"

extern const char* AnyCommonAncode;


// a part of a word: a flexia model and its accent model, the forms of all parts of a word have the same base
struct CParadigmPart
{
    uint16_t	m_FlexiaModelNo;
    uint16_t	m_AccentModelNo;

    bool operator == (const CParadigmPart& X) const;
};

typedef std::vector<CParadigmPart> CFlexiaModelVector;

// a part of a word in a slf text, parts are separated by empty lines
struct CSlfPart
{
    CFlexiaModel	m_FlexiaModel;
    CAccentModel	m_AccentModel;
};


struct CParadigmInfo : public CLemmaInfo
{
    std::string m_Lemma;
    // A word can consist of several parts, for example, variants (воскресение, воскресенье) as they are shown
    // in Wiktionary. The first part, which gives the lemma and the base, is stored in CLemmaInfo, here are the others.
    // The binary dictionary knows nothing about parts, MorphDictBuilder merges them to one flexia model.
    CFlexiaModelVector m_NextParts;
    uint16_t	m_SessionNo;
    uint16_t	m_PrefixSetNo;
    BYTE	m_AuxAccent;
    bool	m_bToDelete;

    CParadigmInfo();
    CParadigmInfo(uint16_t ParadigmNo, uint16_t AccentModelNo, uint16_t SessionNo, BYTE AuxAccent, const char* CommonAncode, uint16_t PrefixSetNo);
    bool operator == (const CParadigmInfo& X) const;

    CParadigmInfo	AnyParadigmInfo();
    bool	HasFlexiaModel(uint16_t FlexiaModelNo) const;
    bool	HasAccentModel(uint16_t AccentModelNo) const;
    bool	IsAnyEqual(const CParadigmInfo& X) const;
    void GetJson(const std::string& lemma, CJsonObject& out) const;
    CParadigmInfo& FromJson(const rapidjson::Value& inj);
};

