#include "morph_dict/agramtab/RusGramTab.h"
#include "morph_dict/agramtab/EngGramTab.h"
#include "morph_dict/agramtab/GerGramTab.h"

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "morph_dict/contrib/doctest/doctest.h"


void check_uniq(CAgramtab& g) 
{
	g.LoadFromRegistry();

	for (uint16_t i = 0; i < g.GetMaxGrmCount(); i++) {
		for (uint16_t k = 0; k < g.GetMaxGrmCount(); k++) {
			if ((g.GetLine(i) != nullptr) && (g.GetLine(k) != nullptr) && (i != k))
			{
				if ((g.GetLine(i)->m_Grammems == g.GetLine(k)->m_Grammems) && 
					(g.GetLine(i)->m_PartOfSpeech == g.GetLine(k)->m_PartOfSpeech))
				{
					std::cerr << "a dublicate found " << g.LineIndexToGramcode(i) << " ? " << g.LineIndexToGramcode(k) << "\n";
					CHECK(false);
				};
			}
		}

	}
}

TEST_CASE("uniq_gram_patterns") {
	check_uniq(CRusGramTab());
	check_uniq(CEngGramTab());
	check_uniq(CGerGramTab());
}

TEST_CASE("russian_coordination") {
	CRusGramTab gramtab;
	gramtab.LoadFromRegistry();
	CHECK(gramtab.GetPlugNouInfo().m_Lemma.length() > 0);
	auto adj = gramtab.GetFirstAncodeByPattern("A mas,sg,nom,anim,inanim");
	CHECK(!adj.empty());
	auto noun = gramtab.GetFirstAncodeByPattern("N mas,sg,nom");
	CHECK(!noun.empty());
	auto type_noun = gramtab.GetFirstAncodeByPattern("* anim");
	CHECK(!type_noun.empty());
	CHECK(gramtab.GleicheGenderNumberCase(type_noun.c_str(), noun.c_str(), adj.c_str()));
}

