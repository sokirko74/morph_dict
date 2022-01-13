#include "morph_dict/LemmatizerBaseLib/MorphanHolder.h"
#include <fstream>
#include <streambuf>

void Misspell(CMorphanHolder& holder, std::string word, std::string canon) {
    word = convert_from_utf8(word.c_str(), holder.m_CurrentLanguage);
    std::string res = holder.CorrectMisspelledWord(word);
    if (canon != res) {
        std::cerr << "test failed: canon=\"" << canon << "\", got \"" << res << "\"\n";
        exit(1);
    }
}

int main(int argc, const char* argv[])
{
    if (argc < 2) {
        std::cerr << "usage: TestMisspell <folder>";
        return 1;
    }
    try {
        std::string folder = argv[1];
        CMorphanHolder holder;
        holder.LoadLemmatizer(morphFioDisclosures, folder);
        if (argc == 2) {
            Misspell(holder, "сакирко_а_в", "СОКИРКО_А_В");
            Misspell(holder, "сакурко_а_в", "СОКИРКО_А_В");
            Misspell(holder, "зокирко_а_в", "ЗОКИРКА_А_В");
            Misspell(holder, "сакурку_а_в", "");
            return 0;
        }
        else {

            auto word = convert_from_utf8(argv[2], holder.m_CurrentLanguage);
            std::cout << holder.CorrectMisspelledWord(word) << "\n";
        }
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
