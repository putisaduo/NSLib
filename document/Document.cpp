#include "StdHeader.h"
#include "Document.h"
#include "Field.h"
#include "util/StringBuffer.h"

using namespace std;
using namespace NSLib::util;
namespace NSLib{ namespace document{ 

DocumentFieldList::DocumentFieldList(Field* f, DocumentFieldList* n) {
  field = f;
  next = n;
}
DocumentFieldList::~DocumentFieldList(){
  if ( next != NULL ){
    delete next;
    next = NULL;
  }
  if ( field != NULL )
    delete field;
}

  
DocumentFieldEnumeration::DocumentFieldEnumeration(const DocumentFieldList* fl)
{ fields = fl; }

DocumentFieldEnumeration::~DocumentFieldEnumeration()
{ }

bool DocumentFieldEnumeration::hasMoreElements() const {
  return fields == NULL ? false : true;
}

Field* DocumentFieldEnumeration::nextElement() {
  Field* result = fields->field;
  fields = fields->next;
  return result;
}


Document::Document():fieldList(NULL) 
{ }

Document::~Document(){
  if ( fieldList!=NULL )
    delete fieldList;
}

// Adds a field to a document.  Several fields may be added with
  // the same name.  In this case, if the fields are indexed, their text is
// treated as though appended for the purposes of search. */
void Document::add(Field& field) {
  fieldList = new DocumentFieldList(&field, fieldList);
}

// Returns a field with the given name if any exist in this document, or
//  null.  If multiple fields may exist with this name, this method returns the
//  last added such added. */
bool Document::getField(const char_t* name, Field*& retField) {
  for (DocumentFieldList* list = fieldList; list != NULL; list = list->next)
    if ( stringCompare(list->field->Name(), name) == 0 ){
      retField = list->field;
      return true;
    }
    
  return false;
}

// Returns the string value of the field with the given name if any exist in
//  this document, or null.  If multiple fields may exist with this name, this
//  method returns the last added such added. */
const char_t* Document::get(const char_t* name) {
  Field *field = NULL;
  if (getField(name,field))
    return field->StringValue();
  else
    return NULL;
}

// Returns an Enumeration of all the fields in a document. */
DocumentFieldEnumeration* Document::fields() {
  return new DocumentFieldEnumeration(fieldList);
}


const char_t* Document::toString() {
  StringBuffer ret(_T("Document{"));
  for (DocumentFieldList* list = fieldList; list != NULL; list = list->next) {
      char_t* tmp = list->field->toString();
    ret.append( tmp );
      if (list->next != NULL)
          ret.append(_T(" "));
    delete[] tmp;
  }
  ret.append(_T("}"));
  return ret.ToString();
} 

}}
