#include "StdHeader.h"
#include "DocumentWriter.h"

#include "util/VoidMap.h"
#include "store/Directory.h"
#include "document/Document.h"
#include "document/Field.h"
#include "FieldInfos.h"
#include "Term.h"
#include "TermInfo.h"
#include "analysis/AnalysisHeader.h"
#include "util/VoidMap.h"
#include "search/Similarity.h"
#include "TermInfosWriter.h"
#include "FieldsWriter.h"

//#include <iostream> //for DEBUG

using namespace std;
using namespace NSLib::util;
namespace NSLib{ namespace index {

    /*Posting*/
  int Posting::getPositionsLength(){
    return positionsLength;
  }
    
  Posting::Posting(Term& t, const int position):
    term (*t.pointer())
  {
    freq = 1;
    
    positions = new int[1];
    positionsLength = 1;
    positions[0] = position;
  }
  Posting::~Posting(){
    delete[] positions;
    term.finalize();
  }



    /*DocumentWriter =======================*/
  
  //static 
  const fchar_t* DocumentWriter::segmentname(const fchar_t* segment, const fchar_t* ext ){
    fchar_t* buf = new fchar_t[MAX_PATH];
    fstringPrintF(buf, CONST_STRING("%s%s"), segment,ext );
    return buf;
  }

  DocumentWriter::DocumentWriter(store::Directory& d, analysis::Analyzer& a, const int mfl):
      analyzer(a),
      directory(d),
      termBuffer(*new Term( _T(""), _T("") ,true)),// avoid consing
      maxFieldLength(mfl),
      fieldInfos(NULL),
      fieldLengths(NULL)
  {
  }

  DocumentWriter::~DocumentWriter(){
    clearPostingTable();
    if ( fieldInfos != NULL )
      delete fieldInfos;
    if ( fieldLengths != NULL )
      delete[] fieldLengths;

    termBuffer.finalize();
  }
  
  void DocumentWriter::clearPostingTable(){
    map<Term*,Posting*,TermCompare>::iterator itr = postingTable.begin();
    while ( itr != postingTable.end() ){
      delete itr->second;
      itr->first->finalize();

      itr++;
    }
    postingTable.clear();
  }

  void DocumentWriter::addDocument(const fchar_t* segment, document::Document& doc) {
    // write field names
    fieldInfos = new FieldInfos();
    fieldInfos->add(doc);
    
    const fchar_t* buf = segmentname(segment, CONST_STRING(".fnm"));
    fieldInfos->write(directory, buf);
    delete[] buf;

    // write field values
    FieldsWriter fieldsWriter(directory, segment, *fieldInfos);
    _TRY {
      fieldsWriter.addDocument(doc);
    } _FINALLY( fieldsWriter.close() );
        
    // invert doc into postingTable
    clearPostingTable();        // clear postingTable
    fieldLengths = new int[fieldInfos->size()];    // init fieldLengths
    for ( int i=0;i<fieldInfos->size();i++ )
      fieldLengths[i] = 0;
    invertDocument(doc);

    // sort postingTable into an array
    Posting** postings = NULL;
    int postingsLength = 0;
    sortPostingTable(postings,postingsLength);

    // write postings
    writePostings(postings,postingsLength, segment);

    // write norms of indexed fields
    writeNorms(doc, segment);
    delete[] postings;
  }

  void DocumentWriter::sortPostingTable(Posting**& array, int& arraySize) {
    // copy postingTable into an array
    arraySize = postingTable.size();
    array = new Posting*[arraySize];
    map<Term*,Posting*,TermCompare>::iterator postings = postingTable.begin();
    int i=0;
    while ( postings != postingTable.end() ){
      array[i] = (Posting*)postings->second;
      postings++;
      i++;
    }
    // sort the array
    quickSort(array, 0, i - 1);
  }


  // Tokenizes the fields of a document into Postings.
  void DocumentWriter::invertDocument(document::Document& doc){
    document::DocumentFieldEnumeration* fields  = doc.fields();
    while (fields->hasMoreElements()) {
      document::Field* field = (document::Field*)fields->nextElement();
      const char_t* fieldName = field->Name();
      int fieldNumber = fieldInfos->fieldNumber(fieldName);

      int position = fieldLengths[fieldNumber];    // position in field

      if (field->IsIndexed()) {
        if (!field->IsTokenized()) {      // un-tokenized field
          //FEATURE: this is bug in java too: if using a Reader this fails
          if ( field->StringValue() == NULL ){
            util::BasicReader* r = field->ReaderValue();
            int rp = r->position();

            r->seek(0);
            int rl = r->available();

            char_t* rv = new char_t[rl+1];
            r->read(rv,0,rl);
            rv[rl]=0;
            
            addPosition(fieldName,  rv, position++);
            delete[] rv;
            r->seek(rp); //reset position
          }else 
            addPosition(fieldName,  field->StringValue(), position++);
        } else {
          util::BasicReader* reader;        // find or make Reader
          bool delReader = false;
          if (field->ReaderValue() != NULL)
            reader = field->ReaderValue();
          else if (field->StringValue() != NULL){
            reader = new util::StringReader(field->StringValue());
            delReader = true;
          }else
            _THROWC(  "field must have either String or Reader value" );

          // Tokenize field and add to postingTable
          analysis::TokenStream& stream = analyzer.tokenStream(fieldName, reader);
          _TRY {
            analysis::Token* t;
            for ( t = (analysis::Token*)stream.next();
                                                      t != NULL;
                                                      t = (analysis::Token*)stream.next()) {
              addPosition(fieldName,  t->TermText(), position++);
              if (position > maxFieldLength) break; //break before delete.
              delete t;
            }
            delete t;
          } _FINALLY ( 
              stream.close(); 
                  delete &stream; 
                  if ( delReader ){ 
                      reader->close();  
                      delete reader; 
                  } 
              );
        }

        fieldLengths[fieldNumber] = position;    // save field length
      }
    }
    delete fields;
  }
 
 
  void DocumentWriter::addPosition(const char_t* field,
                                         const char_t* text,
                                         const int position) {
          
    termBuffer.set(stringDuplicate(field),stringDuplicate(text), false );
                
    Posting* ti = postingTable.get(&termBuffer);
    if (ti != NULL) {          // word seen before
      int freq = ti->freq;
      if (ti->getPositionsLength() == freq) {    // positions array is full
        int *newPositions = new int[freq * 2];    // double size
        int *positions = ti->positions;
        for (int i = 0; i < freq; i++)      // copy old positions to new
          newPositions[i] = positions[i];
        
        delete[] ti->positions;
        ti->positions = newPositions;
        ti->positionsLength = freq*2;
      }
      ti->positions[freq] = position;      // add new position
      ti->freq = freq + 1;        // update frequency
    }
    else {            // word not seen before
      Term* term = new Term( field, text);
      postingTable.put(term->pointer(), new Posting(*term, position));
      term->finalize();
    }
  }

