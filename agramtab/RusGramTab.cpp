// ==========  This file is under  LGPL, the GNU Lesser General Public Licence
// ==========  Dialing Lemmatizer (www.aot.ru)
// ==========  Copyright by Alexey Sokirko

#include "RusGramTab.h"


struct TMorphConstant {
	std::string Cyrillic;
	std::string Latin;
	std::string CyrillicLong;
};

const static TMorphConstant RussianPartOfSpeech[] = {
	{"С", "N", "СУЩЕСТВИТЕЛЬНОЕ"},  // 0
	{"П", "A", "ПРИЛАГАТЕЛЬНОЕ"}, // 1
	{"Г", "V", "ЛИЧНАЯ ФОРМА"}, // 2
	{"МС","PRON", "МЕСТОИМЕНИЕ"}, // 3
	{"МС-П","PA", "МЕСТОИМЕНИЕ-ПРИЛАГАТЕЛЬНОЕ"}, // 4
	{"МС-ПРЕДК","P_PRED", "МЕСТОИМЕНИЕ-ПРЕДИКАТИВ"}, // 5
	{"ЧИСЛ", "NUM", "ЧИСЛИТЕЛЬНОЕ"}, // 6
	{"ЧИСЛ-П","ORD_NUM", "ПОРЯДКОВОЕ ЧИСЛИТЕЛЬНОЕ"}, // 7
	{"Н","ADV", "НАРЕЧИЕ"}, // 8
	{"ПРЕДК", "PRED", "ПРЕДИКАТИВ"}, //9 
	{"ПРЕДЛ", "PREP", "ПРЕДЛОГ"}, // 10
	{"ПОСЛ", "POSL", "ПОСЛЕЛОГ"}, // 11
	{"СОЮЗ", "CONJ", "СОЮЗ"}, // 12
	{"МЕЖД", "INT", "МЕЖДОМЕТИЕ"}, // 13
	{"ВВОДН", "INP", "ВВОДНОЕ СЛОВО"},// 14
	{"ФРАЗ", "COLLOC", "ФРАЗЕОЛОГИЗМ"}, // 15
	{"ЧАСТ", "PARTICLE", "ЧАСТИЦА"}, // 16
	{"КР_ПРИЛ", "ADJ_SHORT", "КР_ПРИЛ"},  // 17
	{"ПРИЧАСТИЕ", "PARTICIPLE", "ПРИЧАСТИЕ"}, //18
	{"ДЕЕПРИЧАСТИЕ","ADV_PARTICIPLE", "ДЕЕПРИЧАСТИЕ"}, //19
	{"КР_ПРИЧАСТИЕ","PARTICIPLE_SHORT", "КРАТКОЕ ПРИЧАСТИЕ"}, // 20
	{"ИНФИНИТИВ", "INFINITIVE", "ИНФИНИТИВ"}  //21
};

const static TMorphConstant Grammems[] = {
	{"мн", "pl"},
	{"ед", "sg"},
	{"им", "nom"},
	{"рд", "gen"},
	{"дт", "dat"},
	{"вн", "acc"},
	{"тв", "ins"},
	{"пр", "prp"},
	{"зв", "voc"},
	{"мр", "mas"},
	{"жр", "fem"},
	{"ср", "neu"},
	{"unused", "unused"},
	{"нст", "pres"},
	{"буд", "fut"},
	{"прш", "past"},
	{"1л", "1p"},
	{"2л", "2p"},
	{"3л", "3p"},
	{"пвл", "imp"},
	{"од", "anim"},
	{"но", "inanim"},
	{"сравн", "compar"},
	{"св", "perf"},
	{"нс", "imperf"},
	{"нп", "intrans"},
	{"пе", "trans"},
	{"дст", "act"},
	{"стр", "pass"},
	{"0", "0"},
	{"аббр", "abbr"},
	{"отч", "patr"},
	{"лок", "loc"},
	{"орг", "org"},
	{"кач", "qual"},
	{"дфст", "sing_tant"},
	{"вопр", "interrog"},
	{"указат", "demon"},
	{"имя", "name"},
	{"фам", "surname"},
	{"безл", "impers"},
	{"жарг", "jargon"},
	{"опч", "misspell"},
	{"разг", "conv"},
	{"притяж", "poss"},
	{"арх", "arch"},
	{"2", "2"},
	{"поэт", "poet"},
	{"проф", "prof"},
	{"прев", "superl"},
	{"полож", "positive"}
};


