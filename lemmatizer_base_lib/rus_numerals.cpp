#include "rus_numerals.h"
#include <algorithm>


const CNumeralToNumber NumeralToNumber[] =
{
	{_R("ОДИН"),_R("ПЕРВЫЙ"),"","","I",1,false,{_R("ОДНО") }},
	{_R("ПОЛТОРА"),"","","","",1.5,false,{_R("ПОЛУТОРО") }},
	{_R("ДВА"),_R("ВТОРОЙ"),_R("ВДВОЕМ"),_R("ДВОЕ"),"II",2,false, {_R("ДВУХЪ"), _R("ДВУХ"),_R("ДВУ")}},
	{_R("ТРИ"),_R("ТРЕТИЙ"),_R("ВТРОЕМ"), _R("ТРОЕ"),"III",3,false, {_R("ТРЕХЪ"),_R("ТРЕХ")}},
	{_R("ЧЕТЫРЕ"),_R("ЧЕТВЕРТЫЙ"),_R("ВЧЕТВЕРОМ"),_R("ЧЕТВЕРО"),"IV",4,false, {_R("ЧЕТЫРЕХЪ"), _R("ЧЕТЫРЕХ")}},
	{_R("ПЯТЬ"),_R("ПЯТЫЙ"),_R("ВПЯТЕРОМ"), _R("ПЯТЕРО"),"V",5,false, {_R("ПЯТИ")}},
	{_R("ШЕСТЬ"),_R("ШЕСТОЙ"),_R("ВШЕСТЕРОМ"), _R("ШЕСТЕРО"),"VI",6,false,{_R("ШЕСТИ")}},
	{_R("СЕМЬ"),_R("СЕДЬМОЙ"),_R("ВСЕМЕРОМ"), _R("СЕМЕРО"),"VII",7,false, {_R("СЕМИ")}},
	{_R("ВОСЕМЬ"),_R("ВОСЬМОЙ"),_R("ВВОСЬМЕРОМ"),_R("ВОСЬМЕРО"),"VIII",8,false, {_R("ВОСЕМИ")}},
	{_R("ДЕВЯТЬ"),_R("ДЕВЯТЫЙ"),_R("ВДЕВЯТЕРОМ"),_R("ДЕВЯТЕРО"),"IX",9 ,false, {_R("ДЕВЯТИ")}},
	{_R("ДЕСЯТЬ"),_R("ДЕСЯТЫЙ"),_R("ВДЕСЯТЕРОМ"),_R("ДЕСЯТЕРО"),"X",10,false, {_R("ДЕСЯТИ")}},
	{_R("ОДИННАДЦАТЬ"),_R("ОДИННАДЦАТЫЙ"),_R("ВОДИННАДЦАТЕРОМ"),"","XI",11,false, {_R("ОДИННАДЦАТИ")}},
	{_R("ДВЕНАДЦАТЬ"),_R("ДВЕНАДЦАТЫЙ"), _R("ВДВЕНАДЦАТЕРОМ"),"","XII",12,false, {_R("ДВЕНАДЦАТИ")}},
	{_R("ТРИНАДЦАТЬ"),_R("ТРИНАДЦАТЫЙ"), _R("ВТРИНАДЦАТЕРОМ"),"","XIII",13,false, {_R("ТРИНАДЦАТИ")}},
	{_R("ЧЕТЫРНАДЦАТЬ"),_R("ЧЕТЫРНАДЦАТЫЙ"),_R("ВЧЕТЫРНАДЦАТЕРОМ"),"","XIV",14,false, {_R("ЧЕТЫРНАДЦАТИ")} },
	{_R("ПЯТНАДЦАТЬ"),_R("ПЯТНАДЦАТЫЙ"), _R("ВПЯТНАДЦАТЕРОМ"),"","XV",15,false, {_R("ПЯТНАДЦАТИ")}},
	{_R("ШЕСТНАДЦАТЬ"),_R("ШЕСТНАДЦАТЫЙ"), _R("ВШЕСТНАДЦАТЕРОМ"),"","XVI",16,false, {_R("ШЕСТНАДЦАТИ")} },
	{_R("СЕМНАДЦАТЬ"),_R("СЕМНАДЦАТЫЙ"), _R("ВСЕМНАДЦАТЕРОМ"),"","XVII",17,false, {_R("СЕМНАДЦАТИ")}},
	{_R("ВОСЕМНАДЦАТЬ"),_R("ВОСЕМНАДЦАТЫЙ"), _R("ВВОСЕМНАДЦАТЕРОМ"),"","XIII",18,false, {_R("ВОСЕМНАДЦАТИ")}},
	{_R("ДЕВЯТНАДЦАТЬ"),_R("ДЕВЯТНАДЦАТЫЙ"), _R("ВДЕВЯТНАДЦАТЕРОМ"),"","XIX",19,false, {_R("ДЕВЯТНАДЦАТИ")} },
	{_R("ДВАДЦАТЬ"),_R("ДВАДЦАТЫЙ"), _R("ВДВАДЦАТЕРОМ"),"","XX",20,false, {_R("ДВАДЦАТИ")}},
	{_R("ТРИДЦАТЬ"),_R("ТРИДЦАТЫЙ"), _R("ВТРИДЦАТЕРОМ"),"","XXX",30,false, {_R("ТРИДЦАТИ")}},
	{_R("СОРОК"),_R("СОРОКОВОЙ"), "","","XL",40,false, {_R("СОРОКА")}},
	{_R("ПЯТЬДЕСЯТ"),_R("ПЯТИДЕСЯТЫЙ"), _R("ВПЯТИДЕСЯТЕРОМ"),"","L",50,false, {_R("ПЯТИДЕСЯТИ")}},
	{_R("ШЕСТЬДЕСЯТ"),_R("ШЕСТИДЕСЯТЫЙ"), _R("ВШЕСТИДЕСЯТЕРОМ"),"","LX",60,false, {_R("ШЕСТИДЕСЯТИ")}},
	{_R("СЕМЬДЕСЯТ"),_R("СЕМИДЕСЯТЫЙ"), _R("ВСЕМИДЕСЯТЕРОМ"),"","LXX",70,false, {_R("СЕМИДЕСЯТИ")}},
	{_R("ВОСЕМЬДЕСЯТ"),_R("ВОСЬМИДЕСЯТЫЙ"), _R("ВВОСЬМИДЕСЯТЕРОМ"),"","LXXX",80,false, {_R("ВОСЬМИДЕСЯТИ")}},
	{_R("ДЕВЯНОСТО"),_R("ДЕВЯНОСТЫЙ"), "","","XC",90,false, {_R("ДЕВЯНОСТО")}},
	{_R("СТО"),_R("СОТЫЙ"),"","","C",100,false, {_R("СТО")}},
	{_R("ДВЕСТИ"),_R("ДВУХСОТЫЙ"),"","","CC",200,false, {_R("ДВУХСОТ")}},
	{_R("ТРИСТА"),_R("ТРЕХСОТЫЙ"),"","","CCC",300,false, {_R("ТРЕХСОТ")}},
	{_R("ЧЕТЫРЕСТА"),_R("ЧЕТЫРЕХСОТЫЙ"),"","CD","I",400,false, {_R("ЧЕТЫРЕХСОТ")}},
	{_R("ПЯТЬСОТ"),_R("ПЯТИСОТЫЙ"), "","","D",500,false, {_R("ПЯТИСОТ")}},
	{_R("ШЕСТЬСОТ"),_R("ШЕСТИСОТЫЙ"), "","","DC",600,false, {_R("ШЕСТИСОТ")}},
	{_R("СЕМЬСОТ"),_R("СЕМИСОТЫЙ"), "","","DCC",700,false, {_R("СЕМИСОТ")}},
	{_R("ВОСЕМЬСОТ"),_R("ВОСЬМИСОТЫЙ"), "","","DCCC",800,false, {_R("ВОСЬМИСОТ")}},
	{_R("ДЕВЯТЬСОТ"),_R("ДЕВЯТИСОТЫЙ"),"","","CM",900,false, {_R("ДЕВЯТИСОТ")}},
	{_R("ТЫСЯЧА"),_R("ТЫСЯЧНЫЙ"),"","","M",1000,true, {_R("ТЫСЯЧЕ")}},
	{_R("МИЛЛИОН"),_R("МИЛЛИОННЫЙ"),"","","",1000000,true,{_R("МИЛЛИОНО")} },
	{_R("МИЛЛИАРД"),_R("МИЛЛИАРДНЫЙ"),"","","",1000000000,true, {_R("МИЛЛИАРДНО")} },
	{_R("ТРИЛЛИОН"),_R("ТРИЛЛИОННЫЙ"),"","","",1000000000000.0,true, {_R("ТРИЛЛИОНО")} },
	{_R("КВАДРИЛЛИОН"),_R("КВАДРИЛЛИОННЫЙ"),"","","",1000000000000000.0,true, {_R("КВАДРИЛЛИОН")} }
};

//ноль не должен быть включен в NumeralToNumber
CNumeralToNumber RUSSIAN_ZERO = { _R("НОЛЬ"),_R("НУЛЕВОЙ"),"","","",0,true,{_R("НУЛЬ")} }; // нульмодемный


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


