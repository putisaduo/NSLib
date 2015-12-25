#include "StdHeader.h"
#ifndef _NSLib_index_SegmentsReader
#define _NSLib_index_SegmentsReader

#include "SegmentHeader.h"

#include "SegmentInfo.h"
#include "IndexReader.h"
#include "document/Document.h"
#include "Terms.h"
#include "SegmentMergeQueue.h"

namespace NSLib{ namespace index{

	class SegmentsTermDocs:public virtual TermDocs {
	private:
		SegmentTermDocs** segTermDocs;
	protected:
		SegmentReader** readers;
		int readersLength;
		const int* starts;
		Term* term;

		int base;
		int pointer;

		SegmentTermDocs* current;              // == segTermDocs[pointer]
	public:
		SegmentsTermDocs(SegmentReader** r, int rLen, const int* s);
		~SegmentsTermDocs();

		int Doc() const;
		int Freq() const;

		void seek(Term* tterm);

		bool next();

		// Optimized implementation. 
		int read(int docs[], int freqs[]);

		// As yet unoptimized implementation. 
		bool skipTo(const int target);

		void close();

		
	protected:
		SegmentTermDocs* termDocs(const SegmentReader& reader) const;
	private:
		SegmentTermDocs* termDocs(const int i) const;

	};

	
	class SegmentsTermEnum:public virtual TermEnum {
	private:
		SegmentMergeQueue& queue;

		Term* term;
		int docFreq;
	public:
		SegmentsTermEnum(SegmentReader** readers, const int readersLength, const int starts[], const Term* t);
		~SegmentsTermEnum();

		bool next();

		Term* getTerm(const bool pointer = true);

		int DocFreq() const;

		void close();
	};


	class SegmentsTermPositions:public SegmentsTermDocs,public virtual TermPositions {
	public:
		SegmentsTermPositions(SegmentReader** r, const int rLen, const int* s);
		~SegmentsTermPositions(){
		}
		int nextPosition();

		//TODO: The virtual members required in TermPositions are defined in the subclass SegmentTermDocs,
		//but why is there a compiler error because of this.
		void seek(Term* term);
		int Doc() const;
		int Freq() const;

		int read(int docs[], int freqs[]);
		bool skipTo(const int target);
		bool next();
		void close();

	protected:
		SegmentTermDocs& termDocs(SegmentReader& reader);
	};


	class SegmentsReader:public IndexReader{
	protected:
		SegmentReader** readers;
		int readersLength;
		int* starts;			  // 1st docno for each segment

	private:
		NSLib::util::VoidMap<const char_t*,l_byte_t*> normsCache;
		int maxDoc;
		int numDocs;

	public:
		SegmentsReader(Directory& directory, SegmentReader** r, int rLen);
		~SegmentsReader();

        DEFINE_MUTEX(NumDocs_LOCK);
		// synchronized
		int NumDocs();

		int MaxDoc() const;

		NSLib::document::Document& document(const int n);

		bool isDeleted(const int n);

		DEFINE_MUTEX(getNorms_LOCK);
		// synchronized
		l_byte_t* getNorms(const char_t* field);

		TermEnum& getTerms() const;

		TermEnum& getTerms(const Term* term) const;

		int docFreq(const Term& t) const;

		TermDocs& termDocs() const;

		TermPositions& termPositions() const;

	private:
	    DEFINE_MUTEX(doDelete_LOCK);
		// synchronized
		void doDelete(const int n);

		int readerIndex(const int n) const;
		
		DEFINE_MUTEX(doClose_LOCK);
		// synchronized 
		void doClose();
	};


}}
#endif
