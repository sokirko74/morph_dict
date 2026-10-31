#include "morph_dict/common/utilit.h"
#include "morph_dict/lemmatizer_base_lib/MorphanHolder.h"
#include "morph_dict/lemmatizer_base_lib/MorphDictBuilder.h"
#include "morph_dict/morph_wizard/flexia_model.h"

#define DOCTEST_CONFIG_IMPLEMENT
#include "morph_dict/contrib/doctest/doctest.h"

std::string TEST_FOLDER;

namespace fs = std::filesystem;

TEST_CASE("find_lemma") {
	auto path = fs::path(TEST_FOLDER) / "Russian1" / "project.mwz";
	MorphoWizard wizard;
	wizard.load_wizard(path.string(), "guest", false, false, false);
	auto w = wizard.find_lemm("/слов.*/", false);
	CHECK(w.size() == 1);
	w = wizard.find_lemm("слов*", false);
	CHECK(w.size() == 1);
	w = wizard.find_lemm("*лово", false);
	CHECK(w.size() == 1);
}


TEST_CASE("slf_create") {
	auto path = fs::path(TEST_FOLDER) / "Russian1" / "project.mwz";
	MorphoWizard wizard;
	wizard.load_wizard(path.string(), "guest", false, false, false);
	auto w = wizard.find_lemm("слово", false);
	std::string type_grm, prefixes;
	auto slf = wizard.get_slf_string(w[0], type_grm, prefixes);
	std::string canon = 
		"слово'                                                                N sg,\r\n"
		"сло'вом                                                              N pl,\r\n";
	CHECK(canon == slf);
}

TEST_CASE("slf_common_left_part_utf8") {
	auto path = fs::path(TEST_FOLDER) / "Russian1" / "project.mwz";
	MorphoWizard wizard;
	wizard.load_wizard(path.string(), "guest", false, false, false);
	// "И" and "Ь" have the same first byte in utf-8
	int error_line;
	wizard.add_lemma_to_dict("ЛИСИЙ N sg\nЛИСЬЯ N pl", "", "", error_line);
	auto w = wizard.find_lemm("ЛИСИЙ", false);
	REQUIRE(1 == w.size());
	const auto& flexia = wizard.m_FlexiaModels[w[0]->second.m_FlexiaModelNo].m_Flexia;
	REQUIRE(2 == flexia.size());
	CHECK("ИЙ" == flexia[0].m_FlexiaStr);
	CHECK("ЬЯ" == flexia[1].m_FlexiaStr);
}

TEST_CASE("paradigm_parts") {
	auto path = fs::path(TEST_FOLDER) / "Russian1" / "project.mwz";
	MorphoWizard wizard;
	wizard.load_wizard(path.string(), "guest", false, false, false);
	int error_line;
	wizard.add_lemma_to_dict("ЗДАНИЕ N sg\nЗДАНИЯ N pl", "", "", error_line);
	auto w = wizard.find_lemm("ЗДАНИЕ", false);
	REQUIRE(1 == w.size());
	auto& info = w[0]->second;
	const auto first = wizard.m_FlexiaModels[info.m_FlexiaModelNo].m_Flexia;
	REQUIRE(2 == first.size());

	CFlexiaModel variant;
	variant.m_Flexia.push_back(CMorphForm(first[0].m_Gramcode, "ЕВ", ""));
	variant.m_Flexia.push_back(CMorphForm(first[1].m_Gramcode, "ЯВ", ""));
	wizard.m_FlexiaModels.push_back(variant);
	uint16_t variant_no = (uint16_t)wizard.m_FlexiaModels.size() - 1;
	info.m_NextParts.push_back({ variant_no, UnknownAccentModelNo });

	StringVector forms;
	wizard.get_wordforms(w[0], forms);
	CHECK(StringVector({ "ЗДАНИЕ", "ЗДАНИЯ", "ЗДАНИЕВ", "ЗДАНИЯВ" }) == forms);

	std::vector<lemma_iterator_t> found;
	wizard.find_lemm_by_prdno(variant_no, found);
	CHECK(1 == found.size());

	rapidjson::Document d;
	CJsonObject out(d);
	info.GetJson("ЗДАНИЕ", out);
	CParadigmInfo restored;
	restored.FromJson(out.get_value());
	CHECK(restored.m_NextParts == info.m_NextParts);

	wizard.pack();
	w = wizard.find_lemm("ЗДАНИЕ", false);
	REQUIRE(1 == w.size());
	wizard.get_wordforms(w[0], forms);
	CHECK("ЗДАНИЯВ" == forms.back());
}

