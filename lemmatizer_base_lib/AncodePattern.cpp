#include "../agramtab/agramtab.h"
#include "MorphanHolder.h"
#include "AncodePattern.h"


CAncodePattern::CAncodePattern()
{
	SetNotWord();
	m_Language = morphUnknown;
};

void    CAncodePattern::SetLanguage(MorphLanguageEnum l) {
	m_Language = l;
}

const	CAgramtab* CAncodePattern::GetGramTab() const
{
	return GetMHolder(m_Language).m_pGramTab;
}

void CAncodePattern::SetNotWord()
{
	m_SearchStatus = NotWord;
	m_iGrammems = 0;
	m_iPoses = 0;
	m_GramCodes = "";
	m_TypeGrammems = 0;
	m_CommonGramCode = "";
	m_SimplePrepNos.clear();
};

const std::string& CAncodePattern::GetGramCodes() const {
    return m_GramCodes;
}

BYTE CAncodePattern::GetLemSign() const {
	switch (m_SearchStatus) {
		case DictionaryWord: return '+';
		case PredictedWord: return '-';
	}
	return 0;
}

void  CAncodePattern::SetGramCodes(const std::string& s) {
    assert (s.length() % 2 == 0);
    m_GramCodes = s;
}

void  CAncodePattern::SetGramCodes(const char* s) {
    SetGramCodes (std::string(s));
}


void CAncodePattern::ResetFlags()
{
    // do not reset m_GramCodes, m_CommonGramCode and m_LemSign - this is the  source  information
	m_iGrammems = 0;
	m_iPoses = 0;
	m_TypeGrammems = 0;
}

void CAncodePattern::CopyAncodePattern(const CAncodePattern& X)
{
	m_Language = X.m_Language;
	m_iGrammems = X.m_iGrammems;
	m_iPoses = X.m_iPoses;
	m_GramCodes = X.m_GramCodes;
	m_SimplePrepNos = X.m_SimplePrepNos;
	m_SearchStatus = X.m_SearchStatus;
	m_CommonGramCode = X.m_CommonGramCode;
	m_TypeGrammems = X.m_TypeGrammems;
};


bool CAncodePattern::HasGrammem(BYTE gram) const 
{ 
	return		((m_iGrammems    & _QM(gram)) > 0)
			||	((m_TypeGrammems & _QM(gram)) > 0);
}

bool CAncodePattern::HasPos(BYTE pos) const
{ 
	return (m_iPoses & (1 << pos)) > 0; 
};


std::string CAncodePattern::GetGrammemsByAncodes() const
{
	std::string Result;
	if (m_GramCodes == "??") {
		return "";
	}
	for (int i = 0; i < m_GramCodes.length(); i += 2)
	{
		uint64_t g;
        if (!GetGramTab()->GetGrammems(m_GramCodes.c_str()+i, g))
		{
				assert (false);
		};
		Result += GetGramTab()->GrammemsToStr(g);
		Result += "; ";

	};
	return Result;
};


bool CAncodePattern::DeleteAncodesByGrammemIfCan(BYTE Grammem)
{
    std::string GramCodes;
    for (int j=0; j < m_GramCodes.length(); j+=2) 
	    if (m_GramCodes[j] != '?')
	    {
		    uint64_t Grammems;
		    GetGramTab()->GetGrammems ( m_GramCodes.c_str() + j, Grammems);
		    if (   (Grammems & _QM(Grammem)) == 0 )
			      GramCodes += std::string(m_GramCodes.c_str() + j, 2);
	    }
    if (GramCodes.empty())
        return false;
    m_GramCodes = GramCodes;
    InitAncodePattern();
    return true;
}

