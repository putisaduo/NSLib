#include "StdHeader.h"
#include "SegmentMerger.h"

using namespace NSLib::util;
namespace NSLib{ namespace index {


	SegmentMerger::SegmentMerger(Directory& dir, const fchar_t* name):
		freqOutput(NULL),
		proxOutput(NULL),
		termInfosWriter(NULL),
		queue(NULL),
		directory(dir),
		segment(name),
		fieldInfos(NULL)
	{
		readers.setDoDelete(NSLib::util::DELETE_TYPE_DELETE);
	}

    SegmentMerger::~SegmentMerger(){
		readers.clear();
		delete fieldInfos;
		//delete queue;
    }

	void SegmentMerger::add(SegmentReader& reader) {
		readers.push_back(&reader);
	}

	SegmentReader& SegmentMerger::segmentReader(const int i) {
		return *(SegmentReader*)readers.at(i);
	}

	void SegmentMerger::merge() {
		_TRY {
			mergeFields();
			mergeTerms();
			mergeNorms();
	      
		} _FINALLY ( 
		    for (uint i = 0; i < readers.size(); i++) {  // close readers
    		    SegmentReader* reader = (SegmentReader*)readers.at(i);
    			reader->close();
			} 
		);
	}



	const fchar_t* SegmentMerger::segmentname(const fchar_t* ext, const int x){
		fchar_t* buf = new fchar_t[MAX_PATH];
		if ( x==-1 )
			fstringPrintF(buf, CONST_STRING("%s%s"), segment,ext );
		else
			fstringPrintF(buf, CONST_STRING("%s%s%d"), segment,ext,x );
		return buf;
	}

	void SegmentMerger::mergeFields() {
		fieldInfos = new FieldInfos();		  // merge field names
		for (unsigned int i = 0; i < readers.size(); i++) {
			SegmentReader* reader = (SegmentReader*)readers.at(i);
			fieldInfos->add(*reader->fieldInfos);
		}
		const fchar_t* buf = segmentname(CONST_STRING(".fnm"));
		fieldInfos->write(directory, buf );
		delete[] buf;
	    
		FieldsWriter* fieldsWriter =			  // merge field values
			new FieldsWriter(directory, segment, *fieldInfos);
		_TRY {
			for (unsigned int i = 0; i < readers.size(); i++) {
				SegmentReader* reader = (SegmentReader*)readers.at(i);
				NSLib::util::BitVector* deletedDocs = reader->deletedDocs;
				int maxDoc = reader->MaxDoc();
				for (int j = 0; j < maxDoc; j++)
					if (deletedDocs == NULL || !deletedDocs->get(j)){ // skip deleted docs
						NSLib::document::Document& doc = reader->document(j);
						fieldsWriter->addDocument( doc );
						delete &doc;
					}
			}
		} _FINALLY(
		    fieldsWriter->close();
			delete fieldsWriter;
		);
	}

	void SegmentMerger::mergeTerms() {
		_TRY {
		    const fchar_t* buf = segmentname(CONST_STRING(".frq"));
			freqOutput = &directory.createFile( buf );
			delete[] buf;
			
			buf = segmentname(CONST_STRING(".prx"));
			proxOutput = &directory.createFile( buf );
			delete[] buf;
			
			termInfosWriter = new TermInfosWriter(directory, segment, *fieldInfos);  
			mergeTermInfos();
	      
		} _FINALLY(
			if (freqOutput != NULL) 		{ freqOutput->close(); delete freqOutput; }
			if (proxOutput != NULL) 		{ proxOutput->close(); delete proxOutput; }
			if (termInfosWriter != NULL) 	{ termInfosWriter->close(); delete termInfosWriter; }
			if (queue != NULL)		        { queue->close(); delete queue; queue=NULL;}
		);
		
	}

