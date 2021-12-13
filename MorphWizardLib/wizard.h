#pragma once 

#include "../common/utilit.h"
#include "morph_dict/AgramtabLib/agramtab_.h"
#include "FormInfo.h"
#include "DumpParadigm.h"
#include "LemmaPredict.h"
#include "OperationMeter.h"
#include <filesystem>

const uint16_t UnknownSessionNo = 0xffff - 1;
const uint16_t UnknownPrefixSetNo = 0xffff - 1;
const BYTE UnknownAccent = 0xff;	// не менять - уже проставлено в mrd

const uint16_t AnyParadigmNo = 0xffff;
const uint16_t AnyAccentModelNo = 0xffff;
const uint16_t AnySessionNo = 0xffff;
const uint16_t AnyPrefixSetNo = 0xffff;
const BYTE AnyAccent = 0xff - 1;
extern const char* AnyCommonAncode;


//----------------------------------------------------------------------------
//	CParadigmInfo is a special class, which is used only in Morphwizard 
//----------------------------------------------------------------------------
struct CParadigmInfo : public CLemmaInfo
{
    uint16_t	m_SessionNo;
    uint16_t	m_PrefixSetNo;
    BYTE	m_AuxAccent;
    bool	m_bToDelete;

    CParadigmInfo();
    CParadigmInfo(uint16_t ParadigmNo, uint16_t AccentModelNo, uint16_t SessionNo, BYTE AuxAccent, const char* CommonAncode, uint16_t PrefixSetNo);
    bool operator == (const CParadigmInfo& X) const;

    static CParadigmInfo	AnyParadigmInfo();				// Nick [17/Dec/2003]
    bool	IsAnyEqual(const CParadigmInfo& X) const;	// Nick [17/Dec/2003]
};


typedef std::multimap<std::string, CParadigmInfo>		LemmaMap;
typedef LemmaMap::iterator lemma_iterator_t;
typedef LemmaMap::const_iterator const_lemma_iterator_t;



//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
class MorphWizardMeter;


//----------------------------------------------------------------------------
class MorphoWizard : public CMorphWizardBase
{

    bool			m_bWasChanged;

    StringVector  m_PosesList;
    StringVector  m_GrammemsList;
    StringVector  m_TypeGrammemsList;


    StringVector			m_Users;
    size_t					m_SessionNo;
    std::vector<CMorphSession>	m_Sessions;

    friend	class MorphWizardMeter;
    MorphWizardMeter* m_pMeter;	// Nick 30.11.2003

    void load_gramtab(std::string path, bool useNationalConstants);
    void ReadSessions(std::ifstream& mrdFile);
    void ReadLemmas(std::ifstream& mrdFile);
public:
    struct AncodeLess
    {
        const CAgramtab* m_pGramTab;
        void init(const CAgramtab* pGramTab);
        bool operator()(const std::string& s1, const std::string& s2)const;
    };
    AncodeLess ancode_less;



    std::vector<std::set<std::string> >	m_PrefixSets;
    TLemmaPredictor m_Predictor;

    // the multimap from lemma to paradigms (the largest list)
    LemmaMap				m_LemmaToParadigm;


    std::filesystem::path m_MwzFolder;
    std::filesystem::path m_GramtabPath;


    //! a table of character properties for regular expressions which depend on CConcIndexator::m_Language
    std::vector<BYTE>			m_PcreCharacterTables;

    bool					m_bLoaded;

    const CAgramtab* m_pGramTab;
    bool						m_ReadOnly;
    std::string						m_LanguageStr;
    std::string						m_MrdPath;
    bool						m_bFullTrace;

    MorphoWizard();
    ~MorphoWizard();

    //=================  general: loading, saving, logging ======================
    void	load_wizard(std::string path, std::string user_name, bool bCreatePrediction = true, bool useNationalConstants=true);
    void	load_mrd(bool guest, bool bCreatePrediction);
    void	log(const std::string& messg);
    void	log(const std::string& lemm, const CFlexiaModel& p, bool is_added);
    bool	is_changed() { return m_bWasChanged; }
    void	save_mrd();
    std::string	get_lock_file_name()   const;
    std::string	get_log_file_name()   const;
    void	MakeReadOnly();
    void	pack();
    uint16_t	GetCurrentSessionNo() const;
    size_t	del_dup_lemm();

    //===============  simple  primitives for ancode  and lemma iterator =========
    std::string					get_pos_string(const lemma_iterator_t it) const;
    std::string					get_pos_string(const std::string& code) const;
    std::string					get_lemm_string(const_lemma_iterator_t it)   const;
    std::string					get_lemm_string_with_accents(const_lemma_iterator_t it)   const;
    std::string					get_base_string(const_lemma_iterator_t it)   const;
    std::string					get_grammem_string(const std::string& code) const;
    std::string					get_grammem_string(lemma_iterator_t it)  const;
    uint64_t					get_all_lemma_grammems(const_lemma_iterator_t it) const;
    std::string					get_common_grammems_string(const_lemma_iterator_t it) const;
    std::string					get_prefix_set(const_lemma_iterator_t it) const;
    std::string                 get_prefix_set(CParadigmInfo i) const;
    std::string					get_pos_string_and_grammems(const std::string& code) const;
    const CMorphSession& get_session(int SessionNo) const;
    bool					IsGerman() const { return m_Language == morphGerman; };
    const StringVector& get_poses() { return m_PosesList; }
    const StringVector& get_grammems() { return m_GrammemsList; }
    const StringVector& get_type_grammems() { return m_TypeGrammemsList; }

