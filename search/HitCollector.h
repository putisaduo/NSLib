#include "StdHeader.h"
#ifndef _NSLib_search_HitCollector_
#define _NSLib_search_HitCollector_


namespace NSLib{ namespace search {

// Lower-level search API.
// @see Searcher#search(Query,HitCollector)
class HitCollector {
public:
  // Called once for every non-zero scoring document, with the document number
  //and its score.
  //
  //<P>If, for example, an application wished to collect all of the hits for a
  //query in a BitSet, then it might:<pre>
  //  Searcher = new IndexSearcher(indexReader);
  //  final BitSet bits = new BitSet(indexReader.maxDoc());
  //  searcher.search(query, new HitCollector() {
  //      public void collect(int doc, float score) {
  //        bits.set(doc);
  //      }
  //    });
  //</pre>
  //
  //<p>Note: This is called in an inner search loop.  For good search
  //performance, implementations of this method should not call {@link
  //Searcher#doc(int)} or {@link
  //org.apache.NSLib.index.IndexReader#document(int)} on every document
  //number encountered.  Doing so can slow searches by an order of magnitude
  //or more. 
  virtual void collect(const int doc, const float score) = 0;
  virtual ~HitCollector(){}
};

}}
#endif
