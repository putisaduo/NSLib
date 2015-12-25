#include "StdHeader.h"

#ifndef _NSLib_util_htmlhighlighter_
#define _NSLib_util_htmlhighlighter_

#include <string>
#include <vector>
#include <wchar.h>

#ifndef _WIN32
# include <ctype.h>
# include <wctype.h>
#endif

using namespace std;

namespace NSLib{ namespace util{
    class HTMLHighLighter{
    public:
		static char_t* highlight(const char_t* content, const char_t* keywords, bool detail);
	private:
		static vector<NSLibString>* getKeywordList(const char_t* keywords);
		static int matchWord(const char_t* content, const char_t* keyword, int from);
		static char_t* trim(const char_t* content);
		static bool isChineseWord(const char_t* word);
	};
}}

#endif
