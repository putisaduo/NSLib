#include "StdHeader.h"
#include "QueryParser.h"

#include "QueryParserConstants.h"
#include "analysis/AnalysisHeader.h"
#include "util/Reader.h"
#include "search/SearchHeader.h"
#include "index/Term.h"

#include "search/WildcardQuery.h"
#include "search/FuzzyQuery.h"
#include "search/PrefixQuery.h"

#include "TokenList.h"
#include "QueryToken.h"
#include "QueryParserBase.h"
#include "Lexer.h"

#include <iostream>

using namespace NSLib::util;
using namespace NSLib::index;
using namespace NSLib::analysis;
using namespace NSLib::search;

namespace NSLib{ namespace queryParser{
QueryParser::~QueryParser(){
  delete[] field;
}
    
QueryParser::QueryParser(const char_t* _field, Analyzer& _analyzer) :
  analyzer(_analyzer),
  field( stringDuplicate( _field) ),
  tokens(NULL)
{
}
    
// Returns a new instance of the QueryParser class with a specified query, field and
// analyzer values.
//
// <param name="query">The query to parse.</param>
// <param name="field">The default field for query terms.</param>
// <param name="analyzer">Used to find terms in the query text.</param>
//static 
Query& QueryParser::Parse(const char_t* query, const char_t* field, Analyzer& analyzer)
{
  QueryParser parser(field, analyzer);
  return parser.Parse(query);
}
    
// Returns a parsed Query instance.
//
// <param name="query">The query value to be parsed.</param>
// <returns>A parsed Query instance.</returns>
Query& QueryParser::Parse(const char_t* query)
{
  cerr << "################ parsing query1." << endl;
  BasicReader* r = new StringReader(query);
  Query* ret = NULL;
  _TRY{
    ret = &Parse( *r );
  }_FINALLY (
    delete r;
  );
    
  return *ret;
}
    
// Returns a parsed Query instance.
//
// <param name="reader">The TextReader value to be parsed.</param>
// <returns>A parsed Query instance.</returns>
Query& QueryParser::Parse(BasicReader& reader)
{
  Lexer lexer(reader);
  tokens = &lexer.Lex();
  cerr << "################ parsing query." << endl;
  if ( tokens->Peek().Type== NSLib::queryParser::EOF_ ) {
    cerr << "No query given." << endl;
    _THROWC( "No query given.");
  }
  return *MatchQuery(field);
}
    
    
// matches for CONJUNCTION
// CONJUNCTION ::= <AND> | <OR>
int QueryParser::MatchConjunction()
{
  switch(tokens->Peek().Type)
  {
  case NSLib::queryParser::AND_:
    delete &tokens->Extract();
    return CONJ_AND;
  case NSLib::queryParser::OR:
    delete &tokens->Extract();
    return CONJ_OR;
  default:
    return CONJ_NONE;
  }
}
    
// matches for MODIFIER
// MODIFIER ::= <PLUS> | <MINUS> | <NOT>
int QueryParser::MatchModifier()
{
  switch(tokens->Peek().Type)
  {
  case NSLib::queryParser::PLUS:
    delete &tokens->Extract();
    return MOD_REQ;
  case NSLib::queryParser::MINUS:
  case NSLib::queryParser::NOT:
    delete &tokens->Extract();
    return MOD_NOT;
  default:
    return MOD_NONE;
  }
}
    
    
// matches for QUERY
// QUERY ::= [MODIFIER] QueryParser::CLAUSE (<CONJUNCTION> [MODIFIER] CLAUSE)*
Query* QueryParser::MatchQuery(const char_t* field)
{
  VoidList<BooleanClause*> clauses;

  Query* q = NULL;
  Query* firstQuery = NULL;
  int mods = MOD_NONE;
  int conj = CONJ_NONE;
    
  mods = MatchModifier();
    
  // match for CLAUSE
  q = MatchClause(field);

  AddClause(clauses, CONJ_NONE, mods, q);
  if(mods == MOD_NONE)
    firstQuery = q;
    
  // match for CLAUSE*
  while(true)
  {
    if(tokens->Peek().Type == NSLib::queryParser::EOF_)
    {
      delete MatchQueryToken(NSLib::queryParser::EOF_);
      break;
    }
    
    if(tokens->Peek().Type == NSLib::queryParser::RPAREN)
    {
      //MatchQueryToken(NSLib::queryParser::RPAREN);
      break;
    }
    
    conj = MatchConjunction();
    
    mods = MatchModifier();
    q = MatchClause(field);
    AddClause(clauses, conj, mods, q);
  }
    
  // finalize query
  if(clauses.size() == 1 && firstQuery != NULL)
  {
    BooleanClause* c = clauses[0];
    c->deleteQuery=false;
    clauses.clear();
    delete c;
    return firstQuery;
  } 
  else 
  {
    BooleanQuery* query = new BooleanQuery();
    for ( uint i=0;i<clauses.size();i++ )
      query->add(*clauses[i]);
    cerr << "got a BooleanQuery with " << clauses.size() << " clauses." << endl; 
    return query;
  }
}
    
// matches for CLAUSE
// CLAUSE ::= [TERM <COLONQueryParser::>] ( TERM | (<LPAREN> QUERY <RPAREN>))
Query* QueryParser::MatchClause(const char_t* field)
{
  Query* q = NULL;
  const char_t* sfield = field;
  bool delField = false;
      
  // match for [TERM <COLON>]
  QueryToken* term = &tokens->Extract();
  if(term->Type == NSLib::queryParser::TERM && 
    tokens->Peek().Type == NSLib::queryParser::COLON)
  {
    delete MatchQueryToken(NSLib::queryParser::COLON);
    sfield = stringDuplicate(term->Value);
    delField= true;
    delete term;
  } else
    tokens->Push(term);
    
  // match for
  // TERM | (<LPAREN> QUERY <RPAREN>)
  if(tokens->Peek().Type == NSLib::queryParser::LPAREN)
  {
    delete MatchQueryToken(NSLib::queryParser::LPAREN);
    q = MatchQuery(sfield);
    delete MatchQueryToken(NSLib::queryParser::RPAREN);
  } 
  else 
    q = MatchTerm(sfield);
    
  if ( delField )
    delete[] sfield;
  return q;
}
    
// matches for TERM
// TERM ::= TERM | PREFIXTERM | WILDTERM | NUMBER
//      [ <FUZZY> ] [ <CARAT> <NUMBER> [<FUZZY>]]
//
//      QueryParser::| (<RANGEIN> | <RANGEEX>) [<CARAT> <NUMBER>]
//      | <QUOTED> [SLOP] [<CARAT> <NUMBER>]
Query* QueryParser::MatchTerm(const char_t* field)
{
  QueryToken* term = NULL;
  QueryToken* slop = NULL;
  QueryToken* boost = NULL;
  bool prefix = false;
  bool wildcard = false;
  bool fuzzy = false;
  bool rangein = false;
  Query* q = NULL;
    
  term = &tokens->Extract();
  switch(term->Type)
  {
    case NSLib::queryParser::TERM:
    case NSLib::queryParser::NUMBER:
    case NSLib::queryParser::PREFIXTERM:
    case NSLib::queryParser::WILDTERM:
      if(term->Type == NSLib::queryParser::PREFIXTERM)
        prefix = true;
      else if(term->Type == NSLib::queryParser::WILDTERM)
        wildcard = true;
    
      if(tokens->Peek().Type == NSLib::queryParser::FUZZY)
      {
        delete MatchQueryToken(NSLib::queryParser::FUZZY);
        fuzzy = true;
      }
      if(tokens->Peek().Type == NSLib::queryParser::CARAT)
      {
        delete MatchQueryToken(NSLib::queryParser::CARAT);
        boost = MatchQueryToken(NSLib::queryParser::NUMBER);
    
        if(tokens->Peek().Type == NSLib::queryParser::FUZZY)
        {
          delete MatchQueryToken(NSLib::queryParser::FUZZY);
          fuzzy = true;
        }
      }
      if(wildcard){
          Term* t = new Term(field, term->Value);
        q = new WildcardQuery(t);
        t->finalize();
      } else if(prefix){
          char_t* buf = stringDuplicate(term->Value);
        buf[stringLength(buf)-1] = 0;
        Term* t = new Term(field, buf);
        q = new PrefixQuery(t);
        t->finalize();
        delete[] buf; 
      } else if(fuzzy){
        Term* t = new Term(field, term->Value);
          q = new FuzzyQuery(t);
        t->finalize();
      } else
        q = GetFieldQuery(field, analyzer, term->Value);
          
      break;
    
    case NSLib::queryParser::RANGEIN:
    case NSLib::queryParser::RANGEEX:
      {
        if(term->Type == NSLib::queryParser::RANGEIN)
          rangein = true;
  
        if(tokens->Peek().Type == NSLib::queryParser::CARAT)
        {
          delete MatchQueryToken(NSLib::queryParser::CARAT);
          boost = MatchQueryToken(NSLib::queryParser::NUMBER);
        }
  
        char_t* tmp = stringDuplicate ( term->Value +1);
        tmp[stringLength(tmp)-1] = 0;
        q = GetRangeQuery(field, analyzer,  tmp, rangein);
        delete[] tmp;
      }
      break;
    
    case NSLib::queryParser::QUOTED:
      if(tokens->Peek().Type == NSLib::queryParser::SLOP)
      {
        slop = MatchQueryToken(NSLib::queryParser::SLOP);
      }
      if(tokens->Peek().Type == NSLib::queryParser::CARAT)
      {
        delete MatchQueryToken(NSLib::queryParser::CARAT);
        boost = MatchQueryToken(NSLib::queryParser::NUMBER);
      }
    
      //todo: check term->Value+1
      q = GetFieldQuery(field, analyzer, term->Value+1);
      if(slop != NULL && q->instanceOf(_T("PhraseQuery")) )
      {
        try
        {
          int s = stringToInteger(slop->Value+1);
          ((PhraseQuery*)q)->setSlop( s );
        }
        catch(...)
        {
          // ignored
        }
      }
      break;
  } // end of switch
  delete term;
    
  if(boost != NULL)
  {
    float f = 1.0F;
    try
    {
      f = stringToFloat(boost->Value);
      //f = Single.Parse(boost->Value, NumberFormatInfo.InvariantInfo);
    }
    catch(...)
    {
      // ignored
    }
    delete boost;
    q->setBoost( f);
  }
    
  return q;
}
    
// matches for QueryToken of the specified type and returns it
// otherwise Exception throws
QueryToken* QueryParser::MatchQueryToken(QueryTokenTypes expectedType)
{
  if(tokens->Count() == 0)
    QueryParserBase::throwParserException("Error: UnExpected End of Program",' ',0,0);
    
  QueryToken* t = &tokens->Extract();
  if (expectedType != t->Type)
  {
    char buf[200];
    sprintf(buf, "Error: Unexpected QueryToken: %d, expected: %d", t->Type, expectedType);
    delete t;
    QueryParserBase::throwParserException(buf,' ',0,0);
  }
  return t;
}

}}
