#include "single_byte_encoding.h"
#include <assert.h>
#include <string.h>


// windows-1251 Russian
const BYTE UpperJO = (BYTE)168;
const BYTE LowerJO = (BYTE)184;
const BYTE RussianUpperA = 192;
const BYTE RussianUpperE = 197;
const BYTE RussianLowerA = 224;
const BYTE RussianLowerE = 229;
const BYTE RussianLowerK = 234;
const BYTE RussianLowerM = 236;
const BYTE RussianLowerN = 237;
const BYTE RussianLowerO = 238;
const BYTE RussianLowerR = 240;
const BYTE RussianLowerS = 241;
const BYTE RussianLowerU = 243;
const BYTE RussianLowerX = 245;

// windows-1252  (ISO/IEC 8859-1)
const BYTE Azirkun = 194;
const BYTE Auml = 196;
const BYTE Ccedille = 199;
const BYTE Egravis = 200;
const BYTE Eakut = 201;
const BYTE Ezirkun = 202;
const BYTE Ntilda = 209;
const BYTE Ozirkun = 212;
const BYTE Ouml = 214;
const BYTE Uzirkun = 219;
const BYTE Uuml = 220;
const BYTE agrave = 224;
const BYTE azirkun = 226;
const BYTE auml = 228;
const BYTE ccedille = 231;
const BYTE egrave = 232;
const BYTE eacute = 233;
const BYTE ezirkun = 234;
const BYTE ntilda = 241;
const BYTE ozirkun = 244;
const BYTE ouml = 246;
const BYTE uzirkun = 251;
const BYTE uuml = 252;


const uint16_t fWordDelim = 1;
const uint16_t RusUpper = 2;
const uint16_t RusLower = 4;
const uint16_t GerUpper = 8;
const uint16_t GerLower = 16;
const uint16_t EngUpper = 32;
const uint16_t EngLower = 64;
const uint16_t OpnBrck = 128;
const uint16_t ClsBrck = 256;
const uint16_t UpRomDigits = 512;
const uint16_t LwRomDigits = 1024;
const uint16_t LatinVowel = 2048;
const uint16_t RussianVowel = 4096;
const uint16_t URL_CHAR = 8192;


