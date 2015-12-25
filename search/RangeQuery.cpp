#include "StdHeader.h"
#include "RangeQuery.h"

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

RangeQuery::RangeQuery(Term* LowerTerm, Term* UpperTerm, const bool Inclusive):
  inclusive(Inclusive),
  query (NULL),
  reader(NULL)
{
  if (LowerTerm == NULL && UpperTerm == NULL)
    _THROWC( "At least one term must be non-NULL");

  if (LowerTerm != NULL)
    lowerTerm = LowerTerm->pointer();
  if (UpperTerm != NULL)
    upperTerm = UpperTerm->pointer();
  
  if (LowerTerm != NULL && UpperTerm != NULL 
      && stringCompare(lowerTerm->Field(), upperTerm->Field()) != 0 )
    _THROWC( "Both terms must be for the same field" );
}
RangeQuery::~RangeQuery(){
  lowerTerm->finalize();
  upperTerm->finalize();
}

const char_t* RangeQuery::getQueryName() const{
  return _T("RangeQuery");
}
    
void RangeQuery::prepare(IndexReader& reader)
{
  this->query = NULL;
  this->reader = &reader;
}
    
float RangeQuery::sumOfSquaredWeights(Searcher& searcher)
{
  return getQuery()->sumOfSquaredWeights(searcher);
}
    
void RangeQuery::normalize(const float norm)
{
  getQuery()->normalize(norm);
}
    
Scorer* RangeQuery::scorer(IndexReader& reader)
{
  return getQuery()->scorer(reader);
}
    
BooleanQuery* RangeQuery::getQuery()
{
  if (query != NULL)
    return query;

  BooleanQuery* q = new BooleanQuery();
  // if we have a lowerTerm, start there. otherwise, start at beginning
  if (lowerTerm == NULL) 
    lowerTerm = new Term(getField(), _T(""));
  TermEnum& _enum = reader->getTerms(lowerTerm);
  _TRY {
    const char_t* lowerText = NULL;
    //char_t* field;
    bool checkLower = false;
                
    if (!inclusive) { // make adjustments to set to exclusive
      if (lowerTerm != NULL) {
        lowerText = lowerTerm->Text();
        checkLower = true;
      }
      if (upperTerm != NULL) {
        // set upperTerm to an actual term in the index
        TermEnum& uppEnum = reader->getTerms(upperTerm);
        upperTerm = uppEnum.getTerm();
      }
    }
    const char_t* testField = getField();
    do {
      Term* term = _enum.getTerm();
      if (term == NULL || stringCompare(term->Field(), testField)!=0 )
        break;

      if (!checkLower || stringCompare(term->Text(),lowerText) > 0) {
        checkLower = false;
        if (upperTerm != NULL) {
          int compare = upperTerm->compareTo( *term );
          // if beyond the upper term, or is exclusive and
          // this is equal to the upper term, break out
          if ((compare < 0) || (!inclusive && compare == 0)) 
            break;
        }
        TermQuery& tq = *new TermQuery(*term);	  // found a match
        tq.setBoost(boost);               // set the boost
        q->add(tq,true, false, false);		// add to q
      }
    } while (_enum.next());
  }_FINALLY ( _enum.close(); delete &_enum;);

  query = q;
  return query;
}

/** Prints a user-readable version of this query. */
const char_t* RangeQuery::toString(const char_t* field)
{
  StringBuffer buffer;
  if ( stringCompare(getField(),field)!=0 )
  {
    buffer.append( getField() );
    buffer.append( _T(":"));
  }
  buffer.append(inclusive ? _T("[") : _T("{"));
  buffer.append(lowerTerm != NULL ? lowerTerm->Text() : _T("NULL"));
  buffer.append(_T("-"));
  buffer.append(upperTerm != NULL ? upperTerm->Text() : _T("NULL"));
  buffer.append(inclusive ? _T("]") : _T("}"));
  if (boost != 1.0f)
  {
    buffer.append( _T("^"));
    buffer.append( boost,4 );
  }
  return buffer.ToString();
}
    
    
const char_t* RangeQuery::getField()
{
  return (lowerTerm != NULL ? lowerTerm->Field() : upperTerm->Field());
}

}}
