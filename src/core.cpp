#include <core.h>
#include <ac_int.h>
#include <cacheMemory.h>

#ifndef __HLS__
#include "simulator.h"
#endif  // __HLS__


ac_int<32, false> valueRead;

struct FtoDC fetch(ac_int<32, false> pc, ac_int<32, false> instruction)
{
  struct FtoDC ftoDC = FtoDC();

  ftoDC.instruction = instruction;
  ftoDC.pc = pc;
  ftoDC.nextPCFetch = pc + 4;
  ftoDC.we = 1;

  return ftoDC;
}

struct DCtoEx decode(struct FtoDC ftoDC,
            ac_int<32, true> registerFile[32])
{

  struct DCtoEx dctoEx = DCtoEx();
    ac_int<32, false> pc = ftoDC.pc;
    ac_int<32, false> instruction = ftoDC.instruction;

    // R-type instruction
    ac_int<7, false> funct7 = instruction.slc<7>(25);
    ac_int<5, false> rs2 = instruction.slc<5>(20);
    ac_int<5, false> rs1 = instruction.slc<5>(15);
    ac_int<3, false> funct3 = instruction.slc<3>(12);
    ac_int<5, false> rd = instruction.slc<5>(7);
    ac_int<7, false> opCode = instruction.slc<7>(0);    // could be reduced to 5 bits because 1:0 is always 11

    //Construction of different immediate values
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


    //Register access
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

    //Initialization of control bits
    dctoEx.useRs1 = 0;
    dctoEx.useRs2 = 0;
    dctoEx.useRd = 0;
    dctoEx.we = ftoDC.we;
    dctoEx.isBranch = 0;



    switch (opCode)
    {
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
        dctoEx.lhs = ftoDC.pc+4;
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
        //Treatment for: STORE INSTRUCTIONS
    case RISCV_ST:
        dctoEx.lhs = valueReg1;
        dctoEx.rhs = imm12_S_signed;
        dctoEx.datac = valueReg2; //Value to store in memory
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
        //TODO

        break;
    default:

        break;

    }

    //If dest is zero, useRd should be at zero
	if (rd == 0){
		dctoEx.useRd = 0;
	}

    //If the instruction was dropped, we ensure that isBranch is at zero
    if (!ftoDC.we){
    	dctoEx.isBranch = 0;
    	dctoEx.useRd = 0;
    	dctoEx.useRs1 = 0;
    	dctoEx.useRs2 = 0;
    	dctoEx.useRs3 = 0;
    }
    return dctoEx;
}

struct MemtoWB memory(struct ExtoMem extoMem)
{
  struct MemtoWB memtoWB = MemtoWB();
    ac_int<2, false> datasize = extoMem.funct3.slc<2>(0);
    bool signenable = !extoMem.funct3.slc<1>(2);
    memtoWB.we = extoMem.we;
    memtoWB.useRd = extoMem.useRd;
    memtoWB.result = extoMem.result;
    memtoWB.rd = extoMem.rd;

    ac_int<32, false> mem_read;

    switch(extoMem.opCode)
    {
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
    }
    return memtoWB;
}


struct WBOut writeback(struct MemtoWB memtoWB)
{
  struct WBOut wbOut = WBOut();
	wbOut.we = memtoWB.we;
    if((memtoWB.rd != 0) && (memtoWB.we) && memtoWB.useRd){
      wbOut.rd = memtoWB.rd;
      wbOut.value = memtoWB.result;
    	wbOut.useRd = 1;
    }
    return wbOut;
}

void branchUnit(ac_int<32, false> nextPC_fetch,
		ac_int<32, false> nextPC_decode,
		bool isBranch_decode,
		ac_int<32, false> nextPC_execute,
		bool isBranch_execute,
		ac_int<32, false> &pc,
		bool &we_fetch,
		bool &we_decode,
		bool stall_fetch){

	if (!stall_fetch){
		if (isBranch_execute){
			we_fetch = 0;
			we_decode = 0;
			pc = nextPC_execute;
		}
		else if (isBranch_decode){
			we_fetch = 0;
			pc = nextPC_decode;
		}
		else {
			pc = nextPC_fetch;
		}
	}

}

