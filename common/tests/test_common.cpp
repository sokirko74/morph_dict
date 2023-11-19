#include "morph_dict/common/utilit.h"
#include <locale>
#include <cwctype>

#define DOCTEST_CONFIG_IMPLEMENT
#include "morph_dict/contrib/doctest/doctest.h"


TEST_CASE("unicode1") {
	std::string eng = "-/QWERTYUIOPASDFGHJKLZXCVBNMMMqwertyuioopasdfghjklzxcvbnm";
	CHECK(CheckEnglishUtf8(eng));

	CHECK(CheckGermanUtf8("Ä"));
	CHECK(!CheckRussianUtf8("Ä"));
	CHECK(!CheckEnglishUtf8("Ä"));

	std::string ger = "ÄÖÜßäöü" + eng;
	CHECK(CheckGermanUtf8(ger));

	std::string rus = "-/йцукенгшщзхъёфывапролджэячсмитьбююЙЦУКЕНГШЩЗХЪЁФЫВАПРОЛДЖЭЯЧСМИТЬБЮЮ";
	CHECK(CheckRussianUtf8(rus));
	std::string lower_rus = "йцукенгшщзхъёфывапролджэячсмитьбюю";
	std::string upper_rus = "ЙЦУКЕНГШЩЗХЪЁФЫВАПРОЛДЖЭЯЧСМИТЬБЮЮ";
	;
	std::string a1 = lower_rus;
	CHECK(upper_rus == MakeUpperUtf8(a1));
	
	a1 = upper_rus;
	CHECK(lower_rus == MakeLowerUtf8(upper_rus));

	std::string ger_lower = "äöüß";
	std::string ger_upper = "ÄÖÜß";

	a1 = ger_lower;
	a1 = MakeUpperUtf8(a1);
	CHECK(ger_upper == a1);

	a1 = ger_upper;
	CHECK(ger_lower == MakeLowerUtf8(a1));
}

TEST_CASE("unicode2") {
	CHECK(ContainsRussianUtf8("zzzЯzzz"));
	CHECK(U'a' == GetFirstUnicodeLetterFromUtf8("abc"));
	CHECK(U'ю' == GetFirstUnicodeLetterFromUtf8("юля"));
	CHECK(U'ẞ' == GetFirstUnicodeLetterFromUtf8("ẞ")); // 3 bytes
	CHECK(IsUnicodeUpperRussianVowel(U'Ю'));
	CHECK(CountLettersInUtf8(std::string("юẞ")) == 2);
}


int main(int argc, char** argv) {
	init_plog(plog::Severity::debug, "test_common.log");
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