    //  ===========   find procedures ================
    void find_lemm_by_grammem(const std::string& pos_and_grammems, std::vector<lemma_iterator_t>& res);
    void find_lemm(std::string lemm, bool bCheckLemmaPrefix, std::vector<lemma_iterator_t>& res);
    void find_lemm_by_regex(std::string pattern, bool bCheckLemmaPrefix, std::vector<lemma_iterator_t>& res);
    void find_lemm_by_user(std::string username, std::vector<lemma_iterator_t>& res);
    void find_wordforms(std::string lemm, std::vector<lemma_iterator_t>& res);
    void find_ancodes(const std::string& ancode, std::vector<lemma_iterator_t>& res);
    void find_lemm_by_prdno(uint16_t no, std::vector<lemma_iterator_t>& res);
    void find_lemm_by_accent_model(int no, std::vector<lemma_iterator_t>& res);

    void find_lemm_by_prd_info(const CParadigmInfo& info, std::vector<lemma_iterator_t>& res);

    //============ Main functions for editing dictionary ====================
    // Mrd -> Slf (Converting from dictionary to text representation)
    std::string	mrd_to_slf(const std::string& lemm, const CFlexiaModel& p, uint16_t AccentModelNo, BYTE AuxAccent, int line_size) const;
    // Slf -> Mrd (Converting from text to dictionary representation)
    void	slf_to_mrd(const std::string& s, std::string& lemm, CFlexiaModel& FlexiaModel, CAccentModel& AccentModel, BYTE& AuxAccent, int& line_no_err) const;
    void	check_paradigm(long line_no);
    void	remove_lemm(lemma_iterator_t it);
    std::string	get_slf_string(lemma_iterator_t it, std::string& dict, std::string& Prefixes, int line_size = 79);
    void	get_wordforms(const_lemma_iterator_t it, StringVector& forms) const;
    CParadigmInfo	add_lemma(const std::string& slf, std::string common_grammems, const std::string& prefixes, int& line_no_err, uint16_t SessionNo = UnknownSessionNo);
    void	set_to_delete_false();
    void	delete_checked_lemms();
    bool	change_prd_info(CParadigmInfo& I, const std::string& Lemma, uint16_t NewParadigmNo, uint16_t newAccentModelNo, bool keepOldAccents);
    std::string	show_differences_in_two_paradigms(uint16_t FlexiaModelNo1, uint16_t FlexiaModelNo2) const;

    bool	slf2ancode(const std::string slf_line, std::string& gramcode) const;
    bool	check_common_grammems(std::string common_grammems) const;
    bool	check_prefixes(std::string prefixes) const;
    bool	attach_form_prefixes_to_bases();
    bool	prepare_for_RML();
    bool	prepare_for_RML2();

    bool				HasMeter() const { return  !!m_pMeter; }
    MorphWizardMeter* GetMeter() { return  m_pMeter; }
    bool	HasUnknownAccents(lemma_iterator_t it) const;
    bool	IsPartialAccented(lemma_iterator_t it) const;
    BYTE	GetLemmaAccent(const_lemma_iterator_t it) const;
    bool	StartSession(std::string user_name);
    void	EndSession();
    std::string  GetUserName() const;
    void	StartLastSessionOfUser(std::string user_name);
    uint16_t	RegisterSession(const CMorphSession& S);
    bool    Filter(std::string flt_str, std::vector<lemma_iterator_t>& found_paradigms) const;

    std::string ToRMLEncoding(std::wstring strText) const;
    std::wstring FromRMLEncoding(std::string s) const;
    std::string create_slf_for_lemm(std::string lemm, size_t flexiaModelNo, int line_size) const;

private:
    BYTE	_GetReverseVowelNo(const std::string& form, uint16_t accentModelNo, uint16_t formInd) const;
    void	SetAccent(uint16_t AccentModelNo, BYTE AuxAccent, int FormNo, std::string& form) const;
    std::string	get_prefix_set_str(uint16_t PrefixSetNo) const;
    void	ReadOnePrefixSet(std::string PrefixSet, std::set<std::string>& Result) const;
    void	ReadPrefixSets(std::ifstream& mrdFile);
    uint16_t	AddPrefixSet(std::string PrefixSetStr);



};

//----------------------------------------------------------------------------
class MorphWizardMeter : public CFileMeterRML
{
public:
    MorphWizardMeter(MorphoWizard& wizard) : m_pWizard(&wizard) { wizard.m_pMeter = this; }
    virtual ~MorphWizardMeter() { m_pWizard->m_pMeter = NULL; }
private:
    MorphoWizard* m_pWizard;
};


extern BYTE  TransferReverseVowelNoToCharNo(const std::string& form, BYTE AccentCharNo, MorphLanguageEnum Language);

