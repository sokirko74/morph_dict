#include "lemma_info.h"

#include "../common/utilit.h"
#include "../common/json.h"
#include "paradigm_consts.h"

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
    void GetJson(const std::string& lemma, CJsonObject& out) const;
    CParadigmInfo& FromJson(const rapidjson::Value& inj);
};

