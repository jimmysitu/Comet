#ifndef CORE_H
#define CORE_H

#ifndef __VIVADO__
#include <ac_int.h>
#endif

#include <portability.h>
#include "riscvISA.h"


#define INSTR_MEMORY_WIDTH 32
#define DATA_MEMORY_WIDTH
#define DRAM_SIZE 8192

/******************************************************************************************
 * Definition of all pipeline registers
 *
 * ****************************************************************************************
 */

struct ForwardReg {
	CORE_UINT(1) forwardWBtoVal1;
	CORE_UINT(1) forwardWBtoVal2;
	CORE_UINT(1) forwardWBtoVal3;

	CORE_UINT(1) forwardMemtoVal1;
	CORE_UINT(1) forwardMemtoVal2;
	CORE_UINT(1) forwardMemtoVal3;

	CORE_UINT(1) forwardExtoVal1;
	CORE_UINT(1) forwardExtoVal2;
	CORE_UINT(1) forwardExtoVal3;
};

struct FtoDC
{
    FtoDC() : pc(0), instruction(0x13), we(1), stall(0)
    {}
    CORE_UINT(32) pc;           	// PC where to fetch
    CORE_UINT(32) instruction;  	// Instruction to execute
    CORE_UINT(32) nextPCFetch;      // Next pc according to fetch

    //Register for all stages
    CORE_UINT(1) we;
    CORE_UINT(1) stall;
};

struct DCtoEx
{
    CORE_UINT(32) pc;       // used for branch
    CORE_UINT(32) instruction;

    CORE_UINT(7) opCode;    // opCode = instruction[6:0]
    CORE_UINT(7) funct7;    // funct7 = instruction[31:25]
    CORE_UINT(3) funct3;    // funct3 = instruction[14:12]

    CORE_INT(32) regVal1;   //  left hand side : operand 1
    CORE_INT(32) regVal2;   // right hand side : operand 2
    CORE_INT(32) imm; // ST, BR, JAL/R,

    // syscall only
    CORE_INT(32) datad;
    CORE_INT(32) datae;

    //For branch unit
    CORE_UINT(32) nextPCDC;
    CORE_UINT(1) isBranch;

    //Information for forward/stall unit
    CORE_UINT(1) useRs1;
    CORE_UINT(1) useRs2;
    CORE_UINT(1) useRs3;
    CORE_UINT(1) useRd;
    CORE_UINT(5) rs1;       // rs1    = instruction[19:15]
    CORE_UINT(5) rs2;       // rs2    = instruction[24:20]
    CORE_UINT(5) rs3;
    CORE_UINT(5) rd;        // rd     = instruction[11:7]

    //Register for all stages
    CORE_UINT(1) we;
    CORE_UINT(1) stall; //TODO add that
};

struct ExtoMem
{
    CORE_UINT(32) pc;
    CORE_UINT(32) instruction;

    CORE_INT(32) result;    // result of the EX stage
    CORE_UINT(5) rd;        // destination register
    CORE_UINT(1) useRd;
    CORE_UINT(1) isLongInstruction;
    CORE_UINT(7) opCode;    // LD or ST (can be reduced to 2 bits)
    CORE_UINT(3) funct3;    // datasize and sign extension bit

    CORE_INT(32) datac;     // data to be stored in memory or csr result

    //For branch unit
    CORE_UINT(32) nextPC;
    CORE_UINT(1) isBranch;

    //Register for all stages
    CORE_UINT(1) we;
    CORE_UINT(1) stall; //TODO add that
};

struct MemtoWB
{
    CORE_INT(32) result;    // Result to be written back
    CORE_UINT(5) rd;        // destination register
    CORE_UINT(1) useRd;

    CORE_INT(32) address;
    CORE_INT(32) valueToWrite;
    CORE_UINT(4) byteEnable;
    CORE_INT(1) isStore;
    CORE_INT(1) isLoad;
    
    //Register for all stages
    CORE_UINT(1) we;
    CORE_UINT(1) stall;

};

struct WBOut
{
	CORE_UINT(32) value;
	CORE_UINT(5) rd;
	CORE_UINT(1) useRd;
    CORE_UINT(1) we;
};

struct Core
{
    FtoDC ftoDC;
    DCtoEx dctoEx;
    ExtoMem extoMem;
    MemtoWB memtoWB;
    struct WBOut wbOut;

    //CoreCtrl ctrl;

    CORE_INT(32) REG1[32];
    CORE_INT(32) REG2[32];

    CORE_UINT(32) pc;

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

int doCore(CORE_UINT(32) im[DRAM_SIZE], CORE_INT(32) dm[DRAM_SIZE]);

class Simulator;



#endif  // CORE_H
