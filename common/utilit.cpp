#include "utilit.h"
#include "util_classes.h"
#include "bserialize.h"
#include <time.h>
#include <errno.h>
#include <limits>
#include <codecvt>
#include <locale>
#include <filesystem>
#include <iostream>
#include <sstream>


#include <plog/Initializers/RollingFileInitializer.h>
#include <plog/Initializers/ConsoleInitializer.h>



// for MessageBox
#ifdef WIN32
	#define NOMINMAX 
	#include "windows.h"
#endif

bool FileExists (const char *FName)
{
	return (access(FName, 0) == 0);
}

file_off_t FileSize (const char *filename)
{
    std::ifstream ifile(filename);
    ifile.seekg(0, std::ios_base::end);//seek to end
    file_off_t size = ifile.tellg();
    ifile.close();
	return size;
}


void(*GlobalErrorMessage)(const std::string&) = 0;

void ErrorMessage (const std::string& Titul, const std::string& Message)
{
	if (GlobalErrorMessage)
	{
		std::string q = Titul + ":"+Message;
		if (q.empty()|| (q[q.length() -1]!='\n'))	q+='\n';
		GlobalErrorMessage(q);
		return;
	};
	
	#ifdef WIN32
		#ifndef _CONSOLE
			MessageBox(0, Message.c_str(), Titul.c_str(), MB_OK);
			return;
		#endif
	#endif

	std::string q = Message;
	if (q.empty() || (q.back() != '\n')) q+='\n';
	fprintf(stderr, "%s: %s", Titul.c_str(), q.c_str());
	
};

void ErrorMessage (const std::string& Message)
{
	ErrorMessage("error", Message);
};

std::string MakeFName ( const std::string& InpitFileName,  const std::string& Ext)
{
	std::string Result = InpitFileName;
	size_t i = Result.find_last_of('.'); // найти последнее расширение
	if ( i  != std::string::npos) 
		Result.erase(i);	
	Result += std::string(".") ;
	Result += Ext;
	return Result;
}


std::string MakePath (const std::string path, const std::string fileName) { 
	std::string result = path;
	
	if (!result.empty()) {
#ifdef WIN32
		if ((result.back() != '\\')
			&& (result.back() != '/')
			)
			result += "\\";
#else

		if (result.back() != '/')
			result += "/";

#endif
	}

	return result + fileName;	  
};


std::string GetParentPath (std::string FileName)
{ 
  size_t i = FileName.rfind("\\");	  
  size_t j = FileName.rfind("/");	  

  if (			(i == std::string::npos) 
		&&		(j == std::string::npos)
	  )
	  return "";
  else
	  if (i == std::string::npos) 
		  return FileName.substr(0, j+1);
	  else
		if (j == std::string::npos) 
			return FileName.substr(0, i+1);
		else
			return FileName.substr(0, std::max(i,j)+1); 
};




char* rtrim (char* s)
{
	size_t len = strlen(s);

	while ( len > 0 && isspace((BYTE)s[len-1]))
		s[--len] = 0;

	return s;

}


char* IntToStr (int Value, char* Buffer)
{
	sprintf (Buffer, "%i", Value);
	return Buffer;
};

std::string& IntToStr (int Value, std::string& oBuffer)
{
	char Buffer[256];
	sprintf (Buffer, "%i", Value);
	oBuffer = Buffer;
	return oBuffer;
};


std::string GetRmlVariable()
{
	const char* rml = getenv("RML");
	std::string s;
	if (rml)
		s = rml;
	else
	{
		throw CExpc("Error! Environment variable \"RML\"is not std::set!");
	}
	 
	Trim(s);
	for (size_t  i=0; i<s.length(); i++)
		if (s[i] == '\\')
			s[i] = '/';
	if (!s.empty() &&  (s[0] == '\"'))
		s.erase(0, 1);
	if (!s.empty() && (s[s.length() - 1] == '\"'))
		s.erase(s.length() - 1);
	if (!s.empty() &&  (s[s.length() - 1] == '/'))
		s.erase(s.length() - 1);
	if (s.empty())
		throw CExpc ("Environment variable \"RML\" is not properly initialized!");
	return s;
}



