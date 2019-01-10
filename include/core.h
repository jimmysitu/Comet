#ifndef CORE_H
#define CORE_H

#include "portability.h"
#include "riscvISA.h"
#include "cache.h"
#include "multicycleoperator.h"


#ifndef __HLS__
#define FTODC_WIDTH 4
#define DCTOEX_WIDTH 19
#define EXTOMEM_WIDTH 10
#define MEMTOWB_WIDTH 7
#define CORECTRL_WIDTH 19
#define COREREG_WIDTH 33
#define TOTAL_REG_WIDTH 92
#endif


struct FtoDC
{
    FtoDC()
    : pc(0), instruction(0x13), realInstruction(false), nextpc(0)
    {}
    ac_int<32, false> pc;           // used for JAL, AUIPC & BR
    ac_int<32, false> instruction;  // Instruction to execute
    bool realInstruction;           // Increment for minstret
    ac_int<32, false> nextpc;       // Next pc to store for JAL & JALR

#ifndef __HLS__
    void dumpContents(int* array) {   //need 4 int -> 4*4=16 bytes
        array[0] = (int)pc;
        array[1] = (int)instruction;
        array[2] = (int)realInstruction;
        array[3] = (int)nextpc;
    }
#endif
};

struct DCtoEx
{
    DCtoEx()
    : pc(0),
  #ifndef __HLS__
      instruction(0),
  #endif
      opCode(RISCV_OPI), funct7(0), funct3(RISCV_OPI_ADDI), rd(0), realInstruction(false),
      lhs(0), rhs(0), datac(0), forward_lhs(false), forward_rhs(false), forward_datac(false),
      forward_mem_lhs(false), forward_mem_rhs(false), forward_mem_datac(false),
      csr(false), CSRid(0), external(false), op(MultiCycleOperator::NONE)
  #ifndef __HLS__
      , datad(0), datae(0), memValue(0)
  #endif
    {}

    ac_int<32, false> pc;       // used for branch
#ifndef __HLS__
    ac_int<32, false> instruction;
#endif

    ac_int<7, false> opCode;    // opCode = instruction[6:0]
    ac_int<7, false> funct7;    // funct7 = instruction[31:25]
    ac_int<3, false> funct3;    // funct3 = instruction[14:12]
 // ac_int<5, false> rs1;       // rs1    = instruction[19:15]
 // ac_int<5, false> rs2;       // rs2    = instruction[24:20]
    ac_int<5, false> rd;        // rd     = instruction[11:7]

    bool realInstruction;

    ac_int<32, true> lhs;   //  left hand side : operand 1
    ac_int<32, true> rhs;   // right hand side : operand 2
    ac_int<32, true> datac; // ST, BR, JAL/R,

    bool forward_lhs;
    bool forward_rhs;
    bool forward_datac;
    bool forward_mem_lhs;
    bool forward_mem_rhs;
    bool forward_mem_datac;

    bool csr;
    ac_int<12, false> CSRid;

    bool external;      // used for external operation
    MultiCycleOperator::MultiCycleOperation op;

#ifndef __HLS__
    // syscall only
    ac_int<32, true> datad;
    ac_int<32, true> datae;
    ac_int<32, true> memValue; //Second data, from register file or immediate value
#endif

#ifndef __HLS__
    void dumpContents(int* array) { //need 19 int -> 19*4=76 bytes
        array[0] = (int)pc;
        array[1] = (int)opCode;
        array[2] = (int)funct7;
        array[3] = (int)funct3;
        array[4] = (int)rd;
        array[5] = (int)realInstruction;
        array[6] = (int)lhs;
        array[7] = (int)rhs;
        array[8] = (int)datac;
        array[9] = (int)forward_lhs;
        array[10] = (int)forward_rhs;
        array[11] = (int)forward_datac;
        array[12] = (int)forward_mem_lhs;
        array[13] = (int)forward_mem_rhs;
        array[14] = (int)forward_mem_datac;
        array[15] = (int)csr;
        array[16] = (int)CSRid;
        array[17] = (int)external;
        array[18] = (int)op;
    }
#endif
};

