#include "wizard_base.h"
#include "../common/utilit.h"
#include <fstream>



static size_t getCount(std::ifstream& mrdFile, const char* sectionName) {
    std::string line;
    if (!getline(mrdFile, line)) {
        throw CExpc("Cannot get size of section  %s", sectionName);
    }
    return atoi(line.c_str());
}


void CMorphWizardBase::SaveFlexiaModelsToJson(CJsonObject& out) const {
    auto m = rapidjson::Value(rapidjson::kArrayType);
    for (const auto& model : m_FlexiaModels) {
        CJsonObject v(out.get_doc());
        model.ToJson(v);
        m.PushBack(v.get_value(), out.get_allocator());
    }
    out.move_to_member("flexia_models", m);
    
};

void  CMorphWizardBase::SerializeFlexiaModelsToAnnotFile(std::ostream& outp) const {
    outp << m_FlexiaModels.size() << "\n";
    for (const auto& f : m_FlexiaModels) {
        outp << f.ToString();
    }
};

void  CMorphWizardBase::SaveAccentModelsToJson(CJsonObject& out) const {
    auto models = rapidjson::Value(rapidjson::kArrayType);
    for (const auto& a : m_AccentModels) {
        auto m = rapidjson::Value(rapidjson::kArrayType);
        for (auto i : a.m_Accents) {
            m.PushBack(i, out.get_allocator());
        };
        models.PushBack(m.Move(), out.get_allocator());
    }
    out.move_to_member("accent_models", models);
};

void  CMorphWizardBase::SerializeAccentModelsToAnnotFile(std::ostream& outp) const {
    outp << m_AccentModels.size() << "\n";
    for (const auto& a : m_AccentModels) {
        for (auto i : a.m_Accents) {
            outp << (int)i << " ";
        }
        outp << "\n";
    }
};