bool CAncodePattern::ModifyGrammems(uint64_t Grammems, part_of_speech_mask_t Poses)
{
	std::string strOldGramcodes = m_GramCodes;	
	uint64_t  savegrammems = m_iGrammems;	
	part_of_speech_mask_t saveposes = m_iPoses;
	m_iGrammems = 0;
	m_iPoses = 0;
	m_GramCodes = "";
	
    if (strOldGramcodes.empty() || strOldGramcodes[0] == '?') 
    {
        int uu =0;;
    }
    else
	for (size_t j=0; j < strOldGramcodes.length(); j+=2)
	{
			uint64_t CurrGrammems = 0;		
			bool b = GetGramTab()->GetGrammems(strOldGramcodes.c_str() + j, CurrGrammems);
			assert (b);
			if (!b)
			{
                ErrorMessage(Format("Cannot get grammems by gramcode %s ",strOldGramcodes.substr(j,2).c_str()));
				m_iGrammems = 0;
				break;
			};
			BYTE CurrPOS = GetGramTab()->GetPartOfSpeech(strOldGramcodes.c_str() + j);
			
			/*
				there are tow possibilities:
					1. "Grammems" contains only one or two grammems, and all gramcodes of this 
						pattern should contain this "Grammems"
					2. "Grammems" is a union of all possible grammems and all gramcodes should be inside
					   this union.

			*/
			if (Poses & (1<<CurrPOS))
				if(    ((Grammems & CurrGrammems) == CurrGrammems) 
					|| ((Grammems & CurrGrammems) == Grammems)
				)
				{
					m_iGrammems |= CurrGrammems;
					m_GramCodes += strOldGramcodes.substr(j,2);
                    BYTE pos = GetGramTab()->GetPartOfSpeech(strOldGramcodes.c_str() + j);
					m_iPoses |= (1 << pos);

				}
			
	}		
	if ( (m_iGrammems == 0) && (savegrammems != 0))
	{
		m_GramCodes = strOldGramcodes;	
		m_iGrammems = savegrammems;	
		m_iPoses = saveposes;
		return false;
	}
	//assert (!m_GramCodes.empty());
	return true;
};

bool CAncodePattern::HasUnkGramCode() const
{
	return m_GramCodes.empty() || ((unsigned char)m_GramCodes[0] == '?');
}

bool CAncodePattern::InitAncodePattern()
{
    ResetFlags();
    

	if	(		!m_GramCodes.empty()
			&& 	(m_GramCodes[0] != '?') 
		)
	{
		for (size_t j=0; j < m_GramCodes.length(); j+=2)
		{
				uint64_t CurrGrammems = 0;		
				bool b = GetGramTab()->GetGrammems(m_GramCodes.c_str() + j, CurrGrammems);
				assert (b);
				if (!b)
				{
                    ErrorMessage(Format("Cannot get grammems by gramcode %s ",m_GramCodes.substr(j,2).c_str()));
				};
				m_iGrammems |= CurrGrammems;
                BYTE pos = GetGramTab()->GetPartOfSpeech(m_GramCodes.c_str() + j);
				m_iPoses |= (1 << pos);
				
		}		
	}
	
	if	(		(m_CommonGramCode.length() > 1)
			&&	(m_CommonGramCode != "??")
		)
	{
		bool b = GetGramTab()->GetGrammems(m_CommonGramCode.c_str(), m_TypeGrammems);
		assert (b);
		if (!b)
		{
			ErrorMessage(Format("Cannot get grammems by type gramcode %s ",m_CommonGramCode.c_str()));
		};

		//добавляем граммкод аббр в m_TypeGrammems,а все полные формы аббр будут в FormGramCodes
		for (size_t j=0; j < m_CommonGramCode.length(); j+=2)
		{
				uint64_t CurrGrammems = 0;		
				bool b = GetGramTab()->GetGrammems(m_CommonGramCode.c_str() + j, CurrGrammems);
				assert (b);
				if (!b)
				{
                    ErrorMessage(Format("Cannot get grammems by gramcode %s ",m_CommonGramCode.substr(j,2).c_str()));
				};
				m_TypeGrammems |= CurrGrammems;				
		}
		if(m_CommonGramCode.length()>2) //аббр
			m_TypeGrammems &= ~(m_iGrammems|256); // rVocativ = 256 = зв падеж, "км  12 2  RLE aa CS? SENT_END +Фаао КИЛОМЕТР абавагадаеажазаиайакал"
	};


	return true;
};

void  CAncodePattern::SetPredictedWord(std::string gram_codes, std::string common_gram_codes)
{
	m_SearchStatus = PredictedWord;
	m_CommonGramCode = common_gram_codes;
    m_GramCodes = gram_codes;
    InitAncodePattern();
};

