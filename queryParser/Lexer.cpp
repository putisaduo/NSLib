#include "StdHeader.h"
#include "Lexer.h"

#include "QueryParserConstants.h"
#include "util/FastCharStream.h"
#include "util/Reader.h"
#include "util/StringBuffer.h"
#include "TokenList.h"
#include "QueryToken.h"
#include "QueryParserBase.h"
#include <iostream>
//#include <io.h>
#include <fcntl.h>

using namespace NSLib::util;

namespace NSLib{ namespace queryParser{
  Lexer::Lexer(const char_t* query):
    reader ( *new FastCharStream( *new StringReader(query) ) ),
    delSR(true)
  {
    // use this
  }
  
  // Initializes a new instance of the Lexer class with the specified
  // TextReader to lex.
  Lexer::Lexer(BasicReader& source):
    reader ( *new FastCharStream(source) ),
    delSR(false)
  {
  }
  
  Lexer::~Lexer(){
    if ( delSR )
      delete &reader.input;
    delete &reader;
  }
  
  // Breaks the input stream onto the tokens list and returns it.
  //
  // <returns>The tokens list.</returns>
  TokenList& Lexer::Lex()
  {
    QueryToken* token = GetNextToken();
    while(true)
    {
      if(token != NULL)
        tokens.Add(token);
      else
      {
        tokens.Add(new QueryToken( NSLib::queryParser::EOF_));
        return tokens;
      }
  
      token = GetNextToken();
    }
  }
  
  QueryToken* Lexer::GetNextToken()
  {
    while(!reader.Eos())
    {
      char_t ch = reader.GetNext();
      
      // skipping whitespaces
      if( isSpace(ch)!=0 )
      {
        cerr << " ";
        continue;
      }
      char_t buf[250] = {ch,'\0'};
      switch(ch)
      {
        case '+':
          cerr << "+";
          return new QueryToken(buf , NSLib::queryParser::PLUS);
        case '-':
          cerr << "-";
          return new QueryToken(buf, NSLib::queryParser::MINUS);
        case '(':
          cerr << "(";
          return new QueryToken(buf, NSLib::queryParser::LPAREN);
        case ')':
          cerr << ")";
          return new QueryToken(buf, NSLib::queryParser::RPAREN);
        case ':':
          return new QueryToken(buf, NSLib::queryParser::COLON);
        case '!':
          return new QueryToken(buf, NSLib::queryParser::NOT);
        case '^':
          return new QueryToken(buf, NSLib::queryParser::CARAT);
        case '~':
          if( isDigit( reader.Peek())!=0 )
          {
            const char_t* number = ReadIntegerNumber(ch);
            QueryToken* ret = new QueryToken(number, NSLib::queryParser::SLOP);
            delete[] number;
            return ret;
          }
          else
          {
            return new QueryToken(buf, NSLib::queryParser::FUZZY);
          }
        case L'"':
          cerr << "\"";
          return ReadQuoted(ch);
        case '[':
          return ReadInclusiveRange(ch);
        case '{':
          return ReadExclusiveRange(ch);
        case ']':
        case '}':
        case '*':
          QueryParserBase::throwParserException("Unrecognized char_t %d at %d::%d.",
                                                ch, reader.Column(), reader.Line() );
        default:
          return ReadTerm(ch);
          
      } // end of swith
  
    }
    return NULL;
  }
  
  // Reads an integer number
  const char_t* Lexer::ReadIntegerNumber(const char_t ch)
  {
    StringBuffer number;
    number.append(ch); //TODO: check this
    while(!reader.Eos() && isDigit(reader.Peek())!=0 )
    {
      number.append(reader.GetNext());
    }
    return number.ToString();
  }
  
  // Reads an inclusive range like [some words]
  QueryToken* Lexer::ReadInclusiveRange(const char_t prev)
  {
    char_t ch = prev;
    StringBuffer range;
    range.append(ch);
  
    while(!reader.Eos())
    {
      ch = reader.GetNext();
      range.append(ch);
  
      if(ch == ']')
        return new QueryToken(range.getBuffer(), NSLib::queryParser::RANGEIN);
    }
    QueryParserBase::throwParserException("Unterminated inclusive range! %c %d::%d",
                                          ' ',reader.Column(),reader.Column());
  return NULL;
  }
  
