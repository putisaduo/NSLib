#include "StdHeader.h"
#ifndef _NSLib_index_FieldsReader_
#define _NSLib_index_FieldsReader_

#include "util/VoidMap.h"
#include "store/Directory.h"
#include "document/Document.h"
#include "document/Field.h"
#include "FieldInfos.h"

namespace NSLib{ namespace index {
  class FieldsReader {
  private:
    const FieldInfos& fieldInfos;
    NSLib::store::InputStream* fieldsStream;
    NSLib::store::InputStream* indexStream;
    int size;
    const static fchar_t* segmentname(const fchar_t* segment, const fchar_t* ext );
  public:
    FieldsReader(Directory& d, const fchar_t* segment, FieldInfos& fn);
    ~FieldsReader();

    void close();

    int Size();

    NSLib::document::Document& doc(int n);
  };
}}
#endif
