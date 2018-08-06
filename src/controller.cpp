#include "controller.h"

void controller(unsigned int memory[DRAM_SIZE],
                ac_int<32, false> iaddress, ac_int<32, false> daddress,
                unsigned int& memdatum, unsigned int coredatum,
                bool ienable, bool denable, bool writeenable,
                bool& idatumvalid, bool& ddatumvalid
            #ifndef __HLS__
                , ac_int<64, false>& cycles
            #endif
                )
{
    if(denable)     // prioritize data for the nocache version
    {
        if(writeenable)
        {
            memory[daddress >> 2] = coredatum;
            simul(cycles += MEMORY_WRITE_LATENCY;)
            gdebug("mW   @%06x   %08x\n", daddress.to_int(), coredatum);
        }
        else
        {
            memdatum = memory[daddress >> 2];
            simul(cycles += MEMORY_READ_LATENCY;)
            gdebug("mR   @%06x   %08x\n", daddress.to_int(), memdatum);
        }

        idatumvalid = false;
        ddatumvalid = true;
    }
    else if(ienable)
    {
        memdatum = memory[iaddress >> 2];
        simul(cycles += MEMORY_READ_LATENCY;)
        idatumvalid = true;
        ddatumvalid = false;
    }
    else
    {
        idatumvalid = false;
        ddatumvalid = false;
        memdatum = 0;
    }
}
