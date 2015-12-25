#include "StdHeader.h"
#ifndef _NSLib_index_SegmentInfos_
#define _NSLib_index_SegmentInfos_

#include "util/VoidList.h"
#include "store/Directory.h"
#include "SegmentInfo.h"

using namespace NSLib::store;
namespace NSLib{ namespace index {
  class SegmentInfos:public NSLib::util::VoidList<SegmentInfo*> {
  public:

    SegmentInfos();
    ~SegmentInfos();
    
    int counter;        // used to name new segments
      
    SegmentInfo& info(int i);

    void read(Directory& directory);

    void write(Directory& directory);
  };
}}
#endif
