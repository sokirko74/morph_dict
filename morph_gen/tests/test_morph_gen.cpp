#include "morph_dict/common/utilit.h"
#include "morph_dict/lemmatizer_base_lib/MorphanHolder.h"
#include "morph_dict/lemmatizer_base_lib/MorphDictBuilder.h"

#define DOCTEST_CONFIG_IMPLEMENT
#include "morph_dict/contrib/doctest/doctest.h"



TEST_CASE("lemmatize_Russian_word") {
	std::string folder = "Russian1";
	CMorphDictBuilder R;
	R.BuildLemmatizer(MakePath(folder, "project.mwz"), true, 5, 3, folder);

	CMorphanHolder Holder;
	Holder.LoadLemmatizer(morphRussian, folder);
	auto word = convert_from_utf8("дума", Holder.m_CurrentLanguage);
	std::string test = Holder.PrintMorphInfoUtf8(word, false, false, true);
	std::string canon = "+ ДУМА С нп,нс ср,жр,мр,пр,тв,вн,дт,рд,им,ед,мн\n"
		"\t+ ДУМА С пе,нс ср,жр,мр,пр,тв,вн,дт,рд,им,ед,мн\n";
	CHECK(canon == test);
}

void Misspell(CMorphanHolder& holder, std::string word_utf8, std::string canon) {

	auto res = holder.CorrectMisspelledWordUtf8(word_utf8);
	if (canon == "" && res.empty()) {
		return;
	}
	CHECK(!res.empty());
	CHECK(canon == res[0].CorrectedString);
}

TEST_CASE("misspell_Russian_word") {
	std::string folder = "Russian2";
	CMorphDictBuilder R;
	R.BuildLemmatizer(MakePath(folder, "project.mwz"), true, -1, -1, folder);

	CMorphanHolder holder;
	holder.LoadLemmatizer(morphFioDisclosures, folder);

	Misspell(holder, "сакирко_а_в", "СОКИРКО_А_В");
	Misspell(holder, "сакурко_а_в", "СОКИРКО_А_В");
	Misspell(holder, "зокирко_а_в", "ЗОКИРКА_А_В");
	Misspell(holder, "сакурку_а_в", "");
}

int main(int argc, char** argv) {
	init_plog(plog::Severity::debug, "morph_gen_test.log");
	doctest::Context context;
	context.applyCommandLine(argc, argv);
	int res;
	try {
		res = context.run(); // run doctest
	}
	catch (std::exception& e)
	{
		PLOGE << e.what();
		return 1;
	}
	// important - query flags (and --exit) rely on the user doing this
	if (context.shouldExit()) {
		return res;
	}
}

