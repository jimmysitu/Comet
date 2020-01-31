#include <MemoryMap.h>
#include <UARTInterface.h>
#include <ac_int.h>
#include <cacheMemory.h>
#include <core.h>

#ifndef __HLS__
#include "simulator.h"
#endif // __HLS__

void fetch(ac_int<32, false> pc, struct FtoDC &ftoDC,
           ac_int<32, false> instruction) {
  ftoDC.instruction = instruction;
  ftoDC.pc = pc;
  ftoDC.nextPCFetch = pc + 4;
  ftoDC.we = 1;
}

void decode(struct FtoDC ftoDC, struct DCtoEx &dctoEx,
            ac_int<32, true> registerFile[32]) {

  ac_int<32, false> pc = ftoDC.pc;
  ac_int<32, false> instruction = ftoDC.instruction;

  // R-type instruction
  ac_int<7, false> funct7 = instruction.slc<7>(25);
  ac_int<5, false> rs2 = instruction.slc<5>(20);
  ac_int<5, false> rs1 = instruction.slc<5>(15);
  ac_int<3, false> funct3 = instruction.slc<3>(12);
  ac_int<5, false> rd = instruction.slc<5>(7);
  ac_int<7, false> opCode = instruction.slc<7>(
      0); // could be reduced to 5 bits because 1:0 is always 11

  // Construction of different immediate values
  ac_int<12, false> imm12_I = instruction.slc<12>(20);
  ac_int<12, false> imm12_S = 0;
  imm12_S.set_slc(5, instruction.slc<7>(25));
  imm12_S.set_slc(0, instruction.slc<5>(7));

  ac_int<12, true> imm12_I_signed = instruction.slc<12>(20);
  ac_int<12, true> imm12_S_signed = 0;
  imm12_S_signed.set_slc(0, imm12_S.slc<12>(0));

  ac_int<13, false> imm13 = 0;
  imm13[12] = instruction[31];
  imm13.set_slc(5, instruction.slc<6>(25));
  imm13.set_slc(1, instruction.slc<4>(8));
  imm13[11] = instruction[7];

  ac_int<13, true> imm13_signed = 0;
  imm13_signed.set_slc(0, imm13);

  ac_int<32, true> imm31_12 = 0;
  imm31_12.set_slc(12, instruction.slc<20>(12));

  ac_int<21, false> imm21_1 = 0;
  imm21_1.set_slc(12, instruction.slc<8>(12));
  imm21_1[11] = instruction[20];
  imm21_1.set_slc(1, instruction.slc<10>(21));
  imm21_1[20] = instruction[31];

  ac_int<21, true> imm21_1_signed = 0;
  imm21_1_signed.set_slc(0, imm21_1);

  ac_int<5, false> funct5 = instruction.slc<5>(27);

  // Register access
  ac_int<32, false> valueReg1 = registerFile[rs1];
  ac_int<32, false> valueReg2 = registerFile[rs2];

  dctoEx.rs1 = rs1;
  dctoEx.rs2 = rs2;
  dctoEx.rs3 = rs2;
  dctoEx.rd = rd;
  dctoEx.opCode = opCode;
  dctoEx.funct3 = funct3;
  dctoEx.funct7 = funct7;
  dctoEx.instruction = instruction;
  dctoEx.pc = pc;

  // Initialization of control bits
  dctoEx.useRs1 = 0;
  dctoEx.useRs2 = 0;
  dctoEx.useRd = 0;
  dctoEx.we = ftoDC.we;
  dctoEx.isBranch = 0;

  switch (opCode) {
  case RISCV_LUI:
    dctoEx.lhs = imm31_12;
    dctoEx.useRs1 = 0;
    dctoEx.useRs2 = 0;
    dctoEx.useRs3 = 0;
    dctoEx.useRd = 1;

    break;
  case RISCV_AUIPC:
    dctoEx.lhs = ftoDC.pc;
    dctoEx.rhs = imm31_12;
    dctoEx.useRs1 = 0;
    dctoEx.useRs2 = 0;
    dctoEx.useRs3 = 0;
    dctoEx.useRd = 1;
    break;
  case RISCV_JAL:
    dctoEx.lhs = ftoDC.pc + 4;
    dctoEx.rhs = 0;
    dctoEx.nextPCDC = ftoDC.pc + imm21_1_signed;
    dctoEx.useRs1 = 0;
    dctoEx.useRs2 = 0;
    dctoEx.useRs3 = 0;
    dctoEx.useRd = 1;
    dctoEx.isBranch = 1;

    break;
  case RISCV_JALR:
    dctoEx.lhs = valueReg1;
    dctoEx.rhs = imm12_I_signed;
    dctoEx.useRs1 = 1;
    dctoEx.useRs2 = 0;
    dctoEx.useRs3 = 0;
    dctoEx.useRd = 1;
    break;
  case RISCV_BR:

    dctoEx.lhs = valueReg1;
    dctoEx.rhs = valueReg2;
    dctoEx.useRs1 = 1;
    dctoEx.useRs2 = 1;
    dctoEx.useRs3 = 0;
    dctoEx.useRd = 0;

    break;
  case RISCV_LD:

    dctoEx.lhs = valueReg1;
    dctoEx.rhs = imm12_I_signed;
    dctoEx.useRs1 = 1;
    dctoEx.useRs2 = 0;
    dctoEx.useRs3 = 0;
    dctoEx.useRd = 1;

    break;

    //******************************************************************************************
    // Treatment for: STORE INSTRUCTIONS
  case RISCV_ST:
    dctoEx.lhs = valueReg1;
    dctoEx.rhs = imm12_S_signed;
    dctoEx.datac = valueReg2; // Value to store in memory
    dctoEx.useRs1 = 1;
    dctoEx.useRs2 = 0;
    dctoEx.useRs3 = 1;
    dctoEx.useRd = 0;
    dctoEx.rd = 0;
    break;
  case RISCV_OPI:
    dctoEx.lhs = valueReg1;
    dctoEx.rhs = imm12_I_signed;
    dctoEx.useRs1 = 1;
    dctoEx.useRs2 = 0;
    dctoEx.useRs3 = 0;
    dctoEx.useRd = 1;
    break;

  case RISCV_OP:

    dctoEx.lhs = valueReg1;
    dctoEx.rhs = valueReg2;
    dctoEx.useRs1 = 1;
    dctoEx.useRs2 = 1;
    dctoEx.useRs3 = 0;
    dctoEx.useRd = 1;

    break;
  case RISCV_SYSTEM:
    if (funct3 != 0) {
      // We are in a CSR
      dctoEx.useRs1 = 1;
      dctoEx.useRs2 = 0;
      dctoEx.useRs3 = 0;
      dctoEx.useRd = 1;
      dctoEx.lhs = valueReg1;
    }

    break;
  case RISCV_ATOMIC:
    dctoEx.useRd = 1;
    dctoEx.useRs1 = 1;
    dctoEx.useRs2 = 0;

    dctoEx.lhs = valueReg1;
    dctoEx.rhs = 0;
    dctoEx.datac = valueReg2;

    // The Atomic LR does not use rs3, all other does
    if (funct5 == RISCV_ATOMIC_LR)
      dctoEx.useRs3 = 0;
    else
      dctoEx.useRs3 = 1;

    break;
  default:

    break;
  }

  // If dest is zero, useRd should be at zero
  if (rd == 0) {
    dctoEx.useRd = 0;
  }

  // If the instruction was dropped, we ensure that isBranch is at zero
  if (!ftoDC.we) {
    dctoEx.isBranch = 0;
    dctoEx.useRd = 0;
    dctoEx.useRs1 = 0;
    dctoEx.useRs2 = 0;
    dctoEx.useRs3 = 0;
  }
}

