#include "StdHeader.h"
#ifndef _NSLib_store_Lock_
#define _NSLib_store_Lock_

using namespace std;

namespace NSLib{ namespace store{
  class NSLock{
  public:
    // Attempt to obtain exclusive access.
    //
    // @return true iff exclusive access is obtained
    virtual bool obtain() = 0;

    // Release exclusive access. 
    virtual void release() = 0;

    virtual ~NSLock()
    {
    }
  };

    
  // Utility class for executing code with exclusive access. 
  class NSLockWith {
  private:
    const static int sleepInterval = 1000;
    const static int maxSleeps = 10;
    
  protected:
    NSLock* lock;
    // Code to execute with exclusive access. 
    virtual void* doBody() = 0;

  // Constructs an executor that will grab the named lock.
  public:
    ~NSLockWith();

    //Calls {@link #doBody} while <i>lock</i> is obtained.  Blocks if lock
    //cannot be obtained immediately.  Retries to obtain lock once per second
    //until it is obtained, or until it has tried ten times.
    void* run();
  };
}}
#endif