struct ExtoMem
{
    ExtoMem()
    : pc(0),
  #ifndef __HLS__
      instruction(0x13),
  #endif
      result(0), rd(0), opCode(RISCV_OPI), funct3(RISCV_OPI_ADDI), realInstruction(false),
      datac(0), csr(false), CSRid(0)
    {}

    ac_int<32, false> pc;
#ifndef __HLS__
    ac_int<32, false> instruction;
#endif

    ac_int<32, true> result;    // result of the EX stage
    ac_int<5, false> rd;        // destination register
    ac_int<7, false> opCode;    // LD or ST (can be reduced to 2 bits)
    ac_int<3, false> funct3;    // datasize and sign extension bit
    bool realInstruction;
    bool external;

    bool csr;
    ac_int<12, false> CSRid;

    ac_int<32, true> datac;     // data to be stored in memory or csr result

#ifndef __HLS__
    void dumpContents(int* array) { //need 10 int -> 10*4=40 bytes
        array[0] = (int)pc;
        array[1] = (int)result;
        array[2] = (int)rd;
        array[3] = (int)opCode;
        array[4] = (int)funct3;
        array[5] = (int)realInstruction;
        array[6] = (int)external;
        array[7] = (int)csr;
        array[8] = (int)CSRid;
        array[9] = (int)datac;
    }
#endif

};

struct MemtoWB
{
    MemtoWB()
    : pc(0),
  #ifndef __HLS__
      instruction(0x13),
  #endif
      result(0), rd(0), realInstruction(false), csr(false), CSRid(0), rescsr(0)
    {}
    ac_int<32, false> pc;           // !!!: only used for debug and tracing
#ifndef __HLS__
    ac_int<32, false> instruction;  // !!!: only used for debug and tracing
#endif

    ac_int<32, true> result;    // Result to be written back
    ac_int<5, false> rd;        // destination register
    bool realInstruction;       // increment minstret ?

    bool csr;
    ac_int<12, false> CSRid;    // CSR to be written back
    ac_int<32, false> rescsr;   // Result for CSR instruction

#ifndef __HLS__
    void dumpContents(int* array) { //need 7 int -> 7*4=28 bytes
        array[0] = (int)pc;
        array[1] = (int)result;
        array[2] = (int)rd;
        array[3] = (int)realInstruction;
        array[4] = (int)csr;
        array[5] = (int)CSRid;
        array[6] = (int)rescsr;
    }
#endif

};

struct CSR
{
    CSR()
    : mcycle(0), minstret(0)
    // some should probably be initialized to some special value
    {}

    ac_int<64, false> mcycle;                   // could be shared according to specification
    ac_int<64, false> minstret;

#ifndef COMET_NO_CSR
    static const ac_int<32, false> mvendorid;   // RO shared by all cores
    static const ac_int<32, false> marchid;     // RO shared by all cores
    static const ac_int<32, false> mimpid;      // RO shared by all cores
    //const ac_int<32, false> mhartid;                  // RO but private to core (and i don't want to template everything)
    ac_int<32, false> mstatus;
    ac_int<32, false> misa;     // writable...
    ac_int<32, false> medeleg;
    ac_int<32, false> mideleg;
    ac_int<32, false> mie;
    ac_int<32, false> mtvec;
    ac_int<32, false> mcounteren;
    ac_int<32, false> mscratch;
    ac_int<32, false> mepc;
    ac_int<32, false> mcause;
    ac_int<32, false> mtval;
    ac_int<32, false> mip;
#endif
};

