#ifndef PIPELINE_REGISTERS_H_
#define PIPELINE_REGISTERS_H_

#include "tools.h"
/******************************************************************************************
 * Definition of all pipeline registers
 *
 * ****************************************************************************************
 */

struct ForwardReg {
  HLS_UINT(1) forwardWBtoVal1;
  HLS_UINT(1) forwardWBtoVal2;
  HLS_UINT(1) forwardWBtoVal3;

  HLS_UINT(1) forwardMemtoVal1;
  HLS_UINT(1) forwardMemtoVal2;
  HLS_UINT(1) forwardMemtoVal3;

  HLS_UINT(1) forwardExtoVal1;
  HLS_UINT(1) forwardExtoVal2;
  HLS_UINT(1) forwardExtoVal3;
};

struct FtoDC {
  FtoDC() : pc(0), instruction(0x13), we(1) {}
  HLS_UINT(32) pc;          // PC where to fetch
  HLS_UINT(32) instruction; // Instruction to execute
  HLS_UINT(32) nextPCFetch; // Next pc according to fetch

  // Register for all stages
  bool we;
};

struct DCtoEx {
  HLS_UINT(32) pc; // used for branch
  HLS_UINT(32) instruction;

  HLS_UINT(7) opCode; // opCode = instruction[6:0]
  HLS_UINT(7) funct7; // funct7 = instruction[31:25]
  HLS_UINT(3) funct3; // funct3 = instruction[14:12]

  HLS_INT(32) lhs;   //  left hand side : operand 1
  HLS_INT(32) rhs;   // right hand side : operand 2
  HLS_INT(32) datac; // ST, BR, JAL/R,

  // For branch unit
  HLS_UINT(32) nextPCDC;
  bool isBranch;

  // Information for forward/stall unit
  bool useRs1;
  bool useRs2;
  bool useRs3;
  bool useRd;
  HLS_UINT(5) rs1; // rs1    = instruction[19:15]
  HLS_UINT(5) rs2; // rs2    = instruction[24:20]
  HLS_UINT(5) rs3;
  HLS_UINT(5) rd; // rd     = instruction[11:7]

  // Register for all stages
  bool we;
};

struct ExtoMem {
  HLS_UINT(32) pc;
  HLS_UINT(32) instruction;

  HLS_INT(32) result; // result of the EX stage
  HLS_UINT(5) rd;     // destination register
  bool useRd;
  bool isLongInstruction;
  HLS_UINT(7) opCode; // LD or ST (can be reduced to 2 bits)
  HLS_UINT(3) funct3; // datasize and sign extension bit

  HLS_INT(32) datac; // data to be stored in memory or csr result

  // For branch unit
  HLS_UINT(32) nextPC;
  bool isBranch;

  // Register for all stages
  bool we;
};

struct MemtoWB {
  HLS_UINT(32) result; // Result to be written back
  HLS_UINT(5) rd;      // destination register
  bool useRd;

  HLS_INT(32) address;
  HLS_UINT(32) valueToWrite;
  HLS_UINT(4) byteEnable;
  bool isStore;
  bool isLoad;

  // Register for all stages
  bool we;
};

struct WBOut {
  HLS_UINT(32) value;
  HLS_UINT(5) rd;
  bool useRd;
  bool we;
};

#endif /* PIPELINE_REGISTERS_H_ */