std::string rClauseTypes[] =
{
	"ГЛ_ЛИЧН",
	"ДПР",
	"КР_ПРЧ",
	"КР_ПРИЛ",
	"ПРЕДК",
	"ПРЧ",
	"ИНФ",
	"ВВОД",
	"ТИРЕ",
	"НСО",
	"СРАВН",
	"КОПУЛ"
};


CRusGramTab::CRusGramTab()
{
	assert(sizeof(Grammems) / sizeof(TMorphConstant) == RussianGrammemsCount);
	assert(sizeof(RussianPartOfSpeech) / sizeof(TMorphConstant) == RUSSIAN_PART_OF_SPEECH_COUNT);
	assert(sizeof(rClauseTypes) / sizeof(std::string) == RUSSIAN_CLAUSE_TYPE_COUNT);

	for (size_t i = 0; i < GetMaxGrmCount(); i++) {
		GetLine(i) = nullptr;
	}
	m_Language = morphRussian;
	m_bUseNationalConstants = true;
}

CRusGramTab :: ~CRusGramTab()
{
	for (size_t i = 0; i < GetMaxGrmCount(); i++) {
		if (GetLine(i) != NULL)
			delete GetLine(i);
	}

}

grammems_mask_t DeduceGrammems(part_of_speech_t PartOfSpeech, grammems_mask_t grammems) {
    // неизменяемые слова как будто принадлежат всем падежам
    if ((_QM(rIndeclinable) & grammems)
        && (PartOfSpeech != PREDK)
            )
        grammems |= rAllCases;

    if ((_QM(rIndeclinable) & grammems)
        && (PartOfSpeech == PRONOUN_P)
            )
        grammems |= rAllGenders | rAllNumbers;


    // слово 'пальто' не изменяется по числам, поэтому может
    // быть использовано в обоих числах
    if (PartOfSpeech != PREDK)
        if ((_QM(rIndeclinable) & grammems) && !(_QM(rSingular) & grammems))
            grammems |= _QM(rPlural) | _QM(rSingular);

    return grammems;
}

unsigned int count_of_bits(grammems_mask_t n)
{
    unsigned int count = 0;
    while (n) {
        count += n & 1;
        n >>= 1;
    }
    return count;
}


void CRusGramTab::LoadFromRegistry()
{
	ReadFromFolder(GetDefaultPath());
}

void CRusGramTab::InitLanguageSpecific(rapidjson::Document& doc)  {
    m_PopularGramCodes.m_InanimIndeclNoun = doc["inanim_indecl_noun"].GetString();
	m_PopularGramCodes.m_MasAbbrNoun = doc["mas_abbr_noun"].GetString();
    m_PopularGramCodes.m_ProductiveNoun = "";
    m_PopularGramCodes.m_ProductiveSingNoun = "";

	for (uint16_t i = 0; i < GetMaxGrmCount(); i++) {
        auto *l = GetLine(i);
        if (l == nullptr) continue;
        l->m_Grammems = DeduceGrammems(l->m_PartOfSpeech, l->m_Grammems);

        if ((l->m_PartOfSpeech == NOUN) && (count_of_bits(l->m_Grammems) == 3)) {
            if ((l->m_Grammems & _QM(rVocativ)) == 0) {
                if ((l->m_Grammems & rAllGenders) && (l->m_Grammems & rAllCases)) {
                    if (l->m_Grammems & rAllNumbers) {
                        std::string ancode = LineIndexToGramcode(i);
                        m_PopularGramCodes.m_ProductiveNoun += ancode;
                    }
                    if (l->m_Grammems & _QM(rSingular)) {
                        std::string ancode = LineIndexToGramcode(i);
                        m_PopularGramCodes.m_ProductiveSingNoun += ancode;
                    }
                }
            }
        }
    }
    assert (m_PopularGramCodes.m_ProductiveNoun.length() == 72); //72 = 6 cases * 2 numbers * 3 genders * 2 chars
    assert (m_PopularGramCodes.m_ProductiveSingNoun.length() == 36); // 72 / 2

    m_PopularGramCodes.m_GenderNumeral = GetAllGramCodes(NUMERAL, 0, AnyGender);
    assert (m_PopularGramCodes.m_GenderNumeral.length() == 18 * 2);

};