const char RML_REGISTRY_FILENAME[] = "rml.ini";


static std::string GetStringInnerFromTheFile (std::string RegistryPath, std::string MainPath, std::string RmlPath) 
{
	std::string FileName = 	MainPath +  "/" + RML_REGISTRY_FILENAME;
	if (access  (FileName.c_str(),04) != 0)
	{
		//fprintf (stderr, "Cannot read %s\n", FileName.c_str());
		throw CExpc ("Cannot read "+FileName);		
	};
	FILE* fp;
	try 
	{
		fp = fopen (FileName.c_str(), "r");
		if (fp == 0)
			throw CExpc ("Cannot open "+FileName);
	}
	catch (...)
	{
		throw CExpc ("Cannot open "+FileName);
	}
	char buffer[2048];
	while (fgets(buffer,2048,fp))
	{
		std::string s = buffer;
		Trim(s);
		if (s.empty()) continue;
		size_t end_field_name = strcspn(s.c_str(), " \t");
		if (end_field_name == s.length())
		{
			fclose(fp);
			throw CExpc ("Cannot parse line "+s);
		};
		std::string Key = s.substr(0,end_field_name);
		if (RegistryPath == Key)
		{
			std::string Value = s.substr(end_field_name);
			Trim(Value);
			if (Value.substr(0, 4) == "$RML")
				Value.replace(0,4, RmlPath);

			fclose(fp);
			return Value;
		};
	};
	fclose (fp);

	return "";
};

std::string GetIniFilePath()
{
	return GetRmlVariable()+"/Bin";
};

std::string GetRegistryString (std::string RegistryPath)
{
	std::string Result = GetStringInnerFromTheFile(RegistryPath, GetIniFilePath(), GetRmlVariable().c_str());
	return Result;
	
};

bool CanGetRegistryString (std::string RegistryPath)  
{
	try {
		std::string s = GetRegistryString(RegistryPath);
		return  s != "";
	}
	catch (...)
	{
		return false;
	};
}


bool	IsRmlRegistered(std::string& Error)
{
	try 
	{
		std::string IniFile = GetIniFilePath() +  "/" + RML_REGISTRY_FILENAME;
	
		if (!FileExists(IniFile.c_str() ))
		{
			Error = "ini file not found: " + IniFile;
			return false;
		};
		if (access  (IniFile.c_str(),04) != 0)
		{
			Error = "ini file exists but cannot be read: " + IniFile;
			return false;
		};

	}
	catch (CExpc c)
	{
			Error = c.what();
			return false;
	}
	catch (...)
	{
		return false;
	}
	return true;
};

struct tm  RmlGetCurrentTime ()
{
	time_t ltime;
	time( &ltime );
	#ifdef	WIN32
		struct tm *today;
		today = localtime( &ltime );
		return *today;
	#else
		//  we should  thread safe  variants
		struct tm today;
		localtime_r( &ltime, &today );
		return today;
	#endif
};




bool GetLanguageByString (std::string s, MorphLanguageEnum& Result)
{
	MakeUpperUtf8(s);
	if(s == "RUSSIAN"){
		Result = morphRussian;
		return true;
	}else if(s == "ENGLISH"){
		Result = morphEnglish;
		return true;
	}
	else if(s == "GERMAN"){
		Result = morphGerman;
		return true;
	}
	else if(s == "GENERIC"){
		Result = morphGeneric;
		return true;
	}
	else if (s == "FIO_DISCLOSURES") {
		Result = morphFioDisclosures;
		return true;
	}
	else
		return false;
	
};

std::string GetStringByLanguage (MorphLanguageEnum Langua)
{
	switch (Langua) {
		case morphRussian: return "Russian";
		case morphEnglish: return "English";
		case morphGerman: return  "German";
		case morphGeneric: return  "Generic";
		case morphFioDisclosures: return  "FIO_DISCLOSURES";
		default :return "unk";
	}
};







