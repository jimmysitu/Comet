#ifndef __CORE_H__
#define __CORE_H__

#include <ac_int.h>
#include <riscvISA.h>

// all the possible memories
#include <alu.h>
#include <cacheMemory.h>
#include <floatAlu.h>
#include <incompleteMemory.h>
#include <pipelineRegisters.h>
#include <simpleMemory.h>

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

struct Core {
  FtoDC ftoDC;
  DCtoEx dctoEx;
  ExtoMem extoMem;
  MemtoWB memtoWB;

  BasicAlu basicALU;
  MultAlu multALU;
  FloatAlu floatALU;
  // memories, yay
  MemoryInterface *dm, *im;

  // CoreCtrl ctrl;

  ac_int<32, true> regFile[64];
  ac_int<32, false> pc;

  // stall
  bool stallSignals[5] = {0, 0, 0, 0, 0};
  bool stallIm, stallDm, stallAlu;
  unsigned long cycle;
  /// Multicycle operation

  /// Instruction cache
  // unsigned int idata[Sets][Blocksize][Associativity];   // made external for modelsim

  /// Data cache
  // unsigned int ddata[Sets][Blocksize][Associativity];   // made external for modelsim
};

// Functions for copying values
void copyFtoDC(struct FtoDC& dest, struct FtoDC src);
void copyDCtoEx(struct DCtoEx& dest, struct DCtoEx src);
void copyExtoMem(struct ExtoMem& dest, struct ExtoMem src);
void copyMemtoWB(struct MemtoWB& dest, struct MemtoWB src);

void doCycle(struct Core& core, bool globalStall);

#endif // __CORE_H__
