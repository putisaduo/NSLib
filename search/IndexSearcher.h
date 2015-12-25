#include "StdHeader.h"
#ifndef _NSLib_search_IndexSearcher_
#define _NSLib_search_IndexSearcher_

#include "SearchHeader.h"
#include "store/Directory.h"
#include "document/Document.h"
#include "index/IndexReader.h"
#include "index/Term.h"
#include "util/BitSet.h"
#include "HitQueue.h"

using namespace NSLib::index;
using namespace NSLib::util;
using namespace NSLib::document;

namespace NSLib{ namespace search {

class SimpleTopDocsCollector:public HitCollector{ 
private:
  float minScore;
  const BitSet* bits;
  HitQueue& hq;
  const int nDocs;
  int* totalHits;
  const float ms;
public:
  SimpleTopDocsCollector(const BitSet* bs, HitQueue& hitQueue, int* totalhits,
                         const int ndocs, const float minScore);

  void collect(const int doc, const float score);
};

class SimpleFilteredCollector:  public HitCollector{
private:
  BitSet* bits;
  HitCollector& results;
public:
  SimpleFilteredCollector(BitSet* bs, HitCollector& collector);
protected:
  void collect(const int doc, const float score);
};

// Implements search over a single IndexReader. 
class IndexSearcher:public Searcher{
private:
  IndexReader& reader;
  bool readerOwner;

public:
  // Creates a searcher searching the index in the named directory.
  IndexSearcher(const fchar_t* path);

  // Creates a searcher searching the provided index. 
  IndexSearcher(IndexReader& r);

  ~IndexSearcher();
        
  // Frees resources associated with this Searcher. 
  void close();

  int docFreq(const Term& term) const;

  // For use by {@link HitCollector} implementations. 
  Document& doc(const int i);

  int maxDoc() const;

  TopDocs& Search(Query& query, const Filter* filter, const int nDocs,
                  char_t* wgroupby);

  // Lower-level search API.
  //
  // <p>{@link HitCollector#collect(int,float)} is called for every non-zero
  // scoring document.
  //
  // <p>Applications should only use this if they need <i>all</i> of the
  // matching documents.  The high-level search API ({@link
  // Searcher#search(Query)}) is usually more efficient, as it skips
  // non-high-scoring hits.
  //
  // @param query to match documents
  // @param filter if non-NULL, a bitset used to eliminate some documents
  // @param results to receive hits
  void Search(Query& query, const Filter* filter, HitCollector& results);
};

}}
#endif