part_of_speech_t	CRusGramTab::GetPartOfSpeechesCount() const
{
	return RUSSIAN_PART_OF_SPEECH_COUNT;
};

size_t CRusGramTab::GetMaxGrmCount() const
{
	return rMaxGrmCount;
}

const char* CRusGramTab::GetPartOfSpeechStr(part_of_speech_t i, NamingAlphabet na) const
{
	if (UseNational(na)) {
		return RussianPartOfSpeech[i].Cyrillic.c_str();
	}
	else {
		return RussianPartOfSpeech[i].Latin.c_str();
	}
};

const char* CRusGramTab::GetPartOfSpeechStrLong(part_of_speech_t i) const
{
	return RussianPartOfSpeech[i].CyrillicLong.c_str();
}

grammem_t	CRusGramTab::GetGrammemsCount()  const
{
	return RussianGrammemsCount;
};

const char* CRusGramTab::GetGrammemStr(size_t i, NamingAlphabet na) const
{
	if (UseNational(na)) {
		return Grammems[i].Cyrillic.c_str();
	}
	else {
		return Grammems[i].Latin.c_str();
	}
};

CAgramtabLine*& CRusGramTab::GetLine(size_t LineNo)
{
	return Lines[LineNo];
}

const CAgramtabLine* CRusGramTab::GetLine(size_t LineNo) const
{
	return Lines[LineNo];
}

size_t CRusGramTab::GramcodeToLineIndex(const char* s)  const
{
	return  (unsigned char)s[0] * 0x100 + (unsigned char)s[1] - rStartUp;
};

std::string CRusGramTab::LineIndexToGramcode(uint16_t i) const
{
	i += rStartUp;
	char res[3];

	res[0] = (i >> 8);
	res[1] = (0xFF & i);
	res[2] = 0;
	return  res;
};


bool CRusGramTab::ProcessPOSAndGrammems(const char* tab_str, part_of_speech_t& PartOfSpeech, grammems_mask_t& grammems) const
{
	if (!CAgramtab::ProcessPOSAndGrammems(tab_str, PartOfSpeech, grammems)) return false;
	grammems = DeduceGrammems(PartOfSpeech, grammems);
	return true;
};

// Стандартное согласование между двумя именами  по  падежу, причем первый код должен
// иметь граммему множественного числа 
inline bool CaseFirstPlural(const CAgramtabLine* l1, const CAgramtabLine* l2)
{
	return (((rAllCases & l1->m_Grammems & l2->m_Grammems) > 0)
		&& (((1 << rPlural) & l1->m_Grammems) > 0)
		);
};


// Стандартное согласование между двумя именами  по роду и числу 
inline bool GenderNumber(const CAgramtabLine* l1, const CAgramtabLine* l2)
{

	return 	((rAllNumbers & l1->m_Grammems & l2->m_Grammems) > 0)
		&& (((l1->m_Grammems & l2->m_Grammems & _QM(rPlural)) > 0)
			|| ((rAllGenders & l1->m_Grammems & l2->m_Grammems) > 0)
			);
}

// Стандартное согласование по лицу и числу (для 1 и 2 лица)
inline bool PersonNumber(const CAgramtabLine* l1, const CAgramtabLine* l2)
{
	size_t t1 = (rAllNumbers & l1->m_Grammems & l2->m_Grammems) > 0;
	size_t t2 = ((rAllPersons & l1->m_Grammems & l2->m_Grammems) > 0);
	return t1 && t2;
}

