#include "StdHeader.h"
#ifndef _NSLib_util_BitVector_
#define _NSLib_util_BitVector_

#include "store/Directory.h"

using namespace std;

namespace NSLib{ namespace util{
  class BitVector {
  private:
    int size;
    int count;

    // !!!This is public just so that methods will inline.  Please don't touch.
    l_byte_t* bits;
    

  public:
    // Constructs a vector capable of holding <code>n</code> bits. 
    BitVector(int n);
    ~BitVector();
    
    // Constructs a bit vector from the file <code>name</code> in Directory
    //  <code>d</code>, as written by the; method.
    BitVector(NSLib::store::Directory& d, const fchar_t* name);

    // Sets the value of <code>bit</code> to one. 
    void set(const int bit);

    // Sets the value of <code>bit</code> to zero. 
    void clear(const int bit);

    // Returns <code>true</code> if <code>bit</code> is one and
    //  <code>false</code> if it is zero. 
    bool get(const int bit);

    // Returns the number of bits in this vector.  This is also one greater than
    //  the number of the largest valid bit number. 
    int Size();

    // Returns the total number of one bits in this vector.  This is efficiently
    //  computed and cached, so that, if the vector is not changed, no
    //  recomputation is done for repeated calls. 
    int Count();

    

    // Writes this vector to the file <code>name</code> in Directory
    //  <code>d</code>, in a format that can be read by the constructor {@link
    //  #BitVector(Directory, String)}.  
    void write(NSLib::store::Directory& d, const fchar_t* name);

  };
}}
#endif
