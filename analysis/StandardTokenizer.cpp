#include "StdHeader.h"
#include "StandardTokenizer.h"

#include "AnalysisHeader.h"
#include "Analyzers.h"
#include "StandardTokenizerConstants.h"
#include "util/StringBuffer.h"
#include "util/FastCharStream.h"
#include "util/Reader.h"

using namespace NSLib::analysis;
using namespace NSLib::util;
namespace NSLib{ namespace analysis { 


  /** Constructs a tokenizer for this Reader. */
  StandardTokenizer::StandardTokenizer(BasicReader& reader):
    rd(*new FastCharStream(reader)),
    start (1),
    maybeAcronym(false),
    maybeHost(false),
    maybeNumber(false),
    prevHasDigit(false)
  {
  }

  StandardTokenizer::~StandardTokenizer(){
    delete &rd;
  }

  void StandardTokenizer::close(){
  }

  /** Returns the next token in the stream, or NULL at EOS.
  * <p>The returned token's type is set to an element of {@link
  * StandardTokenizerConstants#tokenImage}.
  */
  Token* StandardTokenizer::next() {
    while(!rd.Eos())
    {
      char_t ch = rd.GetNext();
      
      // For skipping whitespaces
      if( isSpace((char_t)ch)!=0 )
      {
        continue;
      }

      // Read for Alpha-Nums
      if( isAlNum((char_t)ch)!=0 )
      {
        start = rd.Column();
        return ReadAlphaNum(ch);
      }

      continue;
    }

    return NULL;
  }

  Token* StandardTokenizer::ReadAlphaNum(const char_t prev)
  {
    maybeAcronym = true;
    maybeHost = true;
    maybeNumber = isDigit(prev);

    StringBuffer str;
    str.append(prev);

    char_t ch = prev;
    while(!rd.Eos() && isSpace((char_t)ch)==0 )
    {
      ch = rd.GetNext();

      if(isAlNum((char_t)ch)!=0)
      {
        if( isDigit(ch)!=0)
          prevHasDigit = true;

        str.append( ch );
      }

      switch(ch)
      {
        case '\'':
            return ReadApostrophe(str, ch);
        case '.':
            return ReadNumber(str, ch);
        case '&':
            return ReadCompany(str.getBuffer(), ch);
        case '@':
            return ReadAt(str.getBuffer(), ch);
        case '-':
        case ',':
        case '_':
        case '/':
            return ReadNumber(str, ch);
        default:
          break;
      }
    }

    return new Token(str.getBuffer(), start, rd.Column(), tokenImage[ALPHANUM] );
  }

  //Reads for apostrophe.
  Token* StandardTokenizer::ReadApostrophe(StringBuffer& str, const char_t ch)
  {
    str.append( ch );

    while( isSpace((char_t)rd.Peek())==0 && isAlNum((char_t)rd.Peek())!=0 || rd.Peek() == '\'')
    {
      str.append( rd.GetNext() );
    }

    if(rd.Peek() == '\'')
      str.len--;
    return new Token(str.getBuffer(), start, rd.Column(), tokenImage[APOSTROPHE]);
  }

  //Reads for something@... it may be a COMPANY name or a EMAIL address
  Token* StandardTokenizer::ReadAt(const char_t* str, const char_t prev)
  {
    StringBuffer val(_T(""));
    bool append = true;

    char_t ch = prev;
    while(!rd.Eos() && isSpace((char_t)ch)==0)
    {
      ch = rd.GetNext();

      if( isAlNum((char_t)ch)!=0 && append)
        val.append( ch );
      else if(ch == '.' && val.length() > 0){
        val.prepend(_T("@"));
        val.prepend(str);
        return ReadEmail(val, ch);
      }else
        append = false;
    }

    if(val.length() > 0){
      val.prepend(_T("@"));
      val.prepend(str);
      return new Token(val.getBuffer(), start, rd.Column(), tokenImage[COMPANY]);
    }else{
      return new Token(str, start, rd.Column(), tokenImage[ALPHANUM]);
    }
  }

  //Reads for COMPANYs in format some&amp;home, at&amp;t.
  Token* StandardTokenizer::ReadCompany(const char_t* str, const char_t prev)
  {
    bool append = true;
    StringBuffer val(_T(""));
    char_t ch = prev;

    while(!rd.Eos() && isSpace((char_t)ch)==0)
    {
      ch = rd.GetNext();

      if( isAlNum((char_t)ch)!=0 && append)
        val.append( ch);
      else
        append = false;
    }

    if(val.length() > 0){
      val.prepend(_T("&"));
      val.prepend(str);
      return new Token(val.getBuffer(), start, rd.Column(), tokenImage[COMPANY]);
    }else
      return new Token(str, start, rd.Column(), tokenImage[ALPHANUM]);
  }

  //Reads for EMAILs somebody@somewhere.else.com.
  Token* StandardTokenizer::ReadEmail(StringBuffer& str, const char_t ch)
  {
    str.append(ch);

    while(isSpace((char_t)rd.Peek())==0 && isAlNum((char_t)rd.Peek())!=0 || rd.Peek() == '.')
    {
      str.append(rd.GetNext());
    }

    if(rd.Peek() == '.')
      str.len--;

    return new Token(str.getBuffer(), start, rd.Column(), tokenImage[EMAIL]);
  }

  //Reads for some.
  //It may be a NUMBER like 12.3, an ACRONYM like U.S.A., or a HOST www.som.com.
  Token* StandardTokenizer::ReadNumber(StringBuffer& str, const char_t prev)
  {
    char_t ch = prev;
    str.append( ch );
    StringBuffer val(_T(""));
    bool append = true;
    bool hasDigit = false;

    if(ch != '.') {
      maybeHost = false;
      maybeAcronym = false;
    }

    while(!rd.Eos() && isSpace((char_t)ch)==0)
    {
      ch = rd.GetNext();

      if( Misc::isLetter(ch)!=0 )//&& append)
        str.append( ch );
      else if( isDigit(ch)!=0 && append) {
        // acronyms can't contain numbers
        maybeAcronym = false;

        // check for number
        hasDigit = true;

        val.append( ch );
      } else if(ch == '_' || ch == '-' || ch == '/' || ch == ',' || ch == '.' && append) {
        if(ch != '.') {
          maybeHost = false;
          maybeAcronym = false;
        }
        if( val.length() == 0) {
          append = false;
          str.append(ch);
          continue;
        } else {
          maybeNumber = (prevHasDigit != hasDigit);
          prevHasDigit = hasDigit;
          val.prepend(str.getBuffer());
          return ReadNumber(val, ch);
        }
      }else if ( ch == '@' ) 
        return ReadAt(str.getBuffer(), ch);
      else 
        append = false;
    } // end of while

    if(maybeAcronym && val.length() == 0){
      val.prepend(str.getBuffer());
      return new Token(val.getBuffer(), start, rd.Column(), tokenImage[ACRONYM]);
    }else if(maybeHost){
      val.prepend(str.getBuffer());
      return new Token(val.getBuffer(), start, rd.Column(), tokenImage[HOST]);
    }else if(maybeNumber){
      val.prepend(str.getBuffer());
      return new Token(val.getBuffer(), start, rd.Column(), tokenImage[NUM]);
    }else {
      int idx = stringCSpn( str.getBuffer(), _T("_-./,") );
      str.len = idx;
      return new Token(str.getBuffer(), start, rd.Column(), tokenImage[ALPHANUM]);
    }
  }

}}
