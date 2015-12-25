#include "StdHeader.h"
#ifndef _NSLib_queryParser_Lexer_
#define _NSLib_queryParser_Lexer_

#include "QueryParserConstants.h"

#include "util/FastCharStream.h"
#include "util/Reader.h"
#include "util/StringBuffer.h"

#include "TokenList.h"

using namespace NSLib::util;
namespace NSLib{ namespace queryParser{
	// A simple Lexer that is used by QueryParser.
	class Lexer
	{
	private:
		TokenList tokens;
		FastCharStream& reader;
		bool delSR;

    public:
		// Initializes a new instance of the Lexer class with the specified
		// query to lex.
		Lexer(const char_t* query);

		// Initializes a new instance of the Lexer class with the specified
		// TextReader to lex.
		Lexer(BasicReader& source);

		// Breaks the input stream onto the tokens list and returns it.
		// <returns>The tokens list.</returns>
		TokenList& Lex();
		
		~Lexer();

	private:
		QueryToken* GetNextToken();

		// Reads an integer number
		const char_t* ReadIntegerNumber(const char_t ch);

		// Reads an inclusive range like [some words]
		QueryToken* ReadInclusiveRange(const char_t prev);

		// Reads an exclusive range like {some words}
		QueryToken* ReadExclusiveRange(const char_t prev);

		// Reads quoted string like "something else"
		QueryToken* ReadQuoted(const char_t prev);

		QueryToken* ReadTerm(const char_t prev);


		const char_t* ReadEscape(const char_t prev);
	};
}}
#endif