const uint16_t ASCII[256] = {
	/*null (nul)*/                                     fWordDelim,
	/*start of heading (soh)*/                         fWordDelim,
	/*start of text (stx)*/                            fWordDelim,
	/*end of text (etx)*/                              fWordDelim,
	/*end of transmission (eot)*/                      fWordDelim,
	/*enquiry (enq)*/                                  fWordDelim,
	/*acknowledge (ack)*/                              fWordDelim,
	/*bell (bel)*/                                     fWordDelim,
	/*backspace (bs)*/                                 fWordDelim,
	/*character tabulation (ht)*/                      fWordDelim,
	/*line feed (lf)*/                                 fWordDelim,
	/*line tabulation (vt)*/                           fWordDelim,
	/*form feed (ff)*/                                 fWordDelim,
	/*carriage return (cr)*/                           fWordDelim,
	/*shift out (so)*/                                 fWordDelim,
	/*shift in (si)*/                                  fWordDelim,
	/*datalink escape (dle)*/                          fWordDelim,
	/*device control one (dc1)*/                       fWordDelim,
	/*device control two (dc2)*/                       fWordDelim,
	/*device control three (dc3)*/                     fWordDelim,
	/*device control four (dc4)*/                      fWordDelim,
	/*negative acknowledge (nak)*/                     fWordDelim,
	/*syncronous idle (syn)*/                          fWordDelim,
	/*end of transmission block (etb)*/                fWordDelim,
	/*cancel (can)*/                                   fWordDelim,
	/*end of medium (em)*/                             fWordDelim,
	/*substitute (sub)*/                               fWordDelim,
	/*escape (esc)*/                                   fWordDelim,
	/*file separator (is4)*/                           fWordDelim,
	/*group separator (is3)*/                          fWordDelim,
	/*record separator (is2)*/                         fWordDelim,
	/*unit separator (is1)*/                           fWordDelim,
	/*space ' '*/                                      fWordDelim,
	/*exclamation mark '!'*/                           fWordDelim | URL_CHAR,
	/*quotation mark '"'*/                             fWordDelim,
	/*number sign '#'*/                                fWordDelim,
	/*dollar sign '$'*/                                fWordDelim | URL_CHAR,
	/*percent sign '%'*/                               fWordDelim | URL_CHAR,
	/*ampersand '&'*/                                  fWordDelim | URL_CHAR,
	/*apostrophe '''*/                                 fWordDelim | RusUpper | RusLower, // твердый знак в виде апострофа
	/*left parenthesis '('*/                           fWordDelim | OpnBrck | URL_CHAR,
	/*right parenthesis ')'*/                          fWordDelim | ClsBrck | URL_CHAR,
	/*asterisk '*'*/                                   fWordDelim | URL_CHAR,
	/*plus sign '+'*/                                  fWordDelim | URL_CHAR,
	/*comma ','*/                                      fWordDelim | URL_CHAR,
	/*hyphen-minus '-'*/                               fWordDelim | URL_CHAR,
	/*full stop '.'*/                                  fWordDelim | URL_CHAR,
	/*solidus '/'*/                                    fWordDelim | URL_CHAR,
	/*digit zero '0'*/                                 URL_CHAR,
	/*digit one '1'*/                                  URL_CHAR,
	/*digit two '2'*/                                  URL_CHAR,
	/*digit three '3'*/                                URL_CHAR,
	/*digit four '4'*/                                 URL_CHAR,
	/*digit five '5'*/                                 URL_CHAR,
	/*digit six '6'*/                                  URL_CHAR,
	/*digit seven '7'*/                                URL_CHAR,
	/*digit eight '8'*/                                URL_CHAR,
	/*digit nine '9'*/                                 URL_CHAR,
	/*colon ':'*/                                      fWordDelim | URL_CHAR,
	/*semicolon ';'*/                                  fWordDelim | URL_CHAR,
	/*less-than sign '<'*/                             fWordDelim | OpnBrck,
	/*equals sign '='*/                                fWordDelim | URL_CHAR,
	/*greater-than sign '>'*/                          fWordDelim | ClsBrck,
	/*question mark '?'*/                              fWordDelim | URL_CHAR,
	/*commercial at '@'*/                              fWordDelim | URL_CHAR,
	/*latin capital letter a 'A'*/                     GerUpper | EngUpper | LatinVowel,
	/*latin capital letter b 'B'*/                     GerUpper | EngUpper,
	/*latin capital letter c 'C'*/                     GerUpper | EngUpper,
	/*latin capital letter d 'D'*/                     GerUpper | EngUpper,
	/*latin capital letter e 'E'*/                     GerUpper | EngUpper | LatinVowel,
	/*latin capital letter f 'F'*/                     GerUpper | EngUpper,
	/*latin capital letter g 'G'*/                     GerUpper | EngUpper,
	/*latin capital letter h 'H'*/                     GerUpper | EngUpper,
	/*latin capital letter i 'I'*/                     GerUpper | EngUpper | UpRomDigits | LatinVowel,
	/*latin capital letter j 'J'*/                     GerUpper | EngUpper,
	/*latin capital letter k 'K'*/                     GerUpper | EngUpper,
	/*latin capital letter l 'L'*/                     GerUpper | EngUpper | UpRomDigits,
	/*latin capital letter m 'M'*/                     GerUpper | EngUpper,
	/*latin capital letter n 'N'*/                     GerUpper | EngUpper,
	/*latin capital letter o 'O'*/                     GerUpper | EngUpper | LatinVowel,
	/*latin capital letter p 'P'*/                     GerUpper | EngUpper,
	/*latin capital letter q 'Q'*/                     GerUpper | EngUpper,
	/*latin capital letter r 'R'*/                     GerUpper | EngUpper,
	/*latin capital letter s 'S'*/                     GerUpper | EngUpper,
	/*latin capital letter t 'T'*/                     GerUpper | EngUpper,
	/*latin capital letter u 'U'*/                     GerUpper | EngUpper | LatinVowel,
	/*latin capital letter v 'V'*/                     GerUpper | EngUpper | UpRomDigits,
	/*latin capital letter w 'W'*/                     GerUpper | EngUpper,
	/*latin capital letter x 'X'*/                     GerUpper | EngUpper | UpRomDigits,
	/*latin capital letter y 'Y'*/                     GerUpper | EngUpper,
	/*latin capital letter z 'Z'*/                     GerUpper | EngUpper,
	/*left square bracket '['*/                        fWordDelim | OpnBrck,
	/*reverse solidus '\'*/                            fWordDelim,
	/*right square bracket ']'*/                       fWordDelim | ClsBrck,
	/*circumflex accent '^'*/                          fWordDelim,
	/*low line '_'*/                                   fWordDelim,
	/*grave accent '`'*/                               fWordDelim,
	/*latin small letter a 'a'*/                       GerLower | EngLower | LatinVowel | URL_CHAR,
	/*latin small letter b 'b'*/                       GerLower | EngLower | URL_CHAR,
	/*latin small letter c 'c'*/                       GerLower | EngLower | URL_CHAR,
	/*latin small letter d 'd'*/                       GerLower | EngLower | URL_CHAR,
	/*latin small letter e 'e'*/                       GerLower | EngLower | LatinVowel | URL_CHAR,
	/*latin small letter f 'f'*/                       GerLower | EngLower | URL_CHAR,
	/*latin small letter g 'g'*/                       GerLower | EngLower | URL_CHAR,
	/*latin small letter h 'h'*/                       GerLower | EngLower | URL_CHAR,
	/*latin small letter i 'i'*/                       GerLower | EngLower | LwRomDigits | LatinVowel | URL_CHAR,
	/*latin small letter j 'j'*/                       GerLower | EngLower | URL_CHAR,
	/*latin small letter k 'k'*/                       GerLower | EngLower | URL_CHAR,
	/*latin small letter l 'l'*/                       GerLower | EngLower | LwRomDigits | URL_CHAR,
	/*latin small letter m 'm'*/                       GerLower | EngLower | URL_CHAR,
	/*latin small letter n 'n'*/                       GerLower | EngLower | URL_CHAR,
	/*latin small letter o 'o'*/                       GerLower | EngLower | LatinVowel | URL_CHAR,
	/*latin small letter p 'p'*/                       GerLower | EngLower | URL_CHAR,
	/*latin small letter q 'q'*/                       GerLower | EngLower | URL_CHAR,
	/*latin small letter r 'r'*/                       GerLower | EngLower | URL_CHAR,
	/*latin small letter s 's'*/                       GerLower | EngLower | URL_CHAR,
	/*latin small letter t 't'*/                       GerLower | EngLower | URL_CHAR,
	/*latin small letter u 'u'*/                       GerLower | EngLower | LatinVowel | URL_CHAR,
	/*latin small letter v 'v'*/                       GerLower | EngLower | LwRomDigits | URL_CHAR,
	/*latin small letter w 'w'*/                       GerLower | EngLower | URL_CHAR,
	/*latin small letter x 'x'*/                       GerLower | EngLower | LwRomDigits | URL_CHAR,
	/*latin small letter y 'y'*/                       GerLower | EngLower | URL_CHAR,
	/*latin small letter z 'z'*/                       GerLower | EngLower | URL_CHAR,
	/*left curly bracket '{'*/                         fWordDelim | OpnBrck,
	/*vertical line '|'*/                              fWordDelim,
	/*right curly bracket '}'*/                        fWordDelim | ClsBrck,
	/*tilde '~'*/                                      fWordDelim,
	/*delete ''*/                                     0,
	/*padding character (pad) '_'*/                    fWordDelim,
	/*high octet preset (hop) '_'*/                    0,
	/*break permitted here (bph) '''*/                 0,
	/*no break here (nbh) '_'*/                        fWordDelim,
	/*index (ind) '"'*/                                0,
	/*next line (nel) ':'*/                            fWordDelim,
	/*start of selected area (ssa) '+'*/               fWordDelim,
	/*end of selected area (esa) '+'*/                 fWordDelim,
	/*character tabulation std::set (hts) '_'*/             fWordDelim,
	/*character tabulation with justification (htj) '%'*/ fWordDelim,
	/*line tabulation std::set (vts) '_'*/                  0,
	/*partial line forward (pld) '<'*/                 fWordDelim,
	/*partial line backward (plu) '_'*/                fWordDelim,
	/*reverse line feed (ri) '_'*/                     fWordDelim,
	/*single-shift two (ss2) '_'*/                     fWordDelim,
	/*single-shift three (ss3) '_'*/                   fWordDelim,
	/*device control std::string (dcs) '_'*/                fWordDelim,
	/*private use one (pu1) '''*/                      fWordDelim,
	/*private use two (pu2) '''*/                      fWordDelim,
	/*std::set transmit state (sts) '"'*/                   fWordDelim,
	/*cancel character (cch) '"'*/                     fWordDelim,
	/*message waiting (mw) ''*/                       fWordDelim,
	/*start of guarded area (spa) '-'*/                fWordDelim,
	/*end of guarded area (epa) '-'*/                  fWordDelim,
	/*start of std::string (sos) '_'*/                      fWordDelim,
	/*single graphic character introducer (sgci) 'T'*/ fWordDelim,
	/*single character introducer (sci) '_'*/          fWordDelim,
	/*control sequence introducer (csi) '>'*/          fWordDelim,
	/*std::string terminator (st) '_'*/                     fWordDelim,
	/*operating system command (osc) '_'*/             fWordDelim,
	/*privacy message (pm) '_'*/                       fWordDelim,
	/*application program command (apc) '_'*/          fWordDelim,
	/*no-break space ' '*/                             fWordDelim,
	/*inverted exclamation mark 'Ў'*/                  fWordDelim,
	/*cent sign 'ў'*/                                  fWordDelim,
	/*pound sign '_'*/                                 fWordDelim,
	/*currency sign '¤'*/                              fWordDelim,
	/*yen sign '_'*/                                   fWordDelim,
	/*broken bar '¦'*/                                 fWordDelim,
	/*section sign '§'*/                               fWordDelim,
	/*diaeresis 'Ё'*/                                  fWordDelim | RusUpper | RussianVowel,
	/*copyright sign 'c'*/                             fWordDelim,
	/*feminine ordinal indicator 'Є'*/                 fWordDelim,
	/*left pointing double angle quotation mark '<'*/  fWordDelim,
	/*not sign '¬'*/                                   fWordDelim,
	/*soft hyphen '-'*/                                fWordDelim,
	/*registered sign 'R'*/                            fWordDelim,
	/*macron 'Ї'*/                                     fWordDelim,
	/*degree sign '°'*/                                fWordDelim,
	/*plus-minus sign '+'*/                            fWordDelim,
	/*superscript two '_'*/                            fWordDelim,
	/*superscript three '_'*/                          fWordDelim,
	/*acute '_'*/                                      fWordDelim,
	/*micro sign 'ч'*/                                 fWordDelim | GerLower | GerUpper,
	/*pilcrow sign '¶'*/                               fWordDelim,
	/*middle dot '·'*/                                 fWordDelim,
	/*cedilla 'ё'*/                                    RusLower | RussianVowel,
	/*superscript one '№'*/                            fWordDelim,
	/*masculine ordinal indicator 'є'*/                fWordDelim,
	/*right pointing double angle quotation mark '>'*/ fWordDelim,
	/*vulgar fraction one quarter '_'*/                fWordDelim,
	/*vulgar fraction one half '_'*/                   fWordDelim,
	/*vulgar fraction three quarters '_'*/             fWordDelim,
	/*inverted question mark 'ї'*/                     fWordDelim,
	/*latin capital letter a with grave 'А'*/          RusUpper | RussianVowel,
	/*latin capital letter a with acute 'Б'*/          RusUpper,
	/*latin capital letter a with circumflex 'В'*/     RusUpper | GerUpper | EngUpper | LatinVowel,
	/*latin capital letter a with tilde 'Г'*/          RusUpper,
	/*latin capital letter a with diaeresis 'Д'*/      RusUpper | GerUpper | LatinVowel,
	/*latin capital letter a with ring above 'Е'*/     RusUpper | RussianVowel,
	/*latin capital ligature ae 'Ж'*/                  RusUpper,
	/*latin capital letter c with cedilla 'З'*/        RusUpper | GerUpper | EngUpper,
	/*latin capital letter e with grave 'И'*/          RusUpper | GerUpper | EngUpper | LatinVowel | RussianVowel,
	/*latin capital letter e with acute 'Й'*/          RusUpper | GerUpper | EngUpper | LatinVowel,
	/*latin capital letter e with circumflex 'К'*/     RusUpper | GerUpper | EngUpper | LatinVowel,
	/*latin capital letter e with diaeresis 'Л'*/      RusUpper,
	/*latin capital letter i with grave 'М'*/          RusUpper,
	/*latin capital letter i with acute 'Н'*/          RusUpper,
	/*latin capital letter i with circumflex 'О'*/     RusUpper | RussianVowel,
	/*latin capital letter i with diaeresis 'П'*/      RusUpper,
	/*latin capital letter eth (icelandic) 'Р'*/       RusUpper,
	/*latin capital letter n with tilde 'С'*/          RusUpper | GerUpper | EngUpper,
	/*latin capital letter o with grave 'Т'*/          RusUpper,
	/*latin capital letter o with acute 'У'*/          RusUpper | RussianVowel,
	/*latin capital letter o with circumflex 'Ф'*/     RusUpper | GerUpper | EngUpper | LatinVowel,
	/*latin capital letter o with tilde 'Х'*/          RusUpper,
	/*latin capital letter o with diaeresis 'Ц'*/      RusUpper | GerUpper | EngUpper | LatinVowel,
	/*multiplication sign 'Ч'*/                        RusUpper,
	/*latin capital letter o with stroke 'Ш'*/         RusUpper | UpRomDigits,
	/*latin capital letter u with grave 'Щ'*/          RusUpper,
	/*latin capital letter u with acute 'Ъ'*/          RusUpper,
	/*latin capital letter u with circumflex 'Ы'*/     RusUpper | GerUpper | EngUpper | LatinVowel | RussianVowel,
	/*latin capital letter u with diaeresis 'Ь'*/      RusUpper | GerUpper | LatinVowel,
	/*latin capital letter y with acute 'Э'*/          RusUpper | RussianVowel,
	/*latin capital letter thorn (icelandic) 'Ю'*/     RusUpper | RussianVowel,
	/*latin small letter sharp s (german) 'Я'*/        RusUpper | GerLower | GerUpper | RussianVowel,
	/*latin small letter a with grave 'а'*/            RusLower | RussianVowel,
	/*latin small letter a with acute 'б'*/            RusLower,
	/*latin small letter a with circumflex 'в'*/       RusLower | GerLower | EngLower | LatinVowel,
	/*latin small letter a with tilde 'г'*/            RusLower,
	/*latin small letter a with diaeresis 'д'*/        RusLower | GerLower | LatinVowel,
	/*latin small letter a with ring above 'е'*/       RusLower | RussianVowel,
	/*latin small ligature ae 'ж'*/                    RusLower,
	/*latin small letter c with cedilla 'з'*/          RusLower | GerLower | EngLower,
	/*latin small letter e with grave 'и'*/            RusLower | GerLower | EngLower | LatinVowel | RussianVowel,
	/*latin small letter e with acute 'й'*/            RusLower | GerLower | EngLower | LatinVowel,
	/*latin small letter e with circumflex 'к'*/       RusLower | GerLower | EngLower | LatinVowel,
	/*latin small letter e with diaeresis 'л'*/        RusLower,
	/*latin small letter i with grave 'м'*/            RusLower,
	/*latin small letter i with acute 'н'*/            RusLower,
	/*latin small letter i with circumflex 'о'*/       RusLower | RussianVowel,
	/*latin small letter i with diaeresis 'п'*/        RusLower,
	/*latin small letter eth (icelandic) 'р'*/         RusLower,
	/*latin small letter n with tilde 'с'*/            RusLower | GerLower | EngLower,
	/*latin small letter o with grave 'т'*/            RusLower,
	/*latin small letter o with acute 'у'*/            RusLower | RussianVowel,
	/*latin small letter o with circumflex 'ф'*/       RusLower | GerLower | EngLower | LatinVowel,
	/*latin small letter o with tilde 'х'*/            RusLower,
	/*latin small letter o with diaeresis 'ц'*/        RusLower | GerLower | EngLower | LatinVowel,
	/*division sign 'ч'*/                              RusLower,
	/*latin small letter o with stroke 'ш'*/           RusLower,
	/*latin small letter u with grave 'щ'*/            RusLower,
	/*latin small letter u with acute 'ъ'*/            RusLower,
	/*latin small letter u with circumflex 'ы'*/       RusLower | GerLower | EngLower | LatinVowel | RussianVowel,
	/*latin small letter u with diaeresis 'ь'*/        RusLower | GerLower | LatinVowel,
	/*latin small letter y with acute 'э'*/            RusLower | RussianVowel,
	/*latin small letter thorn (icelandic) 'ю'*/       RusLower | RussianVowel,
	/*latin small letter y with diaeresis  'я'*/       RusLower | RussianVowel
};