	void SegmentMerger::mergeTermInfos() {
		queue = new SegmentMergeQueue(readers.size());
		int base = 0;
		for (unsigned int i = 0; i < readers.size(); i++) {
			SegmentReader* reader = (SegmentReader*)readers.at(i);
			SegmentTermEnum& termEnum = (SegmentTermEnum&)reader->getTerms();
			SegmentMergeInfo& smi = *new SegmentMergeInfo(base, termEnum, *reader);
			base += reader->NumDocs();
			if (smi.next())
				queue->put( &smi);				  // initialize queue
			else{
				smi.close();
				delete &smi.termEnum;
				delete &smi;
			}
		}

		int matchLength = readers.size();
		SegmentMergeInfo** match = new SegmentMergeInfo*[matchLength];
//		for (int i=0;i<matchLength;i++)
//			match[i] = NULL;

		while (queue->Size() > 0) {
			int matchSize = 0;			  // pop matching terms

			match[matchSize++] = queue->pop();
			const Term* term = match[0]->term;
			SegmentMergeInfo* top = queue->top();
		      
			while (top != NULL && term->compareTo(*top->term) == 0) {
				match[matchSize++] = queue->pop();
				top = queue->top();
			}

			mergeTermInfo(match, matchSize);		  // add new TermInfo
		      
			while (matchSize > 0) {
				SegmentMergeInfo* smi = match[--matchSize];
				if (smi->next())
					queue->put(smi);			  // restore queue
				else{
					smi->close();				  // done with a segment
					delete &smi->termEnum; //add:
					delete smi;
				}
			}
		}
		delete[] match;
	}

	void SegmentMerger::mergeTermInfo( SegmentMergeInfo** smis, const int n){
		long_t freqPointer = freqOutput->getFilePointer();
		long_t proxPointer = proxOutput->getFilePointer();

		int df = appendPostings(smis, n);		  // append posting data

		if (df > 0) {
			// add an entry to the dictionary with pointers to prox and freq files
			termInfo.set(df, freqPointer, proxPointer);
			termInfosWriter->add(*smis[0]->term, termInfo);
		}
	}
	       
	int SegmentMerger::appendPostings(SegmentMergeInfo** smis, const int n) {
		int lastDoc = 0;
		int df = 0;					  // number of docs w/ term
		for (int i = 0; i < n; i++) {
			SegmentMergeInfo* smi = smis[i];
			SegmentTermPositions& postings = smi->postings;
			int base = smi->base;
			int* docMap = smi->docMap;
			smi->termEnum.getTermInfo(termInfo);
			postings.seek(&termInfo);
			while (postings.next()) {
				int doc;
				if (docMap == NULL)
					doc = base + postings.doc;		  // no deletions
				else
					doc = base + docMap[postings.doc];	  // re-map around deletions

				if (doc < lastDoc)
					_THROWC(  "docs out of order");

				int docCode = (doc - lastDoc) << 1;	  // use low bit to flag freq=1
				lastDoc = doc;

				int freq = postings.freq;
				if (freq == 1) {
					freqOutput->writeVInt(docCode | 1);	  // write doc & freq=1
				} else {
					freqOutput->writeVInt(docCode);	  // write doc
					freqOutput->writeVInt(freq);		  // write frequency in doc
				}
				  
				int lastPosition = 0;			  // write position deltas
				for (int j = 0; j < freq; j++) {
					int position = postings.nextPosition();
					proxOutput->writeVInt(position - lastPosition);
					lastPosition = position;
				}

				df++;
			}
		}
		return df;
	}

	void SegmentMerger::mergeNorms() {
		for (int i = 0; i < fieldInfos->size(); i++) {
			FieldInfo& fi = fieldInfos->fieldInfo(i);
			if (fi.isIndexed) {
			    const fchar_t* buf = segmentname(CONST_STRING(".f"), i);
				OutputStream& output = directory.createFile( buf );
				delete[] buf;
				_TRY {
					for (unsigned int j = 0; j < readers.size(); j++) {
						SegmentReader* reader = (SegmentReader*)readers.at(j);
						NSLib::util::BitVector* deletedDocs = reader->deletedDocs;
						InputStream* input = reader->normStream(fi.name);
						int maxDoc = reader->MaxDoc();
						_TRY {
							for (int k = 0; k < maxDoc; k++) {
							l_byte_t norm = input != NULL ? input->readByte() : (l_byte_t)0;
							if (deletedDocs == NULL || !deletedDocs->get(k))
								output.writeByte(norm);
							}
						} _FINALLY(
							if (input != NULL){
							    input->close();
								delete input;
							}
						);
					}
				} _FINALLY(
				    output.close();
				    delete &output;
				);
			}
		}
	}

}}
