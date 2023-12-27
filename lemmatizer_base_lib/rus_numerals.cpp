#include "rus_numerals.h"
#include <algorithm>


const CNumeralToNumber NumeralToNumber[] =
{
	{"ОДИН","ПЕРВЫЙ","","","I",1,false,{"ОДНО" }},
	{"ПОЛТОРА","","","","",1.5,false,{"ПОЛУТОРО" }},
	{"ДВА","ВТОРОЙ","ВДВОЕМ","ДВОЕ","II",2,false, {"ДВУХЪ", "ДВУХ","ДВУ"}},
	{"ТРИ","ТРЕТИЙ","ВТРОЕМ", "ТРОЕ","III",3,false, {"ТРЕХЪ","ТРЕХ"}},
	{"ЧЕТЫРЕ","ЧЕТВЕРТЫЙ","ВЧЕТВЕРОМ","ЧЕТВЕРО","IV",4,false, {"ЧЕТЫРЕХЪ", "ЧЕТЫРЕХ"}},
	{"ПЯТЬ","ПЯТЫЙ","ВПЯТЕРОМ", "ПЯТЕРО","V",5,false, {"ПЯТИ"}},
	{"ШЕСТЬ","ШЕСТОЙ","ВШЕСТЕРОМ", "ШЕСТЕРО","VI",6,false,{"ШЕСТИ"}},
	{"СЕМЬ","СЕДЬМОЙ","ВСЕМЕРОМ", "СЕМЕРО","VII",7,false, {"СЕМИ"}},
	{"ВОСЕМЬ","ВОСЬМОЙ","ВВОСЬМЕРОМ","ВОСЬМЕРО","VIII",8,false, {"ВОСЕМИ"}},
	{"ДЕВЯТЬ","ДЕВЯТЫЙ","ВДЕВЯТЕРОМ","ДЕВЯТЕРО","IX",9 ,false, {"ДЕВЯТИ"}},
	{"ДЕСЯТЬ","ДЕСЯТЫЙ","ВДЕСЯТЕРОМ","ДЕСЯТЕРО","X",10,false, {"ДЕСЯТИ"}},
	{"ОДИННАДЦАТЬ","ОДИННАДЦАТЫЙ","ВОДИННАДЦАТЕРОМ","","XI",11,false, {"ОДИННАДЦАТИ"}},
	{"ДВЕНАДЦАТЬ","ДВЕНАДЦАТЫЙ", "ВДВЕНАДЦАТЕРОМ","","XII",12,false, {"ДВЕНАДЦАТИ"}},
	{"ТРИНАДЦАТЬ","ТРИНАДЦАТЫЙ", "ВТРИНАДЦАТЕРОМ","","XIII",13,false, {"ТРИНАДЦАТИ"}},
	{"ЧЕТЫРНАДЦАТЬ","ЧЕТЫРНАДЦАТЫЙ","ВЧЕТЫРНАДЦАТЕРОМ","","XIV",14,false, {"ЧЕТЫРНАДЦАТИ"} },
	{"ПЯТНАДЦАТЬ","ПЯТНАДЦАТЫЙ", "ВПЯТНАДЦАТЕРОМ","","XV",15,false, {"ПЯТНАДЦАТИ"}},
	{"ШЕСТНАДЦАТЬ","ШЕСТНАДЦАТЫЙ", "ВШЕСТНАДЦАТЕРОМ","","XVI",16,false, {"ШЕСТНАДЦАТИ"} },
	{"СЕМНАДЦАТЬ","СЕМНАДЦАТЫЙ", "ВСЕМНАДЦАТЕРОМ","","XVII",17,false, {"СЕМНАДЦАТИ"}},
	{"ВОСЕМНАДЦАТЬ","ВОСЕМНАДЦАТЫЙ", "ВВОСЕМНАДЦАТЕРОМ","","XIII",18,false, {"ВОСЕМНАДЦАТИ"}},
	{"ДЕВЯТНАДЦАТЬ","ДЕВЯТНАДЦАТЫЙ", "ВДЕВЯТНАДЦАТЕРОМ","","XIX",19,false, {"ДЕВЯТНАДЦАТИ"} },
	{"ДВАДЦАТЬ","ДВАДЦАТЫЙ", "ВДВАДЦАТЕРОМ","","XX",20,false, {"ДВАДЦАТИ"}},
	{"ТРИДЦАТЬ","ТРИДЦАТЫЙ", "ВТРИДЦАТЕРОМ","","XXX",30,false, {"ТРИДЦАТИ"}},
	{"СОРОК","СОРОКОВОЙ", "","","XL",40,false, {"СОРОКА"}},
	{"ПЯТЬДЕСЯТ","ПЯТИДЕСЯТЫЙ", "ВПЯТИДЕСЯТЕРОМ","","L",50,false, {"ПЯТИДЕСЯТИ"}},
	{"ШЕСТЬДЕСЯТ","ШЕСТИДЕСЯТЫЙ", "ВШЕСТИДЕСЯТЕРОМ","","LX",60,false, {"ШЕСТИДЕСЯТИ"}},
	{"СЕМЬДЕСЯТ","СЕМИДЕСЯТЫЙ", "ВСЕМИДЕСЯТЕРОМ","","LXX",70,false, {"СЕМИДЕСЯТИ"}},
	{"ВОСЕМЬДЕСЯТ","ВОСЬМИДЕСЯТЫЙ", "ВВОСЬМИДЕСЯТЕРОМ","","LXXX",80,false, {"ВОСЬМИДЕСЯТИ"}},
	{"ДЕВЯНОСТО","ДЕВЯНОСТЫЙ", "","","XC",90,false, {"ДЕВЯНОСТО"}},
	{"СТО","СОТЫЙ","","","C",100,false, {"СТО"}},
	{"ДВЕСТИ","ДВУХСОТЫЙ","","","CC",200,false, {"ДВУХСОТ"}},
	{"ТРИСТА","ТРЕХСОТЫЙ","","","CCC",300,false, {"ТРЕХСОТ"}},
	{"ЧЕТЫРЕСТА","ЧЕТЫРЕХСОТЫЙ","","CD","I",400,false, {"ЧЕТЫРЕХСОТ"}},
	{"ПЯТЬСОТ","ПЯТИСОТЫЙ", "","","D",500,false, {"ПЯТИСОТ"}},
	{"ШЕСТЬСОТ","ШЕСТИСОТЫЙ", "","","DC",600,false, {"ШЕСТИСОТ"}},
	{"СЕМЬСОТ","СЕМИСОТЫЙ", "","","DCC",700,false, {"СЕМИСОТ"}},
	{"ВОСЕМЬСОТ","ВОСЬМИСОТЫЙ", "","","DCCC",800,false, {"ВОСЬМИСОТ"}},
	{"ДЕВЯТЬСОТ","ДЕВЯТИСОТЫЙ","","","CM",900,false, {"ДЕВЯТИСОТ"}},
	{"ТЫСЯЧА","ТЫСЯЧНЫЙ","","","M",1000,true, {"ТЫСЯЧЕ"}},
	{"МИЛЛИОН","МИЛЛИОННЫЙ","","","",1000000,true,{"МИЛЛИОНО"} },
	{"МИЛЛИАРД","МИЛЛИАРДНЫЙ","","","",1000000000,true, {"МИЛЛИАРДНО"} },
	{"ТРИЛЛИОН","ТРИЛЛИОННЫЙ","","","",1000000000000.0,true, {"ТРИЛЛИОНО"} },
	{"КВАДРИЛЛИОН","КВАДРИЛЛИОННЫЙ","","","",1000000000000000.0,true, {"КВАДРИЛЛИОН"} }
};

