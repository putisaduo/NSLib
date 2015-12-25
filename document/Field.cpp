#include "StdHeader.h"
#include "util/Reader.h"
#include "Field.h"

namespace NSLib{ namespace document{ 

Field::Field(const char_t* Name, const char_t* String, bool store, bool index, bool token)
{
  //TODO:
  /*if (Name == NULL)
    _THROWC("name cannot be null");
  if (String == NULL)
    _THROWC("value cannot be null");
      */
        name = stringDuplicate( Name );
        stringValue = stringDuplicate( String );
        readerValue = NULL;
          
  isStored = store;
  isIndexed = index;
  isTokenized = token;
}

Field::Field(const char_t* Name, util::BasicReader* reader):
  name( stringDuplicate(Name) ),
  stringValue( NULL),
  readerValue(reader)
{
  //TODO:
  /*if (name == NULL)
    _THROWC("name cannot be null");
  if (reader == NULL)
    _THROWC("value cannot be null");
      */
        
  isStored = false;
  isIndexed = true;
  isTokenized = true;
}

Field::Field(const char_t* Name, util::BasicReader* reader, bool store, bool index, bool token):
  name( stringDuplicate(Name) ),
  stringValue( NULL),
  readerValue( reader )
{
  //TODO:
  /*if (name == NULL)
    _THROWC("name cannot be null");
  if (reader == NULL)
    _THROWC("value cannot be null");
      */
        
  isStored = store;
  isIndexed = index;
  isTokenized = token;
}
Field::~Field(){
  delete[] name;
  if ( stringValue != NULL )
    delete[] stringValue;
  if ( readerValue != NULL )
    delete readerValue;
}

/*static*/
Field& Field::Keyword(char_t* Name, const char_t* Value) {
  return *new Field(Name, Value, true, true, false);
}

/*static*/
Field& Field::UnIndexed(char_t* Name, const char_t* Value) {
  return *new Field(Name, Value, true, false, false);
}

/*static*/
Field& Field::Text(char_t* Name, const char_t* Value) {
  return *new Field(Name, Value, true, true, true);
}

/*static*/
Field& Field::UnStored(char_t* Name, const char_t* Value) {
  return *new Field(Name, Value, false, true, true);
}

/*static*/
Field& Field::Text(char_t* Name, util::BasicReader* Value) {
  return *new Field(Name, Value);
}



/*===============FIELDS=======================*/
char_t* Field::Name()     { return name; }
const char_t* Field::StringValue()    { return stringValue; }
util::BasicReader* Field::ReaderValue()  { return readerValue; }

bool  Field::IsStored()   { return isStored; }
bool   Field::IsIndexed()   { return isIndexed; }
bool   Field::IsTokenized()   { return isTokenized; }

char_t* Field::toString() {
  if (isStored && isIndexed && !isTokenized)
    return util::Misc::join(_T("Keyword{"), name, _T(":"), (stringValue==NULL?_T("Reader"):stringValue), _T("}"));
  else if (isStored && !isIndexed && !isTokenized)
    return util::Misc::join(_T("Unindexed{"), name, _T(":"), (stringValue==NULL?_T("Reader"):stringValue), _T("}"));
  else if (isStored && isIndexed && isTokenized )
    return util::Misc::join(_T("Text{"), name, _T(":"), (stringValue==NULL?_T("Reader"):stringValue), _T("}"));
  else
    return util::Misc::join(_T("Field Object ("), name, _T(")") );
}

}}
