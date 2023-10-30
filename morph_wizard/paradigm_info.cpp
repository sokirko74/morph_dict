#include "paradigm_info.h"


CParadigmInfo::CParadigmInfo() : CLemmaInfo() {
    m_SessionNo = UnknownSessionNo;
    m_AuxAccent = UnknownAccent;
    m_bToDelete = false;
    m_PrefixSetNo = UnknownPrefixSetNo;
};

CParadigmInfo::CParadigmInfo(uint16_t ParadigmNo, uint16_t AccentModelNo, uint16_t SessionNo, BYTE AuxAccent,
    const char* CommonAncode, uint16_t PrefixSetNo) {
    m_FlexiaModelNo = ParadigmNo;
    m_bToDelete = false;
    m_AccentModelNo = AccentModelNo;
    m_SessionNo = SessionNo;
    m_AuxAccent = AuxAccent;
    strncpy(m_CommonAncode, CommonAncode, CommonAncodeSize);
    m_PrefixSetNo = PrefixSetNo;
};

bool CParadigmInfo::operator==(const CParadigmInfo& X) const {
    return m_FlexiaModelNo == X.m_FlexiaModelNo
        && m_AccentModelNo == X.m_AccentModelNo
        && m_AuxAccent == X.m_AuxAccent
        && !strncmp(m_CommonAncode, X.m_CommonAncode, CommonAncodeSize)
        && m_PrefixSetNo == X.m_PrefixSetNo;
};

CParadigmInfo CParadigmInfo::AnyParadigmInfo() {
    return CParadigmInfo(AnyParadigmNo, AnyAccentModelNo, AnySessionNo,
        AnyAccent, AnyCommonAncode, AnyPrefixSetNo);
}

//----------------------------------------------------------------------------
bool CParadigmInfo::IsAnyEqual(const CParadigmInfo& X) const {
    return (
        (m_FlexiaModelNo == AnyParadigmNo ||
            X.m_FlexiaModelNo == AnyParadigmNo ||
            m_FlexiaModelNo == X.m_FlexiaModelNo)
        && (m_AccentModelNo == AnyAccentModelNo ||
            X.m_AccentModelNo == AnyAccentModelNo ||
            m_AccentModelNo == X.m_AccentModelNo)
        && (m_AuxAccent == AnyAccent ||
            X.m_AuxAccent == AnyAccent ||
            m_AuxAccent == X.m_AuxAccent)
        && (!strncmp(m_CommonAncode, X.m_CommonAncode, CommonAncodeSize)
            || !strncmp(m_CommonAncode, AnyCommonAncode, CommonAncodeSize)
            || !strncmp(X.m_CommonAncode, AnyCommonAncode, CommonAncodeSize))
        && (m_PrefixSetNo == AnyPrefixSetNo ||
            X.m_PrefixSetNo == AnyPrefixSetNo ||
            m_PrefixSetNo == X.m_PrefixSetNo)
        );
}


nlohmann::json CParadigmInfo::GetJson(const std::string& lemma) const {
    nlohmann::json out;
    out["l"] = lemma;
    out["f"] = m_FlexiaModelNo;
    out["a"] = m_AccentModelNo;
    out["s"] = m_SessionNo;
    if (m_CommonAncode[0] != 0) {
        out["t"] = GetCommonAncodeIfCan();
    }
    if (m_PrefixSetNo != UnknownPrefixSetNo) {
        out["p"] = m_PrefixSetNo;
    }
    return out;

}

CParadigmInfo& CParadigmInfo::FromJson(nlohmann::json inj) {
    m_Lemma = inj["l"];
    m_FlexiaModelNo = inj["f"];
    m_AccentModelNo = inj["a"];
    m_SessionNo = inj["s"];
    auto t = inj.value("t", "");
    if (!t.empty()) {
        strncpy(m_CommonAncode, t.c_str(), CommonAncodeSize);
    }
    m_PrefixSetNo = inj.value("p", UnknownPrefixSetNo);
    return *this;
}
