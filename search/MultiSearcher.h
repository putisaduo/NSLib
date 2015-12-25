#include "StdHeader.h"

#include "SearchHeader.h"
#include "document/Document.h"
#include "index/Term.h"
#include "util/PriorityQueue.h"
#include "HitQueue.h"

using namespace NSLib::index;
using namespace NSLib::util;
using namespace NSLib::document;
namespace NSLib{ namespace search{
    
    class MultiHitCollector: public HitCollector{
    private:
      HitCollector* results;
      int start;
    public: 
      MultiHitCollector(HitCollector* _results, int _start);
      void collect(const int doc, const float score) ;
    };
    
    
    /** Implements search over a set of <code>Searchers</code>. */
    class MultiSearcher: public Searcher {
    private:
        Searcher** searchers;
        int searchersLen;
        int* starts;
        int _maxDoc;
    
    public:
      /** Creates a searcher which searches <i>searchers</i>. */
      MultiSearcher(Searcher** _searchers, int _searchersLen);
      ~MultiSearcher();
    
      /** Frees resources associated with this <code>Searcher</code>. */
      void close() ;
    
      int docFreq(const Term& term) const ;
    
      /** For use by {@link HitCollector} implementations. */
      Document& doc(const int n) ;
    
      /** For use by {@link HitCollector} implementations to identify the
       * index of the sub-searcher that a particular hit came from. */
      int searcherIndex(int n) ;
    
      int maxDoc() const;
    
      TopDocs& Search(Query& query, const Filter* filter, const int nDocs, char_t* wgroupby) ;
    
    
      /** Lower-level search API.
       *
       * <p>{@link HitCollector#collect(int,float)} is called for every non-zero
       * scoring document.
       *
       * <p>Applications should only use this if they need <i>all</i> of the
       * matching documents.  The high-level search API ({@link
       * Searcher#search(Query)}) is usually more efficient, as it skips
       * non-high-scoring hits.
       *
       * @param query to match documents
       * @param filter if non-null, a bitset used to eliminate some documents
       * @param results to receive hits
       */
      void Search(Query& query, const Filter* filter, HitCollector& results);
    };

}}