// Стандартное согласование между подлежащим и сказуемым
inline bool SubjectPredicate(const CAgramtabLine* subj_l, const CAgramtabLine* verb_l)
{
	const grammems_mask_t& subj = subj_l->m_Grammems;
	const grammems_mask_t& verb = verb_l->m_Grammems;

	if (!(subj & _QM(rNominativ))
		)
		return false;


	if (((verb & _QM(rPastTense)) > 0)
		|| (verb_l->m_PartOfSpeech == ADJ_SHORT)
		|| (verb_l->m_PartOfSpeech == PARTICIPLE_SHORT)
		)
	{
		// ты вышел 
		// я вышел
		// ты был
		// я красива
		// мы шли
		// вы шли
		if (subj & (_QM(rFirstPerson) | _QM(rSecondPerson)))
			return		(verb & subj & _QM(rPlural))
			|| ((verb & (_QM(rMasculinum) | _QM(rFeminum)))
				&& (verb & subj & _QM(rSingular))
				);
		else
			// он вышел
			// поезд ушел
			// девочка красива
			// девочки красивы 	
			// мальчик красив
			return GenderNumber(subj_l, verb_l);
	}
	else
		if ((verb & _QM(rPresentTense))
			|| (verb & _QM(rFutureTense)))
		{
			//  я выйду
			//  ты выедешь
			//  мы выйдем
			if ((subj & (_QM(rFirstPerson) | _QM(rSecondPerson)))
				|| (verb & (_QM(rFirstPerson) | _QM(rSecondPerson))))
				return PersonNumber(subj_l, verb_l);
			else
				return (rAllNumbers & subj & verb) > 0;
		}
		else
			if (verb & _QM(rImperative))
				return		((subj & _QM(rSecondPerson)) > 0)
				&& ((rAllNumbers & subj & verb) > 0);

	return false;
}


// Стандартное согласование между двумя именами  по  падежу 
inline bool Case(const CAgramtabLine* l1, const CAgramtabLine* l2)
{

	return 	((rAllCases & l1->m_Grammems & l2->m_Grammems) > 0);
}


// Стандартное согласование между существительным и прилашательнымпо роду, числу и падежу, если 
// если существительное одушевленное
bool GenderNumberCaseAnimRussian(const CAgramtabLine* l1, const CAgramtabLine* l2)
{
	const grammems_mask_t& gram_noun = l1->m_Grammems;
	const grammems_mask_t& gram_adj = l2->m_Grammems;

	return			((rAllCases & gram_noun & gram_adj) > 0)
		&& ((rAllNumbers & gram_noun & gram_adj) > 0)
		&& (((_QM(rAnimative) & gram_adj) > 0)
			|| ((rAllAnimative & gram_adj) == 0)
			)
		&& (((rAllGenders & gram_noun & gram_adj) > 0)
			|| ((rAllGenders & gram_noun) == 0)
			|| ((rAllGenders & gram_adj) == 0)
			);
};

// Стандартное согласование между существительным и прилашательнымпо роду, числу и падежу, если 
// если существительное неодушевленное
bool GenderNumberCaseNotAnimRussian(const CAgramtabLine* l1, const CAgramtabLine* l2)
{
	const grammems_mask_t& gram_noun = l1->m_Grammems;
	const grammems_mask_t& gram_adj = l2->m_Grammems;

	return			((rAllCases & gram_noun & gram_adj) > 0)
		&& ((rAllNumbers & gram_noun & gram_adj) > 0)
		&& (((_QM(rNonAnimative) & gram_adj) > 0)
			|| ((rAllAnimative & gram_adj) == 0)
			)
		&& (((rAllGenders & gram_noun & gram_adj) > 0)
			|| ((rAllGenders & gram_noun) == 0)
			|| ((rAllGenders & gram_adj) == 0)
			);
};