std::string	CAncodePattern::GetPartOfSpeechStr() const
{
	for (BYTE i=0; i< GetGramTab()->GetPartOfSpeechesCount(); i++)
        if (HasPos(i))
            return GetGramTab()->GetPartOfSpeechStr(i);
    return "";
};


std::string  CAncodePattern::ToString() const
{
	rapidjson::Document d;
	CJsonObject b(d);
	b.add_int("language", m_Language);
	b.add_int("found_in_morph", m_SearchStatus);
	b.add_int64("type_grammems", m_TypeGrammems);
	b.add_int64("grammems", m_iGrammems);
	b.add_int("poses", m_iPoses);
	b.add_string("type_gram_code", m_CommonGramCode);
	b.add_string("gram_codes", m_GramCodes);
	return b.dump_rapidjson();
};

bool CAncodePattern::FromString(const std::string& line)
{
	rapidjson::Document doc;
	rapidjson::ParseResult ok = doc.Parse(line);
	assert(ok);
	assert(doc.IsObject());

	auto& a = doc["language"];
	m_Language = (MorphLanguageEnum)doc["language"].GetInt();
	m_SearchStatus = (MorphSearchStatus)doc["found_in_morph"].GetInt();
	m_TypeGrammems = doc["type_grammems"].GetInt64();
	m_iGrammems = doc["grammems"].GetInt64();
	m_iPoses = doc["poses"].GetInt();
	m_CommonGramCode = doc["type_gram_code"].GetString();
	m_GramCodes = doc["gram_codes"].GetString();
	return true;
};

// init from GLR grammar
void  CAncodePattern::InitFromGrammarFormat(MorphLanguageEnum l, std::string& grm)
{
	SetLanguage(l);
	Trim(grm);
	if (grm.empty()) return;
	if (startswith(grm, "-"))
	{
		m_SearchStatus = PredictedWord;
		grm.erase(0, 1);
		Trim(grm);
	}
	else
		if (startswith(grm, "+"))
		{
			m_SearchStatus = DictionaryWord;
			grm.erase(0, 1);
			Trim(grm);
		}
	BYTE PartOfSpeech;
	if (   !GetGramTab()->ProcessPOSAndGrammems(grm.c_str(), PartOfSpeech, m_iGrammems)
		&& !GetGramTab()->ProcessPOSAndGrammems(std::string("* " + grm).c_str(), PartOfSpeech, m_iGrammems)
		)
	{
		throw CExpc("Bad morphological description " + grm);
	};

	if (PartOfSpeech != UnknownPartOfSpeech)
		m_iPoses = 1 << PartOfSpeech;
	else
		m_iPoses = 0;
};

std::string CAncodePattern::ToGrammarFormat() const {
	std::string s;
	if (m_SearchStatus == PredictedWord)
		s += "- ";
	if (m_SearchStatus == DictionaryWord)
		s += "+ ";
	if (m_iPoses != 0) {
		s += GetPartOfSpeechStr() + " ";
	}
	if (m_iGrammems != 0) {
		s += GetGramTab()->GrammemsToStr(m_iGrammems) + " ";
	}
	return s;

}

bool CAncodePattern::HasNoInfo() const {
	return m_TypeGrammems == 0 && m_iGrammems == 0 && m_iPoses == 0 && m_SearchStatus == NotWord;
}

bool CAncodePattern::operator < (const CAncodePattern& _X1) const
{
	if (m_iGrammems != _X1.m_iGrammems)
		return m_iGrammems < _X1.m_iGrammems;

	if (m_iPoses != _X1.m_iPoses)
		return m_iPoses < _X1.m_iPoses;

	if (m_SearchStatus != _X1.m_SearchStatus)
		return m_SearchStatus < _X1.m_SearchStatus;

	return m_TypeGrammems < _X1.m_TypeGrammems;

};

bool CAncodePattern::operator == (const CAncodePattern& _X1) const
{
	return		m_iGrammems == _X1.m_iGrammems
		&& m_iPoses == _X1.m_iPoses
		&& m_SearchStatus == _X1.m_SearchStatus
		&& m_TypeGrammems == _X1.m_TypeGrammems;
};
