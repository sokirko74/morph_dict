#include "morph_dict/common/utilit.h"
#include "morph_dict/common/argparse.h"
#include "morph_dict/agramtab/RusGramTab.h"
#include "morph_dict/agramtab/EngGramTab.h"
#include "morph_dict/agramtab/GerGramTab.h"
#include "morph_dict/lemmatizer_base_lib/Lemmatizers.h"
#include "morph_dict/lemmatizer_base_lib/MorphanHolder.h"

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <locale.h>
#include <cstdlib>

CMorphanHolder Holder;



void CheckSpeed(std::istream& inputStream, std::ostream& output) {
    StringVector Forms;
    {
        std::string line;
        while(getline(inputStream, line)) {
            Trim(line);
            if (line.empty()) continue;
            MakeLowerUtf8(line);
            line = convert_from_utf8(line.c_str(), Holder.m_CurrentLanguage);
            Forms.push_back(line);
        };
    };
    size_t Count = Forms.size();

    StringVector Results;
    Results.resize(Count, std::string(255, ' '));
    clock_t start = clock();
    const size_t OutBufferSize = 2000;
    char OutBuffer[OutBufferSize];
    for (size_t i = 0; i < Count; i++) {
        if (!Holder.m_pLemmatizer->GetAllAncodesAndLemmasQuick(Forms[i], true, OutBuffer, OutBufferSize, true)) {
            std::cerr << "Too many variants for " <<  Forms[i] << "\n";
        };
        Results[i] = (char *) OutBuffer;
    };
    long ticks = clock() - start;
    std::cerr << "Count of words = " <<  Count << "\n";
    double seconds = (double) ticks / (double) CLOCKS_PER_SEC;
    std::cerr << "Time = " << seconds <<  "; " << ticks << " ticks" << "\n";
    if (seconds > 0)
        std::cerr << "Speed = " << ((double) Count) / seconds << " words per seconds\n";
    else
        std::cerr << "too few words to measure the speed\n";

    for (size_t i = 0; i < Count; i++)
        output << Forms[i] << " -> " << Results[i] << "\n";
};



void initArgParser(int argc, const char **argv, ArgumentParser& parser) {
    parser.AddOption("--help");
    parser.AddArgument("--input", "input file in utf8");
    parser.AddArgument("--output", "output file");
    parser.AddArgument("--language", "language");
    parser.AddOption("--no-ids");
    parser.AddOption("--sort");
    parser.AddOption("--forms");
    parser.AddOption("--morphan");
    parser.AddArgument("--morph-folder", "use binary dict from this folder", true);
    parser.AddOption("--speed-test", "attention, input file must be in a single-byte encoding");
    parser.AddOption("--echo");
    parser.AddOption("--misspell");
    parser.Parse(argc, argv);
}

int main(int argc, const char **argv) {
    ArgumentParser args;
    initArgParser(argc, argv, args);
    MorphLanguageEnum  language = args.GetLanguage();
    auto printIds = !args.Exists("no-ids");
    auto printForms = args.Exists("forms");
    auto sortParadigms = args.Exists("sort");
    bool bEchoInput = args.Exists("echo");
    
	std::cerr << "Loading..\n";
   
    try {
        if (args.Exists("morph-folder")) {
            Holder.LoadMorphology(language, args.Retrieve("morph-folder"));
        } else {
            Holder.LoadMorphology(language);
        }

        if (args.Exists("speed-test")) {
            CheckSpeed(args.GetInputStream(), args.GetOutputStream());
            return 0;
        };

        std::cerr << "Input a word..\n";
        std::string word;
        while (getline(args.GetInputStream(), word)) {
            Trim(word);
            if (word.empty()) break;
		    if (bEchoInput) {
			    args.GetOutputStream() << word  << "\t";
		    }
            auto word_s8 = convert_from_utf8(word.c_str(), language);
		    std::string result;
            if (args.Exists("misspell")) {
                if (Holder.IsInDictionaryUtf8(word)) {
                    result = "in dictionary\n";
                }
                else {
                    for (auto a: Holder.CorrectMisspelledWordUtf8(word)) {
                        result += Format(" corrected = %s distance= %i;", a.CorrectedString.c_str(), a.StringDistance);
                    }
                }
            }
            else if (args.Exists("morphan")) {
                result = Holder.LemmatizeJson(word.c_str(), printForms, true, true);
            }
            else {
                result = Holder.PrintMorphInfoUtf8(word, printIds, printForms, sortParadigms);
                
            }
            args.GetOutputStream() << result << "\n";
    	};
    }
    catch (CExpc c) {
        std::cerr << c.what() << "\n";
        return 1;
    }

    return 0;
}
