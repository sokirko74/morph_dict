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

struct TPredictItem {
    std::string Input;
    std::string Lemma;
    part_of_speech_t POS;
    grammems_mask_t Grammems;
};


std::vector<TPredictItem> read_base(CMorphanHolder& holder, std::string filename) {
    std::ifstream inp(filename);
    std::vector<TPredictItem> predict_base;
    std::string line;
    holder.m_pGramTab->SetUseNationalConstants(false);
    while(std::getline(inp, line))
    {
        auto parts = split_string(line, '\t');
        if (parts.size() == 0) {
            continue;
        }
        if (parts.size() != 3) {
            auto m = Format("Cannot process line %s,  must be 3 items separated by tabulation",
                line.c_str());
            throw CExpc(m);
        }
        TPredictItem l;
        l.Input = parts[0];
        l.Lemma = parts[1];
        if (!holder.m_pGramTab->ProcessPOSAndGrammems(parts[2].c_str(), l.POS, l.Grammems)) {
            throw CExpc(Format("Cannot process line %s, bad part of speech or grammem", parts[2].c_str()));
        }
        predict_base.push_back(l);

    }
    return predict_base;

}


struct TCheckResult {
    int good = 0;
    int miss = 0;
};


TCheckResult check_word(CMorphanHolder& holder, TPredictItem& i) {
    std::vector<CFormInfo> Paradigms;
    std::string input = convert_from_utf8(i.Input.c_str(), holder.m_CurrentLanguage);
    TCheckResult result;
    holder.m_pLemmatizer->CreateParadigmCollection(false, input, IsUpper((unsigned char)input[0], holder.m_CurrentLanguage,
        m_bUsePrediction, Paradigms);

    size_t paradigms_count = Paradigms.size();
    std::cerr << "generate " << paradigms_count << " paradigms by " << i.Input << "\n";
    //std::cout << "generate \n";
    for (auto p : Paradigms) {
        std::string lemma = convert_to_utf8(p.GetWordForm(0), holder.m_CurrentLanguage);
        if (lemma != i.Lemma) {
            std::cerr << "bad lemma " << lemma << "\n";
            result.miss += 1;
            continue;
        };
        auto ancode = p.GetAncode(0);
        auto pos = holder.m_pGramTab->GetPartOfSpeech(ancode.c_str());
        if (pos != i.POS) {
            std::cerr << "bad part of speench " << holder.m_pGramTab->GetPartOfSpeechStr(pos) << "\n";
            result.miss += 1;
            continue;
        }
        grammems_mask_t g = holder.m_pGramTab->GetAllGrammems(ancode.c_str());
        auto common_ancode = p.GetCommonAncode();
        if (common_ancode.length() > 0) {
            g |= holder.m_pGramTab->GetAllGrammems(common_ancode.c_str());
        }
        if ((i.Grammems & g) != i.Grammems) {
            std::cerr << "grammems " << holder.m_pGramTab->GrammemsToStr(g) << " <> " << holder.m_pGramTab->GrammemsToStr(i.Grammems) << "\n";
            result.miss += 1;
            continue;
        }
        result.good += 1;
        //std::cout << convert_to_utf8(p.GetWordForm(0), morphRussian) << "\n";
        //std::cout << p.GetWordForm(0) << "\n";
    }
    return result;
}



float calc_precision(CMorphanHolder& holder, std::vector<TPredictItem>& base) {
    int all_good = 0;
    int all_miss = 0;
    for (TPredictItem& i : base) {
        TCheckResult res = check_word(holder, i);
        if (res.good > 0) {
            all_good += 1;
        }
        else {
            std::cout << "word " << i.Input << " failed\n";
        }
        all_miss += res.miss;
    }
    std::cout << "all_good = " << all_good << " all_miss = " << all_miss << "\n";
    return (float)all_good / float(all_good + all_miss);
}

int main(int argc, const char **argv) {
    CMorphanHolder Holder;
    Holder.LoadMorphology(morphRussian);
    try {
        auto base = read_base(Holder, argv[1]);
        float prec = calc_precision(Holder, base);
        float min_prec = atof(argv[2]);
        if (prec < min_prec) {
            std::cerr << "test failed prec=" << prec << ", must be greater than " << min_prec << "\n";
            return 1;
        }
        return 0;
    }
    catch (CExpc c) {
        std::cerr << c.what() << "\n";
        return 1;
    }
}
