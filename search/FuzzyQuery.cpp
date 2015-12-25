#include "StdHeader.h"
#include "FuzzyQuery.h"

namespace NSLib{ namespace search{

	FuzzyTermEnum::FuzzyTermEnum(IndexReader& reader, Term* term):
		FilteredTermEnum(reader,term),
		searchTerm(term),
		field(term->Field()),
		text( stringDuplicate(term->Text()) ),
		distance(0),
		fieldMatch(false),
		endEnum(false),
		eWidth(0),
		eHeight(0)
	{
		textlen = stringLength(text);

		Term* t = new Term(searchTerm->Field(), _T("") );
		TermEnum* terms = &reader.getTerms(t);
		setEnum( terms );
	}
	FuzzyTermEnum::~FuzzyTermEnum(){
		delete[] text;
	}

	bool FuzzyTermEnum::EndEnum() {
		return endEnum;
	}

	void FuzzyTermEnum::close(){
		FilteredTermEnum::close();
	}

	bool FuzzyTermEnum::termCompare(Term* term) {
		if ( stringCompare(field, term->Field())==0 ) {
			char_t* target = stringDuplicate(term->Text());
			int targetlen = stringLength(target);

			int dist = editDistance(text, target, textlen, targetlen);
			distance = 1 - ((double)dist / (double)min(textlen, targetlen));
			delete[] target;

			return (distance > FUZZY_THRESHOLD);
		}
		endEnum = true;
		return false;
	}

	float FuzzyTermEnum::difference() {
		return (float)((distance - FUZZY_THRESHOLD) * SCALE_FACTOR);
	}

	//static
	int FuzzyTermEnum::Min(const int a, const int b, const int c){
		int t = (a < b) ? a : b;
		return (t < c) ? t : c;
	}

	int FuzzyTermEnum::editDistance(char_t* s, char_t* t, int n, int m) {
		int i; // iterates through s
		int j; // iterates through t
		char_t s_i; // ith character of s
        
		if (n == 0) return m;
		if (m == 0) return n;
        
		if (eWidth <= n || eHeight <= m) {
			//resize e
			if ( eWidth > 0 || eHeight > 0 )
				delete[] e;
			eWidth = max(eWidth, n+1);
			eHeight = max(eHeight, m+1);
			e = new int[eWidth*eHeight];
		}
		int* d = e; // matrix
		
		// init matrix d
		for (i = 0; i <= n; i++) d[i + (0*eWidth)] = i;
		for (j = 0; j <= m; j++) d[0 + (j*eWidth)] = j;
        
		// start computing edit distance
		for (i = 1; i <= n; i++) {
			s_i = s[i - 1];
			for (j = 1; j <= m; j++) {
				if (s_i != t[j-1])
					d[i + (j*eWidth)] = Min(d[i-1 + (j*eWidth)], d[i + ((j-1)*eWidth)], d[i-1 + ((j-1)*eWidth)])+1;
				else d[i + ((j)*eWidth)] = Min(d[i-1 + ((j)*eWidth)]+1, d[i + ((j-1)*eWidth)]+1, d[i-1 + ((j-1)*eWidth)]);
			}
		}
        
		// we got the result!
		return d[n + ((m)*eWidth)];
	}





   
	FuzzyQuery::FuzzyQuery(Term* term):
		MultiTermQuery(term),
		fuzzyTerm(term->pointer())
	{
	    MultiTermQuery::NSLIB_STYLE_TOSTRING = true;
    }
    
    FuzzyQuery::~FuzzyQuery(){
        fuzzyTerm->finalize();
    }
    
    void FuzzyQuery::prepare(IndexReader& reader) {
        try {
            setEnum(new FuzzyTermEnum(reader, fuzzyTerm));
        } catch (...) {
        }
    }
    
    const char_t* FuzzyQuery::toString(const char_t* field){
        StringBuffer buffer;
        const char_t* b = MultiTermQuery::toString(field);
		buffer.append ( b );
		delete[] b;
        buffer.append( _T("~") );
        
        return buffer.ToString();
    }

	const char_t* FuzzyQuery::getQueryName() const{
		return _T("FuzzyQuery");
	}
}}
