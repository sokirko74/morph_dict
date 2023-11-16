#include "morph_dict/lemmatizer_base_lib/Predict.h"
#include "morph_dict/morph_wizard/wizard.h"
#include "morph_dict/lemmatizer_base_lib/MorphDictBuilder.h"
#include "morph_dict/lemmatizer_base_lib/Lemmatizers.h"

#include "morph_dict/common/argparse.h"

void PrintUsage()
{
    printf("Dialing Morphological Generator (from text format (*.mrd) to binary(*.bin))\n");
    printf("Usage: morph_gen <mwz-file> OutputCatalog <PostfixLength> <MinFreq> [--allow-russian-jo]\n");
    printf("where <mwz-file> is a morph_dict project file\n");
    printf("<PostfixLength> is the length of the predicting postfix (can be 1,2,3,4,5). \n");
    printf("     By these postfixes will be unkonwn words predicted\n");
    printf("     The size of the prediction base depends crucialy upon this value\n");
    printf("<MinFreq> is the minimal frequence of the predicting postfix in the dictionary\n");
    printf("if --allow-russian-jo is swithched, then the program does not convert Russian 'jo' to 'e'\n");

    printf("Example: morph_gen rus.mwz c:/RUS_BIN/ 3 2\n");
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
    parser.Parse(argc, argv);
}


int main(int argc, const char* argv[])
{
    ArgumentParser args;
    initArgParser(argc, argv, args);
    init_plog(args.GetLogLevel(), "morph_gen.log");
    try {
        CMorphDictBuilder R;
        R.BuildLemmatizer(args.Retrieve("input"),
            args.Exists("allow-russian-jo"),
            args.Retrieve("postfix-len", -1),
            args.Retrieve("min-freq", -1),
            args.Retrieve("output-folder")
        );
    }
    catch (const std::exception& ex) {
        LOGE << "exception=" << ex.what();
        return 1;
    }
    catch (...)
    {
        LOGE << "general exception in morph_gen";
        return 1;
    }
    LOGI << "exit with success";
    return 0;
}


