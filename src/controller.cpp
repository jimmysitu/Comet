#include "controller.h"

void controller(unsigned int memory[DRAM_SIZE],
                ac_int<32, false> iaddress, ac_int<32, false> daddress, ac_int<32, false>& address,
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
        memdatum = memory[iaddress];
        idatumvalid = true;
        ddatumvalid = false;
        address = iaddress;
    }
    else if(denable)
    {
        if(writeenable)
            memory[daddress] = coredatum;
        else
            memdatum = memory[daddress];

        idatumvalid = false;
        ddatumvalid = true;
        address = daddress;
    }
    else
    {
        idatumvalid = false;
        ddatumvalid = false;
        address = 0;
    }
}
