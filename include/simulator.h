#ifndef __SIMULATOR_H__
#define __SIMULATOR_H__

#include "core.h"

class Simulator
{
protected:
    Core cores[2];
    bool exitFlag;

public:
    virtual void run()
    {
        exitFlag = false;
        while(!exitFlag){
            doCycle(cores[0], 0);
//            doCycle(cores[1], 0);
            solveSyscall();
            extend();
            printCycle();
        }
        printStat();
    }

    virtual void printCycle() = 0;
    virtual void printStat() = 0;
    virtual void extend() = 0;
    virtual void solveSyscall() = 0;
};

#endif // __SIMULATOR_H__
