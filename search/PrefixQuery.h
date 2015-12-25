#include "StdHeader.h"
#ifndef _NSLib_search_PrefixQuery
#define _NSLib_search_PrefixQuery

#include "index/Term.h"
#include "index/Terms.h"
#include "index/IndexReader.h"
#include "SearchHeader.h"
#include "BooleanQuery.h"
#include "TermQuery.h"
#include "util/StringBuffer.h"

using namespace NSLib::index;
namespace NSLib{ namespace search{ 
  /** A Query that matches documents containing terms with a specified prefix. */
  class PrefixQuery: public Query {
  private:
    Term* prefix;
    BooleanQuery* query;
    IndexReader* reader;
  public:

    /** Constructs a query for terms starting with <code>prefix</code>. */
    PrefixQuery(Term* Prefix);
    ~PrefixQuery();
    const char_t* getQueryName()const;

    void prepare(IndexReader& reader) ;

    float sumOfSquaredWeights(Searcher& searcher);

    void normalize(const float norm) ;

    Scorer* scorer(IndexReader& reader) ;

    BooleanQuery* getQuery() ;

    /** Prints a user-readable version of this query. */
    const char_t* toString(const char_t* field) ;
  };
}}
#endif
