#include "StdHeader.h"
#ifndef _NSLib_search_TopDocs_
#define _NSLib_search_TopDocs_

#include <string>
#include "ScoreDoc.h"

using namespace std;

namespace NSLib{ namespace search {

class TopDocs
{
public:
	const int totalHits;
	ScoreDoc** scoreDocs;
	const int scoreDocsLength;
	bool deleteScoreDocs;

  string groupby_str;

	//TopDocs takes ownership of the ScoreDoc array
	TopDocs(const int th, ScoreDoc **sds, const int sdLength, string groupby_str);
	~TopDocs();
};

}}
#endif

