#ifndef __SIMULATOR_H__
#define __SIMULATOR_H__

#include "core.h"

class Simulator {
protected:
  Core core;
  bool exitFlag;
  bool crashFlag;

public:
  virtual void run()
  {
    crashFlag = false;
    exitFlag = false;
    while (!exitFlag) {
      doCycle(core, 0, crashFlag);
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