BYTE convert_latin_char_similar_russian_lower_char(BYTE ch) {
	switch (ch) {
	case 'a': return RussianLowerA;
	case 'A': return RussianLowerA;
	case 'e': return RussianLowerE;
	case 'E': return RussianLowerE;
	case 'k': return RussianLowerK;
	case 'K': return RussianLowerK;
	case 'M': return RussianLowerM;
		//case 'm' : return RussianLowerM; //latin lower m  looks different
	case 'H': return RussianLowerN;
	case 'o': return RussianLowerO;
	case 'O': return RussianLowerO;
	case '0': return RussianLowerO;
	case 'p': return RussianLowerR;
	case 'P': return RussianLowerR;
	case 'C': return RussianLowerS;
	case 'c': return RussianLowerS;
	case 'y': return RussianLowerU;
	case 'Y': return RussianLowerU;
	case 'X': return RussianLowerX;
	case 'x': return RussianLowerX;
	default: return ch;
	}
}



bool is_upper_roman_digit(BYTE ch)
{
	return	(ASCII[ch] & UpRomDigits) > 0;
}

bool is_lower_roman_digit(BYTE ch)
{
	return (ASCII[ch] & LwRomDigits) > 0;
}


bool is_roman_number(const char* s, size_t len)
{
	if (len == 0) return false;
	if (!s) return false;
	bool bLowRoman = true;
	bool bUpperRoman = true;
	for (size_t i = 0; i < len; i++)
	{
		if ((i > 0) && (s[i] == '-') && (i + 3 >= len)) //  окончания пишутся после дефиса, типа "Павла I-го"
		{
			return bLowRoman || bUpperRoman;
		};

		bLowRoman = bLowRoman && ((ASCII[(BYTE)s[i]] & LwRomDigits) > 0);
		bUpperRoman = bUpperRoman && ((ASCII[(BYTE)s[i]] & UpRomDigits) > 0);

	};
	return    bLowRoman || bUpperRoman;
}