TEST_CASE("slf_parts") {
	auto path = fs::path(TEST_FOLDER) / "Russian1" / "project.mwz";
	MorphoWizard wizard;
	wizard.load_wizard(path.string(), "guest", false, false, false);
	int error_line;
	wizard.add_lemma_to_dict("здание N sg\r\nздания N pl\r\n\r\nзданье N sg\r\nзданья N pl\r\n", "", "", error_line);
	auto w = wizard.find_lemm("ЗДАНИЕ", false);
	REQUIRE(1 == w.size());
	REQUIRE(1 == w[0]->second.m_NextParts.size());
	CHECK("ЗДАН" == wizard.get_base_string(w[0]));
	CHECK("ЬЕ" == wizard.m_FlexiaModels[w[0]->second.m_NextParts[0].m_FlexiaModelNo].get_first_flex());

	// slf -> dictionary -> slf
	std::string type_grm, prefixes;
	auto slf = wizard.get_slf_string(w[0], type_grm, prefixes, 0);
	CHECK(slf == "здание N sg,\r\nздания N pl,\r\n\r\nзданье N sg,\r\nзданья N pl,\r\n");
	auto info = w[0]->second;
	wizard.remove_lemm(w[0]);
	wizard.add_lemma_to_dict(slf, "", "", error_line);
	w = wizard.find_lemm("ЗДАНИЕ", false);
	REQUIRE(1 == w.size());
	CHECK(w[0]->second.m_NextParts == info.m_NextParts);

	// a change of one part is not allowed
	CHECK(!wizard.change_prd_info(w[0]->second, "ЗДАНИЕ", 0, UnknownAccentModelNo, true));

	// the parts are predicted together
	wizard.m_Predictor.CreateIndex();
	auto predicted = wizard.m_Predictor.predict_lemm("СОЗДАНИЕ", 4, 0, false, TLemmPredictSortEnum::Freq);
	REQUIRE(1 == predicted.size());
	CHECK(4 == predicted[0].getFormsCount());
	auto new_slf = wizard.create_slf_for_lemm("СОЗДАНИЕ", predicted[0].m_FlexiaModelNo, 0, predicted[0].m_NextFlexiaModels);
	CHECK(new_slf.find("созданья") != std::string::npos);
	CHECK(new_slf.find("\r\n\r\n") != std::string::npos);

	// parts with different bases, "И" and "Ь" have the same first byte in utf-8
	wizard.add_lemma_to_dict("ЛИСА N sg\n\nЛЬСА N sg", "", "", error_line);
	w = wizard.find_lemm("ЛИСА", false);
	REQUIRE(1 == w.size());
	CHECK("Л" == wizard.get_base_string(w[0]));
	CHECK("ЬСА" == wizard.m_FlexiaModels[w[0]->second.m_NextParts[0].m_FlexiaModelNo].get_first_flex());

	// the line number of an error is counted in the whole text
	CHECK_THROWS(wizard.add_lemma_to_dict("ЛОСЬ N sg\n\nЛОСИ BAD", "", "", error_line));
	CHECK(3 == error_line);
}

