#include "StdHeader.h"
#include "HitQueue.h"

#include "util/PriorityQueue.h"
#include "ScoreDoc.h"

namespace NSLib{ namespace search {
  
HitQueue::HitQueue(const int size) {
  initialize(size,true);
}

HitQueue::~HitQueue(){
}

bool HitQueue::lessThan(ScoreDoc* hitA, ScoreDoc* hitB) {
  if (hitA->score == hitB->score)
    return hitA->doc > hitB->doc; 
  else
    return hitA->score < hitB->score;
}

}}
