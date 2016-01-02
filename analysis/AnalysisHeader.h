#include "StdHeader.h"
#ifndef _NSLib_analysis_AnalysisHeader_
#define _NSLib_analysis_AnalysisHeader_

#include "util/Reader.h"

namespace NSLib{ namespace analysis{

class Token {
private:
  const int startOffset;          // start in source text
  const int endOffset;          // end in source text
  const char_t* type;          // lexical type

public:
  char_t* termText;          // the text of the term


  // Constructs a Token with the given term text, and start & end offsets.
  //  The type defaults to "word."
  Token(const char_t* text, const int start, const int end):
    startOffset (start),
    endOffset (end),
    type ( _T("word") ),
    termText( stringDuplicate(text) )
  {
  }

  ~Token(){
    delete[] termText;
  }

  // Constructs a Token with the given text, start and end offsets, & type. 
  Token(const char_t* text, const int start, const int end, const char_t* typ):
    startOffset (start),
    endOffset (end),
    type ( typ ),
    termText( stringDuplicate(text) )
    //type (stringDuplicate(typ) ) //shouldn't need to duplicate type, already in tokenImage
  {
  }

  // Returns the Token's term text. 
  const char_t* TermText() { return termText; }

  // Returns this Token's starting offset, the position of the first character
  //  corresponding to this token in the source text.
      //
  //  Note that the difference between endOffset() and startOffset() may not be
  //  equal to termText.length(), as the term text may have been altered by a
  //  stemmer or some other filter. 
  int StartOffset() { return startOffset; }

  // Returns this Token's ending offset, one greater than the position of the
  //  last character corresponding to this token in the source text. 
  int EndOffset() { return endOffset; }

  // Returns this Token's lexical type.  Defaults to "word". 
  const char_t* Type() { return type; }
};

class TokenStream {
public:
  // Returns the next token in the stream, or null at EOS. 
  virtual Token* next() = 0;

  // Releases resources associated with this stream. 
  virtual void close() = 0;

  virtual ~TokenStream() { }
};


class Analyzer {
public:
  // Creates a TokenStream which tokenizes all the text in the provided
  //  Reader.  Default implementation forwards to tokenStream(Reader) for 
  //  compatibility with older version.  Override to allow Analyzer to choose 
  //  strategy based on document and/or field.  Must be able to handle null
  //  field name for backward compatibility. 
  virtual TokenStream& tokenStream(const char_t* fieldName, util::BasicReader* reader)=0;
    
  virtual ~Analyzer(){ }
};


class Tokenizer:public TokenStream {
protected:
  // The text source for this Tokenizer. 
  NSLib::util::BasicReader* input;

public:
  // By default, closes the input Reader. 
  virtual void close() {
    input->close();
  }

  virtual ~Tokenizer(){ }
};

class TokenFilter:public TokenStream {
protected:
  // The source of tokens for this filter. 
  TokenStream* input;
  bool deleteTokenStream;
  
  TokenFilter(TokenStream* in, bool deleteTS):
    input(in),
    deleteTokenStream(deleteTS)
  {
  }
  virtual ~TokenFilter(){
    if ( deleteTokenStream )
      delete input;
  }

public:
  // Close the input TokenStream. 
  void close() {
    input->close();
  }

};
}}
#endif
