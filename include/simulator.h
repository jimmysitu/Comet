#ifndef __SIMULATOR_H__
#define __SIMULATOR_H__

#include "core.h"

class Simulator {
protected:
  Core cores[NB_CORES];
  bool exitFlag;

public:
  virtual void run()
  {
    exitFlag = false;
    while (!exitFlag) {
      for (int oneCore = 0; oneCore < NB_CORES; oneCore++)
        doCycle(cores[oneCore], 0);
      solveSyscall();
      extend();
      printCycle();
    }
    printEnd();
  }

  virtual void printCycle()   = 0;
  virtual void printEnd()     = 0;
  virtual void extend()       = 0;
  virtual void solveSyscall() = 0;
};

#endif // __SIMULATOR_H__
