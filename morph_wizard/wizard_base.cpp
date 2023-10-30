#include "wizard_base.h"
#include "../common/utilit.h"
#include <fstream>


bool CMorphWizardBase::read_utf8_line(std::ifstream& inp, std::string& line) const {
    if (!getline(inp, line)) {
        return false;
    }
    line = convert_from_utf8(line.c_str(), m_Language);
    return true;
}

std::string CMorphWizardBase::str_to_utf8(const std::string& line) const {
    return convert_to_utf8(line, m_Language);
}

static size_t getCount(std::ifstream& mrdFile, const char* sectionName) {
    std::string line;
    if (!getline(mrdFile, line)) {
        throw CExpc("Cannot get size of section  %s", sectionName);
    }
    return atoi(line.c_str());
}


nlohmann::json CMorphWizardBase::GetFlexiaModelsJson() const {
    auto m = nlohmann::json::array();
    for (auto f : m_FlexiaModels) {
        m.push_back(f.ToJson());
    }
    return m;
};



nlohmann::json  CMorphWizardBase::GetAccentModelsJson() const {
    auto m = nlohmann::json::array();
    for (auto a : m_AccentModels) {
        m.push_back(a.m_Accents);
    }
    return m;
};

