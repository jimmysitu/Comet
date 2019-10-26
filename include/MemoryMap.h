#ifndef __MEMORYMAP_H
#define __MEMORYMAP_H

#include <UARTInterface.h>
#include <memoryInterface.h>

class MemoryMap : public MemoryInterface {
public:
    ac_int<32, false> startUart, endUart;
    UARTInterface *uartInterface;
    MemoryInterface *memoryInterface;

    MemoryMap(UARTInterface *uartInterface, MemoryInterface *memoryInterface, ac_int<32, false> startUart, ac_int<32, false> endUart){
        this->uartInterface = uartInterface;
        this->memoryInterface = memoryInterface;
        this->startUart = startUart;
        this->endUart = endUart;
    };

    void process(ac_int<32, false> addr, memMask mask, memOpType opType, bool lockRelease, ac_int<4, false> hartid, ac_int<32, false> dataIn, ac_int<32, false>& dataOut, bool& waitOut){

        memOpType uartOp = NONE, memoryOp = NONE;
        if (addr >= startUart && addr<endUart)
            uartOp = opType;
        else
            memoryOp = opType;

        uartInterface->process(addr, mask, uartOp, lockRelease, hartid, dataIn, dataOut, waitOut);
        memoryInterface->process(addr, mask, opType, lockRelease, hartid, dataIn, dataOut, waitOut);
    };


};


#endif
