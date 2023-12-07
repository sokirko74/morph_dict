#pragma once 

#include "morph_dict/common/utilit.h"

enum MorphSearchStatus { 
	DictionaryWord = 1,  // core words
	PredictedWord = 2,  // rare words
	NotWord = 3  // not words like punctuation or numbers
};

class CAgramtab;

class CAncodePattern 
{
    void    ResetFlags();
	std::string			m_GramCodes;
	MorphLanguageEnum m_Language;
protected:
	const	CAgramtab* GetGramTab() const;

public:
	MorphSearchStatus m_SearchStatus;
	grammems_mask_t	m_TypeGrammems;
	grammems_mask_t	m_iGrammems;
	std::string	m_CommonGramCode;
	part_of_speech_mask_t    m_iPoses;
			
	// runtime syntax: all single preposition interpretations
	std::vector<int>		m_SimplePrepNos;

	CAncodePattern();
	void    SetLanguage(MorphLanguageEnum l);
	//from gramcodes to bit masks 
	bool	InitAncodePattern();
	
	void	CopyAncodePattern(const CAncodePattern& X);
    
	void    SetPredictedWord(std::string gram_codes, std::string common_gram_codes="??");
	void    SetNotWord();

	
    bool    DeleteAncodesByGrammemIfCan(BYTE Grammem);
	bool	ModifyGrammems(uint64_t Grammems, part_of_speech_mask_t Poses = 0xffffffff);
	void  SetGramCodes(const std::string&);
	void  SetGramCodes(const char*);

	bool	HasGrammem(BYTE gram) const;
	bool	HasPos(BYTE pos) const;
	std::string	GetGrammemsByAncodes() const;
	std::string	GetPartOfSpeechStr() const;
    const std::string& GetGramCodes() const;
	bool  HasUnkGramCode() const;
	BYTE GetLemSign() const;
	bool HasNoInfo() const;

	std::string  CAncodePattern::ToString() const;
	bool FromString(const std::string& line);
	
	void  InitFromGrammarFormat(MorphLanguageEnum l, std::string& grm);
	std::string ToGrammarFormat() const;

	bool operator < (const CAncodePattern& _X1) const;
	bool operator == (const CAncodePattern& _X1) const;
};
