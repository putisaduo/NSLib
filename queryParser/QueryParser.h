#include "StdHeader.h"
#ifndef _NSLib_queryParser_QueryParser_
#define _NSLib_queryParser_QueryParser_

#include "QueryParserConstants.h"
#include "analysis/AnalysisHeader.h"
#include "util/Reader.h"
#include "search/SearchHeader.h"
#include "index/Term.h"

#include "TokenList.h"
#include "QueryToken.h"
#include "QueryParserBase.h"
#include "Lexer.h"


using namespace NSLib::util;
using namespace NSLib::index;
using namespace NSLib::analysis;
using namespace NSLib::search;
namespace NSLib{ namespace queryParser{
  // <p>It's a query parser.
  // The only method that clients should need to call is Parse().
  // The syntax for query const char_t*s is as follows:
  // A Query is a series of clauses. A clause may be prefixed by:</p>
  // <ul>
  //  <li>a plus (+) or a minus (-) sign, indicating that the 
  //  clause is required or prohibited respectively; or</li>
  //  <li>a term followed by a colon, indicating the field to be searched.
  //  This enables one to construct queries which search multiple fields.</li>
  //  </ul>
  //  <p>
  //  A clause may be either:</p>
  //  <ul>
  //  <li>a term, indicating all the documents that contain this term; or</li>
  //  <li>a nested query, enclosed in parentheses. Note that this may be 
  //  used with a +/- prefix to require any of a set of terms.</li>
  //  </ul>
  //  <p>
  // Thus, in BNF, the query grammar is:</p>
  //  <code>
  //  Query  ::= ( Clause )*
  //  Clause ::= ["+", "-"] [&lt;TERM&gt; ":"] ( &lt;TERM&gt; | "(" Query ")" )
  //  </code>
  //  <p>
  //  Examples of appropriately formatted queries can be found in the test cases.
  //  </p>
  //
  class QueryParser : public QueryParserBase
  {
  private:
    Analyzer& analyzer;
    const char_t* field;
    TokenList* tokens;
  
  public:
    // Initializes a new instance of the QueryParser class with a specified field and
    // analyzer values.
    QueryParser(const char_t* _field, Analyzer& _analyzer);
    ~QueryParser();

    // Returns a new instance of the QueryParser class with a specified query, field and
    // analyzer values.
    // <param name="query">The query to parse.</param>
    // <param name="field">The default field for query terms.</param>
    // <param name="analyzer">Used to find terms in the query text.</param>
    // <returns></returns>
    static Query& Parse(const char_t* query, const char_t* field, Analyzer& analyzer);

    // Returns a parsed Query instance.
    // <param name="query">The query value to be parsed.</param>
    // <returns>A parsed Query instance.</returns>
    Query& Parse(const char_t* query);

    // Returns a parsed Query instance.
    // <param name="reader">The TextReader value to be parsed.</param>
    // <returns>A parsed Query instance.</returns>
    Query& Parse(BasicReader& reader);


  private:
    // matches for CONJUNCTION
    // CONJUNCTION ::= <AND> | <OR>
    int MatchConjunction();

    // matches for MODIFIER
    // MODIFIER ::= <PLUS> | <MINUS> | <NOT>
    int MatchModifier();


    // matches for QUERY
    // QUERY ::= [MODIFIER] CLAUSE (<CONJUNCTION> [MODIFIER] CLAUSE)*
    Query* MatchQuery(const char_t* field);

    // matches for CLAUSE
    // CLAUSE ::= [TERM <COLON>] ( TERM | (<LPAREN> QUERY <RPAREN>))
    Query* MatchClause(const char_t* field);

    // matches for TERM
    // TERM ::= TERM | PREFIXTERM | WILDTERM | NUMBER
    //      [ <FUZZY> ] [ <CARAT> <NUMBER> [<FUZZY>]]
    //
    //      | (<RANGEIN> | <RANGEEX>) [<CARAT> <NUMBER>]
    //      | <QUOTED> [SLOP] [<CARAT> <NUMBER>]
    Query* MatchTerm(const char_t* field);

    // matches for QueryToken of the specified type and returns it
    // otherwise Exception throws
    QueryToken* MatchQueryToken(QueryTokenTypes expectedType);
  };
}}
#endif