// Стандартное согласование между существительным и прилашательнымпо роду, числу и падежу, если 
// если существительное не неодушевленное и не одушевленное
//  (для местоимений, например, "все это было хорошо")
bool GenderNumberCaseRussian(const CAgramtabLine* l1, const CAgramtabLine* l2)
{
	const grammems_mask_t& gram_noun = l1->m_Grammems;
	const grammems_mask_t& gram_adj = l2->m_Grammems;

	return			((rAllCases & gram_noun & gram_adj) > 0)
		&& ((rAllNumbers & gram_noun & gram_adj) > 0)
		&& (((rAllGenders & gram_noun & gram_adj) > 0)
			|| ((rAllGenders & gram_noun) == 0)
			|| ((rAllGenders & gram_adj) == 0)
			);
};

bool FiniteFormCoordRussian(const CAgramtabLine* l1, const CAgramtabLine* l2)
{
	const grammems_mask_t& verb1 = l1->m_Grammems;
	const grammems_mask_t& verb2 = l2->m_Grammems;

	// жил и был
	if (verb1 & verb2 & rAllNumbers)
		if (verb1 & verb2 & _QM(rPastTense))
		{

			if ((verb1 & verb2 & rAllGenders)
				|| ((verb1 & verb2 & rAllNumbers) == _QM(rPlural))
				)
				return true;
		}
		else
			if (verb1 & verb2 & rAllPersons)
				return true;

	return false;

};

/*
Примеры работы функции GleicheGenderNumberCase:
- красивого стол;
- красивых столы;
- красивый пирата;
- красивые пиратов;
- достойных жизни;
- достойные матерей;
- длинные чудовищ
+ на этот дом;
+ красивую маму;
+ в красивый  дом;
+ в длинное облако;
+ примерить его цилиндр;
+ все это
*/
grammems_mask_t CRusGramTab::GleicheGenderNumberCase(const char* common_gram_code_noun, const char* gram_code_noun, const char* gram_code_adj) const
{
	if ((common_gram_code_noun == 0)
		|| !strcmp(common_gram_code_noun, "??")
		|| (strlen(common_gram_code_noun) == 0)
		)
		// нет сведений об одушевленности
		return Gleiche(GenderNumberCaseRussian, gram_code_noun, gram_code_adj);
	else
		if ((GetLine(GramcodeToLineIndex(common_gram_code_noun))->m_Grammems & _QM(rNonAnimative)) > 0)
			// неодушевленный
			return Gleiche(GenderNumberCaseNotAnimRussian, gram_code_noun, gram_code_adj);
		else
			if ((GetLine(GramcodeToLineIndex(common_gram_code_noun))->m_Grammems & _QM(rAnimative)) > 0)
				// одушевленный
				return Gleiche(GenderNumberCaseAnimRussian, gram_code_noun, gram_code_adj);
			else
				// нет сведений об одушевленности
				return Gleiche(GenderNumberCaseRussian, gram_code_noun, gram_code_adj);

}



bool CRusGramTab::GleicheCaseNumber(const char* gram_code1, const char* gram_code2) const
{
	return  Gleiche(CaseNumber, gram_code1, gram_code2) != 0;
}

bool CRusGramTab::GleicheGenderNumber(const char* gram_code1, const char* gram_code2) const
{
	return  Gleiche(GenderNumber, gram_code1, gram_code2) != 0;
}
//with absent grammems check, less strict than GleicheGenderNumber
bool CRusGramTab::ConflictGenderNumber(const char* gram_code1, const char* gram_code2) const
{
	return  Gleiche(GenderNumber0, gram_code1, gram_code2) == 0;
}
bool CRusGramTab::ConflictGrammems(grammems_mask_t g1, grammems_mask_t g2, grammems_mask_t breaks) const
{
	grammems_mask_t BR[] = { rAllCases, rAllNumbers, rAllGenders };
	bool R = true;
	for (int i = 0; i < (sizeof BR) / (sizeof BR[0]) && R; i++)
	{
		if (breaks & BR[i])
			R &= ((BR[i] & g1 & g2) > 0 || !(BR[i] & g1) || !(BR[i] & g2));
	}
	return  !R;
}