//ноль не должен быть включен в NumeralToNumber
CNumeralToNumber RUSSIAN_ZERO = { "НОЛЬ","НУЛЕВОЙ","","","",0,true,{"НУЛЬ"} }; // нульмодемный


CRussianNumerals::CRussianNumerals() {
	for (auto a : NumeralToNumber) {
		m_Numerals.push_back(a);
	}
	m_NumeralsReverseWithZero.insert(m_NumeralsReverseWithZero.end(), m_Numerals.begin(), m_Numerals.end());
	m_NumeralsReverseWithZero.push_back(RUSSIAN_ZERO);
	std::reverse(m_NumeralsReverseWithZero.begin(), m_NumeralsReverseWithZero.end());
	for (auto& n : m_NumeralsReverseWithZero) {
		for (auto& p : n.m_AdjForms) {
			m_AdjPrefixes.push_back(CAdjPrefix(p, &n));
		}
	}
	std::sort(m_AdjPrefixes.begin(), m_AdjPrefixes.end());

}

const std::vector<CNumeralToNumber>& CRussianNumerals::GetAllNumeralReverse() const
{
	return m_NumeralsReverseWithZero;
}

std::string DoubleToStr (double i) 
{
	if (i == 1.5) return "1.5";
	char s[55];
	uint64_t  _i = i;
	#ifdef WIN32
		_i64toa(_i, s, 10);
	#else
			sprintf (s, "%ld", _i);
	#endif

	return s;
};


bool CRussianNumerals::CheckIsNumeral(const std::string& lemma) const {
	for (auto n: m_NumeralsReverseWithZero) {
		if (lemma == n.m_Cardinal || lemma == n.m_Ordinal) {
			return true;
		}
	}
	return false;
}

std::string CRussianNumerals::FindByNumber(uint64_t Number) const 
{
	// without zero
	for (auto n : m_Numerals) {
		if (Number == n.m_Number)
			return n.m_Cardinal;
	}
	return "";
};


int CRussianNumerals::IsAdverbRule(const std::string& Lemma) const
{
	for (auto n : m_Numerals) {
		if (Lemma == n.m_Adverb)
			return n.m_Number;
	}

   return -1;
};


double CRussianNumerals::GetCardinalNumeral(const std::string& word) const 
{
	for (auto n : m_Numerals) {
		if (word == n.m_Cardinal)
			return n.m_Number;
	}
	return -1;
};

double CRussianNumerals::GetOrdinalNumeral(const std::string& word) const 
{
	for (auto n : m_Numerals) {
		if (word == n.m_Ordinal) {
			return  n.m_Number;
		}
	}
	return -1;
};

double CRussianNumerals::GetNounNumeral(const std::string& word) const
{
	for (auto n : m_Numerals) {
		if (word == n.m_CoollectiveNumber)
			return n.m_Number;
	}
	return -1;
};


const CAdjPrefix* CRussianNumerals::FindAdjPrefix(std::string word) const
{
	for (const CAdjPrefix& a: m_AdjPrefixes) {
		if (startswith(word, a.Prefix))
		{
			return &a;
		};
	};
	return nullptr;
}

const  CRussianNumerals RussianNumerals = CRussianNumerals();


