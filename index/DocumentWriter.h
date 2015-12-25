#include "StdHeader.h"
#ifndef _NSLib_index_DocumentWriter_
#define _NSLib_index_DocumentWriter_

#include "analysis/AnalysisHeader.h"
#include "document/Document.h"
#include "store/Directory.h"
#include "FieldInfos.h"
#include "util/VoidMap.h"
#include "document/Field.h"
#include "TermInfo.h"
#include "search/Similarity.h"
#include "TermInfosWriter.h"
#include "FieldsWriter.h"
#include <functional>

using namespace std;
namespace NSLib{ namespace index {
	class Posting {				  // info about a Term in a doc
	public:
		int *positions;				  // positions it occurs at
		int positionsLength;
		Term& term;					  // the Term
		int freq;					  // its frequency in doc
		
		int getPositionsLength();
		Posting(Term& t, const int position);
		~Posting();
	};


	class TermCompare:public binary_function<void*,void*,bool>
	{
	public:
		bool operator()( void* val1, void* val2 ) const{
			Term* t1 = (Term*)val1;
			Term* t2 = (Term*)val2;

			int ret = stringCompare(t1->Field(),t2->Field());
			if( ret == 0 )
				return ( stringCompare(t1->Text(),t2->Text()) < 0 );
			else
				return ( ret < 0 );
		}
	};

	class DocumentWriter {
	private:
		NSLib::analysis::Analyzer& analyzer;
		NSLib::store::Directory& directory;
		FieldInfos* fieldInfos; //array
		const int maxFieldLength;

		// Keys are Terms, values are Postings.
		// Used to buffer a document before it is written to the index.
		NSLib::util::VoidMap<Term*,Posting*,TermCompare> postingTable;
		int *fieldLengths; //array
		Term& termBuffer;

		const static fchar_t* segmentname(const fchar_t* segment, const fchar_t* ext );
		  
	public:
		DocumentWriter(NSLib::store::Directory& d, NSLib::analysis::Analyzer& a, const int mfl);
		~DocumentWriter();

		void addDocument(const fchar_t* segment, NSLib::document::Document& doc);


	private:
		// Tokenizes the fields of a document into Postings.
		void invertDocument(NSLib::document::Document& doc);

		void addPosition(const char_t* field, const char_t* text, const int position);

		void sortPostingTable(Posting**& array, int& arraySize);

		static void quickSort(Posting**& postings, const int lo, const int hi);

		void writePostings(Posting** postings, const int postingsLength, const fchar_t* segment);

		void writeNorms(NSLib::document::Document& doc, const fchar_t* segment);

		void clearPostingTable();

		DocumentWriter& operator=( const DocumentWriter& str )
		{
			throw "CAN'T ASSIGN THIS OBJECT";
		}
	};


}}
#endif