bool CRusGramTab::GleicheSubjectPredicate(const char* gram_code1, const char* gram_code2) const
{
	return  Gleiche(SubjectPredicate, gram_code1, gram_code2) != 0;
}

bool CRusGramTab::GleicheCase(const char* gram_code1, const char* gram_code2) const
{
	return  Gleiche(Case, gram_code1, gram_code2) != 0;
}


long CRusGramTab::GetClauseTypeByName(const char* TypeName) const
{
	for (int i = 0; i < RUSSIAN_CLAUSE_TYPE_COUNT; i++)
	{
		if (rClauseTypes[i] == TypeName)
		{
			return i;
		}
	}

	return -1;
}


const char* CRusGramTab::GetClauseNameByType(long type) const
{
	if (type >= RUSSIAN_CLAUSE_TYPE_COUNT) return 0;
	if (type < 0) return 0;

	return rClauseTypes[type].c_str();
}

/*
	истина для предикативных типов клауз.
*/
bool CRusGramTab::IsStrongClauseRoot(const part_of_speech_mask_t poses) const
{
	return		(poses & (1 << VERB))
		|| (poses & (1 << ADVERB_PARTICIPLE)) // субъект деепричастия  совпадлает с субъектом
											 // главной  клаузы 
		|| (poses & (1 << PARTICIPLE_SHORT))
		|| (poses & (1 << ADJ_SHORT))
		|| (poses & (1 << PREDK));
};


const static std::string SmallNumbers[] = { "ДВА","ТРИ","ЧЕТЫРЕ","ОБА","ПОЛТОРА" };


bool CRusGramTab::IsMorphNoun(part_of_speech_mask_t poses)  const
{
	return  (poses & (1 << NOUN));
};


bool CRusGramTab::is_morph_adj(part_of_speech_mask_t poses) const
{
	return		(poses & (1 << ADJ_FULL))
		|| (poses & (1 << ADJ_SHORT));
};

bool CRusGramTab::is_morph_participle(part_of_speech_mask_t poses) const
{
	return  (poses & (1 << PARTICIPLE))
		|| (poses & (1 << PARTICIPLE_SHORT));
};

bool CRusGramTab::is_morph_pronoun(part_of_speech_mask_t poses) const
{
	return  (poses & (1 << PRONOUN)) != 0;
};


bool CRusGramTab::is_morph_pronoun_adjective(part_of_speech_mask_t poses) const
{
	return  (poses & (1 << PRONOUN_P)) != 0;
};


bool CRusGramTab::is_left_noun_modifier(part_of_speech_mask_t poses, grammems_mask_t grammems) const
{
	return     (poses & (1 << ADJ_FULL))
		|| (poses & (1 << NUMERAL_P))
		|| (poses & (1 << PRONOUN_P))
		|| (poses & (1 << PARTICIPLE));
}


bool CRusGramTab::is_numeral(part_of_speech_mask_t poses) const
{
	return  (poses & (1 << NUMERAL)) != 0;
};

bool CRusGramTab::is_verb_form(part_of_speech_mask_t poses) const
{
	return     is_morph_participle(poses)
		|| (poses & (1 << VERB))
		|| (poses & (1 << INFINITIVE))
		|| (poses & (1 << ADVERB_PARTICIPLE));
};



bool CRusGramTab::is_infinitive(part_of_speech_mask_t poses) const
{
	return (poses & (1 << INFINITIVE)) != 0;
}

bool CRusGramTab::is_morph_predk(part_of_speech_mask_t poses) const
{
	return (poses & (1 << PREDK)) ||
		(poses & (1 << PRONOUN_PREDK));
}

bool CRusGramTab::is_morph_adv(part_of_speech_mask_t poses) const
{
	return (poses & (1 << ADV)) != 0;
}


bool CRusGramTab::is_morph_personal_pronoun(part_of_speech_mask_t poses, grammems_mask_t grammems) const
{
	return		 (poses & (1 << PRONOUN)) != 0
		&& (grammems & (_QM(rFirstPerson) | _QM(rSecondPerson) | _QM(rThirdPerson)));
};



