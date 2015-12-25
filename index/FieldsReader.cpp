#include "StdHeader.h"
#include "FieldsReader.h"

#include "util/VoidMap.h"
#include "store/Directory.h"
#include "document/Document.h"
#include "document/Field.h"
#include "FieldInfos.h"

#include <iostream>

namespace NSLib{ namespace index {

  //static 
  const fchar_t* FieldsReader::segmentname(const fchar_t* segment, const fchar_t* ext ){
    fchar_t* buf = new fchar_t[MAX_PATH];
    fstringPrintF(buf, CONST_STRING("%s%s"), segment,ext );
    return buf;
  }

  FieldsReader::FieldsReader(Directory& d, const fchar_t* segment, FieldInfos& fn):
    fieldInfos(fn)
  {
    const fchar_t* buf = segmentname(segment,CONST_STRING(".fdt"));
    fieldsStream = &d.openFile( buf );
    delete[] buf;

    buf = segmentname(segment,CONST_STRING(".fdx"));
    indexStream = &d.openFile( buf );
    delete[] buf;

    size = (int)indexStream->Length()/8;
    cerr << "FieldsReader::segmentname fdx size=" << size << endl;
  }

  FieldsReader::~FieldsReader(){
  }
    
  void FieldsReader::close() {
    fieldsStream->close();
    indexStream->close();
    _DELETE(fieldsStream);
    _DELETE(indexStream);
  }

  int FieldsReader::Size() {
    return size;
  }

  NSLib::document::Document& FieldsReader::doc(int n) {
    indexStream->seek(n * 8L);
    long_t position = indexStream->readLong();
    fieldsStream->seek(position);
      
    NSLib::document::Document& doc = *new NSLib::document::Document();
    int numFields = fieldsStream->readVInt();
    for (int i = 0; i < numFields; i++) {
      int fieldNumber = fieldsStream->readVInt();
      FieldInfo& fi = fieldInfos.fieldInfo(fieldNumber);

      l_byte_t bits = fieldsStream->readByte();

      char_t* fvalue = fieldsStream->readString(true);
      NSLib::document::Field& f = *new NSLib::document::Field(
              fi.name,      // name
              fvalue, // read value
              true,        // stored
              fi.isIndexed,      // indexed
              (bits & 1) != 0 );
      delete[] fvalue;

      doc.add( f );    // tokenized
    }

    return doc;
  }

}}
