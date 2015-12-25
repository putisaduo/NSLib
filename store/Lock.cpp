#include "StdHeader.h"
#include "Lock.h"
#include <iostream>

using namespace std;

namespace NSLib{ namespace store{

  NSLockWith::~NSLockWith(){
  }

  //Calls {@link #doBody} while <i>lock</i> is obtained.  Blocks if lock
  //cannot be obtained immediately.  Retries to obtain lock once per second
  //until it is obtained, or until it has tried ten times.
  void* NSLockWith::run() {
    bool locked = false;
    void* ret = NULL;
     _TRY {
/*
       locked = lock->obtain();
       int sleepCount = 0;
       while (!locked) {
         if (++sleepCount == maxSleeps) {
           cerr << "Timed out waiting for lock." << endl;
           _THROWC("Timed out waiting for lock.");
         }

         processSleep(sleepInterval);
         locked = lock->obtain();
       }
*/
       ret = doBody();
        
     }_FINALLY( 
       if (locked) 
         lock->release();
     );

     return ret;
  }
}}
