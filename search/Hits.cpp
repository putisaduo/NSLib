#include "SearchHeader.h"

#include "StdHeader.h"
#include "document/Document.h"
#include "index/IndexReader.h"
#include "Filter.h"
#include "ScoreDoc.h"
#include "TopDocs.h"
#include "search/SearchHeader.h"

using namespace NSLib::document;
using namespace NSLib::util;
using namespace NSLib::index;

namespace NSLib{ namespace search {

HitDoc::HitDoc(const float s, const int i):
  next(NULL),
  prev(NULL),
  doc(NULL),
  score(s),
  id(i)
{
}

HitDoc::~HitDoc(){
  delete doc;
}

Hits::Hits(Searcher& s, Query& q, const Filter* f, char_t* wgroupby):
  query(q),
  searcher(s),
  filter(f),
  wgroupby(wgroupby),
  length(0),
  groupby_str(""),
  first(NULL),
  last(NULL),
  numDocs(0),
  maxDocs(5000)
{
  hitDocs.setDoDelete(NSLib::util::DELETE_TYPE_DELETE);
  getMoreDocs(2500);          // retrieve 100 initially
}

Hits::~Hits(){
  //delete &query;
  //delete &searcher;
  //searcher.close();
}

int Hits::Length() const {
  return length;
}

const char* Hits::GroupbyResult() const
{
  return groupby_str.c_str();
}

Document& Hits::doc(const int n){
  HitDoc& hitDoc = getHitDoc(n);

  // Update LRU cache of documents
  remove(hitDoc);          // remove from list, if there
  addToFront(hitDoc);          // add to front of list
  if (numDocs > maxDocs) {        // if cache is full
    HitDoc* oldLast = last;
    remove(*last);          // flush last

    delete oldLast->doc;
    oldLast->doc = NULL;
  }

  if (hitDoc.doc == NULL)
    hitDoc.doc = &searcher.doc(hitDoc.id);    // cache miss: read document
          
  return *hitDoc.doc;
}
    
int Hits::id (const int n){
  return getHitDoc(n).id;
}

float Hits::score(const int n){
  return getHitDoc(n).score;
}

void Hits::getMoreDocs(const int Min){
  int min = Min;
  if ((int)hitDocs.size() > min)
    min = hitDocs.size();

  int n = min * 2;          // double # retrieved
  TopDocs& topDocs = searcher.Search(query, filter, n, wgroupby);
  length = topDocs.totalHits;
  groupby_str = topDocs.groupby_str;
  ScoreDoc** scoreDocs = topDocs.scoreDocs;
  int scoreDocsLength = topDocs.scoreDocsLength;

  float scoreNorm = 1.0f;
  if (length > 0 && scoreDocs[0]->score > 1.0f)
    scoreNorm = 1.0f / scoreDocs[0]->score;

  int end = scoreDocsLength < length ? scoreDocsLength : length;
  for (int i = hitDocs.size(); i < end; i++)
    hitDocs.push_back(new HitDoc(scoreDocs[i]->score*scoreNorm, scoreDocs[i]->doc));
    
  delete &topDocs;
}

HitDoc& Hits::getHitDoc(const int n){
  if (n >= length){
    char_t buf[100];
    stringPrintF(buf, _T("Not a valid hit number: %d"), n);
    _THROWX( buf );
  }
  if (n >= (int)hitDocs.size())
    getMoreDocs(n);

  return *((HitDoc*)(hitDocs.at(n)));
}

// insert at front of cache
void Hits::addToFront(HitDoc& hitDoc)
{
  if (first == NULL)
    last = &hitDoc;
  else
    first->prev = &hitDoc;
      
  hitDoc.next = first;
  first = &hitDoc;
  hitDoc.prev = NULL;

  numDocs++;
}

void Hits::remove(HitDoc& hitDoc)
{    // remove from cache
  if (hitDoc.doc == NULL)  // it's not in the list
    return;            // abort

  if (hitDoc.next == NULL)
    last = hitDoc.prev;
  else
    hitDoc.next->prev = hitDoc.prev;
      
  if (hitDoc.prev == NULL)
    first = hitDoc.next;
  else
    hitDoc.prev->next = hitDoc.next;

  numDocs--;
}

}}
