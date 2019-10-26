#ifndef __UARTInterface_H
#define __UARTInterface_H

#include <memoryInterface.h>

class UARTInterface : public MemoryInterface {
public:

    UARTInterface(){};

    void process(ac_int<32, false> addr, memMask mask, memOpType opType, bool lockRelease, ac_int<4, false> hartid, ac_int<32, false> dataIn, ac_int<32, false>& dataOut, bool& waitOut){
        #ifdef __HLS__
        if ((addr == 0x10013018 || addr == 0x10013008 || addr == 0x10013000) && opType == STORE){
      		//Some initialisation
    		if (addr == 0x10013000)
    			fprintf(stderr, "%c", (char) dataIn);
      	}
        #endif
    };

};


#endif