  // Reads an exclusive range like {some words}
  QueryToken* Lexer::ReadExclusiveRange(const char_t prev)
  {
    char_t ch = prev;
    StringBuffer range;
    range.append(ch);
  
    while(!reader.Eos())
    {
      ch = reader.GetNext();
      range.append(ch);
  
      if(ch == '}')
        return new QueryToken(range.getBuffer(), NSLib::queryParser::RANGEEX);
    }
    QueryParserBase::throwParserException("Unterminated exclusive range! %c %d::%d",
                                          ' ',reader.Column(),reader.Column() );
  return NULL;
  }
  
  // Reads quoted string like "something else"
  QueryToken* Lexer::ReadQuoted(const char_t prev)
  {
    char_t ch = prev;
    StringBuffer quoted;
    quoted.append(ch);
  
    while(!reader.Eos())
    {
      ch = reader.GetNext();
      quoted.append(ch);
  
      if(ch == L'"')
        return new QueryToken(quoted.getBuffer(), NSLib::queryParser::QUOTED);
    }
    //_setmode(_fileno(stdout), _O_U16TEXT);
    //wcerr << L"$$$" << quoted.length() << L"::" << quoted.getBuffer() << L"\n";
    QueryParserBase::throwParserException("Unterminated string! %c %d::%d",
                                          ' ',reader.Column(),reader.Column());
    return NULL;
  }
  
  QueryToken* Lexer::ReadTerm(const char_t prev)
  {
    char_t ch = prev;
    bool completed = false;
    int asteriskCount = 0;
    bool hasQuestion = false;
  
    StringBuffer val;
  
    while(true)
    {
      switch(ch)
      {
      case '\\':
        {
          const char_t* re = ReadEscape(ch);
            val.append( re );
          delete[] re;
        }
        break;
      case '*':
        asteriskCount++;
        val.append(ch);
        break;
      case '?':
        hasQuestion = true;
        val.append(ch);
        break;
      case '\n':
      case '\t':
      case ' ':
      case '+':
      case '-':
      case '!':
      case '(':
      case ')':
      case ':':
      case '^':
      case '[':
      case ']':
      case '{':
      case '}':
      case L'"':
      case '~':
        // create new QueryToken
        reader.UnGet();
        completed = true;
        break;
      default:
        val.append(ch);
        break;
      } // end of switch
  
      if(completed || reader.Eos())
        break;
      else
        ch = reader.GetNext();
    }
  
    // create new QueryToken
    if(hasQuestion)
      return new QueryToken(val.getBuffer(), NSLib::queryParser::WILDTERM);
    else if(asteriskCount == 1 && val.getBuffer()[val.length() - 1] == '*')
      return new QueryToken(val.getBuffer(), NSLib::queryParser::PREFIXTERM);
    else if(asteriskCount > 0)
      return new QueryToken(val.getBuffer(), NSLib::queryParser::WILDTERM); 
    else if( stringICompare(val.getBuffer(), _T("AND"))==0
            || stringCompare(val.getBuffer(), _T("&&"))==0 )
    {
       cerr << " AND ";
      return new QueryToken(val.getBuffer(), NSLib::queryParser::AND_);
    }
    else if( stringICompare(val.getBuffer(), _T("OR"))==0
             || stringCompare(val.getBuffer(), _T("||"))==0)
    {
      cerr << " OR ";
      return new QueryToken(val.getBuffer(), NSLib::queryParser::OR);
    }
    else if( stringICompare(val.getBuffer(), _T("NOT"))==0 )
      return new QueryToken(val.getBuffer(), NSLib::queryParser::NOT);
    else{
      char_t* lwr = stringLower( val.ToString() );
      char_t* upr = stringUpper( val.ToString() );
      bool n = (stringCompare(lwr,upr) == 0);
      delete[] lwr; delete[] upr;
      
      if ( n )
        return new QueryToken(val.getBuffer(), NSLib::queryParser::NUMBER);
      else
        return new QueryToken(val.getBuffer(), NSLib::queryParser::TERM);
    }
  }
  
  
  const char_t* Lexer::ReadEscape(const char_t prev)
  {
    char_t ch = prev;
    StringBuffer val;
    val.append(ch);
  
    ch = reader.GetNext();
    int idx = stringCSpn( val.getBuffer(), _T("\\+-!():^[]{}\"~*") ); 
    if(idx == 0) {
      val.append( ch );
      return val.ToString();
    }
    QueryParserBase::throwParserException("Unrecognized escape sequence at %c %d::%d",
                                          ' ',reader.Column(),reader.Line());
    return 0;
  }

}}
