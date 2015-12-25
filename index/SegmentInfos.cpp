#include "StdHeader.h"
#include "SegmentInfos.h"

#include "store/Directory.h"
#include "SegmentInfo.h"
#include "util/CharConverter.h"

#include <iostream>

using namespace std;

namespace NSLib{ namespace index {

  SegmentInfos::SegmentInfos()
    : util::VoidList<SegmentInfo*>(true, util::DELETE_TYPE_DELETE)
  {
    counter = 0;
  }
    
  SegmentInfo& SegmentInfos::info(int i) {
    return *at(i);
  }
  SegmentInfos::~SegmentInfos(){
    clear();
  }

  void SegmentInfos::read(Directory& directory){
    InputStream& input = directory.openFile(CONST_STRING("segments"));
    cerr << " directory.openFile(segments)" << endl;
    _TRY {
      counter = input.readInt();      // read counter
      cerr << " directory.openFile(segments) read counter" << endl;
      for (int i = input.readInt(); i > 0; i--) { // read segmentInfos
        fchar_t* name = (char*)input.readByteString();
        SegmentInfo* si = new SegmentInfo(name, input.readInt(), directory);
        push_back(si);
        cerr << " segment name " << i << " is " << name << endl;
        delete[] name;
      }
    } _FINALLY(
      input.close();
      delete &input;
    );
  }

  void SegmentInfos::write(Directory& directory){
    OutputStream& output = directory.createFile(CONST_STRING("segments.new"));
    _TRY {
      output.writeInt(counter);        // write counter
      output.writeInt(size());        // write infos
      for (uint i = 0; i < size(); i++) {
        SegmentInfo& si = info(i);
        output.writeString( util::CharConverter::charToWide(si.name, LANG_ENG));
        output.writeInt(si.docCount);
      }
    } _FINALLY(
        output.close();
        delete &output;
    );

    // install new segment info
    directory.renameFile(CONST_STRING("segments.new"), CONST_STRING("segments"));
  }

}}