void memory(struct ExtoMem extoMem, struct MemtoWB &memtoWB) {

  ac_int<2, false> datasize = extoMem.funct3.slc<2>(0);
  bool signenable = !extoMem.funct3.slc<1>(2);
  memtoWB.we = extoMem.we;
  memtoWB.useRd = extoMem.useRd;
  memtoWB.result = extoMem.result;
  memtoWB.rd = extoMem.rd;

  ac_int<5, false> funct5 = extoMem.instruction.slc<5>(27);
  ac_int<32, false> mem_read;

  switch (extoMem.opCode) {
  case RISCV_LD:
    memtoWB.rd = extoMem.rd;

    memtoWB.address = extoMem.result;
    memtoWB.isLoad = 1;
    //    formatread(extoMem.result, datasize, signenable, mem_read); //TODO
    break;
  case RISCV_ST:

    //        mem_read = dataMemory[extoMem.result >> 2];
    // if(extoMem.we) //TODO0: We do not handle non 32bit writes
    //        	dataMemory[extoMem.result >> 2] = extoMem.datac;
    memtoWB.isStore = 1;
    memtoWB.address = extoMem.result;
    memtoWB.valueToWrite = extoMem.datac;
    memtoWB.byteEnable = 0xf;

    break;
  case RISCV_ATOMIC:
    if (funct5 == RISCV_ATOMIC_LR) {
      memtoWB.rd = extoMem.rd;
      memtoWB.address = extoMem.result;
      memtoWB.isLoad = 1;
    } else if (funct5 == RISCV_ATOMIC_SC) {
      memtoWB.isStore = 1;
      memtoWB.address = extoMem.result;
      memtoWB.valueToWrite = extoMem.datac;
      memtoWB.byteEnable = 0xf;
    } else {
      memtoWB.address = extoMem.result;
      memtoWB.isAtomic = 1;
      memtoWB.valueToWrite = extoMem.datac;
      memtoWB.atomicCode = funct5;
    }

    break;
  }
}