std::string Format( const char* format, ... )
{
	va_list arglst;
	const size_t  SmallBufferSize = 15000;
    char SmallBuffer[SmallBufferSize+1];

    va_start( arglst, format );
	int	OutputLength = vsnprintf( SmallBuffer, SmallBufferSize, format, arglst);
	va_end( arglst );

	if (		(OutputLength == -1)
			||	(OutputLength < SmallBufferSize)
		)
		return SmallBuffer;

	if (OutputLength > 10000000) 
	{
		assert (false);
		OutputLength = 10000000;
	};


	char * pBuffer = new char[OutputLength+2];
	if (!pBuffer)
		return SmallBuffer;
	va_start( arglst, format );
	vsnprintf( pBuffer, OutputLength+1, format, arglst);
	va_end( arglst );
	std::string Res = pBuffer;
	delete pBuffer;
	return Res;

};

std::string&  TrimLeft (std::string& str)
{
	if (str.size() == 0) return str;
	size_t i = str.find_first_not_of(" \t\n\r");
	str.erase (0, i);
	return str;
};

std::string&  TrimRight (std::string& str)
{
	if (str.size() == 0) return str;
	size_t i = str.find_last_not_of(" \t\n\r");
	str.erase (i+1);
	return str;
};

std::string& Trim (std::string& str)
{
	TrimLeft(str);
	TrimRight(str);
	return str;
};



const std::string open_brackets = "{[(<";
const std::string close_brackets = "}])>";
const std::string all_brackets = open_brackets+close_brackets;


int isbracket  (BYTE x)
{
	if (open_brackets.find(x) != std::string::npos)	return 1; //открывающие скобки
	if (close_brackets.find(x) != std::string::npos)	return 2; // закрывающие
	return 0; 
}

// выдает соответствующую скобку 
size_t dual_bracket (BYTE x)
{
	size_t i = all_brackets.find(x);
	if (i == std::string::npos) return x;
	size_t s = open_brackets.length();
	if (i >= s)
		return all_brackets[i-s];
	else
		return all_brackets[i+s];
}



// ============     CShortString and  CShortStringHolder =========================

CShortString::CShortString(std::vector<char>::const_iterator pData)
{
	m_pStringPointer = pData;
};
BYTE CShortString::GetLength() const
{	
	return	(BYTE)m_pStringPointer[0];	
}

std::vector<char>::const_iterator	CShortString::GetData() const
{	
	return	m_pStringPointer;	
}

const char*	CShortString::GetString() const
{	
	return	&(m_pStringPointer[1]);	
}


bool  IsLessShortString::operator ()(const CShortString& Item1,	const char* Item2) const
{
	return strcmp(Item1.GetString(), Item2) < 0;
}

bool  IsLessShortString::operator ()(const char* Item1, const CShortString& Item2) const
{
	return strcmp(Item1, Item2.GetString() ) < 0;
}

bool  IsLessShortString::operator ()(const CShortString& Item1,	const CShortString& Item2) const
{
	return strcmp(Item1.GetString(), Item2.GetString()) < 0;
}

void CShortStringHolder::ReadShortStringHolder(std::string filename)
{
	clear();
	uint32_t count;
	size_t BufferSize = (size_t)FileSize(filename.c_str()) - sizeof(count);
	std::ifstream inp(filename, std::ios::binary);
	if (!inp.is_open()) {
		throw CExpc(Format("cannot read %s", filename.c_str()));
	}
	inp.read((char*)&count, sizeof(count));
	if (!inp) {
		inp.close();
		throw CExpc(Format("cannot read count from %s", filename.c_str()));
	}
	m_Buffer.resize(BufferSize);
	inp.read(&m_Buffer[0], BufferSize);
	if (!inp) {
		inp.close();
		throw CExpc(Format("cannot read bytes from %s", filename.c_str()));
	}
	inp.close();


	reserve(count);
	int Offset = 0;
	for (uint32_t i=0; i < count; i++)
	{
		CShortString R(m_Buffer.begin()+Offset);
		push_back(R);
		Offset +=   R.GetLength() + 2; // 1 byte for length and 1 byte for 0 in the end
	};
};

