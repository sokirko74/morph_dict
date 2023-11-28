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

