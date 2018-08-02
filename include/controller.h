#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "portability.h"

void controller(unsigned int memory[DRAM_SIZE], // main memory
                ac_int<32, false> iaddress,     // address from instruction cache
                ac_int<32, false> daddress,     // address from data cache
                unsigned int& memdatum,         // datum from memory
                unsigned int coredatum,         // datum from data cache
                bool ienable,
                bool denable,
                bool writeenable,
                bool& idatumvalid,              // if both cache request at the same time
                bool& ddatumvalid               // this prevents sending wrong data
            #ifndef __HLS__
                , ac_int<64, false>& cycles
            #endif
                );

#endif // CONTROLLER_H