const static std::unordered_set<std::string> Particles = { "ЛИ","ЖЕ","БЫ","УЖ","ТОЛЬКО", "Ж", "Б", "ЛЬ" };

bool CRusGramTab::IsSimpleParticle(const std::string& lemma, part_of_speech_mask_t poses) const
{
	if (!(poses & (1 << PARTICLE))) return false;
	return Particles.find(lemma) != Particles.end();
}









/*
	Есть такие  мест. прилагательные, которые могут выступать в роли существительных:
	"КАЖДЫЙ", "ОДИН", "ДРУГОЙ","ТОТ","КОТОРЫЙ".
	Для  них не имеет смысла вводит аналогичные местоимения,поскольку  все они могут
	быть использованы во всех родах. Эти прилагательные могут вести себя так же, как
	существительные. Например,
	Я купил два яблока, в каждом оказалось много червяков
	Я знаю ту, которая тебя видит
	Один пришел, другой ушел.
	Я вижу дом, который разрушился.
	Вышеуказанные мест. прилагательные отличаются, например, от местоимения _R("этот") и "всякий", поскольку
	трудно себе представить что-то вроде:
	"эта пришла"
	"всякая пришла"
	Но возможно:
	"всякое бывает"
	"это бывает"
	Здесь "всякое" и "это" - простые местоимения ср рода.

	!! Таким образом, я не хочу дублировать в морфологии МС-П, если они могут использованы
	!! во всех родах в качестве существительного.

	1 марта 2001 года, Сокирко
*/
bool CRusGramTab::IsSynNoun(part_of_speech_mask_t poses, const std::string& lemma) const
{
	return   IsMorphNoun(poses)
		|| (poses & (1 << PRONOUN))
		|| ((poses & (1 << PRONOUN_P))
			&& ("КАЖДЫЙ" == lemma)
			|| ("ОДИН" == lemma)
			|| ("ДРУГОЙ" == lemma)
			|| ("ТОТ" == lemma)
			|| ("КОТОРЫЙ" == lemma)
			)
		;
};


const int StandardParamAbbrCount = 8;
const static std::string StandardParamAbbr[StandardParamAbbrCount] =
{ "КГ", "КМ", "СМ","МЛ", "МБ","КБ", "МГЦ", "КВ" };

bool CRusGramTab::IsStandardParamAbbr(const char* WordStrUpper) const
{
	if (strlen(WordStrUpper) > 6) return false;
	for (long i = 0; i < StandardParamAbbrCount; i++)
		if (StandardParamAbbr[i] == WordStrUpper)
			return true;

	return false;
};

std::string CRusGramTab::FilterGramCodes1(const std::string& gram_codes, grammems_mask_t good_grammems, grammems_mask_t bad_grammems) const
{
    if (gram_codes == "??") {
        return gram_codes;
    }
    std::string result;
    for (size_t l = 0; l < gram_codes.length(); l += 2)
    {
        auto g = GetLine(GramcodeToLineIndex(gram_codes.c_str() + l))->m_Grammems;
        if ( (g & good_grammems) && !(g & bad_grammems)) {
            result.append(gram_codes.c_str() + l, 2);
        }
    }
    return result;
}

std::string CRusGramTab::FindGramCodesContaining(const std::string& gram_codes, grammems_mask_t grammems) const
{
    if (gram_codes == "??") {
        return gram_codes;
    }
    std::string result;
    for (size_t l = 0; l < gram_codes.length(); l += 2)
    {
        auto g = GetLine(GramcodeToLineIndex(gram_codes.c_str() + l))->m_Grammems;
        if ( (g & grammems) == grammems) {
            result.append(gram_codes.c_str() + l, 2);
        }
    }
    return result;
}


