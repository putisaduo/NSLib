#include "StdHeader.h"
#include "Arrays.h"

#include <algorithm>
#include <functional>

using namespace std;
namespace NSLib{ namespace util{
	
void Arrays::arraycopy (const char_t* src, const int srcStart, char_t* dest, const int destStart, const int length){
	for ( int i=0;i<length;i++ ){
		dest[destStart+i] = src[srcStart+i];
	}
}
void Arrays::arraycopy (const l_byte_t* src, const int srcStart, l_byte_t* dest, const int destStart, const int length){
	for ( int i=0;i<length;i++ ){
		dest[destStart+i] = src[srcStart+i];
	}
}

}}