TEST_CASE("slf_parts_in_file") {
	auto path = fs::path(TEST_FOLDER) / "Russian1" / "project.mwz";
	MorphoWizard wizard;
	wizard.load_wizard(path.string(), "guest", false, false, false);
	FILE* fp = std::tmpfile();
	REQUIRE(fp != nullptr);
	std::string text = "=====\n// a comment before\nЛОСЬ N sg\n// a comment inside is not a separator\nЛОСЯ N sg\n\nЛОСЬЕ N sg\n\n=====\nЗВЕРЬ N sg\n";
	fwrite(text.data(), 1, text.size(), fp);
	rewind(fp);
	CDumpParadigm paradigm(&wizard);
	int line_no = 0;
	bool error = false;
	std::string errors;
	REQUIRE(paradigm.ReadNextParadigmFromFile(fp, line_no, error, errors));
	CHECK("ЛОСЬ N sg\r\nЛОСЯ N sg\r\n\r\nЛОСЬЕ N sg\r\n\r\n" == paradigm.m_SlfStr);
	int error_line;
	auto info = wizard.add_lemma_to_dict(paradigm.m_SlfStr, "", "", error_line);
	CHECK(1 == info.m_NextParts.size());
	REQUIRE(paradigm.ReadNextParadigmFromFile(fp, line_no, error, errors));
	info = wizard.add_lemma_to_dict(paradigm.m_SlfStr, "", "", error_line);
	CHECK(info.m_NextParts.empty());
	fclose(fp);
}

TEST_CASE("slf_accented") {
	auto path = fs::path(TEST_FOLDER) / "Russian1" / "project.mwz";
	MorphoWizard wizard;
	wizard.load_wizard(path.string(), "guest", false, false, false);
	std::string slf = "ударе'ние N sg";
	int error_line;
	wizard.add_lemma_to_dict(slf, "", "", error_line);

	std::string form = "УДАРЕНИЕ";
	auto w = wizard.find_lemm(form, false);
	CHECK(1 == w.size());
	auto accented = wizard.get_lemm_string_with_accents(w[0]);
	CHECK("УДАРЕ'НИЕ" == accented);
}

TEST_CASE("mrd_serialization") {
	MorphoWizard wizard;
	auto in_path = fs::path(TEST_FOLDER) / "Russian1";
	auto tmp_folder = fs::path(TEST_FOLDER) / "tmp";
	if (fs::exists(tmp_folder)) {
		fs::remove_all(tmp_folder);
	}
	fs::copy(in_path, tmp_folder, fs::copy_options::overwrite_existing | fs::copy_options::recursive);

	auto path = tmp_folder / "project.mwz";
	wizard.load_wizard(path.string(), "guest", false, false, false);
	auto w = wizard.find_lemm("ДУМА", false);
	CHECK(w.size() == 2);
	std::string slf = "ДУМА N sg";
	int error_line;
	wizard.add_lemma_to_dict(slf, "", "", error_line);
	wizard.save_mrd();
	wizard.load_wizard(path.string(), "guest", false, false, false);
	w = wizard.find_lemm("ДУМА", false);
	CHECK(w.size() == 3);

	fs::remove_all(tmp_folder);
}

TEST_CASE("change_prd_info") {
	auto path = fs::path(TEST_FOLDER) / "Russian1" / "project.mwz";
	MorphoWizard wizard;
	wizard.load_wizard(path.string(), "guest", false, false, false);
	auto w = wizard.find_lemm("слово", false);
	REQUIRE(w.size() == 1);
	REQUIRE(wizard.m_FlexiaModels.size() == 3);
	wizard.change_prd_info(w[0]->second, w[0]->first, 2, w[0]->second.m_AccentModelNo, true);
	std::string type_grm, prefixes;
	auto slf = wizard.get_slf_string(w[0], type_grm, prefixes);
	std::string canon =
		"словю'                                                                N pl,\r\n"
		"сло'вюм                                                              N sg,\r\n";
	CHECK(canon == slf);

}
TEST_CASE("prefixes") {
	auto path = fs::path(TEST_FOLDER) / "Russian1" / "project.mwz";
	MorphoWizard wizard;
	wizard.load_wizard(path.string(), "guest", false, false, false);
	std::string slf = "ВОРД N sg\nПО|ВОРД N sg";
	int error_line;
	std::string in_prefix = "ПСЕВДО";
	wizard.add_lemma_to_dict(slf, "", in_prefix, error_line);
	auto lemmas = wizard.find_lemm("ВОРД", false);
	REQUIRE(1 == lemmas.size());
	std::string type_grm, out_prefix;
	slf = wizard.get_slf_string(lemmas[0], type_grm, out_prefix);
	std::string canon =
		"ворд                                                                   N sg,\r\n"
		"по|ворд                                                              N sg,\r\n";
	CHECK(canon == slf);
	CHECK(in_prefix == out_prefix);
}

