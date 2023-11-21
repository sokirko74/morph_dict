#include "accent_model.h"
#include "../common/util_classes.h"


CAccentModel& CAccentModel::FromJson(const rapidjson::Value & inj) {
    m_Accents.clear();
    for (auto& f : inj.GetArray()) {
        m_Accents.push_back(f.GetInt());
    }
    return *this;
}

CAccentModel& CAccentModel::FromString(const std::string& s) {
    m_Accents.clear();
    size_t prev = 0;
    const char delim = ' ';
    size_t i = s.find(delim);
    while (i != s.npos) {
        BYTE b = atoi(s.substr(prev, i - prev).c_str());
        m_Accents.push_back(b);
        prev = i + 1;
        i = s.find(delim, prev);
    }
    return *this;

}
