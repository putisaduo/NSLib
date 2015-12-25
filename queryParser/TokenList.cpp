#include "StdHeader.h"
#include "TokenList.h"

#include "util/VoidList.h"
#include "QueryToken.h"

namespace NSLib{ namespace queryParser{
    TokenList::TokenList()
    {
    }
    TokenList::~TokenList(){
		tokens.setDoDelete(NSLib::util::DELETE_TYPE_DELETE);
    	tokens.clear();
    }
    
    void TokenList::Add(QueryToken* token)
    {
    	tokens.push_front(token);
    }
    
    void TokenList::Push(QueryToken* token)
    {
    	tokens.push_back(token);
    }
    
    QueryToken& TokenList::Peek()
    {
    	return *tokens[tokens.size()-1];
    }
    
    QueryToken& TokenList::Extract()
    {
    	QueryToken& token = Peek();
    	tokens.pop_back();
    	return token;
    }
    
    int TokenList::Count()
    {
    	return tokens.size();
    }
}}
