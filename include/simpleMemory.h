#ifndef __SIMPLE_MEMORY_H__
#define __SIMPLE_MEMORY_H__

#include "memory.h"
#include "memoryInterface.h"

template <unsigned int INTERFACE_SIZE> class SimpleMemory : public MemoryInterface<INTERFACE_SIZE> {
public:
  ac_int<32, false>* data;

  SimpleMemory(ac_int<32, false>* arg) { data = arg; }
  void process(ac_int<32, false> addr, memMask mask, memOpType opType, ac_int<INTERFACE_SIZE * 8, false> dataIn,
               ac_int<INTERFACE_SIZE * 8, false>& dataOut, bool& waitOut);
};

#endif //__SIMPLE_MEMORY_H__
