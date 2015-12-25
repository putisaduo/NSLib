#include "StdHeader.h"
#include "TermInfosWriter.h"

#include "store/Directory.h"
#include "FieldInfos.h"
#include "Term.h"
#include "TermInfo.h"

using namespace NSLib::util;
namespace NSLib{ namespace index {
	
	//static 
	const fchar_t* TermInfosWriter::segmentname( const fchar_t* segment, const fchar_t* ext ){
		fchar_t* buf = new fchar_t[MAX_PATH];
		fstringPrintF(buf, CONST_STRING("%s%s"), segment,ext );
		return buf;
	}
	
	TermInfosWriter::TermInfosWriter(Directory& directory, const fchar_t* segment, FieldInfos& fis):
		lastTerm( new Term( _T(""), _T(""),true) ),
		lastTi ( *new TermInfo() ),
		lastIndexPointer(0),
		other( NULL ),
		size(0),

		fieldInfos(fis),
		isIndex(false)
	{
	    const fchar_t* buf = segmentname(segment, CONST_STRING(".tis"));
	    output = &directory.createFile( buf );
	    delete[] buf;
	    
		output->writeInt(0);// leave space for size
		other = new TermInfosWriter(directory, segment, fis, true);
		other->other = this;
	}

	TermInfosWriter::~TermInfosWriter(){
	}
	
	TermInfosWriter::TermInfosWriter(Directory& directory, const fchar_t* segment, FieldInfos& fis, bool isIndex):
		lastTerm( new Term( _T(""),_T(""),true)),
		lastTi ( *new TermInfo() ),
		lastIndexPointer(0),
		other(NULL),
		size(0),

		fieldInfos(fis),
		isIndex(isIndex),
		output ( NULL )
	{
	    const fchar_t* buf = segmentname(segment, (isIndex ? CONST_STRING(".tii") : CONST_STRING(".tis")));
	    output = &directory.createFile( buf );
	    delete[] buf;
	    
		output->writeInt(0);// leave space for size
	}


	void TermInfosWriter::add(Term& term, const TermInfo& ti) {
		if (!isIndex && term.compareTo(*lastTerm) <= 0)
			_THROWC( "term out of order");
		if (ti.freqPointer < lastTi.freqPointer)
			_THROWC( "freqPointer out of order");
		if (ti.proxPointer < lastTi.proxPointer)
			_THROWC( "proxPointer out of order");

		if (!isIndex && size % NSLIB_WRITER_INDEX_INTERVAL == 0)
			other->add(*lastTerm, lastTi);		  // add an index term

		writeTerm(term);				  // write term
		output->writeVInt(ti.docFreq);		  // write doc freq
		output->writeVLong(ti.freqPointer - lastTi.freqPointer); // write pointers
		output->writeVLong(ti.proxPointer - lastTi.proxPointer);

		if (isIndex) {
			output->writeVLong(other->output->getFilePointer() - lastIndexPointer);
			lastIndexPointer = other->output->getFilePointer(); // write pointer
		}

		lastTi.set(ti);
		size++;
	}

	void TermInfosWriter::close() {
		output->seek(0);				  // write size at start
		output->writeInt(size);
		output->close();
	    
		_DELETE(output);
		if (!isIndex){
			other->close();
			_DELETE( other );
		}

        lastTerm->finalize();
		delete &lastTi;
	}

	void TermInfosWriter::writeTerm(Term& term) {
		int start = stringDifference(lastTerm->Text(), term.Text());
		int length = stringLength(term.Text()) - start;
	    
		output->writeVInt(start);			  // write shared prefix length
		output->writeVInt(length);			  // write delta length
		output->writeChars(term.Text(), start, length);  // write delta chars

		output->writeVInt(fieldInfos.fieldNumber(term.Field())); // write field num

        //TODO last term has to be cleared out.. but user might be using it.
		/*if ( !lastTermDeleted ){
			delete lastTerm;
			lastTerm = NULL;
			lastTermDeleted = true;
		}*/
		lastTerm->finalize();
		lastTerm = term.pointer();
	}

    //static 
	int TermInfosWriter::stringDifference(const char_t* s1, const char_t* s2) {
		int len1 = stringLength(s1);
		int len2 = stringLength(s2);
		int len = len1 < len2 ? len1 : len2;
		for (int i = 0; i < len; i++)
			if ( s1[i] != s2[i])
				return i;
		return len;
	}
}}
