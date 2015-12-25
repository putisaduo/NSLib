#include "StdHeader.h"
#ifndef _NSLib_queryParser_QueryParserConstants_
#define _NSLib_queryParser_QueryParserConstants_

namespace NSLib{ namespace queryParser{

	enum QueryTokenTypes
	{
		AND_,
		OR,
		NOT,
		PLUS,
		MINUS,
		LPAREN,
		RPAREN,
		COLON,
		CARAT,
		QUOTED,
		TERM,
		FUZZY,
		SLOP,
		PREFIXTERM,
		WILDTERM,
		RANGEIN,
		RANGEEX,
		NUMBER,
		EOF_
	};

}}

#endif
