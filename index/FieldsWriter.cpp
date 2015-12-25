#include "StdHeader.h"
#include "FieldsWriter.h"

#include "util/VoidMap.h"
#include "util/Reader.h"
#include "store/Directory.h"
#include "store/OutputStream.h"
#include "document/Document.h"
#include "document/Field.h"
#include "FieldInfos.h"

namespace NSLib{ namespace index {
  
  //static 
  const fchar_t* FieldsWriter::segmentname( const fchar_t* segment, const fchar_t* ext ){
    fchar_t* buf = new fchar_t[MAX_PATH];
    fstringPrintF(buf, CONST_STRING("%s%s"), segment,ext );
    return buf;
  }

  FieldsWriter::FieldsWriter(Directory& d, const fchar_t* segment, FieldInfos& fn):
    fieldInfos(fn)
  {
          const fchar_t* buf = segmentname(segment,CONST_STRING(".fdt"));
          fieldsStream = &d.createFile ( buf );
          delete[] buf;
      
          buf = segmentname(segment,CONST_STRING(".fdx"));
          indexStream = &d.createFile( buf );
          delete[] buf;
    }

    FieldsWriter::~FieldsWriter(){
    }

  void FieldsWriter::close() {
    fieldsStream->close();
    indexStream->close();

    _DELETE( fieldsStream );
    _DELETE( indexStream );
  }

  void FieldsWriter::addDocument(document::Document& doc) {
    indexStream->writeLong(fieldsStream->getFilePointer());
      
    int storedCount = 0;
    document::DocumentFieldEnumeration* fields = doc.fields();
    while (fields->hasMoreElements()) {
      document::Field* field = fields->nextElement();
      if (field->IsStored())
        storedCount++;
    }
    delete fields;
    fieldsStream->writeVInt(storedCount);

    fields = doc.fields();
    while (fields->hasMoreElements()) {
      document::Field* field = fields->nextElement();
      if (field->IsStored()) {
        fieldsStream->writeVInt(fieldInfos.fieldNumber(field->Name()));

        l_byte_t bits = 0;
        if (field->IsTokenized())
          bits |= 1;
        //FEATURE: can this data be compressed? A bit could be set to compress or not compress???
        fieldsStream->writeByte(bits);

        //FEATURE: this problem in Java Lucene too, if using Reader, data is not stored.
        if ( field->StringValue() == NULL ){
          util::BasicReader* r = field->ReaderValue();
          int rp = r->position();
          r->seek(0);
          int rl = r->available();

          char_t* rv = new char_t[rl];
          r->read(rv,0,rl);
          r->seek(rp); //reset position

          //simulate writeString
          fieldsStream->writeVInt(rl);
          fieldsStream->writeChars(rv,0,rl);
          delete[] rv;
        }else
          fieldsStream->writeString(field->StringValue());
      }
    }
    delete fields;
  }

}}
