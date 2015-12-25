#include "StdHeader.h"
#ifndef _NSLib_analysis_StandardFilter
#define _NSLib_analysis_StandardFilter

#include "AnalysisHeader.h"
#include "Analyzers.h"
#include "StandardTokenizerConstants.h"
#include "util/StringBuffer.h"

using namespace std;
using namespace NSLib::analysis;
namespace NSLib{ namespace analysis {

	// Normalizes tokens extracted with {@link StandardTokenizer}. 
	class StandardFilter: public TokenFilter{
	public:
		// Construct filtering <i>in</i>. 
		StandardFilter(TokenStream* in, bool deleteTokenStream);

		~StandardFilter();

		// Returns the next token in the stream, or NULL at EOS.
		// <p>Removes <tt>'s</tt> from the end of words.
		// <p>Removes dots from acronyms.
		Token* next();
	};
}}
#endif
