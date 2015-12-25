#include "StdHeader.h"
#include "SegmentsReader.h"

#include "SegmentInfo.h"
#include "IndexReader.h"
#include "document/Document.h"
#include "Terms.h"
#include "SegmentMergeQueue.h"

namespace NSLib{ namespace index{

SegmentsTermDocs::SegmentsTermDocs(SegmentReader** r, int rLen, const int* s):
	base(0),
	pointer(0),
	segTermDocs ( new SegmentTermDocs*[rLen] ),
	readers(r),
	readersLength(rLen),
	starts(s)
{
	//TODO: do we really need to do this?
	for ( int i=0;i<rLen;i++)
		segTermDocs[i]=NULL;
}

SegmentsTermDocs::~SegmentsTermDocs(){
}

int SegmentsTermDocs::Doc() const {
	return base + current->doc;
}
int SegmentsTermDocs::Freq() const {
	return current->freq;
}

void SegmentsTermDocs::seek( Term* tterm) {
	term = tterm->pointer();
	base = 0;
	pointer = 0;
	current = NULL;
}

bool SegmentsTermDocs::next() {
	if (current != NULL && current->next()) {
		return true;
	} else if (pointer < readersLength) {
		base = starts[pointer];
		current = termDocs(pointer++);
		return next();
	} else
		return false;
}

/** Optimized implementation. */
int SegmentsTermDocs::read(int docs[], int freqs[]) {
	while (true) {
		while (current == NULL) {
			if (pointer < readersLength) {		  // try next segment
				base = starts[pointer];
				current = termDocs(pointer++);
			} else {
				return 0;
			}
		}
		int end = current->read(docs, freqs);
		if (end == 0) {				  // none left in segment
			current = NULL;
		} else {					  // got some
			int b = base;			  // adjust doc numbers
			for (int i = 0; i < end; i++)
				docs[i] += b;
			return end;
		}
	}
}

/** As yet unoptimized implementation. */
bool SegmentsTermDocs::skipTo(const int target) {
	do {
		if (!next())
			return false;
	} while (target > Doc());
	return true;
}

void SegmentsTermDocs::close() {
	for (int i = 0; i < readersLength; i++) {
		if (segTermDocs[i] != NULL){
			segTermDocs[i]->close();
		}
	}
	_DELETE( segTermDocs );
	term->finalize();
}

	

SegmentTermDocs* SegmentsTermDocs::termDocs(const SegmentReader& reader) const {
	SegmentTermDocs& ret = (SegmentTermDocs&)reader.termDocs();
	return &ret;
}

SegmentTermDocs* SegmentsTermDocs::termDocs(const int i) const {
	if (term == NULL)
		return NULL;
	SegmentTermDocs* result = segTermDocs[i];
	if (result == NULL){
		segTermDocs[i] = termDocs(*readers[i]);
		result = segTermDocs[i];
	}
	result->seek(term);

	return result;
}





SegmentsTermEnum::SegmentsTermEnum(SegmentReader** readers, const int readersLength, const int starts[], const Term* t):
	queue( *new SegmentMergeQueue(readersLength))
{
	for (int i = 0; i < readersLength; i++) {
		SegmentReader* reader = readers[i];
		SegmentTermEnum* termEnum;

		if (t != NULL) {
			termEnum = (SegmentTermEnum*)&reader->getTerms(t);
		} else
			termEnum = (SegmentTermEnum*)&reader->getTerms();
				
		SegmentMergeInfo* smi = new SegmentMergeInfo(starts[i], *termEnum, *reader);
		if (t == NULL ? smi->next() : termEnum->getTerm() != NULL)
			queue.put(smi);				  // initialize queue
		else
			smi->close();
	}

	if (t != NULL && queue.Size() > 0) {
		SegmentMergeInfo* top = queue.top();
		term = top->termEnum.getTerm();
		docFreq = top->termEnum.DocFreq();
	}else{
		term = NULL;
		docFreq = 0;
	}

}
SegmentsTermEnum::~SegmentsTermEnum(){
}

bool SegmentsTermEnum::next() {
	SegmentMergeInfo* top = queue.top();
	if (top == NULL) {
		//term->finalize();
		term = NULL;
		return false;
	}
	//term->finalize(); //TODO: need this?
	term = top->term;
	docFreq = 0;
		
	while (top != NULL && term->compareTo(*top->term) == 0) {
		queue.pop(); //don't delete, this is the top
		docFreq += top->termEnum.DocFreq();	  // increment freq
		if (top->next())
			queue.put(top);				  // restore queue
		else{
			top->close();				  // done with a segment
			//delete top; //TODO: need this?
			top = NULL;
		}
		top = queue.top();
	}
	return true;
}

Term* SegmentsTermEnum::getTerm(const bool pointer) {
	if ( pointer && term!=NULL )
		  return term->pointer();
	else
		  return term;
}

int SegmentsTermEnum::DocFreq() const {
	return docFreq;
}

void SegmentsTermEnum::close() {
	queue.close();
	delete &queue;
}





SegmentsTermPositions::SegmentsTermPositions(SegmentReader** r, const int rLen, const int* s):
	SegmentsTermDocs(r,rLen, s)
{
}

int SegmentsTermPositions::nextPosition() {
	return ((SegmentTermPositions*)current)->nextPosition();
}

void SegmentsTermPositions::seek(Term* term){
	SegmentsTermDocs::seek(term);
};
int SegmentsTermPositions::Doc() const{
	return SegmentsTermDocs::Doc();
};
int SegmentsTermPositions::Freq() const{
	return SegmentsTermDocs::Freq();
};

int SegmentsTermPositions::read(int docs[], int freqs[]){
	return SegmentsTermDocs::read(docs,freqs);
};
bool SegmentsTermPositions::skipTo(const int target){
	return SegmentsTermDocs::skipTo(target);
};
bool SegmentsTermPositions::next(){
	return SegmentsTermDocs::next();
}
void SegmentsTermPositions::close(){
	SegmentsTermDocs::close();
}


SegmentTermDocs& SegmentsTermPositions::termDocs(SegmentReader& reader) {
	return (SegmentTermDocs&)reader.termPositions();
}






SegmentsReader::SegmentsReader(Directory& directory, SegmentReader** r, int rLen):
	IndexReader(directory),
	readers(r),
	readersLength(rLen),
	maxDoc(0),
	numDocs(-1)
{
	starts = new int[readersLength + 1];	  // build starts array
	for (int i = 0; i < readersLength; i++) {
		starts[i] = maxDoc;
		maxDoc += readers[i]->MaxDoc();		  // compute maxDocs
	}
	starts[readersLength] = maxDoc;
}
SegmentsReader::~SegmentsReader(){
	delete[] starts;
	starts = NULL;

	for ( int i=0;i<readersLength;i++ )
		delete readers[i];
	delete[] readers;
}

int SegmentsReader::NumDocs() {
	  LOCK_MUTEX(NumDocs_LOCK);
	if (numDocs == -1) {			  // check cache
		int n = 0;				  // cache miss--recompute
		for (int i = 0; i < readersLength; i++)
			n += readers[i]->NumDocs();		  // sum from readers
		numDocs = n;
	}
	UNLOCK_MUTEX(NumDocs_LOCK);
	return numDocs;
}

int SegmentsReader::MaxDoc() const {
	return maxDoc;
}

NSLib::document::Document& SegmentsReader::document(const int n) {
	int i = readerIndex(n);			  // find segment num
	return readers[i]->document(n - starts[i]);	  // dispatch to segment reader
}

bool SegmentsReader::isDeleted(const int n) {
	int i = readerIndex(n);			  // find segment num
	return readers[i]->isDeleted(n - starts[i]);	  // dispatch to segment reader
}

l_byte_t* SegmentsReader::getNorms(const char_t* field){
	  LOCK_MUTEX(getNorms_LOCK);
	l_byte_t* bytes = normsCache.get(field);
	if (bytes != NULL){
		UNLOCK_MUTEX(getNorms_LOCK);
		return bytes;				  // cache hit
      }
        
	bytes = new l_byte_t[MaxDoc()];
	for (int i = 0; i < readersLength; i++)
		readers[i]->getNorms(field, bytes, starts[i]);
	normsCache.put(field, bytes);		  // update cache
		
	UNLOCK_MUTEX(getNorms_LOCK);
	return bytes;
}

TermEnum& SegmentsReader::getTerms() const {
	return *new SegmentsTermEnum(readers, readersLength, starts, NULL);
}

TermEnum& SegmentsReader::getTerms(const Term* term) const {
	return *new SegmentsTermEnum(readers, readersLength, starts, term);
}

int SegmentsReader::docFreq(const Term& t) const {
	int total = 0;				  // sum freqs in segments
	for (int i = 0; i < readersLength; i++)
		total += readers[i]->docFreq(t);
	return total;
}

TermDocs& SegmentsReader::termDocs() const {
	TermDocs* ret =  (TermDocs*)new SegmentsTermDocs(readers,readersLength, starts);
	return *ret;
}

TermPositions& SegmentsReader::termPositions() const {
	TermPositions* ret = (TermPositions*)new SegmentsTermPositions(readers,readersLength, starts);
	return *ret;
}

void SegmentsReader::doDelete(const int n) {
	  LOCK_MUTEX(doDelete_LOCK);
	numDocs = -1;				  // invalidate cache
	int i = readerIndex(n);			  // find segment num
	readers[i]->doDelete(n - starts[i]);		  // dispatch to segment reader
		
	UNLOCK_MUTEX(doDelete_LOCK);
}

int SegmentsReader::readerIndex(const int n) const {	  // find reader for doc n:
	int lo = 0;					  // search starts array
	int hi = readersLength - 1;		  // for first element less
						// than n, return its index
	while (hi >= lo) {
		int mid = (lo + hi) >> 1;
		int midValue = starts[mid];
		if (n < midValue)
			hi = mid - 1;
		else if (n > midValue)
			lo = mid + 1;
		else
			return mid;
	}
	return hi;
}
	
void SegmentsReader::doClose() {
	  LOCK_MUTEX(doClose_LOCK);
	for (int i = 0; i < readersLength; i++){
		readers[i]->close();
	}
	  UNLOCK_MUTEX(doClose_LOCK);
}

}}
