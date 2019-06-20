#include "simpleMemory.h"
/*
void SimpleMemory::setByte(ca_uint<32> addr, ca_uint<8> data) {
  data[addr>>2].set_slc(addr.slc<2>(0) << 3, data);
}

ca_uint<8> SimpleMemory::getByte(ca_uint<32> addr) {
  return data[addr>>2].slc<8>(0);
}

void SimpleMemory::setWord(ca_uint<32> addr, ca_uint<32> data) {
  data[addr>>2] = data;
}

ca_uint<32> SimpleMemory::getWord(ca_uint<32> addr) {
  return  data[addr>>2];
}
*/
void SimpleMemory::process(ca_uint<32> addr, memMask mask, memOpType opType, ca_uint<32> dataIn, ca_uint<32>& dataOut, bool& waitOut) {
  //no latency, wait is always set to false

  ca_int<32> temp;
  ca_uint<8> t8;
  ca_int<1> bit;
  ca_uint<16> t16;

  switch(opType) {
    case STORE:
      switch(mask) {
        case BYTE:
          temp = data[addr>>2];
          data[addr>>2].set_slc(((int) addr.slc<2>(0)) << 3, dataIn.slc<8>(0));
          break;
        case HALF:
          temp = data[addr>>2];
          data[addr>>2].set_slc(addr[1] ? 16 : 0, dataIn.slc<16>(0));

          break;
        case WORD:
          temp = data[addr>>2];
          data[addr>>2] = dataIn;
          break;
      }
      break;
    case LOAD:
      switch(mask) {
        case BYTE:
          t8 = data[addr>>2].slc<8>(((int)addr.slc<2>(0)) << 3);
          bit = t8.slc<1>(7);
          dataOut.set_slc(0, t8);
          dataOut.set_slc(8, (ca_int<24>)bit);
          break;
        case HALF:
          t16 = data[addr>>2].slc<16>(addr[1] ? 16 : 0);
          bit = t16.slc<1>(15);
          dataOut.set_slc(0, t16);
          dataOut.set_slc(16, (ca_int<16>)bit);
          break;
        case WORD:
          dataOut = data[addr>>2];
          break;
        case BYTE_U:
          dataOut = data[addr>>2].slc<8>(((int) addr.slc<2>(0))<<3) & 0xff;
          break;
        case HALF_U:
          dataOut = data[addr>>2].slc<16>(addr[1] ? 16 : 0)& 0xffff;
          break;
      }

      break;
  }
  waitOut = false;
}
