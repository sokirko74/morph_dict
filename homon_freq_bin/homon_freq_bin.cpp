#include <iostream>
#include "morph_dict/common/util_classes.h"
#include "morph_dict/lemmatizer_base_lib/MorphanHolder.h"
#include "morph_dict/common/bserialize.h"
#include "morph_dict/common/argparse.h"
#include <fstream>

// homon_freq_bin <InDataFile> <-F | -C | -L> [Output Directory]
// Подгружает данные из файла <DataFile> (см. StatTxtDat)
// Записывает триады <pid,form-num,weight> в файл "r*homoweight.bin"

struct CGroup {
    int pid;
    std::string anc;
    int m_FormNo;

    CGroup() : pid(0), anc("") {}

    friend bool operator<(const CGroup& x, const CGroup& y);
};

bool operator<(const CGroup& x, const CGroup& y) {
    if (x.anc < y.anc)
        return (1);
    if (x.anc > y.anc)
        return (0);
    return (x.pid < y.pid);
}

typedef std::map<CGroup, int> tagLines;
typedef std::map<std::string, tagLines> tagHomon;
static tagHomon homon;

//////////////////////////////////////////////////////////////////////////////

typedef troika<int, int, int> _homonode_t;
typedef std::vector<_homonode_t> _homoresults_t;

struct less4homonode : public std::less<_homonode_t> {
    bool operator()(const _homonode_t& x, const _homonode_t& y) const {
        return (x.first == y.first
            ? x.second < y.second
            : x.first < y.first);
    }
};

static _homoresults_t homonweight;
CMorphanHolder MorphHolder;

static void addLines(std::string& str, tagLines& lines) {
    std::pair<tagHomon::iterator, bool> res;
    res = homon.insert(tagHomon::value_type(str, lines));
    if (!res.second) {
        LOGD << "Duplicate word: " << str << " skipped.";
    }
}

bool check_part_of_speech(part_of_speech_t pos, part_of_speech_mask_t poses) {
    if (((1 << pos) & poses) > 0) {
        return true;
    }
    if (MorphHolder.m_pGramTab->is_morph_adj(1<<pos) && MorphHolder.m_pGramTab->is_morph_adj(poses)) {
        return true;
    }
    if (MorphHolder.m_pGramTab->is_verb_form(1<<pos) && MorphHolder.m_pGramTab->is_verb_form(poses)) {
        return true;
    }
    return false;
}
bool check_word_form(const CFormInfo& paradigm, uint16_t formNo, std::string wordForm, part_of_speech_t pos, grammems_mask_t gra) {
    std::string ancode = paradigm.GetAncode(formNo); 
    assert(ancode.size() == 2);
    part_of_speech_mask_t poses;
    grammems_mask_t grammems;
    MorphHolder.m_pGramTab->GetPartOfSpeechAndGrammems((const BYTE*)ancode.c_str(), poses, grammems);
    if (!check_part_of_speech(pos, poses)) {
        return false;
    }
    grammems |= MorphHolder.m_pGramTab->GetAllGrammems(paradigm.GetCommonAncode().c_str());
    if ((grammems & gra) != gra) {
        return false;;
    }
    if (paradigm.GetWordForm(formNo) != wordForm) {
        return false;
    }
    return true;

}

