#include "StdHeader.h"
#include "PhraseQuery.h"

#include "SearchHeader.h"
#include "Scorer.h"
#include "BooleanQuery.h"
#include "TermQuery.h"

#include "index/Term.h"
#include "index/Terms.h"
#include "index/IndexReader.h"

#include "util/StringBuffer.h"
#include "util/VoidList.h"

#include "ExactPhraseScorer.h"
#include "SloppyPhraseScorer.h"

using namespace NSLib::index;
using namespace NSLib::util;
namespace NSLib{ namespace search{

  PhraseQuery::PhraseQuery():
    idf(0.0f),
    weight(0.0f),
    slop(0)
  {
  }

  PhraseQuery::~PhraseQuery(){
    for (int i=0;i<(int)terms.size();i++ )
      terms[i]->finalize();
  }

  const char_t* PhraseQuery::getQueryName() const{
    return _T("PhraseQuery");
  }

  void PhraseQuery::add(Term* term) {
    if (terms.size() == 0)
      field = term->Field();
    else if ( stringCompare(term->Field(), field) != 0){
      char_t buf[180];
      stringPrintF(buf, _T("All phrase terms must be in the same field: %s"),term->Field());
      _THROWX(buf);
    }
    terms.push_back(term->pointer());
  }

  float PhraseQuery::sumOfSquaredWeights(Searcher& searcher) {
    for (uint i = 0; i < terms.size(); i++)    // sum term IDFs
      idf += Similarity::idf(*terms.at(i), searcher);

    weight = idf * boost;
    return weight * weight;        // square term weights
  }

  void PhraseQuery::normalize(const float norm) {
    weight *= norm;          // normalize for query
    weight *= idf;          // factor from document
  }

  Scorer* PhraseQuery::scorer(IndexReader& reader)  {
    if (terms.size() == 0)        // optimize zero-term case
      return NULL;
    if (terms.size() == 1) {        // optimize one-term case
      Term* term = terms.at(0);
      TermDocs* docs = &reader.termDocs(term);
      if (docs == NULL)
        return NULL;
      return new TermScorer(*docs, reader.getNorms(term->Field()), weight);
    }

    int tpsLength = terms.size();
    TermPositions** tps = new TermPositions*[tpsLength];
    for (uint i = 0; i < terms.size(); i++) {
      TermPositions* p = &reader.termPositions(terms.at(i));
      if (p == NULL)
        return NULL;
      tps[i] = p;
    }

    if (slop == 0)          // optimize exact case
      return new ExactPhraseScorer(tps, tpsLength,reader.getNorms(field), weight);
    else
      return new SloppyPhraseScorer(tps,tpsLength, slop, reader.getNorms(field), weight);

  }

  //added by search highlighter
  void PhraseQuery::getTerms(Term**& ret, int& size)
  {
    size = terms.size();
    ret = new Term*[size];
    for ( int i=0;i<size;i++ )
      ret[i] = terms[i];
  }
  const char_t* PhraseQuery::toString(const char_t* f) {
    StringBuffer buffer;
    if ( stringCompare(field,f)!= 0) {
      buffer.append(field);
      buffer.append( _T(":"));
    }

    buffer.append( _T("\"") );
    for (uint i = 0; i < terms.size(); i++) {
      buffer.append( terms.at(i)->Text() );
      if (i != terms.size()-1)
        buffer.append(_T(" "));
    }
    buffer.append( _T("\"") );

    if (slop != 0) {
      buffer.append(_T("~"));
      buffer.append(slop);
    }

    if (boost != 1.0f) {
      buffer.append(_T("^"));
      buffer.append( boost,10 );
    }

    return buffer.ToString();
  }

}}
