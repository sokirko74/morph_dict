// ==========  This file is under  LGPL, the GNU Lesser General Public License
// ==========  Dialing Lemmatizer (www.aot.ru)
// ==========  Copyright by Alexey Sokirko, Andrey Putrin

#include "Lemmatizers.h"
#include "Paradigm.h"
#include "morph_dict/morph_wizard/wizard.h"


CFormInfo::CFormInfo()
{
	m_pParent = NULL;
	m_bFound = true;
	m_InnerAnnot.m_LemmaInfoNo = -1;
    
}

void	CFormInfo::Copy(const CFormInfo& from)
{
	m_InnerAnnot = from.m_InnerAnnot;
	m_pParent = from.m_pParent;
	m_InputWordBase = from.m_InputWordBase;
	m_bFound = from.m_bFound;
	m_bFlexiaWasCut = from.m_bFlexiaWasCut;
	m_bPrefixesWereCut = from.m_bPrefixesWereCut;
    m_UserPrefix = from.m_UserPrefix;
    
};


size_t  CFormInfo::GetLemmaPrefixLength() const 
{
	assert (IsValid());
	if (!IsValid()) return 0;
	if (!m_bFound) return 0;
	return m_pParent->m_Prefixes[m_InnerAnnot.m_PrefixNo].length();
};

void	CFormInfo::Create(const CLemmatizer*	pParent, const CAutomAnnotationInner& A, const std::string& InputWordForm, bool bFound)
{
	m_InnerAnnot = A;
	m_pParent = pParent;
	m_bFound = bFound;
	m_InputWordBase = InputWordForm;
	const CMorphForm& M = GetFlexiaModel().m_Flexia[A.m_ItemNo];
	size_t FlexLength = M.m_FlexiaStr.length();

	//  It can be so( if CLemmatizer::PredictByDataBase was used) that 
	//  the flexion  is not suffix of m_InputWordBase, but only part of  it.
	//  If so, then we cannot generate paradigm, since the current form cannot be 
	//  divided into two parts: the base and a known flexion.

	if (	m_bFound
		||	(		(	m_InputWordBase.length()>= FlexLength)
				&&	(m_InputWordBase.substr(m_InputWordBase.length()-FlexLength) == M.m_FlexiaStr)
			)
		)
	{
		m_bFlexiaWasCut = true;
		m_InputWordBase.erase(m_InputWordBase.length() - M.m_FlexiaStr.length());
	}
	else
		m_bFlexiaWasCut = false;

		

	const std::string& LemmPrefix = m_pParent->m_Prefixes[m_InnerAnnot.m_PrefixNo];
	if	(		m_bFound
			||	(		(m_InputWordBase.substr(0, LemmPrefix.length()) == LemmPrefix)
					&&	(m_InputWordBase.substr(LemmPrefix.length(), M.m_PrefixStr.length()) == M.m_PrefixStr)
				)
		)
	{
		m_InputWordBase.erase(0, LemmPrefix.length()+ M.m_PrefixStr.length());
		m_bPrefixesWereCut = true;
	}
	else
		m_bPrefixesWereCut = false;

	
};


const CLemmaInfoAndLemma& CFormInfo::GetLemmaInfo() const
{
	return m_pParent->m_LemmaInfos[m_InnerAnnot.m_LemmaInfoNo];
};

const CFlexiaModel& CFormInfo::GetFlexiaModel() const
{
	const CLemmaInfoAndLemma I = GetLemmaInfo();
	return m_pParent->m_FlexiaModels[I.m_LemmaInfo.m_FlexiaModelNo];
};

uint32_t CFormInfo::GetParadigmId() const 
{
	assert (IsValid());
	if (!IsValid()) return UnknownParadigmId;

	if (!m_bFound)
		return UnknownParadigmId;
	else
		return m_InnerAnnot.GetParadigmId();
};


std::string CFormInfo::GetCommonAncode() const 
{
	assert (IsValid());
	if (!IsValid()) return "??";
    std::string c = GetLemmaInfo().m_LemmaInfo.GetCommonAncodeCopy();
    if (!c.empty()) 
        return c;
    else
	    return "??";
};

char CFormInfo::GetLemSign() const 
{
	return m_bFound ? '+' : '-';
};



bool  CFormInfo::SetParadigmId(uint32_t newVal) 
{
	assert (m_pParent);
	{
		CAutomAnnotationInner A;
		A.SplitParadigmId(newVal);
		if (A.m_LemmaInfoNo > m_pParent->m_LemmaInfos.size()) 
			return false;
		if (A.m_PrefixNo > m_pParent->m_Prefixes.size()) 
			return false;
		A.m_ItemNo = 0;
		A.m_nWeight = m_pParent->GetStatistic().get_HomoWeight(A.GetParadigmId(), 0);
		A.m_ModelNo = m_pParent->m_LemmaInfos[A.m_LemmaInfoNo].m_LemmaInfo.m_FlexiaModelNo;
		m_InnerAnnot = A;
	};

	m_bPrefixesWereCut = true;
	m_bFlexiaWasCut = true;
	m_bFound = true;
	m_InputWordBase = GetSrcNorm();
	m_InputWordBase.erase(m_InputWordBase.length() - GetFlexiaModel().get_first_flex().length());

	return true;
}