bool CRusGramTab::FilterNounNumeral(std::string& gcNoun, const std::string& gcNum, grammems_mask_t& grammems) const
{
	if (gcNoun.length() == 2 || !(grammems & rAllCases)) return false;
    auto c2 = FilterGramCodes1(gcNum, grammems & rAllCases, 0);
    //auto gcNoun1 = gcNoun;
	//GleicheAncode3(CaseNumberGender0, gcNum, c2, gcNoun);
    auto gcNoun2 = GleicheAncode1(CaseNumberGender0, gcNoun, c2);
    gcNoun = gcNoun2;
	return true;
}


std::string RussianCaseNumberGender(const CAgramtab* pGramTab, const std::string& adj, const std::string& common_noun_grm, const std::string& noun)
{
	assert(false);
	//return pGramTab->GleicheGenderNumberCase(common_noun_grm.c_str(), noun.c_str(), adj.c_str());
	return "";
};

grammems_mask_t CRusGramTab::ChangeGleicheAncode1(GrammemCompare CompareFunc, const std::string& wordGramCodes, std::string& groupGramCodes, const grammems_mask_t wordGrammems) const
{
    auto c2 = FilterGramCodes1(wordGramCodes, GetMaxQWORD(),  _QM(rIndeclinable));
	groupGramCodes = GleicheAncode1(CompareFunc,  c2,  groupGramCodes);
	if (groupGramCodes == "") { return 0; }
	const grammems_mask_t gramFilter = rAllCases | rAllGenders | rAllTimes | rAllPersons | rAllAnimative;
	return 	wordGrammems & ~(gramFilter) | GetAllGrammems(groupGramCodes.c_str());
}


bool CaseNumber(const CAgramtabLine* l1, const CAgramtabLine* l2)
{
	return ((rAllCases & l1->m_Grammems & l2->m_Grammems) > 0) &&
		((rAllNumbers & l1->m_Grammems & l2->m_Grammems) > 0);
};

bool CaseGender(const CAgramtabLine* l1, const CAgramtabLine* l2)
{
	return ((rAllCases & l1->m_Grammems & l2->m_Grammems) > 0) &&
		((rAllGenders & l1->m_Grammems & l2->m_Grammems) > 0);
};

bool CaseNumberGender0(const CAgramtabLine* l1, const CAgramtabLine* l2) //with absent grammems check
{
	return ((rAllCases & l1->m_Grammems & l2->m_Grammems) > 0 || !(rAllCases & l1->m_Grammems) || !(rAllCases & l2->m_Grammems)) &&
		((rAllNumbers & l1->m_Grammems & l2->m_Grammems) > 0 || !(rAllNumbers & l1->m_Grammems) || !(rAllNumbers & l2->m_Grammems)) &&
		((rAllGenders & l1->m_Grammems & l2->m_Grammems) > 0 || !(rAllGenders & l1->m_Grammems) || !(rAllGenders & l2->m_Grammems)); ;
};
bool GenderNumber0(const CAgramtabLine* l1, const CAgramtabLine* l2) //with absent grammems check
{
	return ((rAllGenders & l1->m_Grammems & l2->m_Grammems) > 0 || !(rAllGenders & l1->m_Grammems) || !(rAllGenders & l2->m_Grammems)) &&
		((rAllNumbers & l1->m_Grammems & l2->m_Grammems) > 0 || !(rAllNumbers & l1->m_Grammems) || !(rAllNumbers & l2->m_Grammems));
};

bool GrammemsInclusion(const CAgramtabLine* l1, const CAgramtabLine* l2) //with absent grammems check
{
    return (l1->m_Grammems & l2->m_Grammems) == l2->m_Grammems ;
};

bool GrammemsEqu(const CAgramtabLine* l1, const CAgramtabLine* l2) //with absent grammems check
{
    return (l1->m_Grammems == l2->m_Grammems);
};

bool CRusGramTab::PartOfSpeechIsProductive(part_of_speech_t p) const {
	return p == NOUN || p == ADJ_FULL || p == VERB || p == ADV || p == INFINITIVE;
}

bool AnyGender(const CAgramtabLine* l1, const CAgramtabLine* )
{
    return  ((rAllGenders & l1->m_Grammems ) > 0);
};