//  =============  Punctuation Letters ======================
bool is_spc_fill(BYTE x)
{
	return ((x) == ' ' || (x) == '_');
};

//====================================================
//============= ENGLISH                   ============
//====================================================


bool is_generic_upper(BYTE x)
{
	// why ,,
	return		(ASCII[x] & EngUpper) > 0;
};

bool is_generic_lower(BYTE x)
{
	return		(ASCII[x] & EngLower) > 0 || x >= 128;
};

bool is_generic_alpha(BYTE x)
{
	return is_english_alpha(x) || x >= 128;
};

bool is_URL_alpha(BYTE x)
{
	return (ASCII[x] & URL_CHAR) > 0;
};


bool is_english_upper(BYTE x)
{
	return		(ASCII[x] & EngUpper) > 0;
};

bool is_english_upper_vowel(BYTE x)
{
	return			((ASCII[x] & EngUpper) > 0)
		&& ((ASCII[x] & LatinVowel) > 0);
};


bool is_english_lower(BYTE x)
{
	return		(ASCII[x] & EngLower) > 0;
};

bool is_english_lower_vowel(BYTE x)
{
	return			((ASCII[x] & EngLower) > 0)
		&& ((ASCII[x] & LatinVowel) > 0);
};

bool is_english_alpha(BYTE x)
{
	return is_english_lower(x) || is_english_upper(x);
};

