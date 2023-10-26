#include "morph_dict/LemmatizerBaseLib/Predict.h"
#include "morph_dict/MorphWizardLib/wizard.h"
#include "morph_dict/LemmatizerBaseLib/MorphDictBuilder.h"
#include "morph_dict/LemmatizerBaseLib/Lemmatizers.h"

#include "morph_dict/common/argparse.h"

void PrintUsage()
{
    printf("Dialing Morphological Generator (from text format (*.mrd) to binary(*.bin))\n");
    printf("Usage: MorphGen <mwz-file> OutputCatalog <PostfixLength> <MinFreq> [--allow-russian-jo]\n");
    printf("where <mwz-file> is a morph_dict project file\n");
    printf("<PostfixLength> is the length of the predicting postfix (can be 1,2,3,4,5). \n");
    printf("     By these postfixes will be unkonwn words predicted\n");
    printf("     The size of the prediction base depends crucialy upon this value\n");
    printf("<MinFreq> is the minimal frequence of the predicting postfix in the dictionary\n");
    printf("if --allow-russian-jo is swithched, then the program does not convert Russian 'jo' to 'e'\n");
    printf("if --allow-russian-jejo is swithched, then the program supports both 'e' and 'jo'\n");

    printf("Example: MorphGen rus.mwz c:/RUS_BIN/ 3 2\n");
    exit(1);
};


void initArgParser(int argc, const char** argv, ArgumentParser& parser) {
    parser.AddOption("--help");
    parser.AddArgument("--input", "input file");
    parser.AddArgument("--output-folder", "output folder");
    parser.AddArgument("--postfix-len", "postfix len", true);
    parser.AddArgument("--min-freq", "min freq", true);
    parser.AddArgument("--log-level", "log level", true);
    parser.AddOption("--allow-russian-jo");
    parser.AddOption("--allow-russian-jejo");
    parser.Parse(argc, argv);
}


void write_opt_file(std::string opt_file, bool bAllowRussianJo, bool SkipPredictBase) {
}

int main(int argc, const char* argv[])
{
    ArgumentParser args;
    initArgParser(argc, argv, args);
    init_plog(args.GetLogLevel(), "morph_gen.log");
    bool bAllowRussianJeJo = args.Exists("allow-russian-jejo");
    int PostfixLength = -1;
    int MinFreq = -1;

    nlohmann::json opts;
    opts["AllowRussianJo"] = bAllowRussianJeJo || args.Exists("allow-russian-jo");
    opts["SkipPredictBase"] = false;
    if (!args.Exists("postfix-len") || !args.Exists("min-freq")) {
        opts["SkipPredictBase"] = true;
        LOGI << "skip prediction base generation ";
    }
    else {
        PostfixLength = atoi(args.Retrieve("postfix-len").c_str());
        if ((PostfixLength == 0) || (PostfixLength > 5))
        {
            LOGI << "PostfixLength is std::set to " << PostfixLength << "";
            LOGI << "PostfixLength should be between 1 and 5";
            return 1;
        };

        int MinFreq = atoi(args.Retrieve("min-freq").c_str());
        if (MinFreq <= 0) {
            LOGI << "MinFreq should be more than 0";
            return 1;
        };
    }
    MorphLanguageEnum wizardLangua;
    try {

        std::string output_folder = args.Retrieve("output-folder");

        MorphoWizard Wizard;
        Wizard.load_wizard(args.Retrieve("input").c_str(), "guest", false);
        wizardLangua = Wizard.m_Language;
        if (!opts["AllowRussianJo"])
        {
            LOGI << "prepare_for_RML";
            if (!Wizard.prepare_for_RML())
                return 1;
        };
        if (bAllowRussianJeJo)
        {
            LOGI << "prepare_for_RML2";
            if (!Wizard.prepare_for_RML2())
                return 1;
        };
        
        {
            CMorphDictBuilder R(Wizard.m_Language);
            R.GenerateLemmas(Wizard);
            R.GenerateUnitedFlexModels(Wizard);
            R.CreateAutomat(Wizard);
            LOGI << "Saving...";
            auto outFileName = std::filesystem::path(output_folder) / MORPH_MAIN_FILES;
            R.Save(outFileName.string());
            LOGI << "Successful written indices of the main automat to " << outFileName << std::endl;
            if (!opts["SkipPredictBase"]) {
                if (!R.GenPredictIdx(Wizard, PostfixLength, MinFreq, output_folder, opts))
                {
                    LOGI << "Cannot create prediction base";
                    return 1;
                };
            }
        }

        {
            auto opt_path = std::filesystem::path(output_folder) / OPTIONS_FILE;
            LOGI << "writing options file " << opt_path;
            std::ofstream file(opt_path);
            file << opts.dump(4);
            file.close();
        }

        {
            std::filesystem::path src =  Wizard.m_GramtabPath;
            std::filesystem::path trg = output_folder / Wizard.m_GramtabPath.filename();
            if (!std::filesystem::exists(trg) || !std::filesystem::equivalent(src, trg))  {
                std::filesystem::copy_file(src, trg, std::filesystem::copy_options::overwrite_existing);
            }
        }
    }
    catch (CExpc e)
    {
        LOGE << "exception=" << e.m_strCause << std::endl;
        return 1;
    }
    catch (const std::exception& ex) {
        LOGE << "exception=" << ex.what()<< std::endl;
        return 1;
    }
    catch (...)
    {
        LOGE << "Can not Generate,  general exception";
        return 1;
    }
    LOGI << "exit with success";
    return 0;
}


