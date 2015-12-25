#include "StdHeader.h"
#ifndef _NSLib_util_BitSet_
#define _NSLib_util_BitSet_

namespace NSLib{ namespace util {

class BitSet
{
	int size;
	bool *bits;
public:
	BitSet ( int _size ):
		size(_size),
		bits( new bool[_size] )
	{	
		for ( int i=0;i<size;i++ )
			bits[i] = false;
	}
	~BitSet(){
		delete[] bits;
	}

	bool get(const int bit) const{
		if ( bit < 0 || bit > size-1 )
		  _THROWC( "bit out of range" );
		return bits[bit];
	}

  void set(const int bit){
		if ( bit < 0 || bit > size-1 )
			_THROWC( "bit out of range" );
		bits[bit] = true;
	}
};

}}
#endif