BYTE etoupper(BYTE ch)
{
	if (ch >= (BYTE)'a' && ch <= (BYTE)'z')
		return ch -= 'a' - 'A';
	else
		switch (ch) {
		case ccedille: return Ccedille; // C with cedille
		case egrave: return Egravis;  // E with gravis
		case ezirkun: return Ezirkun;  // E with zirkun
		case azirkun: return Azirkun;  // A with zirkun
		case ozirkun: return Ozirkun;  // O with zirkun
		case ouml: return Ouml;  // O umlaut
		case ntilda: return Ntilda;  // N with tilda
		case uzirkun: return Uzirkun;  // U with zirkun
		case eacute: return Eakut;  // E acut
		default: return ch;
		};
};


BYTE etolower(BYTE ch)
{
	if (ch >= (BYTE)'A' && ch <= (BYTE)'Z')
		return ch += 'a' - 'A';
	else
		switch (ch) {
		case Ccedille: return ccedille; // C with cedille
		case Egravis: return egrave;  // E with gravis
		case Ezirkun: return ezirkun;  // E with zirkun
		case Azirkun: return azirkun;  // A with zirkun
		case Ozirkun: return ozirkun;  // O with zirkun
		case Ouml: return ouml;  // O umlaut
		case Ntilda: return ntilda;  // N with tilda
		case Uzirkun: return uzirkun;  // U with zirkun
		case Eakut: return eacute;  // E acut
		default: return ch;
		};
};