  //static
  void DocumentWriter::quickSort(Posting**& postings, const int lo, const int hi) {
    if(lo >= hi)
      return;

    int mid = (lo + hi) / 2;

    if(postings[lo]->term.compareTo(postings[mid]->term) > 0) {
       Posting* tmp = postings[lo];
      postings[lo] = postings[mid];
      postings[mid] = tmp;
    }

    if(postings[mid]->term.compareTo(postings[hi]->term) > 0) {
      Posting* tmp = postings[mid];
      postings[mid] = postings[hi];
      postings[hi] = tmp;
          
      if(postings[lo]->term.compareTo(postings[mid]->term) > 0) {
        Posting* tmp2 = postings[lo];
        postings[lo] = postings[mid];
        postings[mid] = tmp2;
      }
    }

    int left = lo + 1;
    int right = hi - 1;

    if (left >= right)
      return; 

    const Term& partition = postings[mid]->term; //not kept, so no need to finalize
      
    for( ;; ) {
      while(postings[right]->term.compareTo(partition) > 0)
      --right;
          
      while(left < right && postings[left]->term.compareTo(partition) <= 0)
        ++left;
            
      if(left < right) {
        Posting* tmp = postings[left];
        postings[left] = postings[right];
        postings[right] = tmp;
        --right;
      } else {
        break;
      }
    }
  
    quickSort(postings, lo, left);
    quickSort(postings, left + 1, hi);
  }


  void DocumentWriter::writePostings(Posting** postings, const int postingsLength, const fchar_t* segment){
    OutputStream* freq = NULL;
    OutputStream* prox = NULL;
    TermInfosWriter* tis = NULL;

    try {
        const fchar_t* buf = segmentname( segment, CONST_STRING(".frq"));
      freq = &directory.createFile( buf );
      delete[] buf;
      
      buf = segmentname( segment, CONST_STRING(".prx"));
      prox = &directory.createFile( buf );
      delete[] buf;

      tis = new TermInfosWriter(directory, segment, *fieldInfos);
      TermInfo* ti = new TermInfo();

      for (int i = 0; i < postingsLength; i++) {
        const Posting* posting = postings[i];

        // add an entry to the dictionary with pointers to prox and freq files
        ti->set(1, freq->getFilePointer(), prox->getFilePointer());
        tis->add(posting->term, *ti);
        
        // add an entry to the freq file
        int f = posting->freq;
        if (f == 1)          // optimize freq=1
          freq->writeVInt(1);        // set low bit of doc num.
        else {
          freq->writeVInt(0);        // the document number
          freq->writeVInt(f);        // frequency in doc
        }
        
        int lastPosition = 0;        // write positions
        int* positions = posting->positions;
        for (int j = 0; j < f; j++) {      // use delta-encoding
          int position = positions[j];
          prox->writeVInt(position - lastPosition);
          lastPosition = position;
        }
      }
      delete ti;
        }_FINALLY ( 
            if (freq != NULL) { freq->close(); delete freq; }
            if (prox != NULL) { prox->close(); delete prox; }
            if (tis  != NULL)  { tis->close(); delete tis; } 
        );
  }

  void DocumentWriter::writeNorms(document::Document& doc, const fchar_t* segment) {
    document::DocumentFieldEnumeration* fields  = doc.fields();
    while (fields->hasMoreElements()) {
      document::Field* field
                          = (document::Field*)fields->nextElement();
      if (field->IsIndexed()) {
        int fieldNumber = fieldInfos->fieldNumber(field->Name());

        fchar_t* fb = new fchar_t[MAX_PATH]; 
        fstringPrintF(fb, CONST_STRING("%s.f%d"),segment,fieldNumber);
        OutputStream& norm = directory.createFile(fb);
        delete[] fb;
        
        _TRY {
          norm.writeByte(search::Similarity::normb(fieldLengths[fieldNumber]));
        }_FINALLY ( 
            norm.close();
            delete &norm; 
        );
      }
    }
    delete fields;
  }

}}