struct CoreCtrl
{
    CoreCtrl()
    : lock(0), freeze_fetch(false), cachelock(false), init(false), sleep(true)
    {
        #pragma hls_unroll yes
        for(int i(0); i < 3; ++i)
        {
            prev_rds[i] = 0;
            prev_opCode[i] = RISCV_OPI;
            prev_res[i] = 0;
            branch[i] = false;
        }
        #pragma hls_unroll yes
        for(int i(0); i < 2; ++i)
        {
            jump_pc[i] = 0;
        }
    }

    ac_int<5, false> prev_rds[3];
    ac_int<7, false> prev_opCode[3];
    ac_int<2, false> lock;          // used to lock dc stage after JAL & JALR

    bool freeze_fetch;              // used for LD dependencies
    bool cachelock;                 // stall Ft, DC & Ex when cache is working
    bool init;                      // is core initialized?
    bool sleep;                     // sleep core, can be waken up by other core

    // used to break dependencies, because using extoMem or memtoWB
    // implies a dependency from stage ex or mem to dc (i.e. they
    // are not completely independent)...
    ac_int<32, true> prev_res[3];
    bool branch[3];
    ac_int<32, true> jump_pc[2];

#ifndef __HLS__
    void dumpContents(int* array) { //need 19 int -> 19*4=76 bytes
        array[0] = (int)prev_rds[0];
        array[1] = (int)prev_rds[1];
        array[2] = (int)prev_rds[2];
        array[3] = (int)prev_opCode[0];
        array[4] = (int)prev_opCode[1];
        array[5] = (int)prev_opCode[2];
        array[6] = (int)lock;
        array[7] = (int)freeze_fetch;
        array[8] = (int)cachelock;
        array[9] = (int)init;
        array[10] = (int)sleep;
        array[11] = (int)prev_res[0];
        array[12] = (int)prev_res[1];
        array[13] = (int)prev_res[2];
        array[14] = (int)branch[0];
        array[15] = (int)branch[1];
        array[16] = (int)branch[2];
        array[17] = (int)jump_pc[0];
        array[18] = (int)jump_pc[1];
    }
#endif

};

struct Core
{
    Core()
    : ftoDC(), dctoEx(), extoMem(), memtoWB(), csrs(), ctrl(), pc(0),
      irequest(), ireply(), drequest(), dreply()
    {
        #pragma hls_unroll yes
        for(int i(0); i < 32; ++i)
        {
            REG[i] = 0;
        }
        REG[2] = STACK_INIT;
    }

    FtoDC ftoDC;
    DCtoEx dctoEx;
    ExtoMem extoMem;
    MemtoWB memtoWB;
    CSR csrs;

    CoreCtrl ctrl;

    ac_int<32, true> REG[32];
    ac_int<32, false> pc;

    /// Multicycle operation
    MultiCycleOperator mcop;
    MultiCycleRes mcres;

    /// Instruction cache
    //unsigned int idata[Sets][Blocksize][Associativity];   // made external for modelsim
    ICacheRequest irequest;
    ICacheReply ireply;

    /// Data cache
    //unsigned int ddata[Sets][Blocksize][Associativity];   // made external for modelsim
    DCacheRequest drequest;
    DCacheReply dreply;

#ifndef __HLS__
    void dumpContents(int* array) { //need 33 int -> 33*4=132 bytes
        for(int i=0; i<32; i++) {
            array[i] = (int)REG[i];
        }
        array[32] = (int)pc;
    }
#endif
};

class Simulator;

void doStep(ac_int<32, false> startpc, bool &exit,
            MultiCycleOperator& mcop, MultiCycleRes mcres,
            unsigned int im[DRAM_SIZE], unsigned int dm[DRAM_SIZE],
            unsigned int cim[Sets][Blocksize][Associativity], unsigned int cdm[Sets][Blocksize][Associativity],
            ac_int<IWidth, false> memictrl[Sets], ac_int<DWidth, false> memdctrl[Sets]
        #ifndef __HLS__
            , Simulator* syscall
        #endif
            );


#endif  // CORE_H
