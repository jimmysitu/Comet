#ifndef __MEMORY_INTERFACE_H__
#define __MEMORY_INTERFACE_H__

#include <ac_int.h>

// Define the BYTE size of the memory interface (that is the size of dataIn and dataOut)
//#define INTERFACE_SIZE 8
// #define LOG_INTERFACE_SIZE 3

typedef enum { BYTE = 0, HALF, WORD, BYTE_U, HALF_U, VECT } memMask;

typedef enum { NONE = 0, LOAD, STORE } memOpType;

template <unsigned int INTERFACE_SIZE> class MemoryInterface {
protected:
  bool wait;

public:
  virtual void process(ac_int<32, false> addr, memMask mask, memOpType opType, ac_int<INTERFACE_SIZE * 8, false> dataIn,
                       ac_int<INTERFACE_SIZE * 8, false>& dataOut, bool& waitOut) = 0;
};

#endif //__MEMORY_INTERFACE_H__
