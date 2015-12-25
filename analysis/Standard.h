#include "StdHeader.h"
#ifndef _NSLib_analysis_standard__
#define _NSLib_analysis_standard__


using namespace NSLib::util;
using namespace NSLib::analysis;

namespace NSLib{ namespace analysis { namespace standard {
    

	// Normalizes tokens extracted with {@link StandardTokenizer}. 
	class StandardFilter: public TokenFilter{
	public:
		// Construct filtering <i>in</i>. 
		StandardFilter(TokenStream* in);

		~StandardFilter();

		// Returns the next token in the stream, or NULL at EOS.
		// <p>Removes <tt>'s</tt> from the end of words.
		// <p>Removes dots from acronyms.
		Token* next();
	};
	
	
	
	
	// A grammar-based tokenizer constructed with JavaCC.
	//
	// <p> This should be a good tokenizer for most European-language documents.
	//
	// <p>Many applications have specific tokenizer needs.  If this tokenizer does
	// not suit your application, please consider copying this source code
	// directory to your project and maintaining your own grammar-based tokenizer.
	class StandardTokenizer: public Tokenizer {
	private:
		bool maybeAcronym;
		bool maybeHost;
		bool maybeNumber;
		bool prevHasDigit;
	public:
		FastCharStream& rd;
		int start;

		// Constructs a tokenizer for this Reader. 
		StandardTokenizer(Reader& reader);

		~StandardTokenizer();

		void close();

		// Returns the next token in the stream, or NULL at EOS.
		// <p>The returned token's type is set to an element of {@link
		// StandardTokenizerConstants#tokenImage}.
		Token* next();

		Token* ReadAlphaNum(const char_t prev);

		//Reads for apostrophe.
		Token* ReadApostrophe(StringBuffer& str, const char_t ch);

		//Reads for something@... it may be a COMPANY name or a EMAIL address
		Token* ReadAt(const char_t* str, const char_t prev);

		//Reads for COMPANYs in format some&amp;home, at&amp;t.
		Token* ReadCompany(const char_t* str, const char_t prev);

		//Reads for EMAILs somebody@somewhere.else.com.
		Token* ReadEmail(StringBuffer& str, const char_t ch);

		//Reads for some.
		//It may be a NUMBER like 12.3, an ACRONYM like U.S.A., or a HOST www.som.com.
		Token* ReadNumber(StringBuffer& str, const char_t prev);
	};
	
	
	
	enum TokenTypes
	{
		ALPHANUM,
		APOSTROPHE,
		ACRONYM,
		COMPANY,
		EMAIL,
		HOST,
		NUM,
		_EOF
	};
	
	const static char_t *tokenImage[] = { _T("<ALPHANUM>"),
								  _T("<APOSTROPHE>"),
								  _T("<ACRONYM>"),
								  _T("<COMPANY>"),
								  _T("<EMAIL>"),
								  _T("<HOST>"),
								  _T("<NUM>"),
								  _T("<EOF>"),
								  _T("<P>"),
								  _T("<HAS_DIGIT>"),
								  _T("<ALPHA>"),
								  _T("<LETTER>"),
								  _T("<DIGIT>"),
								  _T("<NOISE>"),};
	//An array containing some common English words that are usually not
	//useful for searching.
	const static char_t* STOP_WORDS [] = 
	{
        _T("a"), _T("and"), _T("are"), _T("as"), _T("at"), _T("be"), _T("but"), _T("by"),
		_T("for"), _T("if"), _T("in"), _T("into"), _T("is"), _T("it"),
		_T("no"), _T("not"), _T("of"), _T("on"), _T("or"), _T("s"), _T("such"),
		_T("t"), _T("that"), _T("the"), _T("their"), _T("then"), _T("there"), _T("these"),
		_T("they"), _T("this"), _T("to"), _T("was"), _T("will"), _T("with")
	};
	const static int STOP_WORDS_LENGTH = 34;




	//Represents a standard analyzer.
	class StandardAnalyzer : public Analyzer 
	{
	private:
		VoidMap<const char_t*,const char_t*> stopTable;
	public:
		// <summary> Builds an analyzer. </summary>
		StandardAnalyzer()
		;

		//<summary> Builds an analyzer with the given stop words. </summary>
		StandardAnalyzer(const char_t* stopWords[],const int stopWordsLength)
		;

		~StandardAnalyzer();


		// <summary>
		// Constructs a StandardTokenizer filtered by a 
		// StandardFilter, a LowerCaseFilter and a StopFilter.
		// </summary>
		TokenStream& tokenStream(const char_t* fieldName, Reader* reader) 
		;
	};
}}}
#endif