template<class T>
bool CShortStringHolder::CreateFromSequence(T begin, T end)
{
	m_Buffer.clear();
	uint32_t Count = 0;
	for (; begin != end; begin++)
	{
		size_t length = begin->length();
		if (length > 254)
		{
			std::string s = *begin + " - too long";
			ErrorMessage(s.c_str(), "Short std::string convertor");
			return false;
		};

		m_Buffer.push_back((BYTE)length);
		// add with terminating null 
		m_Buffer.insert(m_Buffer.end(), begin->c_str(), begin->c_str() + length+1);

		Count++;
	}

	
	size_t Offset = 0;
	clear();
	for (uint32_t i=0; i < Count; i++)
	{
		CShortString R(m_Buffer.begin()+Offset);
		push_back(R);
		Offset +=   R.GetLength() + 2;
	};

	return true;
}


bool CShortStringHolder::CreateFromVector(const StringVector& in)
{
	return CreateFromSequence<StringVector::const_iterator>(in.begin(), in.end());
}

bool CShortStringHolder::CreateFromSet(const StringSet& in)
{
	return CreateFromSequence<StringSet::const_iterator>(in.begin(), in.end());
}

void CShortStringHolder::WriteShortStringHolder(const std::string& FileName) const
{
	std::ofstream outp(FileName.c_str(), std::ios::binary);
	if (!outp.is_open()) {
		throw CExpc(Format("cannot write to %s", FileName.c_str()));
	};
    assert (size() < std::numeric_limits<uint32_t>::max());
    uint32_t nLength = size();
	outp.write((char*)&nLength, sizeof(nLength));
	outp.write((char*)&m_Buffer[0], m_Buffer.size());
	outp.close();
}


// ============     CMyTimeSpan and  CMyTimeSpanHolder =========================


CMyTimeSpan::CMyTimeSpan (long SequenceId, long InterfaceNestId)
{
	m_TimeSpan = 0;
	m_InterfaceNestId = InterfaceNestId;
	m_SequenceId = SequenceId;
	m_InvokeCount = 0;
};

void CMyTimeSpan::GetStrRepresentation(const char* Name, char* buffer, double AllClocksCount) const
{
    buffer[0] = 0;
	for (long i=0; i<m_InterfaceNestId; i++)
		strcat(buffer,"\t");
	char t[300];
	sprintf (t, "%s  = %6.0f seconds;%6.0f ticks ;%lu calls", 
		Name, 
		m_TimeSpan/(double)CLOCKS_PER_SEC, 

		m_TimeSpan, 
		m_InvokeCount
		);	
	strcat (buffer, t);
	if (AllClocksCount > 0)
	{
		char q[50];
		double dd = (AllClocksCount== 0)? 0 : (m_TimeSpan/AllClocksCount)*100;
		sprintf (q, "; %6.0f %%", dd);
		strcat (buffer, q);

	};

};



CMyTimeSpanHolder::CMyTimeSpanHolder()
{
	m_SequenceId = 0;
	m_bTimeSpanHolderEnabled = true;
};

void CMyTimeSpanHolder::StartTimer(const std::string& Name, long NestId)
{
	if (!m_bTimeSpanHolderEnabled)  return ;
	Iterator it = m_TimeSpans.find(Name);

	if(it == m_TimeSpans.end())
	{
		CMyTimeSpan X (m_SequenceId, NestId);

		std::pair<Iterator, bool> p = m_TimeSpans.insert(std::pair<std::string, CMyTimeSpan>(Name, X));

		it = p.first;
	};

	it->second.m_StartTime = clock();
	it->second.m_InvokeCount++;

	m_SequenceId++;
};

//  finish timer "Name" and returns the elapsed time in ticks
double CMyTimeSpanHolder::EndTimer(const std::string& Name)
{
	if (!m_bTimeSpanHolderEnabled)  return 0;

	Iterator it = m_TimeSpans.find(Name);

	if(it == m_TimeSpans.end()) 
	{
		ErrorMessage (Format("Profiler error: no timer with name %s", Name.c_str()) );
		return 0;
	};

	it->second.m_TimeSpan += (double)(clock() - it->second.m_StartTime) ;

	
	return it->second.m_TimeSpan;
};


