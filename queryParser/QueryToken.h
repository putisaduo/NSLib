#include "StdHeader.h"
#ifndef _NSLib_queryParser_QueryToken_
#define _NSLib_queryParser_QueryToken_

#include "QueryParserConstants.h"

namespace NSLib{ namespace queryParser{

	// Token class that used by QueryParser.
	class QueryToken
	{
	public:
		//Internal constant.
		const char_t* Value;
		//Internal constant.
		const int Start;
		//Internal constant.
		const int End;
		//Internal constant.
		const QueryTokenTypes Type;

		// Initializes a new instance of the Token class NSLIB_EXPORT.
		QueryToken(const char_t* value, const int start, const int end, const QueryTokenTypes type);

		~QueryToken();

		// Initializes a new instance of the Token class NSLIB_EXPORT.
		QueryToken(const char_t* value, const QueryTokenTypes type);

		// Initializes a new instance of the Token class NSLIB_EXPORT.
		QueryToken(const QueryTokenTypes type);

		// Returns a string representation of the Token.
		//public override string ToString();
	};
}}
#endif
