#include "StdHeader.h"
#include "TopDocs.h"

#include "ScoreDoc.h"
namespace NSLib{ namespace search {
	
TopDocs::TopDocs(const int th, ScoreDoc **sds, const int sdLength, 
                 string groupby_str):
	totalHits (th),
	scoreDocs (sds),
	scoreDocsLength( sdLength),
	deleteScoreDocs(true),
  groupby_str(groupby_str)
{
}

TopDocs::~TopDocs(){
	if ( deleteScoreDocs ){
		for ( int i=0;i<scoreDocsLength;i++ )
			delete scoreDocs[i];
	}
	delete[] scoreDocs;
}

}}
