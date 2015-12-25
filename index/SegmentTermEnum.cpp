#include "StdHeader.h"
#include "SegmentTermEnum.h"

#include "Terms.h"
#include "FieldInfos.h"
#include "Term.h"
#include "TermInfo.h"
#include "util/Arrays.h"

namespace NSLib{ namespace index {

	SegmentTermEnum::SegmentTermEnum(InputStream& i, FieldInfos& fis, const bool isi):
	    position(-1),
	    term( new Term( _T(""),_T("") ,true) ),
		input(i),
		fieldInfos(fis),
		isIndex(isi),
		termInfo ( new TermInfo() ),
		indexPointer ( 0 ),
		buffer(new char_t[0]),
		bufferLength(0),
		prev(NULL),
		isClone(false)
	{
		
		size = input.readInt();
	}

	SegmentTermEnum::SegmentTermEnum(SegmentTermEnum& clone):
		position(clone.position),
		term( clone.term==NULL?NULL:clone.term->pointer() ),
		input( clone.input.clone() ),
		fieldInfos( clone.fieldInfos),
		isIndex( clone.isIndex ),
		termInfo ( new TermInfo(*clone.termInfo) ),
		indexPointer ( clone.indexPointer  ),
		buffer( new char_t[clone.bufferLength] ),
		bufferLength(clone.bufferLength),
		prev( clone.prev==NULL?NULL:clone.prev->pointer() ),
		size( clone.size ),
		isClone(true)
	{
		NSLib::util::Arrays::arraycopy(clone.buffer,0,buffer,0,bufferLength);
	}

	SegmentTermEnum::~SegmentTermEnum(){
		prev->finalize();
		term->finalize();
		delete[] buffer;
		delete termInfo;

		if ( isClone ){
			input.close();
			delete &input;
		}
	}

	bool SegmentTermEnum::next() {
		if (position++ >= size-1) {
			term->finalize();
			term = NULL;
			return false;
		}

		if ( prev != NULL )
			prev->finalize();
		prev = term;
		term = &readTerm();

		termInfo->docFreq = input.readVInt();	  // read doc freq
		termInfo->freqPointer += input.readVLong();	  // read freq pointer
		termInfo->proxPointer += input.readVLong();	  // read prox pointer
	    
		if (isIndex)
		    indexPointer += input.readVLong();	  // read index pointer

		return true;
	}
	
	
	Term* SegmentTermEnum::getTerm(const bool pointer) {
		if ( pointer && term!=NULL )
			return term->pointer();
		else
			return term;
	}
	
	void SegmentTermEnum::close() {
		input.close();
	}

	int SegmentTermEnum::DocFreq() const {
		return termInfo->docFreq;
	}

	Term& SegmentTermEnum::readTerm() {
		int start = input.readVInt();
		int length = input.readVInt();
		uint totalLength = start + length;

		//TODO: check this, not copying buffer every time.
		if ( bufferLength < int(totalLength+1))
			growBuffer(totalLength);
	    
		input.readChars(buffer, start, length);
		buffer[totalLength] = 0;
		
		return *new Term( fieldInfos.fieldName(input.readVInt()), buffer );
	}

	void SegmentTermEnum::growBuffer(const int length) {
		delete[] buffer;
		buffer = new char_t[length+1];
		bufferLength = length+1;
		stringCopy(buffer,term->Text());
	}


	void SegmentTermEnum::seek(const long_t pointer, const int p, Term& t, TermInfo& ti) {
		input.seek(pointer);
		position = p;
		
		term->finalize();
		term = t.pointer();

		prev->finalize();
		prev = NULL;
		
		termInfo->set(ti);
		growBuffer( stringLength(term->Text()) );		  // copy term text into buffer
	}
	
	TermInfo* SegmentTermEnum::getTermInfo()const {
		return new TermInfo(*termInfo); //clone
	}

	void SegmentTermEnum::getTermInfo(TermInfo& ti)const {
		ti.set(*termInfo);
	}
	
	long_t SegmentTermEnum::freqPointer()const {
		return termInfo->freqPointer;
	}

	long_t SegmentTermEnum::proxPointer()const {
		return termInfo->proxPointer;
	}
	
	SegmentTermEnum* SegmentTermEnum::clone() {
		//TODO: check this
		return new SegmentTermEnum(*this);
	}

}}