std::string CMyTimeSpanHolder::GetStrRepresentation (double AllClocksCount) const
{
	std::string Result;

	if (!m_bTimeSpanHolderEnabled)  return "profiler is disabled\n";

	
	std::map<int, ConstIterator>			TimerSequence;

	for (ConstIterator It = m_TimeSpans.begin(); It != m_TimeSpans.end();It++)
		TimerSequence[It->second.m_SequenceId] = It;

	for (std::map<int, ConstIterator>::const_iterator It1 = TimerSequence.begin(); It1 != TimerSequence.end();It1++)
	{
		char s[1000];
		ConstIterator It = It1->second;
		It->second.GetStrRepresentation(It->first.c_str(), s, AllClocksCount);
		Result += s;
		Result += '\n';
	};
	return Result;
};


void CMyTimeSpanHolder::ClearTimers() 
{
	m_TimeSpans.clear();
	m_SequenceId = 0;
};


std::string LoadFileToString(std::string path, bool convert_zero_to_one)
{
	std::ifstream t(path);
	if (!t.good()) {
		throw CExpc("cannot read file %s", path.c_str());
	}
	std::stringstream buffer;
	buffer << t.rdbuf();
	std::string s = buffer.str();
	if (convert_zero_to_one) {
		std::replace(s.begin(), s.end(), '\x0', '\x1');
	}
	return s;
};

bool is_pseudo_graph(BYTE x)
{
	return  (x==134) 
		  ||(x==135) 
		  ||(x==128) 
		  ||(x==131) 
		  ||(x==136) 
		  ||(x==137) 
		  ||((x>=139) && (x<=175))
		  ||((x>=176) && (x<=181))
		  ||((x>=186) && (x<=191));
};


// ======================== CExcp ================
CExpc::CExpc(const std::string& Cause) 
{
    m_strCause = Cause;
};

void CExpc::add_to_message(std::string m) {
	m_strCause += m;

}


CExpc::CExpc(const char* format, ... )
{
   	va_list arglst;
	const size_t  SmallBufferSize = 512;
    char SmallBuffer[SmallBufferSize+1];

    va_start( arglst, format );
	vsnprintf( SmallBuffer, SmallBufferSize, format, arglst);
	va_end( arglst );
    m_strCause = SmallBuffer;
}

char const* CExpc::what() const noexcept {
	return m_strCause.c_str();
}



// ====================   StringTokenizer =======================

void StringTokenizer::initialize(const char *_text, const char *_delims)
{
	delims = _delims;
	i = -1;
	size_t leng = strlen(_text);
	text_ptr = new char[leng+2];
	text = text_ptr;
	strcpy(text, _text);
	text[leng+1] = 0;
};

StringTokenizer::StringTokenizer(const char *_text, const char *_delims)
{
	initialize(_text, _delims);
}

StringTokenizer::~StringTokenizer(){
	delete []text_ptr;
}

const char * StringTokenizer::get_rest () const 
{ 
	return text;	
};
const char * StringTokenizer::val() const  
{
	return _val;
}
int  StringTokenizer::count() const  
{
	return i;
}
bool StringTokenizer::has_next() const 
{
	return (*text) != 0;
}
bool StringTokenizer::is_delim(char ch) const
{
		return delims.find_first_of(ch) != std::string::npos;
}


void StringTokenizer::reinitialize(const char *_text, const char *_delims)
{
	assert (text_ptr);
	delete []text_ptr;
	initialize(_text, _delims);
}

const char * StringTokenizer::operator ()()
{
	++i;
	// skip delims
	while(*text && is_delim(*text))
		text++;

	if(*text == 0)	return 0;

	char *ret_val = text;

	while(*text && !is_delim(*text))
		text++;
	*text++ = 0;
	_val = ret_val;
	if (_val == 0)
		throw CExpc ("Exception in StringTokenizer::operator ()");
	return ret_val;
}
std::string StringTokenizer::next_token ()
{
	const char* s = operator()();
	if (s) 
		return s;
	else 
		return std::string();
}




