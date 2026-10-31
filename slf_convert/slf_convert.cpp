// Converts paradigms in the slf format (Tools/Paradigm Export of MorphWizard) to a dictionary (morphs.json)
// and back, without the GUI.
//   slf_convert --to-mrd --slf paradigms.slf --mwz folder/project.mwz
//      adds all paradigms of the slf file to the dictionary of the project, an absent mrd file is created empty,
//      the gramtab (gramtab.json) is read from the folder of the project
//   slf_convert --to-slf --mwz folder/project.mwz --slf paradigms.slf
//      writes all paradigms of the dictionary

#include "../morph_wizard/wizard.h"
#include "../morph_wizard/DumpParadigm.h"
#include "morph_dict/common/argparse.h"

#include <filesystem>

namespace fs = std::filesystem;


void initArgParser(int argc, const char** argv, ArgumentParser& parser) {
    parser.AddOption("--help");
    parser.AddOption("--to-mrd", "add paradigms from the slf file to the dictionary");
    parser.AddOption("--to-slf", "write all paradigms of the dictionary to the slf file");
    parser.AddArgument("--slf", "slf file");
    parser.AddArgument("--mwz", "project file of MorphWizard (project.mwz)");
    parser.AddArgument("--user", "user name for the session", true);
    parser.Parse(argc, argv);
}

static void create_empty_mrd(const fs::path& mwz_path) {
    std::ifstream mwzFile(mwz_path);
    if (!mwzFile.is_open()) {
        throw CExpc("Cannot open file " + mwz_path.string());
    }
    rapidjson::IStreamWrapper isw(mwzFile);
    rapidjson::Document d;
    d.ParseStream(isw);
    auto mrd_path = fs::absolute(mwz_path).parent_path() / d["MRD_FILE"].GetString();
    if (fs::exists(mrd_path)) return;
    std::ofstream outp(mrd_path);
    outp << "{\"flexia_models\": [], \"accent_models\": [], \"sessions\": [], \"prefix_sets\": [], \"lemmas\": []}\n";
}

static void slf_to_mrd(MorphoWizard& wizard, const std::string& slf_path) {
    FILE* fp = fopen(slf_path.c_str(), "rb");
    if (fp == nullptr) {
        throw CExpc("Cannot open file " + slf_path);
    }
    CDumpParadigm paradigm(&wizard);
    int line_no = 0;
    size_t added = 0, errors_count = 0;
    while (true) {
        bool bError = false;
        std::string errors;
        int start_line_no = line_no;
        if (!paradigm.ReadNextParadigmFromFile(fp, line_no, bError, errors)) break;
        if (bError) {
            std::cerr << errors;
            ++errors_count;
            continue;
        }
        int line_no_err = 0;
        try {
            wizard.add_lemma_to_dict(paradigm.m_SlfStr, paradigm.m_TypeGrammemsStr, paradigm.m_PrefixesStr, line_no_err);
            ++added;
        }
        catch (CExpc& e) {
            // line_no_err counts the lines of the paradigm, start_line_no is the separator before it
            std::cerr << "line " << paradigm.m_FirstSlfLineNo + line_no_err << ": " << e.what() << "\n";
            ++errors_count;
        }
        if (start_line_no == line_no) break;
    }
    fclose(fp);
    std::cerr << "added " << added << " paradigms, " << errors_count << " errors\n";
}

static void mrd_to_slf(MorphoWizard& wizard, const std::string& slf_path) {
    std::ofstream outp(slf_path, std::ios::binary);
    if (!outp.is_open()) {
        throw CExpc("Cannot open file " + slf_path);
    }
    CDumpParadigm paradigm(&wizard);
    for (auto it = wizard.m_LemmaToParadigm.begin(); it != wizard.m_LemmaToParadigm.end(); ++it) {
        paradigm.SetEmpty();
        paradigm.m_SlfStr = wizard.get_slf_string(it, paradigm.m_TypeGrammemsStr, paradigm.m_PrefixesStr, 0);
        outp << paradigm.ToStringUtf8();
    }
    std::cerr << "written " << wizard.m_LemmaToParadigm.size() << " paradigms\n";
}

int main(int argc, const char* argv[]) {
    ArgumentParser args;
    initArgParser(argc, argv, args);
    if (args.Exists("to-mrd") == args.Exists("to-slf")) {
        std::cerr << "specify one of --to-mrd and --to-slf\n";
        return 1;
    }
    try {
        std::string user = args.Exists("user") ? args.Retrieve("user") : "guest";
        if (args.Exists("to-mrd")) {
            create_empty_mrd(args.Retrieve("mwz"));
        }
        MorphoWizard wizard;
        wizard.m_bFullTrace = false;
        wizard.load_wizard(args.Retrieve("mwz"), user, false, false, false);
        if (args.Exists("to-mrd")) {
            slf_to_mrd(wizard, args.Retrieve("slf"));
            wizard.save_mrd();
        }
        else {
            mrd_to_slf(wizard, args.Retrieve("slf"));
        }
    }
    catch (CExpc& e) {
        std::cerr << e.what() << "\n";
        return 1;
    }
    return 0;
}
