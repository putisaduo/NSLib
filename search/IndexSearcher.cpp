#include "StdHeader.h"
#include "IndexSearcher.h"

#include "SearchHeader.h"
#include "store/Directory.h"
#include "document/Document.h"
#include "index/IndexReader.h"
#include "index/Term.h"
#include "util/BitSet.h"
#include "HitQueue.h"
#include "util/CharConverter.h"

#include <map>
#include <string>
#include <iostream>

using namespace NSLib::index;
using namespace NSLib::util;
using namespace NSLib::document;

namespace NSLib{ namespace search {
  
SimpleTopDocsCollector::SimpleTopDocsCollector(const BitSet* bs, HitQueue& hitQueue, 
                         int* totalhits,const int ndocs, const float minScore):
  bits(bs),
  hq(hitQueue),
  nDocs(ndocs),
  totalHits(totalhits),
  ms(minScore)
{
}

void SimpleTopDocsCollector::collect(const int doc, const float score) {
  if (score > 0.0f &&        // ignore zeroed buckets
    (bits==NULL || bits->get(doc))) 
  {    // skip docs not in bits
    totalHits[0]++;
    if (score >= ms) {
      hq.put(new ScoreDoc(doc, score));    // update hit queue
      if (hq.Size() > nDocs) {      // if hit queue overfull
        delete hq.pop();        // remove lowest in hit queue
        minScore = hq.top()->score; // reset minScore
      }
    }
  }
}
  
SimpleFilteredCollector::SimpleFilteredCollector(BitSet* bs, HitCollector& collector): 
  bits(bs),
  results(collector)
{
}

void SimpleFilteredCollector::collect(const int doc, const float score) {
  cerr<< score << " " << bits << " collector::collect " << endl;
  if (bits->get(doc)) {      // skip docs not in bits
    results.collect(doc, score);
  }
}


/** Creates a searcher searching the index in the named directory.  */
IndexSearcher::IndexSearcher(const fchar_t* path):
  reader( IndexReader::open(path) ),
  readerOwner(true)
{
}
      
/** Creates a searcher searching the provided index. */
IndexSearcher::IndexSearcher(IndexReader& r):
  reader(r),
  readerOwner(false)
{
}

IndexSearcher::~IndexSearcher(){
}
      
/** Frees resources associated with this Searcher. */
void IndexSearcher::close(){
  if ( readerOwner ){
    reader.close();
    delete &reader;
  }
}

int IndexSearcher::docFreq(const Term& term) const{
  return reader.docFreq(term);
}

/** For use by {@link HitCollector} implementations. */
Document& IndexSearcher::doc(const int i) {
  return reader.document(i);
}

int IndexSearcher::maxDoc() const {
  return reader.MaxDoc();
}

TopDocs& IndexSearcher::Search(Query& query, const Filter* filter,
                               const int nDocs, char_t* wgroupby)
{
  cerr << "IndexSearcher::Search" << endl;
  Scorer* scorer = Query::scorer(query, *this, reader);
  if (scorer == NULL)
    return *new TopDocs(0, new ScoreDoc*[0], 0, "");

  const BitSet* bits = filter != NULL ? filter->bits(reader) : NULL;
  HitQueue& hq = *new HitQueue(nDocs);
  int* totalHits = new int[1];
  totalHits[0] = 0;
    
  typedef std::map<std::string, int> GroupbyMap;
  GroupbyMap groupby;

  SimpleTopDocsCollector hitCol(bits, hq, totalHits, nDocs, 0.0f);
  scorer->score( hitCol, reader.MaxDoc());

  ScoreDoc** scoreDocs = new ScoreDoc*[hq.Size()];
  int scoreDocsLength = hq.Size();
  std::cerr << " search got " << scoreDocsLength << " results"<< endl;

  for (int i = hq.Size()-1; i >= 0; i--) {  // put docs in array
    scoreDocs[i] = hq.pop();
    if (wgroupby==NULL) 
      continue;
    Field* field = NULL;
    Document& doc = reader.document(scoreDocs[i]->doc);
    if (doc.getField(wgroupby, field)) {
      const char_t* v = field->StringValue();
      char* value = NSLib::util::CharConverter::wideToChar(v, "English");
      //std::string value = v;
      if (groupby.find(value) == groupby.end())
        groupby[value] = 1;
      else
        groupby[value] = groupby[value] + 1;

      delete[] value;
    }
    delete &doc;
  }

  string groupby_str;  
  for (GroupbyMap::iterator it = groupby.begin(); it!=groupby.end(); ++it) {
    //"1928": "39", "1929": "56"
    groupby_str += groupby_str.size()>0 ? ", " : "";
    char tmp[1024];
    sprintf(tmp, "\"%s\": \"%d\"", it->first.c_str(), it->second); 
    groupby_str += tmp;
  }

  int totalHitsInt = totalHits[0];
  delete &hq;
  delete bits;
  delete[] totalHits;
  delete scorer;

  return *new TopDocs(totalHitsInt, scoreDocs, scoreDocsLength, groupby_str);
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
* @param filter if non-NULL, a bitset used to eliminate some documents
* @param results to receive hits
*/
void IndexSearcher::Search(Query& query, const Filter* filter, HitCollector& results)
{
  BitSet* bs = NULL;
  HitCollector& collector = results;
  if (filter != NULL) {
    bs = filter->bits(reader);
    SimpleFilteredCollector fc(bs,results);
    collector = fc;
  }

  Scorer* scorer = Query::scorer(query, *this, reader);
  if (scorer == NULL){
    delete bs;    
    return;
  }

  scorer->score(collector, reader.MaxDoc());
  delete bs;
}

}}