void writeback(struct MemtoWB memtoWB, struct WBOut &wbOut) {
  wbOut.we = memtoWB.we;
  if ((memtoWB.rd != 0) && (memtoWB.we) && memtoWB.useRd) {
    wbOut.rd = memtoWB.rd;
    wbOut.value = memtoWB.result;
    wbOut.useRd = 1;
  }
}

void branchUnit(ac_int<32, false> nextPC_fetch, ac_int<32, false> nextPC_decode,
                bool isBranch_decode, ac_int<32, false> nextPC_execute,
                bool isBranch_execute, ac_int<32, false> &pc, bool &we_fetch,
                bool &we_decode, bool we_execute, bool stall_fetch) {

  if (!stall_fetch) {
    if (isBranch_execute && we_execute) {
      we_fetch = 0;
      we_decode = 0;
      pc = nextPC_execute;
    } else if (isBranch_decode && we_decode) {
      we_fetch = 0;
      pc = nextPC_decode;
    } else if (we_fetch) {
      pc = nextPC_fetch;
    }
  }
}

void forwardUnit(ac_int<5, false> decodeRs1, bool decodeUseRs1,
                 ac_int<5, false> decodeRs2, bool decodeUseRs2,
                 ac_int<5, false> decodeRs3, bool decodeUseRs3,

                 ac_int<5, false> executeRd, bool executeUseRd,
                 bool executeIsLongComputation,

                 ac_int<5, false> memoryRd, bool memoryUseRd,

                 ac_int<5, false> writebackRd, bool writebackUseRd,

                 bool stall[5], struct ForwardReg &forwardRegisters) {

  if (decodeUseRs1) {
    if (executeUseRd && decodeRs1 == executeRd) {
      if (executeIsLongComputation) {
        stall[0] = 1;
        stall[1] = 1;
      } else {
        forwardRegisters.forwardExtoVal1 = 1;
      }
    } else if (memoryUseRd && decodeRs1 == memoryRd) {
      forwardRegisters.forwardMemtoVal1 = 1;
    } else if (writebackUseRd && decodeRs1 == writebackRd) {
      forwardRegisters.forwardWBtoVal1 = 1;
    }
  }

  if (decodeUseRs2) {
    if (executeUseRd && decodeRs2 == executeRd) {
      if (executeIsLongComputation) {
        stall[0] = 1;
        stall[1] = 1;
      } else {
        forwardRegisters.forwardExtoVal2 = 1;
      }
    } else if (memoryUseRd && decodeRs2 == memoryRd)
      forwardRegisters.forwardMemtoVal2 = 1;
    else if (writebackUseRd && decodeRs2 == writebackRd)
      forwardRegisters.forwardWBtoVal2 = 1;
  }

  if (decodeUseRs3) {
    if (executeUseRd && decodeRs3 == executeRd) {
      if (executeIsLongComputation) {
        stall[0] = 1;
        stall[1] = 1;
      } else {
        forwardRegisters.forwardExtoVal3 = 1;
      }
    } else if (memoryUseRd && decodeRs3 == memoryRd)
      forwardRegisters.forwardMemtoVal3 = 1;
    else if (writebackUseRd && decodeRs3 == writebackRd)
      forwardRegisters.forwardWBtoVal3 = 1;
  }
}

