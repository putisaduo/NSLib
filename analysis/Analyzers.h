#include "StdHeader.h"
#ifndef _NSLib_analysis_Analyzers_
#define _NSLib_analysis_Analyzers_

#include "util/VoidMap.h"
#include "util/Reader.h"
#include "AnalysisHeader.h"

using namespace NSLib::util;
namespace NSLib{ namespace analysis {
    class CharTokenizer:public Tokenizer {
  private:
    int offset, bufferIndex, dataLen;
    const static int MAX_WORD_LEN = 255;
    const static int IO_BUFFER_SIZE = 1024;
    char_t buffer[MAX_WORD_LEN+1];
    char_t ioBuffer[IO_BUFFER_SIZE+1];
    //Reader* input; input is in tokenizer base class (bug fix thanks to Andy Osipienko)
  protected:
    // Returns true iff a character should be included in a token.  This
    // tokenizer generates as tokens adjacent sequences of characters which
    // satisfy this predicate.  Characters for which this is false are used to
    // define token boundaries and are not included in tokens.
    virtual bool isTokenChar(const char_t c) = 0;

    // Called on each token character to normalize it before it is added to the
    // token.  The default implementation does nothing.  Subclasses may use this
    // to, e.g., lowercase tokens. 
    virtual char_t normalize(const char_t c) { return c; }
  
  public:
    CharTokenizer(BasicReader* in):
      //input(in), ; input is in tokenizer base class (bug fix thanks to Andy Osipienko)
      offset(0),
      bufferIndex(0),
      dataLen(0)
    {
        input = in; //; input is in tokenizer base class (bug fix thanks to Andy Osipienko)
    }

        virtual ~CharTokenizer(){
        }
    
    // Returns the next token in the stream, or null at EOS. 
    // *** This is not a pointer. Use of it must deleted.
    Token* next() {
      int length = 0;
      int start = offset;
      while (true) {
        char_t c;

        offset++;
        if (bufferIndex >= dataLen) {
          dataLen = input->read(ioBuffer,0,IO_BUFFER_SIZE);
          bufferIndex = 0;
        };
        if (dataLen <= 0 ) {
          if (length > 0)
            break;
          else
            return NULL;
          }
        else
          c = ioBuffer[bufferIndex++];
            
        if (isTokenChar(c)) {                       // if it's a token char_t

          if (length == 0)        // start of token
            start = offset-1;

          buffer[length++] = normalize(c);          // buffer it, normalized

          if (length == MAX_WORD_LEN)      // buffer overflow!
            break;

        } else if (length > 0)        // at non-Letter w/ chars
          break;            // return 'em 
      }
      buffer[length]=0;
      return new Token( buffer, start, start+length);
    }
  };
  
  
  
  
  class LetterTokenizer:public CharTokenizer {
  public:
    // Construct a new LetterTokenizer. 
    LetterTokenizer(BasicReader* in):
      CharTokenizer(in)
    { }
  
    ~LetterTokenizer(){ }
  protected:
    // Collects only characters which satisfy
    // {@link Character#isLetter(char_t)}.
    bool isTokenChar(const char_t c) {
      return Misc::isLetter(c)!=0;
    }
  };
  
  
  
  
  
  // LowerCaseTokenizer performs the function of LetterTokenizer
  // and LowerCaseFilter together.  It divides text at non-letters and converts
  // them to lower case.  While it is functionally equivalent to the combination
  // of LetterTokenizer and LowerCaseFilter, there is a performance advantage
  // to doing the two tasks at once, hence this (redundant) implementation.
  // <P>
  // Note: this does a decent job for most European languages, but does a terrible
  // job for some Asian languages, where words are not separated by spaces.
  class LowerCaseTokenizer:public LetterTokenizer {
  public:
    // Construct a new LowerCaseTokenizer. 
    LowerCaseTokenizer(BasicReader* in): LetterTokenizer(in)
    { }
  
    ~LowerCaseTokenizer(){ }
    void close(){}

  protected:
    // Collects only characters which satisfy
    // {@link Character#isLetter(char_t)}.
    char_t normalize(const char_t chr) {
      return toLower(chr);
    }
  };
  
  

  class WhitespaceTokenizer: public CharTokenizer {
  public:
    // Construct a new WhitespaceTokenizer. 
    WhitespaceTokenizer(BasicReader* in):CharTokenizer(in) { }
    ~WhitespaceTokenizer(){ }

