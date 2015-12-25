#ifndef _NSLib_index_SegmentTermEnum_
#define _NSLib_index_SegmentTermEnum_
#include "StdHeader.h"

#include "Terms.h"
#include "FieldInfos.h"
#include "TermInfo.h"

namespace NSLib{ namespace index {
	class SegmentTermEnum:public TermEnum{
	private:
		FieldInfos& fieldInfos;

		Term* term;
		TermInfo* termInfo;

		bool isIndex;

		char_t* buffer;
		bool isClone;
		int bufferLength;

	public:
		InputStream& input;
		int size;
		int position;
		long_t indexPointer;
		Term* prev;

		SegmentTermEnum(InputStream& i, FieldInfos& fis, const bool isi );
		SegmentTermEnum( SegmentTermEnum& );
		~SegmentTermEnum();


		// Increments the enumeration to the next element.  True if one exists.
		bool next();
		
		// Returns the current Term in the enumeration.
		//	Initially invalid, valid after next() called for the first time.
		Term* getTerm(const bool pointer = true);

		// Closes the enumeration to further activity, freeing resources. 
		void close();

		// Returns the docFreq from the current TermInfo in the enumeration.
		//	Initially invalid, valid after next() called for the first time.
		int DocFreq() const;
    private:
		Term& readTerm();

		void growBuffer(const int length);

	public:
		void seek(const long_t pointer, const int p, Term& t, TermInfo& ti);
		
		// Returns the current TermInfo in the enumeration.
		//	Initially invalid, valid after next() called for the first time.
		TermInfo* getTermInfo()const;

		// Sets the argument to the current TermInfo in the enumeration.
		//	Initially invalid, valid after next() called for the first time.
		void getTermInfo(TermInfo& ti)const;

		// Returns the freqPointer from the current TermInfo in the enumeration.
		//	Initially invalid, valid after next() called for the first time.
		long_t freqPointer()const;

		// Returns the proxPointer from the current TermInfo in the enumeration.
		//	Initially invalid, valid after next() called for the first time.
		long_t proxPointer()const;
		
		SegmentTermEnum* clone();
	};
}}

#endif
