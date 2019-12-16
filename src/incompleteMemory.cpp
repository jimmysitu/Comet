#include <incompleteMemory.h>

#define UART_BASE 0x10013000
#define CLINT_BASE 0x02000000


#ifdef __CATAPULT__
ac_int<64, false> mtime;
ac_int<64, false> mtimecmp;
ac_int<15, false> divider;
ac_int<1, false> interruptTimer, interruptSoftware = 0;
#endif

void IncompleteMemory::process(ac_int<32, false> addr, memMask mask, memOpType opType, bool lockRelease, ac_int<4, false> hartid, ac_int<32, false> dataIn, ac_int<32, false>& dataOut, bool& waitOut) {
	//no latency, wait is always set to false
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

	else if (opType == STORE){
		data[addr>>2] = dataIn;
	}
	else if (opType == LOAD){
		dataOut = data[addr>>2];
	}

}
