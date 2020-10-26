#ifndef __MEMORY_INTERFACE_H__
#define __MEMORY_INTERFACE_H__

#include "tools.h"


typedef enum { BYTE = 0, HALF, WORD, BYTE_U, HALF_U, LONG } memMask;

typedef enum { NONE = 0, LOAD, STORE } memOpType;

template <unsigned int INTERFACE_SIZE> class MemoryInterface {
protected:
  bool wait;

public:
#ifdef __VIVADO__
  void process(HLS_UINT(32) addr, memMask mask, memOpType opType, HLS_UINT(INTERFACE_SIZE * 8) dataIn,
                       HLS_UINT(INTERFACE_SIZE * 8)& dataOut, bool& waitOut){}
#else
  virtual void process(HLS_UINT(32) addr, memMask mask, memOpType opType, HLS_UINT(INTERFACE_SIZE * 8) dataIn,
                       HLS_UINT(INTERFACE_SIZE * 8)& dataOut, bool& waitOut) = 0;
#endif
};

template <unsigned int INTERFACE_SIZE> class IncompleteMemory : public MemoryInterface<INTERFACE_SIZE> {
public:
  HLS_UINT(32)* data;

public:
  IncompleteMemory(HLS_UINT(32)* arg) { data = arg; }
  void process(HLS_UINT(32) addr, memMask mask, memOpType opType, HLS_UINT(INTERFACE_SIZE * 8) dataIn,
               HLS_UINT(INTERFACE_SIZE * 8)& dataOut, bool& waitOut)
  {

    // Incomplete memory only works for 32 bits
    assert(INTERFACE_SIZE == 4);

    // no latency, wait is always set to false
    waitOut = false;
    if (opType == STORE) {
      data[(addr >> 2) & 0xffffff] = dataIn;
    } else if (opType == LOAD) {
      dataOut = data[(addr >> 2) & 0xffffff];
    }
  }
};

template <unsigned int INTERFACE_SIZE> class SimpleMemory : public MemoryInterface<INTERFACE_SIZE> {
public:
  HLS_UINT(32)* data;

  SimpleMemory(HLS_UINT(32)* arg) { data = arg; }
  void process(HLS_UINT(32) addr, memMask mask, memOpType opType, HLS_UINT(INTERFACE_SIZE * 8) dataIn,
               HLS_UINT(INTERFACE_SIZE * 8)& dataOut, bool& waitOut)
  {

    HLS_INT(32) temp;
    HLS_UINT(8) t8;
    HLS_INT(1) bit;
    HLS_UINT(16) t16;

    // no latency, wait is always set to 0
    waitOut = false;
    switch (opType) {
      case STORE:
        switch (mask) {
          case BYTE_U:
          case BYTE:
            temp = data[addr >> 2];
            data[addr >> 2].SET_SLC(((int)addr.SLC(2, 0)) << 3, dataIn.template SLC(8, 0));
            break;
          case HALF:
          case HALF_U:
            temp = data[addr >> 2];
            data[addr >> 2].SET_SLC(addr[1] ? 16 : 0, dataIn.template SLC(16, 0));

            break;
          case WORD:
            temp            = data[addr >> 2];
            data[addr >> 2] = dataIn;
            break;
          case LONG:
            for (int oneWord = 0; oneWord < INTERFACE_SIZE / 4; oneWord++)
              data[(addr >> 2) + oneWord] = dataIn.template SLC(32, (32 * oneWord));
            break;
        }
        break;
      case LOAD:
        switch (mask) {
          case BYTE:
            t8  = data[addr >> 2].SLC(8, (((int)addr.SLC(2, 0)) << 3));
            bit = t8.SLC(1, 7);
            dataOut.SET_SLC(0, t8);
            dataOut.SET_SLC(8, (HLS_INT(24))bit);
            break;
          case HALF:
            t16 = data[addr >> 2].SLC(16, (addr[1] ? 16 : 0));
            bit = t16.SLC(1, 15);
            dataOut.SET_SLC(0, t16);
            dataOut.SET_SLC(16, (HLS_INT(16))bit);
            break;
          case WORD:
            dataOut = data[addr >> 2];
            break;
          case LONG:
            for (int oneWord = 0; oneWord < INTERFACE_SIZE / 4; oneWord++)
              dataOut.SET_SLC(32 * oneWord, data[(addr >> 2) + oneWord]);
            break;
          case BYTE_U:
            dataOut = data[addr >> 2].SLC(8, (((int)addr.SLC(2, 0)) << 3)) & 0xff;
            break;
          case HALF_U:
            dataOut = data[addr >> 2].SLC(16, (addr[1] ? 16 : 0)) & 0xffff;
            break;
        }
        break;
      default:
        break;

    }
  }
};

#endif //__MEMORY_INTERFACE_H__

