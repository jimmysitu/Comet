#ifndef __SIMULATOR_H__
#define __SIMULATOR_H__

#include "core.h"

class Simulator
{
protected:
    Core core;
    bool exitFlag;

public:
    virtual void run()
    {
        exitFlag = false;
        while(!exitFlag){
            doCycle(core, 0);
            solveSyscall();
            extend();
            everyCycle();
        }
        printStat();
    }

    virtual void everyCycle() = 0;
    virtual void printStat() = 0;
    virtual void extend() = 0;
    virtual void solveSyscall() = 0;
};

#endif // __SIMULATOR_H__
