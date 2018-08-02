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
    if(ienable)
    {
        memdatum = memory[iaddress >> 2];
        simul(cycles += MEMORY_READ_LATENCY;)
        idatumvalid = true;
        ddatumvalid = false;
    }
    else if(denable)
    {
        if(daddress == 0x223a0)
            gdebug("test\n");
        if(writeenable)
        {
            memory[daddress >> 2] = coredatum;
            simul(cycles += MEMORY_WRITE_LATENCY;)
        }
        else
        {
            memdatum = memory[daddress >> 2];
            simul(cycles += MEMORY_READ_LATENCY;)
        }

        idatumvalid = false;
        ddatumvalid = true;
    }
    else
    {
        idatumvalid = false;
        ddatumvalid = false;
        memdatum = 0;
    }
}