//====================================================
//============= GERMAN                  ============
//====================================================

bool is_german_upper(BYTE x)
{
	return		(ASCII[x] & GerUpper) > 0;
};

bool is_german_lower(BYTE x)
{
	return		(ASCII[x] & GerLower) > 0;
};

bool is_german_lower_vowel(BYTE x)
{
	return			(ASCII[x] & GerLower)
		&& (ASCII[x] & LatinVowel);
};

bool is_german_upper_vowel(BYTE x)
{
	return			(ASCII[x] & GerUpper)
		&& (ASCII[x] & LatinVowel);

};

bool is_german_alpha(BYTE x)
{
	return is_german_lower(x) || is_german_upper(x);
};

BYTE gtoupper(BYTE ch)
{
	if (is_english_lower(ch))
		return etoupper(ch);
	else
		switch (ch)
		{
		case auml: return Auml;;
		case uuml: return Uuml;
		case ouml: return Ouml;
		default: return ch;
		};
};

BYTE gtolower(BYTE ch)
{
	if (is_english_upper(ch))
		return etolower(ch);
	else
		switch (ch)
		{
		case Auml: return auml;;
		case Uuml: return uuml;
		case Ouml: return ouml;
		default: return ch;
		};
};

//====================================================
//============= RUSSIAN                  ============
//====================================================

