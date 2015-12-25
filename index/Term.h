#include "StdHeader.h"
#ifndef _NSLib_index_Term_
#define _NSLib_index_Term_


namespace NSLib{ namespace index{
	class Term: public NSLib::util::IGCollectable<Term> {
	private:
		char_t* field;
		char_t* text;
		bool intrn;

	public:
		// Constructs a Term with the given field and text.
		// Field and text is copied and stored.
		Term(const char_t* fld, const char_t* txt);
		
		// Fields deleted only if intern is false. Field and text is not copied.
		Term(char_t* fld, char_t* txt, const bool intern, const bool canDelete=true);
		
		~Term();

		// Returns the field of this term, an interned char_t*.   The field indicates
		//	the part of a document which this term came from. 
		const char_t* Field() const;

		// Returns the text of this term.  In the case of words, this is simply the
		//	text of the word.  In the case of dates and other types, this is an
		//	encoding of the object as a char_t*.  
		const char_t* Text() const;

		// Compares two terms, returning true iff they have the same
		//	field and text. 
		bool equals(const Term* o) const;

		// Resets the field and text of a Term.
		void set(char_t* fld, char_t* txt, const bool intern);

		// Combines the hashCode() of the field and the text. 
		int hashCode();

		// Compares two terms, returning an integer which is less than zero iff this
		//	term belongs after the argument, equal zero iff this term is equal to the
		//	argument, and greater than zero iff this term belongs after the argument.
		//
		//	The ordering of terms is first by field, then by text.
		int compareTo(const Term& other) const;

		char_t* toString() const;
		  
	};
}}
#endif
