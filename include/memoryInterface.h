#ifndef __MEMORY_INTERFACE_H__
#define __MEMORY_INTERFACE_H__

#include <ca_int.h>

typedef enum {
  BYTE = 0,
  HALF,
  WORD,
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
  virtual void process(ca_uint<32> addr, memMask mask, memOpType opType, ca_uint<32> dataIn, ca_uint<32>& dataOut, bool& waitOut) =0;
};

#endif //__MEMORY_INTERFACE_H__
