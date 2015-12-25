#include "StdHeader.h"
#include "QueryToken.h"

#include "QueryParserConstants.h"

namespace NSLib{ namespace queryParser{
    QueryToken::QueryToken(const char_t* value, const int start, const int end, const QueryTokenTypes type):
    	Value( stringDuplicate(value) ),
    	Start(start),
    	End(end),
    	Type(type)
    {
    }
    
    QueryToken::~QueryToken(){
    	delete[] Value;
    }
    
    // Initializes a new instance of the Token class NSLIB_EXPORT.
    //
    QueryToken::QueryToken(const char_t* value, const QueryTokenTypes type):
    	Value(stringDuplicate(value)),
    	Start(0),
    	End(0),
    	Type(type)
    {
    	// use this
    }
    
    // Initializes a new instance of the Token class NSLIB_EXPORT.
    //
    QueryToken::QueryToken(const QueryTokenTypes type):
    	Value(NULL),
    	Start(0),
    	End(0),
    	Type(type)
    {
    	// use this
    }
    
    // Returns a string representation of the Token.
    //
    //QueryToken::ToString()
    //{
    //	return "<" + Type + "> " + Value;	
    //}
}}
