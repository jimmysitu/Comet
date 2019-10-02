#ifndef __SIMPLE_MEMORY_H__
#define __SIMPLE_MEMORY_H__

#include "memoryInterface.h"
#include "memory.h"

class SimpleMemory: public MemoryInterface {
public:
  ac_int<32, false> *data;
  ac_int<4, false> *locks;

  SimpleMemory(ac_int<32, false> *arg, ac_int<4, false> *locks){
	  data = arg;
	  this->locks = locks;
  }
  void process(ac_int<32, false> addr, memMask mask, memOpType opType, bool lockRelease, ac_int<4, false> hartid, ac_int<32, false> dataIn, ac_int<32, false>& dataOut, bool& waitOut);
};

#endif //__SIMPLE_MEMORY_H__
