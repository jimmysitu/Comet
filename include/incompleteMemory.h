#ifndef __INCOMPLETE_MEMORY_H__
#define __INCOMPLETE_MEMORY_H__

#include "memoryInterface.h"
#include "memory.h"

class IncompleteMemory: public MemoryInterface {
public:
  ca_uint<32> *data;

public:
  IncompleteMemory(ca_uint<32> *arg){
	  data = arg;
  }

  void process(ca_uint<32> addr, memMask mask, memOpType opType, ca_uint<32> dataIn, ca_uint<32>& dataOut, bool& waitOut) {
    //no latency, wait is always set to false
    waitOut = false;
    if (opType == STORE){
        data[addr>>2] = dataIn;
    }
    else if (opType == LOAD){
        dataOut = data[addr>>2];
    }

  }
};

#endif //__INCOMPLETE_MEMORY_H__
