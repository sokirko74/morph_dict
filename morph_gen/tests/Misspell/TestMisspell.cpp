#include "morph_dict/lemmatizer_base_lib/MorphanHolder.h"
#include <fstream>
#include <streambuf>

void Misspell(CMorphanHolder& holder, std::string word_utf8, std::string canon) {

    auto res = holder.CorrectMisspelledWordUtf8(word_utf8);
    if (canon == "" && res.empty()) {
        return;
    }
    if (res.empty()) {
        std::cerr << "test failed: canon=\"" << canon << "\", got no results\n";
        exit(1);
    }
    if (canon != res[0].CorrectedString) {
        std::cerr << "test failed: canon=\"" << canon << "\", got \"" << res[0].CorrectedString << "\"\n";
        exit(1);
    }
}

int main(int argc, const char* argv[])
{
    if (argc != 2) {
        std::cerr << "usage: TestMisspell <folder>";
        return 1;
    }
    try {
        std::string folder = argv[1];
        CMorphanHolder holder;
        holder.LoadLemmatizer(morphFioDisclosures, folder);
        Misspell(holder, "сакирко_а_в", "СОКИРКО_А_В");
        Misspell(holder, "сакурко_а_в", "СОКИРКО_А_В");
        Misspell(holder, "зокирко_а_в", "ЗОКИРКА_А_В");
        Misspell(holder, "сакурку_а_в", "");
        return 0;
    }
    catch (CExpc c) {
        std::cerr << c.m_strCause << "\n";
        return 1;
    }
    catch (...) {
        std::cerr << "general exception\n";
        return 1;
    }
}