uint32_t  CFormInfo::GetCount () const 
{
	assert (IsValid());
	if (!IsValid()) return 0;
	return (uint32_t)GetFlexiaModel().m_Flexia.size();
}


std::string CFormInfo::GetSrcNorm() const 
{
	assert (IsValid());
	if (!IsValid()) return "";

	std::string result = m_pParent->m_Bases[GetLemmaInfo().m_LemmaStrNo].GetString();
	result += GetFlexiaModel().get_first_flex();
	return result;
}

std::string CFormInfo::GetSrcNormUtf8() const {
	return convert_to_utf8(GetSrcNorm(), m_pParent->GetLanguage());
}

int CFormInfo::GetHomonymWeightWithForm(uint16_t pos) const 
{
	assert (IsValid());
	if (!IsValid()) return 0;
	return  m_pParent->GetStatistic().get_HomoWeight(GetParadigmId(), pos);
}

int  CFormInfo::GetWordWeight() const
{
	assert (IsValid());
	if (!IsValid()) return 0;

	return m_pParent->GetStatistic().get_WordWeight(GetParadigmId());
}

void	CFormInfo::AttachLemmatizer(const CLemmatizer* parent)
{
	m_pParent = parent;
}

bool  CFormInfo::IsValid() const
{
	return		m_pParent 
			&&	(m_InnerAnnot.m_LemmaInfoNo != -1);
};

std::string  CFormInfo::GetSrcAncode() const
{
	assert (IsValid());
	if (!IsValid()) return "";

	const CLemmaInfoAndLemma& I = GetLemmaInfo();

	return GetFlexiaModel().m_Flexia[m_InnerAnnot.m_ItemNo].m_Gramcode;
}

std::string CFormInfo::GetAncode(uint16_t pos) const 
{
	assert (IsValid());
	if (!IsValid()) return "";

	return GetFlexiaModel().m_Flexia[pos].m_Gramcode.substr(0,2);
}



std::string CFormInfo::GetWordForm (uint16_t pos) const 
{
	assert (IsValid());
	if (!IsValid()) return "";

	const CMorphForm& F = GetFlexiaModel().m_Flexia[pos];
	
	std::string Result;
    
    if (!m_UserPrefix.empty())
        Result += m_UserPrefix;

  	if (m_bPrefixesWereCut)
		Result += m_pParent->m_Prefixes[m_InnerAnnot.m_PrefixNo]+F.m_PrefixStr;

    Result += m_InputWordBase;

	if (m_bFlexiaWasCut)
		Result += F.m_FlexiaStr;

    // "m_pParent->m_Prefixes[m_InnerAnnot.m_PrefixNo]" is a prefix thta was ascribed to the whole paradigm
    // "F.m_PrefixStr" is a prefix that is ascribed to the current word form

	return Result;
}

std::string CFormInfo::GetWordFormUtf8(uint16_t pos) const {
	return convert_to_utf8(GetWordForm(pos), m_pParent->GetLanguage());
}


int  CFormInfo::GetHomonymWeight() const
{	
	assert (IsValid());
	if (!IsValid()) return 0;
    if (!m_bFound) return 0;
	

	return m_InnerAnnot.m_nWeight;
}

// method GetAccentedVowel is slow, but it is not used frequently in workflow
BYTE	CFormInfo::GetAccentedVowel(uint16_t pos) const
{
	if (!m_bFound) return UnknownAccent;
	assert (IsValid());
	if (!IsValid()) return UnknownAccent;
	

	const CLemmaInfoAndLemma& I = GetLemmaInfo();

	if (I.m_LemmaInfo.m_AccentModelNo == UnknownAccentModelNo)
		return UnknownAccent;

	BYTE BackVowelNo = m_pParent->m_AccentModels[I.m_LemmaInfo.m_AccentModelNo].m_Accents[pos];
	std::wstring s = utf8_to_wstring(GetWordFormUtf8(pos));
	return MapReverseVowelNoToCharNo(s, BackVowelNo);
};

BYTE	CFormInfo::GetSrcAccentedVowel() const
{
	return GetAccentedVowel(m_InnerAnnot.m_ItemNo);
};

void CFormInfo::SetUserPrefix(const std::string& UserPrefix)
{
    m_UserPrefix = UserPrefix;
}

void CFormInfo::SetUserUnknown()
{
    m_bFound = false;
}

std::string CFormInfo::FormatAsInPlmLine() const
{
    std::string Result;
    Result += GetLemSign() + GetCommonAncode() + " ";
    Result += GetWordForm(0) +  " ";
    Result += GetSrcAncode();

    if (m_bFound ) 
        Result += Format(" %lu %i", GetParadigmId(), GetHomonymWeight());
	else
		Result += " -1 0";
    return convert_to_utf8(Result, m_pParent->m_Language);
};

std::string CFormInfo::GetGramInfoStr(const CAgramtab* gramtab) const
{
	std::string s;
	s += " " + gramtab->GetTabStringByGramCode(GetCommonAncode().c_str());
	const std::string& ancode = (m_bFound) ? GetAncode(0) : GetSrcAncode();
	for (int i = 0; i < ancode.length(); i += 2)
		s += " " + gramtab->GetTabStringByGramCode(ancode.c_str() + i);
	return s;
}
