#include "StdHeader.h"
#include "BooleanScorer.h"

#include "Scorer.h"
#include "Similarity.h"

#include <typeinfo>
#include <iostream>

using namespace NSLib::util;
namespace NSLib{ namespace search{


BooleanScorer::BooleanScorer():
  coordFactors (NULL),
  maxCoord (1),
  currentDoc ( 0 ),
  requiredMask (0),
  prohibitedMask (0),
  nextMask (1),
  scorers(NULL)
{  
  bucketTable = new BucketTable(*this);
}

BooleanScorer::~BooleanScorer(){
  delete bucketTable; //TODO:
  if ( coordFactors != NULL )
    delete coordFactors;
  if ( scorers != NULL )
    delete scorers;
}


void BooleanScorer::add(Scorer& scorer, const bool required, const bool prohibited) {
  int mask = 0;
  if (required || prohibited) {
    if (nextMask == 0)
      _THROWC( "More than 32 required/prohibited clauses in query.");
    mask = nextMask;
    nextMask = ( nextMask << 1 );
  } else
    mask = 0;

  if (!prohibited)
    maxCoord++;

  if (prohibited)
    prohibitedMask |= mask;        // update prohibited mask
  else if (required)
    requiredMask |= mask;        // update required mask

  //scorer, HitCollector, and scorers is delete in the SubScorer
  scorers = new SubScorer(scorer, required, prohibited,
        bucketTable->newCollector(mask), scorers);
}

void BooleanScorer::computeCoordFactors(){
  coordFactors = new float[maxCoord];
  for (int i = 0; i < maxCoord; i++)
    coordFactors[i] = Similarity::coord(i, maxCoord);
}

void BooleanScorer::score(HitCollector& results, const int maxDoc) {
  if (coordFactors == NULL)
    computeCoordFactors();

  //cerr << "BooleanScorer::score " << currentDoc << ": " << maxDoc << endl;
  while (currentDoc < maxDoc) {
    currentDoc = min(currentDoc+BucketTable::SIZE, maxDoc);
    for (SubScorer* t = scorers; t != NULL; t = t->next)
      t->scorer.score(t->collector, currentDoc);
    bucketTable->collectHits(results);
  }
}
  
SubScorer::SubScorer(Scorer& scr, const bool r, const bool p, HitCollector& c,
                     SubScorer* nxt):
    scorer ( scr ),
    required ( r ),
    prohibited ( p ),
    collector ( c ),
    next ( nxt )
{
}
SubScorer::~SubScorer(){
  delete &scorer;
  delete &collector;
  delete next;
}

Bucket::Bucket():
    doc(-1)
{
}
Bucket::~Bucket(){
  //if ( next != NULL )
  //  delete next;
}


BucketTable::BucketTable(BooleanScorer& scr):
  scorer(scr),
  first(NULL)
{
}

void BucketTable::clear(){
  //delete first;
  first = NULL;
}

void BucketTable::collectHits(HitCollector& results) 
{
  const int required = scorer.requiredMask;
  const int prohibited = scorer.prohibitedMask;
  const float* coord = scorer.coordFactors;

  for (Bucket* bucket = first; bucket!=NULL; bucket = bucket->next) {
    if ((bucket->bits & prohibited) == 0 &&    // check prohibited
        (bucket->bits & required) == required)
    {// check required
      results.collect(bucket->doc,      // add to results
                      bucket->score * coord[bucket->coord]);
    }
  }
  clear(); //first = NULL;          // reset for next round
}

int BucketTable::size() const { return SIZE; }

HitCollector& BucketTable::newCollector(const int mask) {
  //cerr << "\tBucketTable::newCollector " << this->first << endl;
  return *new Collector(mask, *this);
}


Collector::Collector(const int msk, BucketTable& bucketTbl):
  mask(msk),
  bucketTable(bucketTbl)
{
}
  
void Collector::collect(const int doc, const float score) {
  BucketTable& table = bucketTable;
  const int i = doc & BucketTable::MASK;
  Bucket& bucket = table.buckets[i];
  //if (bucket == NULL)
  //  bucket = new Bucket();
  //table.buckets[i] = bucket;
  //cerr << "Collector::collect " << doc << " " << score << endl;
      
  if (bucket.doc != doc) {        // invalid bucket
    bucket.doc = doc;        // set doc
    bucket.score = score;        // initialize score
    bucket.bits = mask;        // initialize mask
    bucket.coord = 1;        // initialize coord

    bucket.next = table.first;      // push onto valid list
    table.first = &bucket;
  } else {            // valid bucket
    bucket.score += score;        // increment score
    bucket.bits |= mask;        // add bits in mask
    bucket.coord++;          // increment coord
  }
}

}}
