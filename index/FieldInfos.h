#include "StdHeader.h"
#ifndef _NSLib_index_FieldInfos_
#define _NSLib_index_FieldInfos_

#include "store/Directory.h"
#include "document/Document.h"
#include "document/Field.h"
#include "util/VoidMap.h"
#include "util/VoidList.h"
#include "FieldInfo.h"

using namespace std;
using namespace NSLib::store;
namespace NSLib{ namespace index {
  class FieldInfos {
  private:
    NSLib::util::VoidList<FieldInfo*> byNumber;
    NSLib::util::VoidMap<const char_t*,FieldInfo*> byName;
  public:
    FieldInfos();
    ~FieldInfos();

    FieldInfos(Directory& d, const fchar_t* name);
  public:
    // Adds field info for a Document. 
    void add(NSLib::document::Document& doc);

    // Merges in information from another FieldInfos. 
    void add(FieldInfos& other);

    void add( char_t* name, const bool isIndexed);

    int fieldNumber(const char_t* fieldName)const;


    FieldInfo& fieldInfo(const char_t* fieldName) const;
    const char_t* fieldName(const int fieldNumber)const;

    FieldInfo& fieldInfo(const int fieldNumber) const;

    int size()const;

    void write(NSLib::store::Directory& d, const fchar_t* name);

    void write(NSLib::store::OutputStream& output);
  private:
    void read(NSLib::store::InputStream& input);
    void addInternal( char_t* name, const bool dupName,const bool isIndexed);

  };
}}
#endif