void forwardUnit(
		ac_int<5, false> decodeRs1,
		bool decodeUseRs1,
		ac_int<5, false> decodeRs2,
		bool decodeUseRs2,
		ac_int<5, false> decodeRs3,
		bool decodeUseRs3,

		ac_int<5, false> executeRd,
		bool executeUseRd,
		bool executeIsLongComputation,

		ac_int<5, false> memoryRd,
		bool memoryUseRd,

		ac_int<5, false> writebackRd,
		bool writebackUseRd,

		bool stall[5],
		struct ForwardReg &forwardRegisters){

	if (decodeUseRs1){
		if (executeUseRd && decodeRs1 == executeRd){
			if (executeIsLongComputation){
				stall[0] = 1;
				stall[1] = 1;
			}
			else {
				forwardRegisters.forwardExtoVal1 = 1;
			}
		}
		else if (memoryUseRd && decodeRs1 == memoryRd){
			forwardRegisters.forwardMemtoVal1 = 1;
		}
		else if (writebackUseRd && decodeRs1 == writebackRd){
			forwardRegisters.forwardWBtoVal1 = 1;

		}
	}

	if (decodeUseRs2){
		if (executeUseRd && decodeRs2 == executeRd){
			if (executeIsLongComputation){
				stall[0] = 1;
				stall[1] = 1;
			}
			else {
				forwardRegisters.forwardExtoVal2 = 1;
			}
		}
		else if (memoryUseRd && decodeRs2 == memoryRd)
			forwardRegisters.forwardMemtoVal2 = 1;
		else if (writebackUseRd && decodeRs2 == writebackRd)
			forwardRegisters.forwardWBtoVal2 = 1;

	}

	if (decodeUseRs3){
		if (executeUseRd && decodeRs3 == executeRd){
			if (executeIsLongComputation){
				stall[0] = 1;
				stall[1] = 1;
			}
			else {
				forwardRegisters.forwardExtoVal3 = 1;
			}
		}
		else if (memoryUseRd && decodeRs3 == memoryRd)
			forwardRegisters.forwardMemtoVal3 = 1;
		else
			if (writebackUseRd && decodeRs3 == writebackRd)
				forwardRegisters.forwardWBtoVal3 = 1;
	}
}

void doControl(bool globalStall, struct FtoDC ftoDC_temp, struct FtoDC &ftoDC,
     struct DCtoEx dctoEx_temp, struct DCtoEx &dctoEx,
     struct ExtoMem extoMem_temp, struct ExtoMem &extoMem,
     struct MemtoWB memtoWB_temp, struct MemtoWB &memtoWB,
     bool wasLoad, struct WBOut &wbOut,
     ac_int<32, false> pc_in, ac_int<32, false> &pc_out, IncompleteMemory &dm,
     ac_int<32, true> regFile[32])
{

     struct ForwardReg forwardRegisters;
     struct WBOut wbOut_temp;

     if (wasLoad)
         wbOut.value = valueRead;

     extoMem.funct3 = 0;

     bool localStall = globalStall;
     bool stallSignals[5];

     stallSignals[0] = 0; stallSignals[1] = 0; stallSignals[2] = 0;
stallSignals[3] = 0; stallSignals[4] = 0;
     bool stallIm = false;
     bool stallDm = false;
     bool stallAlu = false;
     bool localStallAlu = false;

     //We update localStall value according to stallAlu
     localStall |= stallAlu;

     //resolve stalls, forwards
     if (!localStall)
         forwardUnit(dctoEx_temp.rs1, dctoEx_temp.useRs1,
                 dctoEx_temp.rs2, dctoEx_temp.useRs2,
                 dctoEx_temp.rs3, dctoEx_temp.useRs3,
                 extoMem_temp.rd, extoMem_temp.useRd,
extoMem_temp.isLongInstruction,
                 memtoWB_temp.rd, memtoWB_temp.useRd,
                 wbOut_temp.rd, wbOut_temp.useRd,
                 stallSignals, forwardRegisters);

     if (!stallSignals[STALL_MEMORY] && !localStall && memtoWB_temp.we
&& !stallIm){

        memMask mask;
        //TODO: carry the data size to memToWb
        switch (extoMem.funct3) {
          case 0:
           mask = BYTE;
           break;
          case 1:
           mask = HALF;
           break;
         case 2:
           mask = WORD;
           break;
         case 4:
           mask = BYTE_U;
           break;
         case 5:
           mask = HALF_U;
           break;
         //Should NEVER happen
         default:
           mask = WORD;
           break;
        }
        dm.process(memtoWB_temp.address, mask,
memtoWB_temp.isLoad ? LOAD : (memtoWB_temp.isStore ? STORE : NONE),
memtoWB_temp.valueToWrite, valueRead, stallDm);
     }
     //commit the changes to the pipeline register
     if (!stallSignals[STALL_FETCH] && !localStall && !stallIm && !stallDm){
         ftoDC = ftoDC_temp;
     }

     if (!stallSignals[STALL_DECODE] && !localStall && !stallIm &&
!stallDm){
         dctoEx = dctoEx_temp;

         if (forwardRegisters.forwardExtoVal1 && extoMem_temp.we)
             dctoEx.lhs = extoMem_temp.result;
         else if (forwardRegisters.forwardMemtoVal1 && memtoWB_temp.we)
             dctoEx.lhs = memtoWB_temp.result;
         else if (forwardRegisters.forwardWBtoVal1 && wbOut_temp.we)
             dctoEx.lhs = wbOut_temp.value;

         if (forwardRegisters.forwardExtoVal2 && extoMem_temp.we)
             dctoEx.rhs = extoMem_temp.result;
         else if (forwardRegisters.forwardMemtoVal2 && memtoWB_temp.we)
             dctoEx.rhs = memtoWB_temp.result;
         else if (forwardRegisters.forwardWBtoVal2 && wbOut_temp.we)
             dctoEx.rhs = wbOut_temp.value;

         if (forwardRegisters.forwardExtoVal3 && extoMem_temp.we)
             dctoEx.datac = extoMem_temp.result;
         else if (forwardRegisters.forwardMemtoVal3 && memtoWB_temp.we)
             dctoEx.datac = memtoWB_temp.result;
         else if (forwardRegisters.forwardWBtoVal3 && wbOut_temp.we)
             dctoEx.datac = wbOut_temp.value;
     }

     if (stallSignals[STALL_DECODE] && !stallSignals[STALL_EXECUTE] &&
!stallIm && !stallDm && !localStall){
         dctoEx.we = 0; dctoEx.useRd = 0; dctoEx.isBranch = 0;
dctoEx.instruction = 0; dctoEx.pc = 0;
     }

     if (!stallSignals[STALL_EXECUTE] && !localStall && !stallIm &&
!stallDm){
         extoMem = extoMem_temp;
     }

     if (!stallSignals[STALL_MEMORY] && !localStall && !stallIm &&
!stallDm){
         memtoWB = memtoWB_temp;
     }

     if (wbOut_temp.we && wbOut_temp.useRd && !localStall && !stallIm &&
!stallDm){
         regFile[wbOut_temp.rd] = wbOut_temp.value;
     }


     branchUnit(ftoDC_temp.nextPCFetch, dctoEx_temp.nextPCDC,
dctoEx_temp.isBranch, extoMem_temp.nextPC, extoMem_temp.isBranch,
pc_out, ftoDC.we, dctoEx.we, stallSignals[STALL_FETCH] || stallIm ||
stallDm || localStall);

}

