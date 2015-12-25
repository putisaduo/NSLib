#include "StdHeader.h"
#ifndef _NSLib_index_TermInfosWriter_
#define _NSLib_index_TermInfosWriter_

#include "store/Directory.h"
#include "FieldInfos.h"
#include "TermInfo.h"
#include "Term.h"

namespace NSLib{ namespace index {
	
	// This stores a monotonically increasing set of <Term, TermInfo> pairs in a
	// Directory.  A TermInfos can be written once, in order.  
	class TermInfosWriter {
	private:
		FieldInfos& fieldInfos;
		OutputStream* output;
		Term* lastTerm;
		TermInfo& lastTi;
		int size;
		long_t lastIndexPointer;
		bool isIndex;
		TermInfosWriter* other;
	
		const static fchar_t* segmentname( const fchar_t* segment, const fchar_t* ext );

		//inititalize
		TermInfosWriter(Directory& directory, const fchar_t* segment, FieldInfos& fis, bool isIndex);
	public:
	    //TODO2: how do you call another constructor... constructor here is initialising same variables as
	    //the private constructor. (This happens in a few other places as well).
		TermInfosWriter(Directory& directory, const fchar_t* segment, FieldInfos& fis);
		~TermInfosWriter();


		// Adds a new <Term, TermInfo> pair to the set.
		//	Term must be lexicographically greater than all previous Terms added.
		//	TermInfo pointers must be positive and greater than all previous.
		void add(Term& term, const TermInfo& ti);

		// Called to complete TermInfos creation. 
		void close();

	private:

		void writeTerm(Term& term);
		static int stringDifference(const char_t* s1, const char_t* s2);
	};
}}
#endif
