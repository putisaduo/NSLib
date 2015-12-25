#include "StdHeader.h"
#ifndef _NSLib_search_BooleanScorer_
#define _NSLib_search_BooleanScorer_

#include "HitCollector.h"
#include "Scorer.h"

namespace NSLib{ namespace search{
  

//predefine for use in BooleanScorer class NSLIB_EXPORT.
class BucketTable;

struct Bucket {
  int  doc;          // tells if bucket is valid
  float  score;          // incremental score
  int  bits;            // used for bool constraints
  int  coord;            // count of terms in score
  Bucket*  next;          // next valid bucket

  Bucket();
  ~Bucket();
};

struct SubScorer {
public:
  Scorer& scorer;
  bool required;
  bool prohibited;
  HitCollector& collector;
  SubScorer* next;
  SubScorer(Scorer& scr, const bool r, const bool p, HitCollector& c, SubScorer* nxt);
  ~SubScorer();
};

class BooleanScorer: public Scorer {
private:
  int currentDoc;

  SubScorer* scorers;
  BucketTable* bucketTable;

  int maxCoord;
  int nextMask;

public:
  int requiredMask;
  int prohibitedMask;
  float* coordFactors;

  BooleanScorer();
  ~BooleanScorer();
  void add(Scorer& scorer, const bool required, const bool prohibited);

private:
  void computeCoordFactors();
  void score(HitCollector& results, const int maxDoc);
};

class BucketTable {    
private:
  BooleanScorer& scorer;

public:
  const static int SIZE = 1 << 10;
  const static int MASK = SIZE - 1;
  Bucket buckets[SIZE];

  BucketTable(BooleanScorer& scr);
  void collectHits(HitCollector& results);
  int size() const;
  HitCollector& newCollector(const int mask);

  Bucket* first;        // head of valid list

  void clear();
  ~BucketTable(){
    clear();
  }
};

class Collector: public virtual HitCollector {
private:
  BucketTable& bucketTable;
  int mask;
public:
  Collector(const int mask, BucketTable& bucketTable);

  void collect(const int doc, const float score);
};

}}
#endif