std::string ConvertASCIIToHtmlSymbols(const std::string& txt)
{
	std::string out;
	char asciiCode[2];
	asciiCode[1] = 0;
	for( int i=0; i<txt.length(); i++ )
	{
		asciiCode[0] = txt[i];
		switch (asciiCode[0])
		{
			case ' ':
				out += " ";
			break;
			case '\t':
				out += "    ";
			break;
			case '"':
				out += "&quot;";
			break;
			case '&':
				out += "&amp;";
			break;
			case '<':
				out += "&lt;";
			break;
			case '>':
				out += "&gt;";
			break;
			default:
				out += asciiCode;
		}
	}
	return out;
}



size_t HashValue(const char *pc) 
{
    static const uint32_t mask[sizeof(uint32_t)] = {
        0x00000000, 0x000000FF, 0x0000FFFF, 0x00FFFFFF
    };
    size_t len =     strlen(pc);
    size_t h = len;
    size_t i = len / sizeof(uint32_t);
    const uint32_t *c = (const uint32_t *)pc;

    while (i--)
        h ^= *c++;

    i = len % sizeof(uint32_t);
    if (i > 0)
        h ^= (*c & mask[i]); 

    return h;
}


size_t FindFloatingPoint(const std::string& s)
{
	if (s.length() < 3) return -1;
	size_t c = s.rfind(",");
	if (c == std::string::npos) 
		c = s.rfind(".");
	return c == std::string::npos ? -1 : c;
}




//#if __cplusplus > 201402L

// "c:/*.txt" -> c:/a.txt, c:/b.txt
std::vector<std::string> list_path_by_file_mask(std::string filemask)
{
	auto path = std::filesystem::path(filemask);
	std::vector<std::string> filenames;
	for (const auto& p : std::filesystem::directory_iterator(path.parent_path())) 	{
		if (p.path().extension() == path.extension()) {
			filenames.push_back(p.path().string());
		}
	}
	return filenames;
};

std::string join_string(const std::vector<std::string>& items, const std::string& delimiter) {
	std::string result;
	if (items.empty()) {
		return result;
	}
	result = items[0];
	for (size_t i = 1; i < items.size(); ++i) {
		result += delimiter + items[i];
	}
	return result;
}

std::vector<std::string> split_string(const std::string& s, char delim) {
	std::stringstream ss(s);
	std::string item;
	std::vector<std::string> elems;
	while (std::getline(ss, item, delim)) {
		if (item.size() > 0) {
			elems.push_back(item);
		}
	}
	return elems;
}

static MorphLanguageEnum log_language = morphUnknown;

class MyFormatter
{
public:
	static plog::util::nstring header()
	{
		return plog::util::nstring();
	}

	static plog::util::nstring format(const plog::Record& record)
	{
		plog::util::nostringstream ss;
		ss << std::setfill(PLOG_NSTR(' ')) << std::setw(5) << std::left << severityToString(record.getSeverity()) << PLOG_NSTR(" ");
		ss << PLOG_NSTR("[") << record.getFunc() << PLOG_NSTR("@") << record.getLine() << PLOG_NSTR("] ");
        if (log_language != morphUnknown) {
            ss << convert_to_utf8(record.getMessage(), log_language) << PLOG_NSTR("\n");
        }
        else {
            ss << record.getMessage() << PLOG_NSTR("\n");
        }
		return ss.str();
	}
};


static plog::ConsoleAppender<MyFormatter> consoleAppender; 

void init_plog(plog::Severity severity, std::string filename, bool overwrite, MorphLanguageEnum langua) {
	if (overwrite) {
		if (std::filesystem::exists(filename)) {
			std::filesystem::remove(filename);
		}
	}
    log_language = langua;
	plog::init<MyFormatter>(severity, filename.c_str()).addAppender(&consoleAppender);
}

bool startswith(const std::string& main, const std::string& prefix) {
	return main.rfind(prefix, 0) == 0;
}

bool endswith(const std::string& main, const std::string& suffix) {
	if (main.length() >= suffix.length()) {
		return (0 == main.compare(main.length() - suffix.length(), suffix.length(), suffix));
	}
	else {
		return false;
	}
}

uint64_t GetMaxQWORD()
{
#ifdef WIN32
	return  0xffffffffffffffff;
#else
	return std::numeric_limits<uint64_t>::max();
#endif
};

