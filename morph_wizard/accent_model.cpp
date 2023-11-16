#include "accent_model.h"
#include "../common/util_classes.h"


/*std::string CAccentModel::ToString() const {
    std::string Result;
    for (size_t i = 0; i < m_Accents.size(); i++) {
        Result += Format("%i;", m_Accents[i]);
    };
    return Result;
};
*/

CAccentModel& CAccentModel::FromJson(nlohmann::json inj) {
    m_Accents.clear();
    for (auto f : inj) {
        m_Accents.push_back(f);
    }
    return *this;
}

CAccentModel& CAccentModel::FromString(const std::string& s) {
    m_Accents.clear();
    size_t prev = 0;
    size_t i = s.find(' ');
    while (i != s.npos) {
        BYTE b = atoi(s.substr(prev, i - prev).c_str());
        m_Accents.push_back(b);
        prev = i + 1;
        i = s.find(';', prev);
    }
    return *this;

}
