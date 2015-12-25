#include "StdHeader.h"
#ifndef _NSLib_search_RangeQuery_
#define _NSLib_search_RangeQuery_

#include "SearchHeader.h"
#include "Scorer.h"
#include "BooleanQuery.h"
#include "TermQuery.h"

#include "index/Term.h"
#include "index/Terms.h"
#include "index/IndexReader.h"

#include "util/StringBuffer.h"


using namespace NSLib::index;
using namespace NSLib::util;
namespace NSLib{ namespace search{

// A Query that matches documents within an exclusive range. 
class RangeQuery: public Query
{
private: 
  Term* lowerTerm;
  Term* upperTerm;
  bool inclusive;
  IndexReader* reader;
  BooleanQuery* query;

public:
  // Constructs a query selecting all terms greater than 
  // <code>lowerTerm</code> but less than <code>upperTerm</code>.
  // There must be at least one term and either term may be NULL--
  // in which case there is no bound on that side, but if there are 
  // two term, both terms <b>must</b> be for the same field.
  RangeQuery(Term* LowerTerm, Term* UpperTerm, const bool Inclusive);
  ~RangeQuery();

  const char_t* getQueryName() const;

  void prepare(IndexReader& reader);

  float sumOfSquaredWeights(Searcher& searcher);

  void normalize(const float norm);

  Scorer* scorer(IndexReader& reader);

  BooleanQuery* getQuery();
        
  // Prints a user-readable version of this query. 
  const char_t* toString(const char_t* field);
        
private:
  const char_t* getField();
};

}}
#endif
