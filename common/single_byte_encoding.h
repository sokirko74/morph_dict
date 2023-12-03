#pragma once
#include  "base_types.h"
#include "../contrib/iconv_subset/iconv.h"
#include "vector"

// single byte chars for Graphan
const BYTE GraphematicalTAB    = (BYTE) 16;
const BYTE cParagraph    = (BYTE) 21;
const BYTE Apostrophe    = (BYTE) 39;
const BYTE cHyphenChar   = (BYTE) 45;

// important char of windows-1252
const BYTE Nu = 181;
const BYTE szlig = 223;

// windows-1251
const BYTE cEllipseChar  = (BYTE) 133;
const BYTE GraphematicalSPACE = (BYTE)160;
const BYTE OpenQuoteMark1251  = (BYTE) 171;
const BYTE cCompanyChar  = (BYTE) 176;
const BYTE cPiChar       = (BYTE) 182;
const BYTE GraphematicalEOLN = (BYTE)182;
const BYTE cIonChar      = (BYTE) 183;
const BYTE cNumberChar   = (BYTE) 185;
const BYTE CloseQuoteMark1251  = (BYTE) 187;



extern  bool is_english_upper(BYTE x);
extern  bool is_english_lower(BYTE x);
extern  bool is_german_upper(BYTE x);
extern  bool is_german_lower(BYTE x);
extern  bool is_russian_upper(BYTE x);
extern  bool is_russian_lower(BYTE x);

extern  bool is_upper_consonant(BYTE x, MorphLanguageEnum Langua);
extern  bool is_upper_vowel(BYTE x, MorphLanguageEnum Langua);

extern  bool is_english_alpha(BYTE x);
extern  bool is_russian_alpha(BYTE x);
extern  bool is_german_alpha(BYTE x);

extern  bool is_alpha(BYTE x);
extern  bool is_lower_alpha(BYTE x, MorphLanguageEnum langua);
extern  bool is_upper_alpha(BYTE x, MorphLanguageEnum langua);

extern  BYTE ReverseChar(BYTE ch, MorphLanguageEnum langua);
extern std::string& RmlMakeUpper(std::string& word, MorphLanguageEnum langua);

std::string convert_from_utf8(const char* utf8str, const MorphLanguageEnum langua);
std::string convert_to_utf8(const std::string& str, const MorphLanguageEnum langua);


extern void ConvertJO2Je(std::string& src);
extern void ConvertJO2Je(char* src, size_t Length);
extern std::string ConvertASCIIToHtmlSymbols(const std::string& txt);


template <class T, class Pred, class Conv>
T& RegisterConverter(T& word, size_t Len, Pred P, Conv C)
{
	for (size_t i = 0; i < Len; i++)
		if (P((BYTE)word[i]))
			word[i] = C((BYTE)word[i]);

	return word;
}



void MakeUpperVector(std::vector<char>& v, MorphLanguageEnum Langua);

inline std::string _R(const char* buffer) {
	return convert_from_utf8(buffer, morphRussian);
}

inline std::string _R(const std::string& s) {
	return _R(s.c_str());
}

extern bool		IsSuperEqualChar(BYTE ch1, BYTE ch2, MorphLanguageEnum langua);
extern int		CompareWithoutRegister(const char* s1, const char* s2, size_t l, MorphLanguageEnum langua);
extern BYTE		force_rus_char(BYTE ch);
extern BYTE convert_html_entity_to_char(const std::string& entity);
