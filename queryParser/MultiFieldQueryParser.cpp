#include "StdHeader.h"
#include "MultiFieldQueryParser.h"
#include "analysis/AnalysisHeader.h"
#include "search/BooleanQuery.h"
#include "search/SearchHeader.h"
#include "QueryParser.h"

using namespace NSLib::index;
using namespace NSLib::util;
using namespace NSLib::search;
using namespace NSLib::analysis;

namespace NSLib{ namespace queryParser{

    //static 
    Query& MultiFieldQueryParser::Parse(const char_t* query, const char_t** fields, const int fieldsLen, Analyzer& analyzer)
    {
        BooleanQuery* bQuery = new BooleanQuery();
        for (int i = 0; i < fieldsLen; i++)
        {
			Query& q = QueryParser::Parse(query, fields[i], analyzer);
            bQuery->add(q, true, false, false);
        }
        return *bQuery;
    }

    //static 
    Query& MultiFieldQueryParser::Parse(const char_t* query, const char_t** fields, const int fieldsLen, const l_byte_t* flags, Analyzer& analyzer)
    {
        BooleanQuery* bQuery = new BooleanQuery();
        for (int i = 0; i < fieldsLen; i++)
        {
			Query& q = QueryParser::Parse(query, fields[i], analyzer);
            l_byte_t flag = flags[i];
            switch (flag)
            {
                case REQUIRED_FIELD:
                    bQuery->add(q, true, true, false);
                    break;
                case PROHIBITED_FIELD:
                    bQuery->add(q, true, false, true);
                    break;
                default:
                    bQuery->add(q, true, false, false);
                    break;
            }
        }
        return *bQuery;
    }
}}
