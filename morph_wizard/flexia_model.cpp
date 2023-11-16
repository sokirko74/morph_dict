#include "flexia_model.h"
#include "../common/util_classes.h"

const char FlexModelCommDelim[] = "q//q";
const char WiktionaryMorphTemplateDelim[] = ";";

CMorphForm::CMorphForm(std::string Gramcode, std::string FlexiaStr, std::string PrefixStr)
{
    m_Gramcode = Gramcode;
    m_FlexiaStr = FlexiaStr;
    m_PrefixStr = PrefixStr;
    assert(!m_Gramcode.empty());
};

CMorphForm::CMorphForm(nlohmann::json j) {
    j.at("flexia").get_to(m_FlexiaStr);
    j.at("gramcode").get_to(m_Gramcode);
    m_PrefixStr = j.value("prefix", "");
}

CMorphForm::CMorphForm(const std::string& s) {
    FromString(s);
}

nlohmann::json CMorphForm::ToJson() const {
    nlohmann::json j;
    j["flexia"] = m_FlexiaStr;
    j["gramcode"] = m_Gramcode;
    if (!m_PrefixStr.empty()) {
        j["prefix"] = m_PrefixStr;
    }
    return j;
}

bool CMorphForm::operator == (const CMorphForm& X) const
{
    return			m_Gramcode == X.m_Gramcode
        && m_FlexiaStr == X.m_FlexiaStr
        && m_PrefixStr == X.m_PrefixStr;
};

std::string CMorphForm::ToString() const {
    return Format("%s\t%s\t%s", m_Gramcode.c_str(), m_FlexiaStr.c_str(), m_PrefixStr.c_str());
}

CMorphForm& CMorphForm::FromString(const std::string& s) {
    // speed is important for debugging in RML, lemmatizer is used very frequently 
    int i1 = s.find('\t');
    m_Gramcode = s.substr(0, i1);
    int i2 = s.rfind('\t');
    m_FlexiaStr = s.substr(i1 + 1, i2 - i1 - 1);
    m_PrefixStr = s.substr(i2 + 1);
    return *this;
}



std::string CFlexiaModel::get_first_flex() const {
    assert(!m_Flexia.empty());
    return m_Flexia[0].m_FlexiaStr;
};


std::string CFlexiaModel::get_first_code() const {
    assert(!m_Flexia.empty());
    return m_Flexia[0].m_Gramcode;
}


bool CFlexiaModel::has_ancode(const std::string& search_ancode) const {
    for (size_t i = 0; i < m_Flexia.size(); i++) {
        size_t match = m_Flexia[i].m_Gramcode.find(search_ancode);
        if ((match != std::string::npos) && (match % 2 == 0))
            return true;
    }
    return false;
};


nlohmann::json CFlexiaModel::ToJson() const {
    auto endings = nlohmann::json::array();
    for (auto f : m_Flexia) {
        auto o = f.ToJson();
        endings.push_back(o);
    };
    nlohmann::json r;
    r["endings"] = endings;
    if (!m_Comments.empty()) {
        r["comments"] = m_Comments;
    }
    if (!m_Comments.empty()) {
        r["wiki"] = m_WiktionaryMorphTemplate;
    }
    return r;
};

CFlexiaModel& CFlexiaModel::FromJson(nlohmann::json inj) {
    m_Flexia.clear();
    for (auto f : inj["endings"]) {
        CMorphForm form(f);
        m_Flexia.push_back(form);
    }
    m_Comments = inj.value("comments", "");
    m_WiktionaryMorphTemplate = inj.value("wiki", "");
    return *this;
}

std::string CFlexiaModel::ToString() const {
    std::ostringstream sp;
    for (const auto& p : m_Flexia) {
        sp << p.ToString() << ";";
    }
    sp << "\n";
    return sp.str();
}

CFlexiaModel& CFlexiaModel::FromString(const std::string& s) {
    m_Flexia.clear();
    size_t prev = 0;
    size_t i = s.find(';');
    while (i != s.npos) {
        CMorphForm form(s.substr(prev, i - prev));
        m_Flexia.emplace_back(form);
        prev = i + 1;
        i = s.find(';', prev);
    }
    return *this;
}

