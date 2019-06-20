#ifndef __SIMPLE_MEMORY_H__
#define __SIMPLE_MEMORY_H__

#include "memoryInterface.h"
#include "memory.h"

class SimpleMemory: public MemoryInterface {
public:
  ca_uint<32> *data;

  SimpleMemory(ca_uint<32> *arg){
	  data = arg;
  }
  void process(ca_uint<32> addr, memMask mask, memOpType opType, ca_uint<32> dataIn, ca_uint<32>& dataOut, bool& waitOut);
};

#endif //__SIMPLE_MEMORY_H__
