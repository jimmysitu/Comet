#ifndef __CORE_H__
#define __CORE_H__

#include <ac_int.h>
#include <riscvISA.h>

// all the possible memories
#include <cacheMemory.h>
#include <memoryInterface.h>
#include <pipelineRegisters.h>

#define DRAM_SIZE ((size_t)1 << 26)
#define STACK_INIT (DRAM_SIZE - 0x1000)

#ifndef MEMORY_INTERFACE
#define MEMORY_INTERFACE SimpleMemory
#endif

/******************************************************************************************
 * Stall signals enum
 * ****************************************************************************************
 */
typedef enum { STALL_FETCH = 0, STALL_DECODE = 1, STALL_EXECUTE = 2, STALL_MEMORY = 3, STALL_WRITEBACK = 4 } StallNames;

// This is ugly but otherwise with have a dependency : alu.h includes core.h (for pipeline regs) and core.h includes
// alu.h...

#define NB_CORES 8

class ReqAckUnit {
public:
  ac_int<NB_CORES, false> status, maskOut;
  ac_int<NB_CORES, false> notifyIn, notifyOut, statusIn;

  ac_int<3, false> state = 0;

  bool process(struct DCtoEx dctoEx, ac_int<32, false>& result, bool& stall);
};

class MultiplicationUnit {
public:
  ac_int<32, false> quotient, remainder;
  // ac_int<33, false>
  ac_int<6, false> state = 0;
  bool resIsNeg;
  int i;
  ac_int<32, false> dataAUnsigned, dataBUnsigned;

  bool process(struct DCtoEx dctoEx, ac_int<32, false>& result, bool& stall);
};

struct Core {
  FtoDC ftoDC;
  DCtoEx dctoEx;
  ExtoMem extoMem;
  MemtoWB memtoWB;

  MultiplicationUnit multiplicationUnit;

  MemoryInterface<4>*dm, *im;

  ac_int<32, true> regFile[32];
  ac_int<32, false> pc;

  // stall
  bool stallSignals[5] = {0, 0, 0, 0, 0};
  bool stallIm, stallDm, stallMultAlu;
  unsigned long cycle;
  /// Multicycle operation

  /// Instruction cache
  // unsigned int idata[Sets][Blocksize][Associativity];   // made external for
  // modelsim

  /// Data cache
  // unsigned int ddata[Sets][Blocksize][Associativity];   // made external for
  // modelsim
};

// Functions for copying values
void copyFtoDC(struct FtoDC& dest, struct FtoDC src);
void copyDCtoEx(struct DCtoEx& dest, struct DCtoEx src);
void copyExtoMem(struct ExtoMem& dest, struct ExtoMem src);
void copyMemtoWB(struct MemtoWB& dest, struct MemtoWB src);

void doCycle(struct Core& core, bool globalStall);

#endif // __CORE_H__
