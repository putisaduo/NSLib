#include "StdHeader.h"
#include "FieldInfos.h"

#include "store/Directory.h"
#include "document/Document.h"
#include "document/Field.h"
#include "util/VoidMap.h"
#include "FieldInfo.h"

#include <iostream>

using namespace std;
namespace NSLib{ namespace index {

  FieldInfos::FieldInfos() {
    add( _T(""), false);
    byName.setDoDelete(false,NSLib::util::DELETE_TYPE_DELETE);
    byName.setDoDelete(true,NSLib::util::DELETE_TYPE_DELETE_ARRAY);
  }
  
  FieldInfos::~FieldInfos(){

  }

  FieldInfos::FieldInfos(Directory& d, const fchar_t* name) {
    InputStream& input = d.openFile(name);
    _TRY {
      read(input);
    } _FINALLY (
        input.close();
        delete &input;
    );

    byName.setDoDelete(false,NSLib::util::DELETE_TYPE_DELETE);
    byName.setDoDelete(true,NSLib::util::DELETE_TYPE_DELETE_ARRAY);
  }

  void FieldInfos::add(NSLib::document::Document& doc) {
    NSLib::document::DocumentFieldEnumeration* fields  = doc.fields();
    while (fields->hasMoreElements()) {
      NSLib::document::Field* field = fields->nextElement();
      add(field->Name(), field->IsIndexed());
    }
    delete fields;
  }

  void FieldInfos::add(FieldInfos& other) {
    for (int i = 0; i < other.size(); i++) {
      FieldInfo& fi = other.fieldInfo(i);
      add(fi.name, fi.isIndexed);
    }
  }

  void FieldInfos::add( char_t* name, const bool isIndexed) {
    if ( byName.exists(name) ){
      FieldInfo& fi = fieldInfo(name);
      if (fi.isIndexed != isIndexed)
        fi.isIndexed = true;
    }else
      addInternal(name, true,isIndexed);
  }

  int FieldInfos::fieldNumber(const char_t* fieldName)const {
    try{
      FieldInfo* fi = &fieldInfo(fieldName);
      return fi->number;
    }catch(...){}
    return -1;
  }


  FieldInfo& FieldInfos::fieldInfo(const char_t* fieldName) const {
    return *byName.get(fieldName);
  }
  const char_t* FieldInfos::fieldName(const int fieldNumber)const {
    return fieldInfo(fieldNumber).name;
  }

  FieldInfo& FieldInfos::fieldInfo(const int fieldNumber) const {
    return *byNumber.at(fieldNumber);
  }

  int FieldInfos::size()const {
    return byNumber.size();
  }

  void FieldInfos::write(NSLib::store::Directory& d, const fchar_t* name) {
    OutputStream& output = d.createFile(name);
    _TRY {
      write(output);
    } _FINALLY (
      output.close();
      delete &output;
    );
  }

  void FieldInfos::write(NSLib::store::OutputStream& output) {
    output.writeVInt(size());
    for (int i = 0; i < size(); i++) {
      FieldInfo& fi = fieldInfo(i);
      output.writeString(fi.name);
      output.writeByte((l_byte_t)(fi.isIndexed ? 1 : 0));
    }
  }

  void FieldInfos::read(NSLib::store::InputStream& input) {
    int size = input.readVInt();
    cerr << "  FieldInfos::read " << size << " fields" << endl;
    for (int i = 0; i < size; i++){
      char_t* rs = input.readString(true); //calling convention is right to left ihere
      addInternal(rs, false, input.readByte() != 0);
    }
  }
  void FieldInfos::addInternal( char_t* name, const bool dupName, const bool isIndexed) {
    FieldInfo* fi = new FieldInfo(name, isIndexed, byNumber.size());
    byNumber.push_back(fi);
    const char_t* tmp = name;
    if ( dupName )
      tmp = stringDuplicate( name );
    byName.put( tmp, fi);
  }

}}
