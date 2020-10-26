#ifndef __CORE_H__
#define __CORE_H__

#include "tools.h"
#include <riscvISA.h>

// all the possible memories
#include <cacheMemory.h>
#include <memoryInterface.h>
#include <pipelineRegisters.h>

#define DRAM_SIZE ((size_t)1 << 26)
#define STACK_INIT (DRAM_SIZE - 0x1000)

#ifdef USE_CACHE
#define MEMORY_INTERFACE CacheMemory
#endif
#ifndef MEMORY_INTERFACE
#define MEMORY_INTERFACE SimpleMemory
#endif

/******************************************************************************************
 * Stall signals enum
 * ****************************************************************************************
 */
typedef enum { STALL_FETCH = 0, STALL_DECODE = 1, STALL_EXECUTE = 2, STALL_MEMORY = 3, STALL_WRITEBACK = 4 } StallNames;

// This is ugly but otherwise with have a dependency : alu.h includes core.h
// (for pipeline regs) and core.h includes alu.h...

struct Core {
  FtoDC ftoDC;
  DCtoEx dctoEx;
  ExtoMem extoMem;
  MemtoWB memtoWB;

  // Interface size are configured with 4 bytes interface size (32 bits)
#ifdef __VIVADO__
#ifdef USE_CACHE
  MEMORY_INTERFACE<4,16,64> *dm, *im;
#else
  MEMORY_INTERFACE<4> *dm, *im;
#endif // USE_CACHE
#else
  MemoryInterface<4> *dm, *im;
#endif

  HLS_INT(32) regFile[32];
  HLS_UINT(32) pc;

  // stall
  HLS_UINT(1) stallSignals[5] = {0, 0, 0, 0, 0};
  bool stallIm, stallDm;
  unsigned long cycle;
  /// Multicycle operation

  /// Instruction cache
  // unsigned int idata[Sets][Blocksize][Associativity];   // made external for
  // modelsim

  /// Data cache
  // unsigned int ddata[Sets][Blocksize][Associativity];   // made external for
  // modelsim
};

void doCycle(struct Core& core, HLS_UINT(1) globalStall);

#endif // __CORE_H__

