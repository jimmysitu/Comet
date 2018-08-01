#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "portability.h"

struct MemoryController
{
    ac_int<32, false> address;
    unsigned int datum;
    bool IorD;          // 0 = instruction, 1 = data, useless both cache snoop whatever they need
    bool writeenable;
    bool fetch;         // 0 = writeback, 1 = fetch
};

void controller(unsigned int memory[DRAM_SIZE], // main memory
                ac_int<32, false> iaddress,     // address from instruction cache
                ac_int<32, false> daddress,     // address from data cache
                ac_int<32, false>& address,     // (useless?)address from memory, used to check if data received is correct
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
