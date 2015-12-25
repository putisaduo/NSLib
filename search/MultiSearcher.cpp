#include "StdHeader.h"
#include "MultiSearcher.h"

#include "SearchHeader.h"
#include "document/Document.h"
#include "index/Term.h"
#include "util/PriorityQueue.h"
#include "HitQueue.h"

using namespace NSLib::index;
using namespace NSLib::util;
using namespace NSLib::document;

namespace NSLib{ namespace search{

/** Creates a searcher which searches <i>searchers</i>. */
MultiSearcher::MultiSearcher(Searcher** _searchers, int _searchersLen):
  searchers(_searchers),
  searchersLen(_searchersLen),
	_maxDoc(0)
{
  starts = new int[_searchersLen + 1];	  // build starts array
  for (int i = 0; i < _searchersLen; i++) {
    starts[i] = _maxDoc;
    _maxDoc += searchers[i]->maxDoc();		  // compute maxDocs
  }
  starts[_searchersLen] = _maxDoc;
}

MultiSearcher::~MultiSearcher()
{
  delete[] starts;
}

/** Frees resources associated with this <code>Searcher</code>. */
void MultiSearcher::close() {
  for (int i = 0; i < searchersLen; i++)
    searchers[i]->close();
}

int MultiSearcher::docFreq(const Term& term) const
{
  int docFreq = 0;
  for (int i = 0; i < searchersLen; i++)
    docFreq += searchers[i]->docFreq(term);
  return docFreq;
}

/** For use by {@link HitCollector} implementations. */
Document& MultiSearcher::doc(const int n)
{
  int i = searcherIndex(n);			  // find searcher index
  return searchers[i]->doc(n - starts[i]);	  // dispatch to searcher
}

/** For use by {@link HitCollector} implementations to identify the
  * index of the sub-searcher that a particular hit came from. */
int MultiSearcher::searcherIndex(int n)
{	  // find searcher for doc n:
  // replace w/ call to Arrays.binarySearch in Java 1.2
  int lo = 0;					  // search starts array
  int hi = searchersLen - 1;		  // for first element less
						// than n, return its index
  while (hi >= lo) {
    int mid = (lo + hi) >> 1;
    int midValue = starts[mid];
    if (n < midValue)
	  hi = mid - 1;
    else if (n > midValue)
	  lo = mid + 1;
    else
	  return mid;
  }
  return hi;
}

int MultiSearcher::maxDoc() const
{
  return _maxDoc;
}

TopDocs& MultiSearcher::Search(Query& query, const Filter* filter, const int nDocs, char_t* wgroupby) 
{
  HitQueue* hq = new HitQueue(nDocs);
  float minScore = 0.0f;
  int totalHits = 0;

  for (int i = 0; i < searchersLen; i++) {  // search each searcher
    TopDocs* docs = &searchers[i]->Search(query, filter, nDocs, wgroupby);
    totalHits += docs->totalHits;		  // update totalHits
    ScoreDoc** scoreDocs = docs->scoreDocs;
	  int scoreDocsLen = docs->scoreDocsLength;
    for (int j = 0; j <scoreDocsLen; j++) { // merge scoreDocs into hq
	    ScoreDoc* scoreDoc = scoreDocs[j];
	    if (scoreDoc->score < minScore)
        break; // no more scores > minScore

      scoreDoc->doc += starts[i];		  // convert doc
    	hq->put(scoreDoc);			  // update hit queue
    	if (hq->Size() > nDocs) {		  // if hit queue overfull
    	  delete hq->pop();				  // remove lowest in hit queue
    	  minScore = ((ScoreDoc*)hq->top())->score; // reset minScore
    	}
    }
	  docs->deleteScoreDocs = false;
	  delete docs;
  }

  string groupby_str;	

  int scoreDocsLen = hq->Size();
  ScoreDoc** scoreDocs = new ScoreDoc*[scoreDocsLen];
  for (int i = hq->Size()-1; i >= 0; i--)	  // put docs in array
    scoreDocs[i] = (ScoreDoc*)hq->pop();

  //cleanup
  delete hq;

  return *new TopDocs(totalHits, scoreDocs,scoreDocsLen, groupby_str);
}

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
void MultiSearcher::Search(Query& query, const Filter* filter, HitCollector& results)
{
  for (int i =  0; i < searchersLen; i++) {
    int start = starts[i];

    results = *new MultiHitCollector(&results,start);
    searchers[i]->Search(query, filter, results );
  }
}
  
  
  
  
MultiHitCollector::MultiHitCollector(HitCollector* _results, int _start):
  results(_results),
  start(_start)
{
}

void MultiHitCollector::collect(const int doc, const float score) 
{
  results->collect(doc + start, score);
}


}}
