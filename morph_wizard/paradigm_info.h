#include "lemma_info.h"

#include "../common/utilit.h"
#include "../common/json.h"


const uint16_t UnknownSessionNo = 0xffff - 1;
const uint16_t UnknownPrefixSetNo = 0xffff - 1;
const BYTE UnknownAccent = 0xff;	// не менять - уже проставлено в mrd
const uint16_t AnyParadigmNo = 0xffff;
const uint16_t AnyAccentModelNo = 0xffff;
const uint16_t AnySessionNo = 0xffff;
const uint16_t AnyPrefixSetNo = 0xffff;
const BYTE AnyAccent = 0xff - 1;
extern const char* AnyCommonAncode;


struct CParadigmInfo : public CLemmaInfo
{
    std::string m_Lemma;
    uint16_t	m_SessionNo;
    uint16_t	m_PrefixSetNo;
    BYTE	m_AuxAccent;
    bool	m_bToDelete;

    CParadigmInfo();
    CParadigmInfo(uint16_t ParadigmNo, uint16_t AccentModelNo, uint16_t SessionNo, BYTE AuxAccent, const char* CommonAncode, uint16_t PrefixSetNo);
    bool operator == (const CParadigmInfo& X) const;

    CParadigmInfo	AnyParadigmInfo();
    bool	IsAnyEqual(const CParadigmInfo& X) const;
    nlohmann::json GetJson(const std::string& lemma) const;
    CParadigmInfo& FromJson(nlohmann::json inj);
};

