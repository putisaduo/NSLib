#include "StdHeader.h"
#include "TermInfo.h"


namespace NSLib{ namespace index {

	TermInfo::TermInfo():
		docFreq(0),
		freqPointer(0),
		proxPointer(0)
	{
    }

    TermInfo::~TermInfo(){
    }

	TermInfo::TermInfo(const int df, const long_t fp, const long_t pp):
     freqPointer(fp),
     proxPointer(pp),
     docFreq(df)
    {
	}


	TermInfo::TermInfo(const TermInfo& ti) {
		docFreq = ti.docFreq;
		freqPointer = ti.freqPointer;
		proxPointer = ti.proxPointer;
	}

	void TermInfo::set(const int df, const long_t fp, const long_t pp) {
		docFreq = df;
		freqPointer = fp;
		proxPointer = pp;
	}

	void TermInfo::set(const TermInfo& ti) {
		docFreq = ti.docFreq;
		freqPointer = ti.freqPointer;
		proxPointer = ti.proxPointer;
	}

}}