static void loadDat(std::istream& ifs, MorphLanguageEnum langua) {
    std::string str;
    tagLines lines;

    std::string line;
    int lin = 0;
    while (getline(ifs, line)) {
        std::string buf_s = convert_from_utf8(line.c_str(), langua);
        Trim(buf_s);
        lin++;
        if (buf_s.empty()) {
            continue;
        }
        auto elems = split_string(buf_s, ' ');
        if (elems.size() != 4 && elems.size() != 5) {
            throw std::runtime_error(Format("Error in line: %i skipped", lin));
        }
        std::string wordForm = elems[0];
        std::string lemma = elems[1];
        std::string partOfSpeechStr = elems[2];
        int freq = atoi(elems.back().c_str());
        if (freq < 0)
        {
            throw std::runtime_error("bad freq on line " + line);
        }
        grammems_mask_t gra = 0;
        part_of_speech_t pos;
        if (elems.size() == 5) {
            auto s = partOfSpeechStr + " " + elems[3];
            if (!MorphHolder.m_pGramTab->ProcessPOSAndGrammemsIfCan(s.c_str(), &pos, &gra)) {
                throw std::runtime_error(Format("Bad part of speech or grammems: line %i ", lin));
            }
        }
        else {
            pos = MorphHolder.m_pGramTab->GetPartOfSpeechByStr(partOfSpeechStr);
            if (pos == UnknownPartOfSpeech) {
                throw std::runtime_error(Format("Bad part of speech: line %i ", lin));
            }
        }
        if (wordForm != "*") {
            if (!str.empty() && lines.size() > 0)
                addLines(str, lines);
            str = wordForm;
            lines.clear();
        }
        std::vector<CFormInfo> ParadigmCollection;
        if (!MorphHolder.m_pLemmatizer->CreateParadigmCollection(true, lemma, true, false, ParadigmCollection))
            throw CExpc(Format("Cannot lemmatize \"%s\"", lemma.c_str()));
        size_t foundCount = 0;
        for (auto p : ParadigmCollection) {
            for (uint16_t j = 0; j < p.GetCount(); ++j) {
                if (check_word_form(p, j, str, pos, gra)) {
                    CGroup group;
                    group.pid = p.GetParadigmId();
                    group.m_FormNo = j;
                    group.anc = p.GetAncode(j);
                    auto res = lines.insert(tagLines::value_type(group, freq));
                    if (!res.second) {
                        LOGD << "Duplicate pid/anc: line:" << lin << " skipped" << std::endl;
                    }
                    else {
                        foundCount++;
                    }
                }
            }
        }
        if (foundCount == 0) {
            throw std::runtime_error(Format("Can't calculate pid/anc: Line N %i", lin));
        }
    }
    if (!str.empty() && lines.size() > 0)
        addLines(str, lines);
    LOGI << "done";
}

static bool saveBin(std::string name) {
    LOGI << "Saving " << name.c_str() << "... ";
    tagHomon::iterator it;
    for (it = homon.begin(); it != homon.end(); it++) {
        std::string str = it->first;
        tagLines::iterator lin;
        for (lin = it->second.begin(); lin != it->second.end(); lin++) {
            int pid = lin->first.pid;
            int pos = lin->first.m_FormNo;
            int val = lin->second;

            _homoresults_t::value_type _newitem;
            _newitem.first = pid;
            _newitem.second = pos;
            _newitem.third = val;
            homonweight.push_back(_newitem);
        }
    }

    std::sort(homonweight.begin(), homonweight.end(), less4homonode());

    WriteVector(name, homonweight);;
    LOGI << "done";
    return true;
}

void initArgParser(int argc, const char** argv, ArgumentParser& parser) {
    parser.AddOption("--help");
    parser.AddArgument("--input", "input file");
    parser.AddArgument("--output", "output file");
    parser.AddArgument("--morph-folder", "morph_folder");
    parser.AddArgument("--log-level", "log level", true);
    parser.AddArgument("--language", "language");
    parser.Parse(argc, argv);
}

int main(int argc, const char** argv) {
    ArgumentParser args;
    initArgParser(argc, argv, args);
    init_plog(args.GetLogLevel(), "stat_dat_bin.log");
    try {
        MorphHolder.LoadLemmatizer(args.GetLanguage(), args.Retrieve("morph-folder"));

        loadDat(args.GetInputStream(), args.GetLanguage());

        if (saveBin(args.CloseOutputStreamAndGetName()))
            return 0;
    }
    catch (CExpc e) {
        LOGE << "exception occurred:" << e.what();
    }
    catch (std::exception e) {
        LOGE << "exception occurred:" << e.what();
    }
    catch (...) {
        LOGE << "A general exception";
    }

    return 1;
}