bool is_russian_upper(BYTE x)
{
	return		(ASCII[x] & RusUpper) > 0;
};

bool is_russian_lower(BYTE x)
{
	return		(ASCII[x] & RusLower) > 0;
};

bool is_russian_lower_vowel(BYTE x)
{
	return			((ASCII[x] & RusLower) > 0)
		&& ((ASCII[x] & RussianVowel) > 0);

};
bool is_russian_upper_vowel(BYTE x)
{
	return			((ASCII[x] & RusUpper) > 0)
		&& ((ASCII[x] & RussianVowel) > 0);
};

bool is_russian_lower_consonant(BYTE x)
{
	return !is_russian_lower_vowel(x) && is_russian_lower(x);
}
bool is_russian_upper_consonant(BYTE x)
{
	return !is_russian_upper_vowel(x) && is_russian_upper(x);
}
bool is_russian_consonant(BYTE x)
{
	return is_russian_lower_consonant(x) || is_russian_upper_consonant(x);
}


bool is_russian_alpha(BYTE x)
{
	return is_russian_lower(x) || is_russian_upper(x);
};

BYTE rtoupper(BYTE ch)
{
	if (ch == LowerJO) return UpperJO;
	if (ch == Apostrophe) return Apostrophe;

	if (is_russian_lower(ch))
		return  ch - (RussianLowerA - RussianUpperA);
	else
		return ch;
};

BYTE rtolower(BYTE ch)
{
	if (ch == UpperJO) return LowerJO;
	if (ch == Apostrophe) return Apostrophe;

	if (is_russian_upper(ch))
		return ch + RussianLowerA - RussianUpperA;
	else
		return ch;
};


// ================== IsSuperEqualChar проверяет, что символы равны с точностью до регистра
//  и с точностью до "языка", m.е. латинское "a" будет равно  русскому "а".
bool IsSuperEqualChar(BYTE ch1, BYTE ch2, MorphLanguageEnum langua)
{
	if ((ch1 == ch2)
		|| (ch1 == ReverseChar(ch2, langua))
		)
		return true;

	ch1 = convert_latin_char_similar_russian_lower_char(ch1);
	ch2 = convert_latin_char_similar_russian_lower_char(ch2);

	if (is_russian_upper(ch1))
		ch1 = rtolower(ch1);

	if (is_russian_upper(ch2))
		ch2 = rtolower(ch2);

	if (is_english_upper(ch1))
		ch1 = tolower(ch1);

	if (is_english_upper(ch2))
		ch2 = tolower(ch2);

	return ch1 == ch2;
}



//========================================= ===========
//============= General encoding functions ============
//====================================================

bool is_upper_vowel(BYTE x, MorphLanguageEnum Langua)
{
	switch (Langua)
	{
	case morphFioDisclosures:
	case morphRussian: return is_russian_upper_vowel(x);
	case morphEnglish: return is_english_upper_vowel(x);
	case morphGerman: return is_german_upper_vowel(x);
	};
	assert(false);
	return false;
};

bool is_upper_consonant(BYTE x, MorphLanguageEnum Langua)
{
	if (!is_upper_alpha(x, Langua)) return false;
	return !is_upper_vowel(x, Langua);
};

bool is_alpha(BYTE x)
{
	return is_russian_alpha(x) || is_german_alpha(x);
};



bool is_lower_alpha(BYTE x, MorphLanguageEnum Langua)
{
	switch (Langua)
	{
	case morphFioDisclosures:
	case morphRussian: return is_russian_lower(x);
	case morphEnglish: return is_english_lower(x);
	case morphGerman: return is_german_lower(x);
	case morphGeneric: return is_generic_lower(x);
	};
	assert(false);
	return false;
};

bool is_upper_alpha(BYTE x, MorphLanguageEnum Langua)
{
	switch (Langua)
	{
	case morphFioDisclosures:
	case morphRussian: return is_russian_upper(x);
	case morphEnglish: return is_english_upper(x);
	case morphGerman: return is_german_upper(x);
	case morphGeneric: return is_generic_upper(x);
	};
	assert(false);
	return false;
};

bool isnspace(BYTE x)
{
	return x == ' ' || x == '\t';
};



