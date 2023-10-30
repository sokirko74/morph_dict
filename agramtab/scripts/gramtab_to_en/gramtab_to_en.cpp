#include "morph_dict/agramtab/RusGramTab.h"
#include "morph_dict/agramtab/EngGramTab.h"
#include "morph_dict/agramtab/GerGramTab.h"
#include "../../common/util_classes.h"
#include "../../common/json.h"
#include "morph_dict/morph_wizard/wizard.h"

#include "iostream"


void PrintUsage()
{
	std::cout << "Usage: CheckGramTab  (Russian|English|German)\n" <<
		 "Example: CheckGramTab  Russian\n";
	exit(-1);
};


bool convert(CAgramtab* gramtab)
{
	gramtab->LoadFromRegistry();
	gramtab->SetUseNationalConstants(true);
	char s[2000];
	
	while (std::cin.getline(s, 1999)) {
		std::string input = convert_from_utf8(s, gramtab->m_Language);
		auto fields = split_string(input, ';');
		part_of_speech_t pos;
		grammems_mask_t grammems;
		
		bool res = gramtab->ProcessPOSAndGrammems(fields[1].c_str(), pos, grammems);
		assert(res);
		
		auto out = Format("%s %s",
			gramtab->GetPartOfSpeechStr(pos, naLatin),
			gramtab->GrammemsToStr(grammems, naLatin).c_str()
		);
		fields[1] = out;
		std::string out_s = join_string(fields, ";");
		out_s = convert_to_utf8(out_s, gramtab->m_Language);

		std::cout << out_s << "\n";
	}
	return true;

}

int main(int argc, char* argv[])
{
	if (argc != 2)
		PrintUsage();
		
	try {
		std::string Lng = argv[1];
		if (Lng == "Russian")
		{
			CRusGramTab P;
			if (!convert(&P)) return 1;
		}
		else
			if (Lng == "English")
			{
				CEngGramTab P;
				if (!convert(&P)) return 1;
			}
			else
				if (Lng == "German")
				{
					CGerGramTab P;
					if (!convert(&P)) return 1;
				}
				else
					PrintUsage();
		
	}
	catch (CExpc e) {
		std::cerr << e.m_strCause << "\n";
		return false;
	}
return 0;
}

