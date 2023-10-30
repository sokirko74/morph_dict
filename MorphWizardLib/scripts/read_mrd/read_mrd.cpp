#include "morph_dict/AgramtabLib/RusGramTab.h"
#include "morph_dict/AgramtabLib/EngGramTab.h"
#include "morph_dict/AgramtabLib/GerGramTab.h"
#include "../../common/util_classes.h"
#include "../../common/json.h"
#include "morph_dict/MorphWizardLib/wizard.h"

#include "fstream"


void PrintUsage()
{
	std::cout << "Usage: read_mrd  (Russian|English|German)\n" <<
		 "Example: CheckGramTab  Russian\n";
	exit(-1);
};



int main(int argc, char* argv[])
{
	MorphoWizard Wizard;
	//std::string path = "C:/RML/Source/morph_dict/data/Russian/project.mwz";
	std::string path = "C:/RML/Source/morph_dict/MorphGen/tests/Misspell/Russian/project.mwz";
	Wizard.load_wizard(path, "guest", false);
	Wizard.m_MrdPath = "morphs.json";
	Wizard.save_mrd();

	return 0;
}

