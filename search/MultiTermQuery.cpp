#include "StdHeader.h"
#include "MultiTermQuery.h"

using namespace NSLib::index;
using namespace NSLib::util;
namespace NSLib{ namespace search{
    /** Set the TermEnum to be used */
    void MultiTermQuery::setEnum(FilteredTermEnum* _enum) {
        this->_enum = _enum;
    }

    /** Constructs a query for terms matching <code>term</code>. */
    MultiTermQuery::MultiTermQuery(Term* t):
        term(t),
		query(NULL),
		_enum(NULL),
		NSLIB_STYLE_TOSTRING(false)
    {
        //this.query = query; //TODO2: this is in NSLib too... what is it doing?
    }
	MultiTermQuery::~MultiTermQuery(){
	}
    
    
    float MultiTermQuery::sumOfSquaredWeights(Searcher& searcher){
        return getQuery()->sumOfSquaredWeights(searcher);
    }
    
    void MultiTermQuery::normalize(const float norm) {
            getQuery()->normalize(norm);
    }
    
    Scorer* MultiTermQuery::scorer(IndexReader& reader) {
        return getQuery()->scorer(reader);
    }
    
    //marked public by search highlighter
    BooleanQuery* MultiTermQuery::getQuery() {
        if (query == NULL) {
            BooleanQuery* q = new BooleanQuery();
            _TRY {
                do {
                    Term* t = _enum->getTerm();
                    if (t != NULL) {
                        TermQuery* tq = new TermQuery(*t);	// found a match
                        tq->setBoost(boost * _enum->difference()); // set the boost
                        q->add(*tq,true, false, false);		// add to q
                    }
                    t->finalize();
                } while (_enum->next());
            } _FINALLY ( _enum->close() );
            query = q;
        }
        return query;
    }
    
    /** Prints a user-readable version of this query. */
    const char_t* MultiTermQuery::toString(const char_t* field){
        StringBuffer buffer;
        
        if (!NSLIB_STYLE_TOSTRING) {
            Query* q = NULL;
            try {
                q = getQuery();
            } catch (...) {}
            if (q != NULL) {
                buffer.append(_T("("));
                buffer.append ( q->toString(field) );
                buffer.append(_T(")") );
                return buffer.ToString();
            }
        }
        
        if ( stringCompare(term->Field(),field)!=0 ) {
            buffer.append(term->Field());
            buffer.append( _T(":"));
        }
        buffer.append(term->Text());
        if (boost != 1.0f) {
            buffer.append( _T("^") );
            buffer.append( boost,1);
        }
        return buffer.ToString();
    }

}}
