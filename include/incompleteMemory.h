#ifndef __INCOMPLETE_MEMORY_H__
#define __INCOMPLETE_MEMORY_H__

#include "memoryInterface.h"
#include "memory.h"
#include <ac_int.h>

class IncompleteMemory: public MemoryInterface {
public:
  ac_int<32, false> *data;

public:
  IncompleteMemory(ac_int<32, false> *arg){
	  data = arg;
  }

  void process(ac_int<32, false> addr, memMask mask, memOpType opType, bool lockRelease, ac_int<4, false> hartid, ac_int<32, false> dataIn, ac_int<32, false>& dataOut, bool& waitOut);
};

#endif //__INCOMPLETE_MEMORY_H__