    void close(){ }
  protected:
    // Collects only characters which do not satisfy
    // {@link Character#isWhitespace(char_t)}.
    bool isTokenChar(const char_t c) {
        return isSpace(c)==0; //(return true if NOT a space)
    }
  };
  
  
  // An Analyzer that uses WhitespaceTokenizer. 
  class WhitespaceAnalyzer: public Analyzer {
   public:
    TokenStream& tokenStream(const char_t* fieldName, BasicReader* reader) {
      return *new WhitespaceTokenizer(reader);
    }
    ~WhitespaceAnalyzer(){ }
  };
    
    
    
    
    
  class SimpleAnalyzer: public Analyzer {
  public:
    TokenStream& tokenStream(const char_t* fieldName, BasicReader* reader) {
      return *new LowerCaseTokenizer(reader);
    }
    ~SimpleAnalyzer(){ }
  };
  
  
  
  
  class LowerCaseFilter: public TokenFilter {
  public:
    LowerCaseFilter(TokenStream* in, bool deleteTokenStream)
      : TokenFilter(in,deleteTokenStream) 
    { }
    
    ~LowerCaseFilter(){
    }
    Token* next(){
      Token* t = input->next();
      if (t == NULL)
      return NULL;
          
      stringLower( t->termText );
      return t;
    }
  };
    
    
    
  // Removes stop words from a token stream. 
  class StopFilter: public TokenFilter {
  private:
    VoidMap< char_t*, char_t*>& table;
  public:
    // Constructs a filter which removes words from the input
    //  TokenStream that are named in the array of words. 
    StopFilter(TokenStream* in, bool deleteTokenStream, 
               char_t* stopWords[], int stopWordsLength)
      : TokenFilter(in, deleteTokenStream),
        table(*new VoidMap< char_t*, char_t*>)
    {
      //TODO: table.setDoDelete(true,NSLib::util::DELETE_TYPE_DELETE);
      fillStopTable( table, stopWords, stopWordsLength);
    }

    ~StopFilter(){ }

    // Constructs a filter which removes words from the input
    //  TokenStream that are named in the Hashtable.
    StopFilter(TokenStream* in, bool deleteTokenStream, 
               VoidMap< char_t*, char_t*>& stopTable)
      : TokenFilter(in, deleteTokenStream),
        table(stopTable)
    { } 
      
    // Builds a Hashtable from an array of stop words, appropriate for passing
    //  into the StopFilter constructor.  This permits this table construction to
    //  be cached once when an Analyzer is constructed. 
    static void fillStopTable(VoidMap< char_t*, char_t*>& stopTable,
                              char_t* stopWords[], int length)
    {
      for (int i = 0; i < length; i++)
        stopTable.put(stopWords[i], stopWords[i]);
    }

    // Returns the next input Token whose termText() is not a stop word. 
    Token* next() {
      // return the first non-stop word found
      for (Token* token = input->next(); token != NULL; token = input->next()){
        if (!table.exists(token->termText)){
          return token;
        }else
          delete token;
      }

      // reached EOS -- return null
      return NULL;
    }
  };
  
  
  
  
    //An array containing some common English words that are usually not
  //useful for searching.
  static const char_t* ENGLISH_STOP_WORDS [] = 
  {
    _T("a"), _T("and"), _T("are"), _T("as"), _T("at"), _T("be"), _T("but"), _T("by"),
    _T("for"), _T("if"), _T("in"), _T("into"), _T("is"), _T("it"),
    _T("no"), _T("not"), _T("of"), _T("on"), _T("or"), _T("s"), _T("such"),
    _T("t"), _T("that"), _T("the"), _T("their"), _T("then"), _T("there"), _T("these"),
    _T("they"), _T("this"), _T("to"), _T("was"), _T("will"), _T("with")
  };
  const static int ENGLISH_STOP_WORDS_LENGTH = 34;
  
  // Filters LetterTokenizer with LowerCaseFilter and StopFilter. 
  class StopAnalyzer: public Analyzer {
    VoidMap< char_t*, char_t*> stopTable;
    
  public:
    // Builds an analyzer which removes words in ENGLISH_STOP_WORDS. 
    StopAnalyzer(){
      StopFilter::fillStopTable(stopTable, const_cast<char_t**>(ENGLISH_STOP_WORDS),
                                ENGLISH_STOP_WORDS_LENGTH);
    }
        
    // Builds an analyzer which removes words in the provided array. 
    StopAnalyzer( char_t* stopWords[], int length) {
      StopFilter::fillStopTable(stopTable,stopWords,length);
    }
    // Filters LowerCaseTokenizer with StopFilter. 
    TokenStream& tokenStream(const char_t* fieldName, BasicReader* reader) {
      return *new StopFilter(new LowerCaseTokenizer(reader),true, stopTable);
    }
  };

}}
#endif
