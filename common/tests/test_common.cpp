#include "morph_dict/common/utilit.h"
#include <locale>
#include <cwctype>
#include <codecvt>

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
	CHECK(FirstLetterIsUpper(std::string("мама")) == false);
	CHECK(FirstLetterIsUpper(std::string("Мама")) == true);
	CHECK(FirstLetterIsUpper(std::string("Mother")) == true);
	CHECK(FirstLetterIsUpper(std::string("mother")) == false);
	CHECK(FirstLetterIsUpper(std::string("122")) == false);
	CHECK(FirstLetterIsUpper(std::string("!@")) == false);
}

TEST_CASE("check_english") {
	// possibly unused
	const std::locale c_locale("C");
	CHECK(std::isalpha('a', c_locale));
	CHECK(std::isalpha('z', c_locale));
	CHECK(std::isalpha('A', c_locale));
	CHECK(!std::isalpha('1', c_locale));
	CHECK(!std::isalpha(U'Я', c_locale));

}

TEST_CASE("check_utf16") {
	std::string rus_utf8 = "zмама";
	CHECK(9 == rus_utf8.length());
	std::wstring wstr = utf8_to_wstring(rus_utf8);
	CHECK(5 == wstr.length());
	std::string s = wstring_to_utf8(wstr);
	CHECK(s == rus_utf8);
}

TEST_CASE("check_punct") {
	std::string puncts = "!\"#$%&'()*+,-./:;<=>?@[\]^_`{|}";
	for (auto i : puncts) {
		CHECK(std::iswpunct(i) != 0);
	}
	CHECK(std::iswpunct(U'Я') == 0);
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

