#pragma once
#include  "morph_dict/common/utilit.h"

enum NumberFormEnum {
	nfeCardinal = 0,
	nfeOridnal = 1,
	nfeGenitiv = 2
};

struct CNumeralToNumber
{
	std::string	m_Cardinal; // _"два"
	std::string m_Ordinal; // "второй"
	std::string m_Adverb; // "ВДВОЕМ"
	std::string m_CoollectiveNumber; //  "ДВОЕ"
	std::string m_RomanNumber; // II
	double		m_Number; // 2
	bool        m_bNoun; // false
	std::vector<std::string> m_AdjForms; // "двух" (двухламповый), дву, четырехъ

};

class CAdjPrefix {
public:
	std::string Prefix;
	const CNumeralToNumber* Parent;

	CAdjPrefix(std::string prefix, const CNumeralToNumber* parent) : Prefix(prefix),
		Parent(parent) {
	};

	bool operator < (const CAdjPrefix& other) const {
		return this->Prefix.length() > other.Prefix.length();
	}
};


class  CRussianNumerals {

	std::vector<CNumeralToNumber> m_Numerals;
	std::vector<CNumeralToNumber> m_NumeralsReverseWithZero;
	std::vector<CAdjPrefix> m_AdjPrefixes;
	
public:
	CRussianNumerals();
	const std::vector<CNumeralToNumber>& CRussianNumerals::GetAllNumeralReverse() const;
	const std::vector<CNumeralToNumber>& CRussianNumerals::GetAllNumeral() const;
	bool CheckIsNumeral(const std::string& lemma) const;
	std::string FindByNumber(uint64_t Number)  const;
	int IsAdverbRule(const std::string& Lemma) const;
	double GetCardinalNumeral(const std::string& word) const;
	double GetOrdinalNumeral(const std::string& word) const;
	double GetNounNumeral(const std::string& word) const;
	const CAdjPrefix* FindAdjPrefix(std::string word) const;
	
};

extern const CRussianNumerals RussianNumerals;

std::string DoubleToStr(double i);
