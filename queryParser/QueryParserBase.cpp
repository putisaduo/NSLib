#include "StdHeader.h"
#include "QueryParserBase.h"

#include "search/BooleanClause.h"
#include "util/VoidList.h"
#include "analysis/AnalysisHeader.h"
#include "analysis/Analyzers.h"
#include "index/Term.h"
#include "search/TermQuery.h"
#include "search/PhraseQuery.h"
#include "search/RangeQuery.h"

using namespace NSLib::search;
using namespace NSLib::util;
using namespace NSLib::analysis;
using namespace NSLib::index;

namespace NSLib{ namespace queryParser{

QueryParserBase::QueryParserBase():
  PhraseSlop(0)
{
}
    
QueryParserBase::~QueryParserBase(){
}
    
    
// Adds the next parsed clause.
void QueryParserBase::AddClause(VoidList<BooleanClause*>& clauses, int conj, int mods, Query* q) 
{
  bool required;
  bool prohibited;
    
  // If this term is introduced by AND, make the preceding term required,
  // unless it's already prohibited
  if (conj == CONJ_AND) 
  {
    BooleanClause* c = clauses[clauses.size()-1];
    if (!c->prohibited)
      c->required = true;
  }
    
  // We might have been passed a NULL query; the term might have been
  // filtered away by the analyzer. 
  if (q == NULL)
    return;
    
  // We set REQUIRED if we're introduced by AND or +; PROHIBITED if
  // introduced by NOT or -; make sure not to set both.
  prohibited = (mods == MOD_NOT);
  required = (mods == MOD_REQ);
  if (conj == CONJ_AND && !prohibited)
    required = true;
        
  clauses.push_back(new BooleanClause(*q,true, required, prohibited));
}

void QueryParserBase::throwParserException(const char* message, char ch, int col, int line )
{
  char msg[1024];
  memset(msg,0,1024*sizeof(char));
  sprintf(msg, message,ch,col,line);  
  throw exception( );
}
    
// Returns a query for the specified field.
Query* QueryParserBase::GetFieldQuery(const char_t* field, Analyzer& analyzer, const char_t* queryText) 
{
  // Use the analyzer to get all the tokens, and then build a TermQuery,
  // PhraseQuery, or nothing based on the term count
    
  StringReader reader(queryText);
  TokenStream& source = analyzer.tokenStream(field, &reader);
  StringArrayConst v;
  v.setDoDelete(NSLib::util::DELETE_TYPE_DELETE_ARRAY);
      
  Token* t = NULL;
  while (true) {
    try {
      t = source.next();
    } catch(...) {
      t = NULL;
    }
        
    if (t == NULL)
      break;
        
    v.push_back( stringDuplicate(t->TermText()));
    delete t;
  }
  if (v.size() == 0){
    delete &source;
    return NULL;
  } else if (v.size() == 1){
    Term* t = new Term(field, v[0]);
    Query* ret = new TermQuery( *t );
    t->finalize();
    
    delete &source;
    return ret;
  } else {
    PhraseQuery* q = new PhraseQuery;
    q->setSlop(PhraseSlop);
        
    StringArrayConstIterator itr = v.begin();
    while ( itr != v.end() ) {
      const char_t* data = *itr;
      Term* t = new Term(field, data);
      q->add(t);
      t->finalize();
      itr++;
    }
    delete &source;
    return q;
  }
}
    
// Returns a range query.
Query* QueryParserBase::GetRangeQuery(const char_t* field, Analyzer& analyzer, const char_t* queryText, bool inclusive)
{
  // Use the analyzer to get all the tokens.  There should be 1 or 2.
  BasicReader* reader = new StringReader(queryText);
  TokenStream& source = analyzer.tokenStream(field, reader);
      
  Term* terms[2];
  Token* t;
    
  for (int i = 0; i < 2; i++) {
    try {
      t = source.next();
    } catch (...) {
      t = NULL;
    }
    if (t != NULL) {
      const char_t* text = t->TermText();
      terms[i] = new Term(field, text);
      delete t;
    } else 
      terms[i] = new Term(field, _T(""));
  }
  Query* ret = new RangeQuery(terms[0], terms[1], inclusive);
  terms[0]->finalize();
  terms[1]->finalize();
  delete &source;
  delete reader;

  return ret;
}

}}
