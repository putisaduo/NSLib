#include "StdHeader.h"
#include "Similarity.h"

#include "index/Term.h"
#include "SearchHeader.h"


using namespace std;
namespace NSLib{ namespace search {
	
static float _SIMILARITY_NORM_TABLE[256];
static bool _SIMILARITY_NORM_TABLE_MADE=false;

//static 
float* Similarity::SIMILARITY_NORM_TABLE(){
	if ( !_SIMILARITY_NORM_TABLE_MADE ){
		for (int i = 0; i < 256; i++)
			_SIMILARITY_NORM_TABLE[i] = i / 255.0F;
		_SIMILARITY_NORM_TABLE_MADE = true;
	}
	return _SIMILARITY_NORM_TABLE;
}

//static 
l_byte_t Similarity::normb(const int numTerms) {
	// Scales 1/sqrt(numTerms) into a byte, i.e. 256/sqrt(numTerms).
	// Math.ceil is used to ensure that even very long_t documents don't get a
	// zero norm byte, as that is reserved for zero-lengthed documents and
	// deleted documents.
	return (l_byte_t) ceil(255.0 / sqrt( (double)numTerms));
}

//static 
float Similarity::normf(const l_byte_t normByte) {
	// Un-scales from the byte encoding of a norm into a float, i.e.,
	// approximately 1/sqrt(numTerms).
	return SIMILARITY_NORM_TABLE()[normByte & 0xFF];
}

//static 
float Similarity::tf(const int freq) {
	return (float)floatSquareRoot((float)freq);
}

//static 
float Similarity::tf(const float freq) {
	return (float)floatSquareRoot(freq);
}
		
//static 
float Similarity::idf(const NSLib::index::Term& term, const Searcher& searcher) {
	// Use maxDoc() instead of numDocs() because its proportional to docFreq(),
	// i.e., when one is inaccurate, so is the other, and in the same way.
	return idf(searcher.docFreq(term), searcher.maxDoc());
}

//static 
float Similarity::idf(const int docFreq, const int numDocs){
	return (float)(floatLog(numDocs/(float)(docFreq+1)) + 1.0);
}
	
//static 
float Similarity::coord(const int overlap, const int maxOverlap) {
	return overlap / (float)maxOverlap;
}

}}
