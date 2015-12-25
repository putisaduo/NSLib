#include "StdHeader.h"
#ifndef _NSLib_queryParser_QueryParserBase_
#define _NSLib_queryParser_QueryParserBase_

#include "search/BooleanClause.h"
#include "util/VoidList.h"
#include "analysis/AnalysisHeader.h"
#include "analysis/Analyzers.h"
#include "index/Term.h"
#include "search/TermQuery.h"
#include "search/PhraseQuery.h"
#include "search/RangeQuery.h"

using namespace NSLib::search;
using namespace NSLib::util;
using namespace NSLib::analysis;
using namespace NSLib::index;
namespace NSLib{ namespace queryParser{
	// Contains routines that used by QueryParser.
	class QueryParserBase
	{
    protected:
		static const int CONJ_NONE   = 0;
		static const int CONJ_AND    = 1;
		static const int CONJ_OR     = 2;

		static const int MOD_NONE    = 0;
		static const int MOD_NOT     = 10;
		static const int MOD_REQ     = 11;

    public:
		int PhraseSlop; //0


		QueryParserBase();
		~QueryParserBase();

		
		static void throwParserException(const char* message, char ch, int col, int line );

    protected:
		// Adds the next parsed clause.
		void AddClause(VoidList<BooleanClause*>& clauses, int conj, int mods, Query* q);

		// Returns a query for the specified field.
		Query* GetFieldQuery(const char_t* field, Analyzer& analyzer, const char_t* queryText);

		// Returns a range query.
		Query* GetRangeQuery(const char_t* field, Analyzer& analyzer, const char_t* queryText, bool inclusive);
	};
}}
#endif
