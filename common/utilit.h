// ==========  This file is under  LGPL, the GNU Lesser General Public License
// ==========  Dialing Syntax Analysis (www.aot.ru)
// ==========  Copyright by Alexey Sokirko
#pragma once
#include  <stdio.h>


#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from windows headers

#include  "single_byte_encoding.h"


#include <assert.h>
#include <stdarg.h>
#include <ctype.h>
#include <string>
#include <algorithm>
#include <stdexcept>
#include <vector>
#include <iostream>
#include <set>
#include <map>
#include <string.h>
#include <unordered_set>
#include <limits>
#include <plog/Log.h>
#include <filesystem>
#include <cwctype>

namespace fs = std::filesystem;

#pragma warning (disable : 4018)
#pragma warning (disable : 4244)
#pragma warning (disable : 4103)
#pragma warning  (disable : 4530)
#pragma warning  (disable : 4251)
#pragma warning  (disable : 4996)
		

typedef unsigned char BYTE;

#ifdef _MSC_VER 
	#include  <io.h>
	#include <fcntl.h>
#else
	#include  <unistd.h>
	const   unsigned int _MAX_PATH = 512;
#endif

typedef std::vector<std::string> StringVector;
typedef std::unordered_set<std::string> StringHashSet;
typedef std::set<std::string> StringSet;
typedef std::vector<uint32_t> DwordVector;

typedef  uint8_t part_of_speech_t;
const BYTE	UnknownPartOfSpeech = 0xff;
typedef  uint8_t grammem_t;
typedef  uint8_t graph_descr_t;
typedef  uint32_t part_of_speech_mask_t;
typedef  uint64_t grammems_mask_t;

//  uint64_t mask
#define _QM(X) (((uint64_t)1)<<(X))


const int	UnknownSyntaxElement = 0xffff;

// morph_dict&syntax
extern bool GetLanguageByString(std::string s, MorphLanguageEnum& Result);
extern std::string GetStringByLanguage(MorphLanguageEnum Langua);



template <class T1, class T2, class T3>
struct	troika : public std::pair<T1, T2>
{
	T3 third;
};


typedef uint64_t file_off_t;

class CExpc : public std::exception
{
	std::string	m_strCause;
public:
	CExpc(const std::string& cause);
	CExpc(const char* format, ...);
	char const* what() const noexcept override;
	void add_to_message(std::string m);
};
	
extern uint64_t GetMaxQWORD();


// working with files
extern bool			FileExists (const char *FName);
extern file_off_t	FileSize (const char *filename);
extern std::vector<std::string> list_path_by_file_mask(std::string filemask);
extern std::string	CreateTempFileName();
extern std::string LoadFileToString(std::string path, bool convert_zero_to_one=false);
extern std::string	MakeFName(const std::string& InpitFileName, const std::string& Ext);
extern std::string   MakePath(const std::string path, const std::string fileName);
extern std::string	GetParentPath(std::string FileName);


// error  messages
extern void		ErrorMessage (const std::string& Titul, const std::string& Message);
extern void		ErrorMessage (const std::string& Message);

// working with registry  (RML/Bin/rml.ini)
extern std::string	GetRegistryString (std::string RegistryPath);
extern bool		CanGetRegistryString (std::string RegistryPath);
extern bool		IsRmlRegistered(std::string& Error);
extern struct tm  RmlGetCurrentTime ();
extern std::string	GetIniFilePath();
extern std::string	GetRmlVariable();
extern std::string   BuildRMLPath (const char* s);

// working with std::strings (ASCII)
extern char*	rtrim (char* s);
extern char*	IntToStr (int Value, char* Buffer);
extern std::string&	IntToStr (int Value, std::string& oBuffer);
extern std::string	Format( const char* format, ... );
extern std::string&  TrimLeft (std::string& str);
extern std::string&  TrimRight (std::string& str);
extern std::string&	Trim (std::string& str);
extern int isbracket  (BYTE x);
extern size_t dual_bracket (BYTE x);
extern bool is_upper_roman_digit (BYTE ch);
extern bool is_lower_roman_digit (BYTE ch);
extern bool is_roman_number ( const char *s, size_t len);
extern bool is_pseudo_graph(BYTE x);
extern bool is_spc_fill (BYTE x);
extern bool isnspace(BYTE x);
extern size_t FindFloatingPoint(const std::string& s);
extern bool startswith(const std::string& main, const std::string& prefix);
extern bool endswith(const std::string& main, const std::string& suffix);
extern std::string join_string(const std::vector<std::string>& items, const std::string& delimiter);
extern std::vector<std::string> split_string(const std::string& s, char delim);


// multibyte encoding
extern std::wstring utf8_to_wstring(const std::string& str);
extern std::string wstring_to_utf8(const std::wstring& str);
extern std::string& MakeUpperUtf8(std::string& s_utf8);
extern std::string& MakeLowerUtf8(std::string& s_utf8);
extern std::string& MakeTitleUtf8(std::string& s_utf8);
extern size_t CountLettersInUtf8(const std::string& s_utf8);
extern bool CheckRussianUtf8(const std::string& s);
extern bool CheckRussianLowerUtf8(const std::string& s);
extern bool ContainsRussianUtf8(const std::string& s);
extern bool CheckEnglishUtf8(const std::string& s);
extern bool CheckGermanUtf8(const std::string & s);
extern bool IsUnicodeUpperGermanVowel(uint32_t u);
extern bool IsUnicodeUpperEnglishVowel(uint32_t u);
extern bool IsUnicodeUpperRussianVowel(uint32_t u);
extern uint32_t GetFirstUnicodeLetterFromUtf8(const std::string& s);
extern bool IsUpperVowel(uint32_t u);
extern bool FirstLetterIsUpper(const std::string& s);
extern bool IsUnicodeAlpha(uint16_t u);
uint32_t toupper_utf32(uint32_t c);
bool CheckLanguage(const std::string& s, MorphLanguageEnum langua);



// logging
extern void init_plog(plog::Severity severity, std::string filename, bool overwrite = true, MorphLanguageEnum langua = morphUnknown);

// helpers
template<class _II, class _Ty> inline
bool _find(_II It, const _Ty& _V)
{
	return !(find(It.begin(), It.end(), _V) == It.end());
}

template<class T>
size_t get_variants_count(const std::vector<std::vector<T>>& base) {
	size_t i = 1;
	for (auto& v : base) {
		assert(!v.empty());
		i *= v.size();
	}
	return i;
}

template<class T>
bool get_next_variant(const std::vector<std::vector<T>>& base, std::vector<size_t>& cur_variant) {

	if (cur_variant.empty()) {
		cur_variant.resize(base.size(), 0);
		return true;
	}
	assert(base.size() == cur_variant.size());

	for (size_t i = 0; i < cur_variant.size(); ++i) {
		if (cur_variant[i] + 1 < base[i].size()) {
			++cur_variant[i];
			return true;
		}
		cur_variant[i] = 0;
	}
	return false;
}
