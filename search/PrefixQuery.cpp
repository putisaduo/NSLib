#include "StdHeader.h"
#include "PrefixQuery.h"

using namespace NSLib::index;
namespace NSLib{ namespace search{ 


  /** Constructs a query for terms starting with <code>prefix</code>. */
  PrefixQuery::PrefixQuery(Term* Prefix):
    prefix(Prefix->pointer())
  {
    //this.reader = reader; //again, why?
  }
  PrefixQuery::~PrefixQuery(){
    prefix->finalize();
  }
  const char_t* PrefixQuery::getQueryName()const{
    return _T("PrefixQuery");
  }

  void PrefixQuery::prepare(IndexReader& reader) {
    query = NULL;
    this->reader = &reader;
  }

  float PrefixQuery::sumOfSquaredWeights(Searcher& searcher){
    return getQuery()->sumOfSquaredWeights( searcher );
  }

  void PrefixQuery::normalize(const float norm) {
    //try {
      getQuery()->normalize(norm);
    //} catch (IOException e) {
    //throw new RuntimeException(e.toString());
    //}
  }

  Scorer* PrefixQuery::scorer(IndexReader& reader) {
    return getQuery()->scorer(reader);
  }

  BooleanQuery* PrefixQuery::getQuery() {
    if (query == NULL) {
      BooleanQuery* q = new BooleanQuery();
      TermEnum& _enum = reader->getTerms(prefix);
      _TRY {
        const char_t* prefixText = prefix->Text();
        const char_t* prefixField = prefix->Field();
        do {
          Term* term = _enum.getTerm();
          //if (term!=NULL && stringFind(term->Text(),prefixText)==term->Text()
          if (term!=NULL && stringFind(term->Text(),prefixText)==0
              && stringCompare(term->Field(), prefixField)==0)
          {
            TermQuery* tq = new TermQuery(*term);    // found a match
            tq->setBoost(boost);        // set the boost
            q->add(*tq,true, false, false);      // add to q
          } else {
            break;
          }
        } while (_enum.next());
      }_FINALLY ( _enum.close() );
      
      query = q;
    }
    return query;
  }

  /** Prints a user-readable version of this query. */
  const char_t* PrefixQuery::toString(const char_t* field) {
    NSLib::util::StringBuffer buffer;
    if ( stringCompare(prefix->Field(),field) != 0 ) {
      buffer.append(prefix->Field());
      buffer.append(_T(":") );
    }
    buffer.append(prefix->Text());
    buffer.append(_T("*"));
    if (boost != 1.0f) {
      buffer.append(_T("^"));
      buffer.append( boost,10);
    }
    return buffer.ToString();
  }

}}
