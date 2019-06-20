#ifndef __CORE_H__
#define __CORE_H__

#include <ca_int.h>
#include <riscvISA.h>

//all the possible memories
#include <simpleMemory.h>
#include <incompleteMemory.h>
#include <cacheMemory.h>
#include <alu.h>
#include <pipelineRegisters.h>

#ifndef MEMORY_INTERFACE
#define MEMORY_INTERFACE SimpleMemory
#endif


/******************************************************************************************
* Stall signals enum
* ****************************************************************************************
*/
typedef enum {
	STALL_FETCH = 0,
	STALL_DECODE = 1,
	STALL_EXECUTE = 2,
	STALL_MEMORY = 3,
	STALL_WRITEBACK = 4
} StallNames;


//This is ugly but otherwise with have a dependency : alu.h includes core.h (for pipeline regs) and core.h includes alu.h...



struct Core
{
    FtoDC ftoDC;
    DCtoEx dctoEx;
    ExtoMem extoMem;
    MemtoWB memtoWB;

    BasicAlu basicALU;
	MultAlu multALU;

	//memories, yay
	MemoryInterface *dm, *im;

    //CoreCtrl ctrl;

    ca_int<32> regFile[32];
    ca_uint<32> pc;

	//stall
	bool stallSignals[5] = {0, 0, 0, 0, 0};
    bool stallIm, stallDm, stallAlu;
    unsigned long cycle;
    /// Multicycle operation

    /// Instruction cache
    //unsigned int idata[Sets][Blocksize][Associativity];   // made external for modelsim


    /// Data cache
    //unsigned int ddata[Sets][Blocksize][Associativity];   // made external for modelsim

};

//Functions for copying values
void copyFtoDC(struct FtoDC &dest, struct FtoDC src);
void copyDCtoEx(struct DCtoEx &dest, struct DCtoEx src);
void copyExtoMem(struct ExtoMem &dest, struct ExtoMem src);
void copyMemtoWB(struct MemtoWB &dest, struct MemtoWB src);

void doCycle(struct Core &core, bool globalStall);



#endif  // __CORE_H__
