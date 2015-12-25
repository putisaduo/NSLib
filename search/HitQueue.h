#include "StdHeader.h"
#ifndef _NSLib_search_HitQueue_
#define _NSLib_search_HitQueue_

#include "util/PriorityQueue.h"
#include "ScoreDoc.h"

namespace NSLib{ namespace search {

class HitQueue: public NSLib::util::PriorityQueue<ScoreDoc*> {
public:
  HitQueue(const int size);
  ~HitQueue();

protected:
  bool lessThan(ScoreDoc* hitA, ScoreDoc* hitB);
};

}}
#endif
