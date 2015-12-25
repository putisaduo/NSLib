#include "StdHeader.h"
#include "StandardFilter.h"

#include "AnalysisHeader.h"
#include "Analyzers.h"
#include "StandardTokenizerConstants.h"
#include "util/StringBuffer.h"

using namespace std;
using namespace NSLib::analysis;
namespace NSLib{ namespace analysis {

	StandardFilter::StandardFilter(TokenStream* in, bool deleteTokenStream):
	  TokenFilter(in, deleteTokenStream)
	{
	}

	StandardFilter::~StandardFilter(){
	}

	/** Returns the next token in the stream, or NULL at EOS.
	* <p>Removes <tt>'s</tt> from the end of words.
	* <p>Removes dots from acronyms.
	*/
	Token* StandardFilter::next() {
		Token* t = input->next();

		if (t == NULL)
			return NULL;

		const char_t* text = t->TermText();
		const int textLength = stringLength(text);
		const char_t* type = t->Type();

		if ( stringCompare(type, tokenImage[APOSTROPHE])==0 && ( stringICompare(text+textLength-2, _T("'s"))==0  ) ) {
			// remove 's
			char_t* buf = stringDuplicate(text);
			buf[textLength-2]=0;
			Token* ret = new Token( buf, t->StartOffset(), t->EndOffset(), type);
			delete[] buf;
			return ret;

		} else if ( stringCompare(type, tokenImage[ACRONYM])==0 ) {		  // remove dots
			StringBuffer trimmed;
			for (int i = 0; i < textLength; i++) {
				char_t c = text[i];
				if (c != '.')
					trimmed.append(c);
			}
			return new Token (trimmed.getBuffer(), t->StartOffset(), t->EndOffset(), type);

		} else {
			return t;
		}

	}
}}
