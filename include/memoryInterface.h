#ifndef __MEMORY_INTERFACE_H__
#define __MEMORY_INTERFACE_H__

#include <ac_int.h>


extern ac_int<1, false> interruptTimer, interruptSoftware;
extern ac_int<64, false> mtime;
extern ac_int<64, false> mtimecmp;
extern ac_int<15, false> divider;

typedef enum {
  BYTE = 0,
  HALF,
  WORD,
  LONG,
  BYTE_U,
  HALF_U
} memMask;

typedef enum {
  NONE = 0,
  LOAD,
  STORE
} memOpType;

class MemoryInterface {
protected:
  bool wait;

public:
  virtual void process(ac_int<32, false> addr, memMask mask, memOpType opType, bool lockRelease, ac_int<4, false> hartid, ac_int<32, false> dataIn, ac_int<32, false>& dataOut, bool& waitOut) =0;
};


#endif //__MEMORY_INTERFACE_H__
