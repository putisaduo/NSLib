#include "StdHeader.h"
#include "Term.h"

namespace NSLib{ namespace index{
	
	Term::Term(const char_t* fld, const char_t* txt ):
		intrn(false),
	    field (stringDuplicate(fld)),
	    text ( stringDuplicate(txt))
	{
	    
	}

	Term::Term(char_t* fld, char_t* txt,const bool intrn,const bool canDelete):
		intrn(intrn),
		field(fld),
		text(txt)
	{
		NSLib::util::IGCollectable<Term>::_canDelete = canDelete;
	}

	
	Term::~Term(){
		if ( !intrn ){
			delete[] field;
			delete[] text;
		}
	}

	const char_t* Term::Field() const { return field; }

	const char_t* Term::Text() const { return text; }

	bool Term::equals(const Term* o) const {
		if (o == NULL)
			return false;
		//Term* o = (Term*)v;
		return stringCompare(field,o->field) == stringCompare(text,o->text) == 0;
	}

	void Term::set(char_t* fld, char_t* txt,const bool intern)
	{
		if ( !intrn ){
			delete[] field;
			delete[] text;
		}
		field = fld;
		text = txt;
		intrn = intern;
	}

	int Term::compareTo(const Term& other) const {
		int ret = stringCompare(field,other.field);
		
		if ( ret == 0)
			return stringCompare(text,other.text);
		else
			return ret;
	}

	char_t* Term::toString()  const{
		return NSLib::util::Misc::join( _T("Term<"), field, _T(":"), text, _T(">"));
	}
		
}}
