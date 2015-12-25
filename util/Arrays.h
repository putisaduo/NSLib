#include "StdHeader.h"
#ifndef _NSLib_util_Arrays_
#define _NSLib_util_Arrays_

#include <algorithm>
#include <functional>

namespace NSLib{ namespace util{

using namespace std;
class Arrays {
	//Sorts the specified array of objects into ascending order, according
	//to the <i>natural comparison method</i> of its elements.  All
	//elements in the array must implement the Comparable interface.
	//Furthermore, all elements in the array must be <i>mutually
	//comparable</i> (that is, e1.compareTo(e2) must not throw a
	//typeMismatchException for any elements e1 and e2 in the array).
	//<p>
	//This sort is guaranteed to be <em>stable</em>:  equal elements will
	//not be reordered as a result of the sort.
	//<p>
	//The sorting algorithm is a modified mergesort (in which the merge is
	//omitted if the highest element in the low sublist is less than the
	//lowest element in the high sublist).  This algorithm offers guaranteed
	//n*log(n) performance, and can approach linear performance on nearly
	//sorted lists.
	//
	//@param a the array to be sorted.
	//@WException ClassCastException array contains elements that are not
	//		  <i>mutually comparable</i> (for example, Strings and
	//		  Integers).
	//@see Comparable
public:
	static void arraycopy (const l_byte_t* src, const int srcStart, l_byte_t* dest, const int destStart, const int length);
	static void arraycopy (const char_t* src, const int srcStart, char_t* dest, const int destStart, const int length);

private:

};

}}

#endif