void doCycle(struct Core &core, // Core containing all values
             bool globalStall) {
  bool localStall = globalStall;

  core.stallSignals[0] = 0;
  core.stallSignals[1] = 0;
  core.stallSignals[2] = 0;
  core.stallSignals[3] = 0;
  core.stallSignals[4] = 0;
  core.stallIm = false;
  core.stallDm = false;
  core.stallAlu = false;
  bool localStallAlu = false;

  // declare temporary structs
  struct FtoDC ftoDC_temp;
  ftoDC_temp.pc = 0;
  ftoDC_temp.instruction = 0;
  ftoDC_temp.nextPCFetch = 0;
  ftoDC_temp.we = 0;
  struct DCtoEx dctoEx_temp;
  dctoEx_temp.isBranch = 0;
  dctoEx_temp.useRs1 = 0;
  dctoEx_temp.useRs2 = 0;
  dctoEx_temp.useRs3 = 0;
  dctoEx_temp.useRd = 0;
  dctoEx_temp.we = 0;
  struct ExtoMem extoMem_temp;
  extoMem_temp.useRd = 0;
  extoMem_temp.isBranch = 0;
  extoMem_temp.we = 0;
  struct MemtoWB memtoWB_temp;
  memtoWB_temp.useRd = 0;
  memtoWB_temp.isStore = 0;
  memtoWB_temp.we = 0;
  memtoWB_temp.isLoad = 0;
  memtoWB_temp.isAtomic = 0;
  struct WBOut wbOut_temp;
  wbOut_temp.useRd = 0;
  wbOut_temp.we = 0;
  wbOut_temp.rd = 0;
  struct ForwardReg forwardRegisters;
  forwardRegisters.forwardExtoVal1 = 0;
  forwardRegisters.forwardExtoVal2 = 0;
  forwardRegisters.forwardExtoVal3 = 0;
  forwardRegisters.forwardMemtoVal1 = 0;
  forwardRegisters.forwardMemtoVal2 = 0;
  forwardRegisters.forwardMemtoVal3 = 0;
  forwardRegisters.forwardWBtoVal1 = 0;
  forwardRegisters.forwardWBtoVal2 = 0;
  forwardRegisters.forwardWBtoVal3 = 0;

  // declare temporary register file
  ac_int<32, false> nextInst, multResult = 0, csrResult = 0;
  bool csrSetPc = false;

  if (!localStall && !core.stallDm)
    core.im->process(core.pc, WORD, LOAD, false, core.csrUnit.mhartid.slc<4>(0),
                     0, nextInst, core.stallIm);

  fetch(core.pc, ftoDC_temp, nextInst);
  decode(core.ftoDC, dctoEx_temp, core.regFile);
  core.basicALU.process(core.dctoEx, extoMem_temp,
                        core.stallAlu); // calling ALU: execute stage
  bool multUsed = core.multALU.process(
      core.dctoEx, multResult, core.stallAlu); // calling ALU: execute stage
  bool csrUsed = core.csrUnit.process(core.dctoEx, csrResult, csrSetPc);
  if (multUsed)
    extoMem_temp.result = multResult;
  if (csrUsed)
    extoMem_temp.result = csrResult;
  if (csrSetPc) {
    extoMem_temp.nextPC = csrResult;
    extoMem_temp.isBranch = true;
  }

  memory(core.extoMem, memtoWB_temp);
  writeback(core.memtoWB, wbOut_temp);

  // We update localStall value according to stallAlu
  localStall |= core.stallAlu;

  // resolve stalls, forwards
  if (!localStall)
    forwardUnit(dctoEx_temp.rs1, dctoEx_temp.useRs1, dctoEx_temp.rs2,
                dctoEx_temp.useRs2, dctoEx_temp.rs3, dctoEx_temp.useRs3,
                extoMem_temp.rd, extoMem_temp.useRd,
                extoMem_temp.isLongInstruction, memtoWB_temp.rd,
                memtoWB_temp.useRd, wbOut_temp.rd, wbOut_temp.useRd,
                core.stallSignals, forwardRegisters);

  if (!core.stallSignals[STALL_MEMORY] && !localStall && memtoWB_temp.we &&
      !core.stallIm) {

    memMask mask = static_cast<memMask>(core.extoMem.funct3.to_uint());
    // TODO: carry the data size to memToWb
    //       switch (core.extoMem.funct3) {
    //         case 0:
    //          mask = BYTE;
    //          break;
    //         case 1:
    //          mask = HALF;
    //          break;
    //        case 2:
    //          mask = WORD;
    //          break;
    //        case 4:
    //          mask = BYTE_U;
    //          break;
    //        case 5:
    //          mask = HALF_U;
    //          break;
    //        //Should NEVER happen
    //        default:
    //          mask = WORD;
    //          break;
    //       } TODO
    core.dm->process(
        memtoWB_temp.address, mask,
        memtoWB_temp.isLoad ? LOAD : (memtoWB_temp.isStore ? STORE : NONE),
        false, core.csrUnit.mhartid.slc<4>(0), memtoWB_temp.valueToWrite,
        memtoWB_temp.result, core.stallDm);
  }
  // We commit the fetch stage if not stalled, not under cache stall and not
  // under global stall
  if (!core.stallSignals[STALL_FETCH] && !localStall && !core.stallIm &&
      !core.stallDm) {
    core.ftoDC = ftoDC_temp;
  }

  // We commit the decode stage if not stalled, not under cache stall and not
  // under global stall
  if (!core.stallSignals[STALL_DECODE] && !localStall && !core.stallIm &&
      !core.stallDm) {
    core.dctoEx = dctoEx_temp;

    // Commitings means also solving the forwarding
    if (forwardRegisters.forwardExtoVal1 && extoMem_temp.we)
      core.dctoEx.lhs = extoMem_temp.result;
    else if (forwardRegisters.forwardMemtoVal1 && memtoWB_temp.we)
      core.dctoEx.lhs = memtoWB_temp.result;
    else if (forwardRegisters.forwardWBtoVal1 && wbOut_temp.we)
      core.dctoEx.lhs = wbOut_temp.value;

    if (forwardRegisters.forwardExtoVal2 && extoMem_temp.we)
      core.dctoEx.rhs = extoMem_temp.result;
    else if (forwardRegisters.forwardMemtoVal2 && memtoWB_temp.we)
      core.dctoEx.rhs = memtoWB_temp.result;
    else if (forwardRegisters.forwardWBtoVal2 && wbOut_temp.we)
      core.dctoEx.rhs = wbOut_temp.value;

    if (forwardRegisters.forwardExtoVal3 && extoMem_temp.we)
      core.dctoEx.datac = extoMem_temp.result;
    else if (forwardRegisters.forwardMemtoVal3 && memtoWB_temp.we)
      core.dctoEx.datac = memtoWB_temp.result;
    else if (forwardRegisters.forwardWBtoVal3 && wbOut_temp.we)
      core.dctoEx.datac = wbOut_temp.value;
  }

  // If the decode is stalled but not the execute (and if there is no cache miss
  // and so on), we insert a bubble in the pipeline
  if (core.stallSignals[STALL_DECODE] && !core.stallSignals[STALL_EXECUTE] &&
      !core.stallIm && !core.stallDm && !localStall) {
    core.dctoEx.we = 0;
    core.dctoEx.useRd = 0;
    core.dctoEx.isBranch = 0;
    core.dctoEx.instruction = 0;
    core.dctoEx.pc = 0;
  }

  // We commit the execute stage if not stalled, not under cache stall and not
  // under global stall
  if (!core.stallSignals[STALL_EXECUTE] && !localStall && !core.stallIm &&
      !core.stallDm) {
    core.extoMem = extoMem_temp;
  }

  // We commit the memory stage if not stalled, not under cache stall and not
  // under global stall
  if (!core.stallSignals[STALL_MEMORY] && !localStall && !core.stallIm &&
      !core.stallDm) {
    core.memtoWB = memtoWB_temp;
  }

  // We commit the WB stage if not stalled, not under cache stall and not under
  // global stall and if WB is enabled
  if (wbOut_temp.we && wbOut_temp.useRd && !localStall && !core.stallIm &&
      !core.stallDm) {
    core.regFile[wbOut_temp.rd] = wbOut_temp.value;
    core.cycle++;
  }

  // Handling an enventual trap/interruption
  if ((interruptTimer || interruptSoftware) && (core.csrUnit.mstatus & 0x8)) {
    // We have to trap : we wait for the pipeline to be emptied
    core.ftoDC.we = 0;
    if (!(core.memtoWB.we || core.extoMem.we || core.dctoEx.we)) {
      core.csrUnit.mepc = core.pc;
      core.csrUnit.mcause =
          0x80000000 |
          (interruptTimer
               ? 7
               : 3); // FIXME currently only timer can cause interrupts
      core.pc = core.csrUnit.mtvec;
      core.csrUnit.mstatus[7] = 1;
      core.csrUnit.mstatus[3] = 0;
    }
  }

  branchUnit(core.ftoDC.nextPCFetch, core.dctoEx.nextPCDC, core.dctoEx.isBranch,
             core.extoMem.nextPC, core.extoMem.isBranch, core.pc, core.ftoDC.we,
             core.dctoEx.we, core.extoMem.we,
             core.stallSignals[STALL_FETCH] || core.stallIm || core.stallDm ||
                 localStall);
}

// void doCore(IncompleteMemory im, IncompleteMemory dm, bool globalStall)
void doCore(bool globalStall, ac_int<32, false> imData[DRAM_SIZE >> 2],
            ac_int<32, false> dmData[DRAM_SIZE >> 2]) {

  Core core;
  IncompleteMemory imi = IncompleteMemory(imData);
  IncompleteMemory imd = IncompleteMemory(dmData);

  CacheMemory imInterface = CacheMemory(&imi, false);
  CacheMemory dmInterface = CacheMemory(&imd, false);

  core.im = &imInterface;
  core.dm = &dmInterface;
  core.pc = 0;

  while (1) {
    doCycle(core, globalStall);
  }
}