void doCycle(struct Core &core, bool globalStall, ac_int<32, false> imData[DRAM_SIZE>>2], ac_int<32, false> dmData[DRAM_SIZE>>2])
{
  static IncompleteMemory imInterface = IncompleteMemory(imData);
  static IncompleteMemory dmInterface = IncompleteMemory(dmData);

	//declare temporary structs
  struct FtoDC ftoDC_temp; ftoDC_temp.pc = 0; ftoDC_temp.instruction = 0; ftoDC_temp.nextPCFetch = 0; ftoDC_temp.we = 0;
  struct DCtoEx dctoEx_temp; dctoEx_temp.isBranch = 0; dctoEx_temp.useRs1 = 0; dctoEx_temp.useRs2 = 0; dctoEx_temp.useRs3 = 0; dctoEx_temp.useRd = 0; dctoEx_temp.we = 0;
  struct ExtoMem extoMem_temp; extoMem_temp.useRd = 0; extoMem_temp.isBranch = 0; extoMem_temp.we = 0;
  struct MemtoWB memtoWB_temp; memtoWB_temp.useRd = 0; memtoWB_temp.isStore = 0; memtoWB_temp.we = 0; memtoWB_temp.isLoad = 0;
  struct WBOut wbOut_temp; wbOut_temp.useRd = 0; wbOut_temp.we = 0; wbOut_temp.rd = 0;
  struct ForwardReg forwardRegisters; forwardRegisters.forwardExtoVal1 = 0; forwardRegisters.forwardExtoVal2 = 0; forwardRegisters.forwardExtoVal3 = 0; forwardRegisters.forwardMemtoVal1 = 0; forwardRegisters.forwardMemtoVal2 = 0; forwardRegisters.forwardMemtoVal3 = 0; forwardRegisters.forwardWBtoVal1 = 0; forwardRegisters.forwardWBtoVal2 = 0; forwardRegisters.forwardWBtoVal3 = 0;

  ac_int<32, false> nextInst;


  if(!globalStall) {
    //get the next instruction
    bool stallIm = false;


    imInterface.process(core.pc, WORD, LOAD, 0, nextInst, stallIm); //never stalled

    //run all pipeline stages
    ftoDC_temp = fetch(core.pc, nextInst);
    dctoEx_temp = decode(core.ftoDC, core.regFile);
    extoMem_temp = core.basicALU.process(core.dctoEx);
    memtoWB_temp = memory(core.extoMem);
    wbOut_temp = writeback(core.memtoWB);

    //run control unit
    doControl(false, ftoDC_temp, core.ftoDC, dctoEx_temp, core.dctoEx, extoMem_temp, core.extoMem,
              memtoWB_temp, core.memtoWB, core.memtoWB.isLoad, wbOut_temp,
              core.pc, core.pc, dmInterface, core.regFile);
  }
}


//void doCore(IncompleteMemory im, IncompleteMemory dm, bool globalStall)
void doCore(bool globalStall, ac_int<32, false> imData[DRAM_SIZE>>2], ac_int<32, false> dmData[DRAM_SIZE>>2])
{
    Core core;
    IncompleteMemory imInterface = IncompleteMemory(imData);
    IncompleteMemory dmInterface = IncompleteMemory(dmData);

//    CacheMemory dmCache = CacheMemory(&dmInterface, false);

    core.im = &imInterface;
    core.dm = &dmInterface;
    core.pc = 0;

    while(1) {
        doCycle(core, globalStall);
    }
}
