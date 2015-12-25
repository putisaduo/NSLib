#include "StdHeader.h"
#ifndef _NSLib_analysis_standard_StandardTokenizer
#define _NSLib_analysis_standard_StandardTokenizer

#include "AnalysisHeader.h"
#include "Analyzers.h"
#include "StandardTokenizerConstants.h"
#include "util/StringBuffer.h"
#include "util/FastCharStream.h"
#include "util/Reader.h"

using namespace NSLib::analysis;
using namespace NSLib::util;
namespace NSLib{ namespace analysis { 
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
    StandardTokenizer(BasicReader& reader);

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
}}
#endif
