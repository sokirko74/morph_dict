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

CMorphForm::CMorphForm(const rapidjson::Value& j) {
    m_FlexiaStr = j["flexia"].GetString();
    m_Gramcode = j["gramcode"].GetString();
    auto a = rapidjson::Pointer("/prefix").Get(j);
    if (a) {
        m_PrefixStr = a->GetString();
    }
    
}

CMorphForm::CMorphForm(const std::string& s) {
    FromString(s);
}


void CMorphForm::ToJson(CJsonObject& out) const {
    out.add_string("flexia", m_FlexiaStr);
    out.add_string("gramcode", m_Gramcode);
    if (!m_PrefixStr.empty()) {
        out.add_string("prefix", m_PrefixStr);
    }
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


const std::string& CFlexiaModel::get_first_flex() const {
    assert(!m_Flexia.empty());
    return m_Flexia[0].m_FlexiaStr;
};


const std::string& CFlexiaModel::get_first_code() const {
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


void CFlexiaModel::ToJson(CJsonObject& out) const {
    rapidjson::Value endings(rapidjson::kArrayType);
    for (const auto& f : m_Flexia) {
        CJsonObject o(out.get_doc());
        f.ToJson(o);
        endings.PushBack(o.get_value().Move(), out.get_allocator());
    };
    
    out.move_to_member("endings", endings);

    if (!m_Comments.empty()) {
        out.add_string("comments",  m_Comments);
    }
    
    if (!m_WiktionaryMorphTemplate.empty()) {
        out.add_string("wiki", m_WiktionaryMorphTemplate);
    }
};


CFlexiaModel& CFlexiaModel::FromJson(const rapidjson::Value& inj) {
    m_Flexia.clear();
    for (const auto& f : inj["endings"].GetArray()) {
        CMorphForm form(f);
        m_Flexia.push_back(form);
    }
    auto c = rapidjson::Pointer("/comments").Get(inj);
    if (c) m_Comments = c->GetString();
    c = rapidjson::Pointer("/wiki").Get(inj);
    if (c) m_WiktionaryMorphTemplate = c->GetString();
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

