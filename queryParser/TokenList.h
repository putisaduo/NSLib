#include "StdHeader.h"
#ifndef _NSLib_queryParser_TokenList_
#define _NSLib_queryParser_TokenList_

#include "util/VoidList.h"
#include "QueryToken.h"
namespace NSLib{ namespace queryParser{

	// Represents a list of the tokens.
	class TokenList
	{
	private:
		NSLib::util::VoidList<QueryToken*> tokens;

    public:
		TokenList();
		~TokenList();

		void Add(QueryToken* token);

		void Push(QueryToken* token);

		QueryToken& Peek();

		QueryToken& Extract();

		int Count();
	};
}}
#endif