BYTE ReverseChar(BYTE ch, MorphLanguageEnum langua)
{
	if (langua == morphGerman)
	{
		if (is_english_lower(ch))
			return etoupper(ch);
		else
			if (is_english_upper(ch))
				return etolower(ch);
			else switch (ch)
			{
			case Auml: return auml;
			case auml: return Auml;;
			case Uuml: return uuml;
			case uuml: return Uuml;
			case Ouml: return ouml;
			case ouml: return Ouml;
			default: return ch;
			};
	}
	else if (langua == morphGeneric)
	{
		if (is_english_lower(ch))
			return etoupper(ch);
		else
			if (is_english_upper(ch))
				return etolower(ch);

	}
	else
		if (is_russian_lower(ch))
			return rtoupper(ch);
		else
			if (is_russian_upper(ch))
				return rtolower(ch);
			else
				if (is_english_lower(ch))
					return etoupper(ch);
				else
					if (is_english_upper(ch))
						return etolower(ch);

	return ch;
};


bool is_arab_digit(BYTE c)
{
	return isdigit(c) != 0;
}




// конвертирует из прописные  кириллицы в строчную
std::string& EngRusMakeLower(std::string& word)
{
	size_t len = word.length();

	for (size_t i = 0; i < len; i++)
		if (is_russian_upper((BYTE)word[i]))
			word[i] = rtolower((BYTE)word[i]);
		else
			word[i] = etolower((BYTE)word[i]);

	return word;
}


// конвертирует из прописные  кириллицы в строчную
char* EngRusMakeLower(char* word)
{
	if (word == 0) return 0;
	size_t len = strlen(word);
	for (size_t i = 0; i < len; i++)
		if (is_russian_upper((BYTE)word[i]))
			word[i] = rtolower((BYTE)word[i]);
		else
			word[i] = etolower((BYTE)word[i]);

	return word;
}

template <class T, class Pred, class Conv>
T& RegisterConverter(T& word, size_t Len, Pred P, Conv C)
{
	for (size_t i = 0; i < Len; i++)
		if (P((BYTE)word[i]))
			word[i] = C((BYTE)word[i]);

	return word;
}


template <class T>
T& GerEngRusMakeUpperTemplate(T& word, MorphLanguageEnum Langua, size_t Len)
{
	if (Len == 0) return word;

	if (Langua == morphGerman)
		return RegisterConverter(word, Len, is_german_lower, gtoupper);
	else
		for (size_t i = 0; i < Len; i++)
			if (is_russian_lower((BYTE)word[i]))
				word[i] = rtoupper((BYTE)word[i]);
			else
				if (is_english_lower((BYTE)word[i]))
					word[i] = etoupper((BYTE)word[i]);

	return word;
};


std::string& RmlMakeUpper(std::string& word, MorphLanguageEnum langua)
{
	GerEngRusMakeUpperTemplate(word, langua, word.length());
	return word;
};


template<class T>
void ConvertJO2JeTemplate(T& src, size_t Length)
{
	for (size_t i = 0; i < Length; i++)
	{
		if (((BYTE)src[i]) == LowerJO)
			src[i] = RussianLowerE;
		else
			if (((BYTE)src[i]) == UpperJO)
				src[i] = RussianUpperE;
	}

};

void ConvertJO2Je(char* src, size_t Length)
{
	ConvertJO2JeTemplate(src, Length);
};


void ConvertJO2Je(std::string& src)
{

	ConvertJO2JeTemplate(src, src.length());
};

std::string convert_from_utf8(const char* utf8str, const MorphLanguageEnum langua) {
	if (langua == morphRussian || langua == morphFioDisclosures) {
		return convert_utf8_to_cp1251(utf8str);
	}
	return convert_utf8_to_cp1252(utf8str);
}


std::string convert_to_utf8(const std::string& str, const MorphLanguageEnum langua) {
	if (langua == morphRussian || langua == morphFioDisclosures) {
		return convert_cp1251_to_utf8(str);
	}
	return convert_cp1252_to_utf8(str);
}

BYTE convert_html_entity_to_char(const std::string& entity) {
	if (entity == "amp") { return (BYTE)'&'; }
	else if (entity == "lt") { return (BYTE)'<'; }
	else if (entity == "gt") { return (BYTE)'>'; }
	else if (entity == "nbsp") { return (BYTE)' '; }
	else if (entity == "ouml") { return ouml; }
	else if (entity == "auml") { return auml; }
	else if (entity == "uuml") { return uuml; }
	else if (entity == "Ouml") { return Ouml; }
	else if (entity == "Auml") { return Auml; }
	else if (entity == "Uuml") { return Uuml; }
	else if (entity == "szlig") { return szlig; }
	else if (entity == "agrave") { return agrave; }
	else if (entity == "egrave") { return egrave; }
	else if (entity == "eacute") { return eacute; }
	else return 0;
}

