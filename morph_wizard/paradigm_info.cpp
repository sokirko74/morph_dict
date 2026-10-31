#include "paradigm_info.h"


bool CParadigmPart::operator==(const CParadigmPart& X) const {
    return m_FlexiaModelNo == X.m_FlexiaModelNo && m_AccentModelNo == X.m_AccentModelNo;
}

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
    SetCommonAncode(CommonAncode);
    m_PrefixSetNo = PrefixSetNo;
};

bool CParadigmInfo::operator==(const CParadigmInfo& X) const {
    return m_FlexiaModelNo == X.m_FlexiaModelNo
        && m_AccentModelNo == X.m_AccentModelNo
        && m_AuxAccent == X.m_AuxAccent
        && !CompareCommonAncode(X.GetCommonAncode())
        && m_PrefixSetNo == X.m_PrefixSetNo
        && m_NextParts == X.m_NextParts;
};

bool CParadigmInfo::HasFlexiaModel(uint16_t FlexiaModelNo) const {
    if (m_FlexiaModelNo == FlexiaModelNo) return true;
    for (const auto& p : m_NextParts) {
        if (p.m_FlexiaModelNo == FlexiaModelNo) return true;
    }
    return false;
}

bool CParadigmInfo::HasAccentModel(uint16_t AccentModelNo) const {
    if (m_AccentModelNo == AccentModelNo) return true;
    for (const auto& p : m_NextParts) {
        if (p.m_AccentModelNo == AccentModelNo) return true;
    }
    return false;
}

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
        && (   !CompareCommonAncode(X.GetCommonAncode())
            || !CompareCommonAncode(AnyCommonAncode)
            || !X.CompareCommonAncode(AnyCommonAncode))
        && (m_PrefixSetNo == AnyPrefixSetNo ||
            X.m_PrefixSetNo == AnyPrefixSetNo ||
            m_PrefixSetNo == X.m_PrefixSetNo)
        );
}


void  CParadigmInfo::GetJson(const std::string& lemma, CJsonObject& out) const {
    out.add_string("l", lemma);
    out.add_int("f", m_FlexiaModelNo);
    out.add_int("a", m_AccentModelNo);
    out.add_int("s", m_SessionNo);
    if (GetCommonAncode()[0] != 0) {
        out.add_string("t", GetCommonAncode());
    }
    if (m_PrefixSetNo != UnknownPrefixSetNo) {
        out.add_int("p", m_PrefixSetNo);
    }
    if (!m_NextParts.empty()) {
        rapidjson::Value parts(rapidjson::kArrayType);
        for (const auto& p : m_NextParts) {
            rapidjson::Value part(rapidjson::kArrayType);
            part.PushBack(p.m_FlexiaModelNo, out.get_allocator());
            part.PushBack(p.m_AccentModelNo, out.get_allocator());
            parts.PushBack(part.Move(), out.get_allocator());
        }
        out.move_to_member("n", parts);
    }
}

CParadigmInfo& CParadigmInfo::FromJson(const rapidjson::Value& inj) {
    m_Lemma = inj["l"].GetString();
    m_FlexiaModelNo = inj["f"].GetInt();
    m_AccentModelNo = inj["a"].GetInt();
    m_SessionNo = inj["s"].GetInt();
    auto t = rapidjson::Pointer("/t").Get(inj);
    if (t) {
        SetCommonAncode(t->GetString());
    }
    else {
        SetCommonAncode("");
    }
    t = rapidjson::Pointer("/p").Get(inj);
    if (t) {
        m_PrefixSetNo = t->GetInt();
    }
    else {
        m_PrefixSetNo = UnknownPrefixSetNo;
    }
    m_NextParts.clear();
    t = rapidjson::Pointer("/n").Get(inj);
    if (t) {
        for (const auto& p : t->GetArray()) {
            m_NextParts.push_back({ (uint16_t)p[0].GetInt(), (uint16_t)p[1].GetInt() });
        }
    }
    return *this;
}