TEST_CASE("prefix_set_with_commas") {
	auto path = fs::path(TEST_FOLDER) / "Russian1" / "project.mwz";
	MorphoWizard wizard;
	wizard.load_wizard(path.string(), "guest", false, false, false);
	// get_prefix_set_str (slf export) joins prefixes with commas
	std::string in_prefix = "ГИГА,ТЕРА";
	REQUIRE(wizard.check_prefixes(in_prefix));
	int error_line;
	wizard.add_lemma_to_dict("БАЙТ N sg", "", in_prefix, error_line);
	auto lemmas = wizard.find_lemm("БАЙТ", false);
	REQUIRE(1 == lemmas.size());
	CHECK(in_prefix == wizard.get_prefix_set(lemmas[0]->second));
}

TEST_CASE("slf_export_import") {
	auto path = fs::path(TEST_FOLDER) / "Russian1" / "project.mwz";
	MorphoWizard wizard;
	wizard.load_wizard(path.string(), "guest", false, false, false);
	auto w = wizard.find_lemm("слово", false);
	REQUIRE(w.size() == 1);

	// export as in morphwizard: Tools/Paradigm Export
	CDumpParadigm out(&wizard);
	out.m_SlfStr = wizard.get_slf_string(w[0], out.m_TypeGrammemsStr, out.m_PrefixesStr, 50);
	out.m_Session.m_UserName = "алексей";
	out.m_Session.m_SessionStart = "10:00, 28 October 2026";
	out.m_Session.m_LastSessionSave = "11:00, 28 October 2026";
	std::string dump = out.ToStringUtf8();
	CHECK(dump.find("слово'") != std::string::npos);

	auto tmp_path = fs::path(TEST_FOLDER) / "export.slf";
	{
		std::ofstream f(tmp_path, std::ios::binary);
		f << dump;
	}
	wizard.remove_lemm(w[0]);
	CHECK(wizard.find_lemm("слово", false).empty());

	// import
	FILE* fp = fopen(tmp_path.string().c_str(), "rb");
	REQUIRE(fp != nullptr);
	CDumpParadigm in(&wizard);
	int line_no = 0;
	bool bError = false;
	std::string errors;
	REQUIRE(in.ReadNextParadigmFromFile(fp, line_no, bError, errors));
	fclose(fp);
	fs::remove(tmp_path);
	CHECK(!bError);
	CHECK(errors.empty());
	CHECK(in.m_Session.m_UserName == "алексей");
	int error_line;
	wizard.add_lemma_to_dict(in.m_SlfStr, in.m_TypeGrammemsStr, in.m_PrefixesStr, error_line);
	w = wizard.find_lemm("слово", false);
	REQUIRE(w.size() == 1);
	std::string type_grm, prefixes;
	CHECK(wizard.get_slf_string(w[0], type_grm, prefixes, 50) == out.m_SlfStr);
}

TEST_CASE("convert_jo_to_je") {
	auto path = fs::path(TEST_FOLDER) / "Russian1" / "project.mwz";
	MorphoWizard wizard;
	wizard.load_wizard(path.string(), "guest", false, false, true);
	wizard.convert_je_to_jo();
	auto w = wizard.find_lemm(convert_from_utf8("елка", morphRussian), false);
	CHECK(1 == w.size());
}

int main(int argc, char** argv) {
	init_plog(plog::Severity::debug, "morph_wizard_test.log");
	doctest::Context context;
	context.applyCommandLine(argc, argv);
	for (int i = 1; i < argc; ++i) {
		if (std::string(argv[i]) == "--directory") {
			TEST_FOLDER = std::string(argv[i + 1]);
		}
	}
	//dt_removed args(argv);
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
