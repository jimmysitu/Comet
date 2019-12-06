#include "simpleMemory.h"
#include <core.h>

  /*****************************************************************
   * TODO: This should be moved somewhere else
   * Memory mapped IO elements
   * -> CLINT
   * -> UART
   *****************************************************************/

#define UART_BASE 0x10013000
#define CLINT_BASE 0x02000000

/*
void SimpleMemory::setByte(ac_int<32, false> addr, ac_int<8, false> data) {
  data[addr>>2].set_slc(addr.slc<2>(0) << 3, data);
}

ac_int<8, false> SimpleMemory::getByte(ac_int<32, false> addr) {
  return data[addr>>2].slc<8>(0);
}

void SimpleMemory::setWord(ac_int<32, false> addr, ac_int<32, false> data) {
  data[addr>>2] = data;
}

ac_int<32, false> SimpleMemory::getWord(ac_int<32, false> addr) {
  return  data[addr>>2];
}
*/
void SimpleMemory::process(ac_int<32, false> addr, memMask mask, memOpType opType, bool lockRelease, ac_int<4, false> hartid, ac_int<32, false> dataIn, ac_int<32, false>& dataOut, bool& waitOut) {
  //no latency, wait is always set to false

  ac_int<32, true> temp;
  ac_int<8, false> t8;
  ac_int<1, true> bit;
  ac_int<16, false> t16;


  /*****************************************************************
   * TODO: This should be moved somewhere else
   * Memory mapped IO elements
   * -> CLINT
   * -> UART
   *****************************************************************/

  divider++;
  if (divider == 0){
	  mtime++;
	  if (mtime >= mtimecmp){
		  //We raise an interrupt
		  interruptTimer = 1;
	  }
  }

  waitOut = false;


	if (addr >= CLINT_BASE && addr < CLINT_BASE + 0xBFF8+8){
	//We are in the part that control CLINT
		if (addr == CLINT_BASE + 0){
			if (opType == STORE){
				interruptSoftware = (dataIn != 0);
			}
		}
		else if (addr == CLINT_BASE + 0xbff8){
			if (opType == LOAD){
				dataOut = mtime.slc<32>(0);
			}
			else if (opType == STORE){
				mtime.set_slc(0, dataIn);
			}
		}
		else if (addr == CLINT_BASE + 0xbffc){
			if (opType == LOAD){
				dataOut = mtime.slc<32>(32);
			}
			else if (opType == STORE){
				mtime.set_slc(32, dataIn);
			}
		}
		else if (addr == CLINT_BASE + 0x4000){
			if (opType == LOAD){
				dataOut = mtimecmp.slc<32>(0);
			}
			else if (opType == STORE){
				mtimecmp.set_slc(0, dataIn);
				interruptTimer = 0;
			}
		}
		else if (addr == CLINT_BASE + 0x4004){
			if (opType == LOAD){
				dataOut = mtimecmp.slc<32>(32);
			}
			else if (opType == STORE){
				mtimecmp.set_slc(32, dataIn);
				interruptTimer = 0;
			}
		}
	}
	else if (opType == LOAD && addr == 0x10013000){
		dataOut = 0;
	}
	else if ((addr == 0x10013018 || addr == 0x10013008 || addr == 0x10013000) && opType == STORE){
  		//Some initialisation
		if (addr == 0x10013000)
			fprintf(stderr, "%c", (char) dataIn);
  	}
	else if (addr == 0x100abcde && opType == STORE){
  		//Some initialisation
		if (addr == 0x100abcde)
			fprintf(stderr, "%c", (char) dataIn);
  	}
	else {
		switch(opType) {
		case STORE:
			//We ensure that the memory location is not locked
			if (this->locks[addr>>4] == 0xf && this->locks[addr>>4] != hartid){
				waitOut = true; //If we try accessing a locked value, we stall
			}
			else{

				//First we handle the lock mechanism (on a store, we release)
				if (lockRelease)
					this->locks[addr>>4] = 0xf; //We remove the lock

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
			}
		  break;
		case LOAD:
			//We ensure that the memory location is not locked
			if (this->locks[addr>>4] == 0xf && this->locks[addr>>4] != hartid){
				waitOut = true; //If we try accessing a blocked value, we stall
			}
			else{
				//First we handle the lock mechanism (on a load, we lock)
				if (lockRelease)
					this->locks[addr>>4] = 0xf; //We add a lock

				switch(mask) {
				case BYTE:
				  t8 = data[addr>>2].slc<8>(((int)addr.slc<2>(0)) << 3);
				  bit = t8.slc<1>(7);
				  dataOut.set_slc(0, t8);
				  dataOut.set_slc(8, (ac_int<24, true>)bit);
				  break;
				case HALF:
				  t16 = data[addr>>2].slc<16>(addr[1] ? 16 : 0);
				  bit = t16.slc<1>(15);
				  dataOut.set_slc(0, t16);
				  dataOut.set_slc(16, (ac_int<16, true>)bit);
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
			}

		  break;
		}
	}
}
