#include <core.h>

#ifndef __VIVADO__
#include <ac_int.h>
#endif

#include <portability.h>

#ifndef __HLS__
#include "simulator.h"
#endif  // __HLS__


struct FtoDC fetch(CORE_UINT(32) pc,
           CORE_UINT(32) instructionMemory[DRAM_SIZE])
{
	struct FtoDC ftoDC;
#pragma HLS INLINE
    ftoDC.instruction = instructionMemory[pc/4];
    ftoDC.pc = pc;
    ftoDC.nextPCFetch = pc + 4;

    ftoDC.stall = 0;
    ftoDC.we = 1;

    return ftoDC;
}

struct DCtoEx decode(struct FtoDC ftoDC)
{

	struct DCtoEx dctoEx;
#pragma HLS INLINE
    CORE_UINT(32) pc = ftoDC.pc;
    CORE_UINT(32) instruction = ftoDC.instruction;

    // R-type instruction
    CORE_UINT(7) funct7 = instruction.SLC(7, 25);
    CORE_UINT(5) rs2 = instruction.SLC(5, 20);
    CORE_UINT(5) rs1 = instruction.SLC(5, 15);
    CORE_UINT(3) funct3 = instruction.SLC(3, 12);
    CORE_UINT(5) rd = instruction.SLC(5, 7);
    CORE_UINT(7) opCode = instruction.SLC(7, 0);    // could be reduced to 5 bits because 1:0 is always 11

    //Construction of different immediate values
    CORE_UINT(12) imm12_I = instruction.SLC(12, 20);
    CORE_UINT(12) imm12_S = 0;
    imm12_S.SET_SLC(5, instruction.SLC(7, 25));
    imm12_S.SET_SLC(0, instruction.SLC(5, 7));

    CORE_INT(12) imm12_I_signed = instruction.SLC(12, 20);
    CORE_INT(12) imm12_S_signed = 0;
    imm12_S_signed.SET_SLC(0, imm12_S.SLC(12, 0));

    CORE_UINT(13) imm13 = 0;
    imm13[12] = instruction[31];
    imm13.SET_SLC(5, instruction.SLC(6, 25));
    imm13.SET_SLC(1, instruction.SLC(4, 8));
    imm13[11] = instruction[7];

    CORE_INT(13) imm13_signed = 0;
    imm13_signed.SET_SLC(0, imm13);

    CORE_INT(32) imm31_12 = 0;
    imm31_12.SET_SLC(12, instruction.SLC(20, 12));

    CORE_UINT(21) imm21_1 = 0;
    imm21_1.SET_SLC(12, instruction.SLC(8, 12));
    imm21_1[11] = instruction[20];
    imm21_1.SET_SLC(1, instruction.SLC(10, 21));
    imm21_1[20] = instruction[31];

    CORE_INT(21) imm21_1_signed = 0;
    imm21_1_signed.SET_SLC(0, imm21_1);



	dctoEx.datad = 0; //TODO remove
	dctoEx.datae = 0; //TODO remove

	dctoEx.regVal1 = 0;//registerFile1[rs1];
	dctoEx.regVal2 = 0;//registerFile1[rs2];
	dctoEx.imm = 0;

    dctoEx.nextPCDC = 0;

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
    dctoEx.useRs3 = 0;
    dctoEx.useRd = 0;
    dctoEx.we = ftoDC.we;
    dctoEx.stall = 0;
    dctoEx.isBranch = 0;



    switch (opCode)
    {
    case RISCV_LUI:
        dctoEx.imm = imm31_12;
        dctoEx.useRs1 = 0;
        dctoEx.useRs2 = 0;
        dctoEx.useRs3 = 0;
        dctoEx.useRd = 1;

        break;
    case RISCV_AUIPC:
        dctoEx.imm = imm31_12;
        dctoEx.useRs1 = 0;
        dctoEx.useRs2 = 0;
        dctoEx.useRs3 = 0;
        dctoEx.useRd = 1;
        break;
    case RISCV_JAL:
        dctoEx.nextPCDC = ftoDC.pc + imm21_1_signed;
        dctoEx.useRs1 = 0;
        dctoEx.useRs2 = 0;
        dctoEx.useRs3 = 0;
        dctoEx.useRd = 1;
        dctoEx.isBranch = 1;

        break;
    case RISCV_JALR:
        dctoEx.imm = imm12_I_signed;
        dctoEx.useRs1 = 1;
        dctoEx.useRs2 = 0;
        dctoEx.useRs3 = 0;
        dctoEx.useRd = 1;

        break;
    case RISCV_BR:
        dctoEx.useRs1 = 1;
        dctoEx.useRs2 = 1;
        dctoEx.useRs3 = 0;
        dctoEx.useRd = 0;

        break;
    case RISCV_LD:
        dctoEx.imm = imm12_I_signed;
        dctoEx.useRs1 = 1;
        dctoEx.useRs2 = 0;
        dctoEx.useRs3 = 0;
        dctoEx.useRd = 1;

        break;

        //******************************************************************************************
        //Treatment for: STORE INSTRUCTIONS
    case RISCV_ST:
        dctoEx.imm = imm12_S_signed;
        dctoEx.useRs1 = 1;
        dctoEx.useRs2 = 0;
        dctoEx.useRs3 = 1;
        dctoEx.useRd = 0;
        break;
    case RISCV_OPI:
        dctoEx.imm = imm12_I_signed;
        dctoEx.useRs1 = 1;
        dctoEx.useRs2 = 0;
        dctoEx.useRs3 = 0;
        dctoEx.useRd = 1;
        break;

    case RISCV_OP:

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

    //If the instruction was dropped, we ensure that isBranch is at zero
    if (!ftoDC.we)
    	dctoEx.isBranch = 0;

    return dctoEx;

}

struct ExtoMem execute(struct DCtoEx dctoEx)
{
	struct ExtoMem extoMem;
#pragma HLS INLINE

    extoMem.pc = dctoEx.pc;
    extoMem.opCode = dctoEx.opCode;
    extoMem.rd = dctoEx.rd;
    extoMem.funct3 = dctoEx.funct3;
    extoMem.we = dctoEx.we;
    extoMem.isBranch = 0;
    extoMem.useRd = dctoEx.useRd;

    extoMem.result = 0;
    extoMem.datac = 0;
    extoMem.nextPC = 0;

    CORE_UINT(13) imm13 = 0;
    imm13[12] = dctoEx.instruction[31];
    imm13.SET_SLC(5, dctoEx.instruction.SLC(6, 25));
    imm13.SET_SLC(1, dctoEx.instruction.SLC(4, 8));
    imm13[11] = dctoEx.instruction[7];

    CORE_INT(13) imm13_signed = 0;
    imm13_signed.SET_SLC(0, imm13);

    CORE_UINT(5) shamt = dctoEx.instruction.SLC(5, 20);


    // switch must be in the else, otherwise external op may trigger default case
    switch(dctoEx.opCode)
    {
    case RISCV_LUI:
        extoMem.result = dctoEx.imm;
        break;
    case RISCV_AUIPC:
        extoMem.result = dctoEx.pc + dctoEx.imm;
        break;
    case RISCV_JAL:
        //Note: in current version, the addition is made in the decode stage
        //The value to store in rd (pc+4) is stored in lhs
        extoMem.result = dctoEx.pc + 4;
        break;
    case RISCV_JALR:
        //Note: in current version, the addition is made in the decode stage
        //The value to store in rd (pc+4) is stored in lhs
    	extoMem.nextPC = dctoEx.regVal1 + dctoEx.imm;
    	extoMem.isBranch = 1;

        extoMem.result = dctoEx.pc+4;
        break;
    case RISCV_BR:
        extoMem.nextPC = extoMem.pc + imm13_signed;
        
        switch(dctoEx.funct3)
        {
        case RISCV_BR_BEQ:
            extoMem.isBranch = (dctoEx.regVal1 == dctoEx.regVal2);
            break;
        case RISCV_BR_BNE:
            extoMem.isBranch = (dctoEx.regVal1 != dctoEx.regVal2);
            break;
        case RISCV_BR_BLT:
            extoMem.isBranch = (dctoEx.regVal1 < dctoEx.regVal2);
            break;
        case RISCV_BR_BGE:
            extoMem.isBranch = (dctoEx.regVal1 >= dctoEx.regVal2);
            break;
        case RISCV_BR_BLTU:
            extoMem.isBranch = ((CORE_UINT(32))dctoEx.regVal1 < (CORE_UINT(32))dctoEx.regVal2);
            break;
        case RISCV_BR_BGEU:
            extoMem.isBranch = ((CORE_UINT(32))dctoEx.regVal1 >= (CORE_UINT(32))dctoEx.regVal2);
            break;
        }
        break;
    case RISCV_LD:
        extoMem.result = dctoEx.regVal1 + dctoEx.imm;
        break;
    case RISCV_ST:
        extoMem.result = dctoEx.regVal1 + dctoEx.imm;
        extoMem.datac = dctoEx.regVal2;
        break;
    case RISCV_OPI:
        switch(dctoEx.funct3)
        {
        case RISCV_OPI_ADDI:
            extoMem.result = dctoEx.regVal1 + dctoEx.imm;
            break;
        case RISCV_OPI_SLTI:
            extoMem.result = dctoEx.regVal1 < dctoEx.imm;
            break;
        case RISCV_OPI_SLTIU:
            extoMem.result = (CORE_UINT(32))dctoEx.regVal1 < (CORE_UINT(32))dctoEx.imm;
            break;
        case RISCV_OPI_XORI:
            extoMem.result = dctoEx.regVal1 ^ dctoEx.imm;
            break;
        case RISCV_OPI_ORI:
            extoMem.result = dctoEx.regVal1 | dctoEx.imm;
            break;
        case RISCV_OPI_ANDI:
            extoMem.result = dctoEx.regVal1 & dctoEx.imm;
            break;
        case RISCV_OPI_SLLI: // cast rhs as 5 bits, otherwise generated hardware is 32 bits
            // & shift amount held in the lower 5 bits (riscv spec)
            extoMem.result = dctoEx.regVal1 << (CORE_UINT(5))shamt;
            break;
        case RISCV_OPI_SRI:
            if (dctoEx.funct7.SLC(1, 5)) //SRAI
                extoMem.result = dctoEx.regVal1 >> (CORE_UINT(5))shamt;
            else //SRLI
                extoMem.result = (CORE_UINT(32))dctoEx.regVal1 >> (CORE_UINT(5))shamt;
            break;
        }
        break;
    case RISCV_OP:
        if(dctoEx.funct7.SLC(1, 0))     // M Extension
        {
            CORE_INT(33) mul_reg_a = dctoEx.regVal1;
            CORE_INT(33) mul_reg_b = dctoEx.regVal2;
            CORE_INT(66) longResult = 0;
            switch (dctoEx.funct3)  //Switch case for multiplication operations (RV32M)
            {
            case RISCV_OP_M_MULHSU:
                mul_reg_b[32] = 0;
                break;
            case RISCV_OP_M_MULHU:
                mul_reg_a[32] = 0;
                mul_reg_b[32] = 0;
                break;
            }
            longResult = mul_reg_a * mul_reg_b;
            if(dctoEx.funct3 == RISCV_OP_M_MULH || dctoEx.funct3 == RISCV_OP_M_MULHSU || dctoEx.funct3 == RISCV_OP_M_MULHU)
                extoMem.result = longResult.SLC(32, 32);
            else
                extoMem.result = longResult.SLC(32, 0);
        }
        else{
            switch(dctoEx.funct3){
            case RISCV_OP_ADD:
                if (dctoEx.funct7.SLC(1, 5))   // SUB
                    extoMem.result = dctoEx.regVal1 - dctoEx.regVal2;
                else   // ADD
                    extoMem.result = dctoEx.regVal1 + dctoEx.regVal2;
                break;
            case RISCV_OP_SLL:
                extoMem.result = dctoEx.regVal1 << (CORE_UINT(5))dctoEx.regVal2;
                break;
            case RISCV_OP_SLT:
                extoMem.result = dctoEx.regVal1 < dctoEx.regVal2;
                break;
            case RISCV_OP_SLTU:
                extoMem.result = (CORE_UINT(32))dctoEx.regVal1 < (CORE_UINT(32))dctoEx.regVal2;
                break;
            case RISCV_OP_XOR:
                extoMem.result = dctoEx.regVal1 ^ dctoEx.regVal2;
                break;
            case RISCV_OP_SR:
                if(dctoEx.funct7.SLC(1, 5))   // SRA
                    extoMem.result = dctoEx.regVal1 >> (CORE_UINT(5))dctoEx.regVal2;
                else  // SRL
                    extoMem.result = (CORE_UINT(32))dctoEx.regVal1 >> (CORE_UINT(5))dctoEx.regVal2;
                break;
            case RISCV_OP_OR:
                extoMem.result = dctoEx.regVal1 | dctoEx.regVal2;
                break;
            case RISCV_OP_AND:
                extoMem.result = dctoEx.regVal1 & dctoEx.regVal2;
                break;
            }
        }
        break;
    case RISCV_MISC_MEM:    // this does nothing because all memory accesses are ordered and we have only one core
        break;

    case RISCV_SYSTEM:
        switch(dctoEx.funct3)
        { // case 0: mret instruction, dctoEx.memValue should be 0x302
        case RISCV_SYSTEM_ENV:
#ifndef __HLS__
        	//TODO handling syscall correctly
            //extoMem.result = sim->solveSyscall(dctoEx.lhs, dctoEx.rhs, dctoEx.datac, dctoEx.datad, dctoEx.datae, exit);
#endif
            break;
        case RISCV_SYSTEM_CSRRW:    // lhs is from csr, rhs is from reg[rs1]
            extoMem.datac = dctoEx.regVal2;       // written back to csr
            extoMem.result = dctoEx.regVal1;      // written back to rd
            break;
        case RISCV_SYSTEM_CSRRS:
            extoMem.datac = dctoEx.regVal1 | dctoEx.regVal2;
            extoMem.result = dctoEx.regVal1;
            break;
        case RISCV_SYSTEM_CSRRC:
            extoMem.datac = dctoEx.regVal1 & ((CORE_UINT(32))~dctoEx.regVal2);
            extoMem.result = dctoEx.regVal1;
            break;
        case RISCV_SYSTEM_CSRRWI:
            extoMem.datac = dctoEx.regVal2;
            extoMem.result = dctoEx.regVal1;
            break;
        case RISCV_SYSTEM_CSRRSI:
            extoMem.datac = dctoEx.regVal1 | dctoEx.regVal2;
            extoMem.result = dctoEx.regVal1;
            break;
        case RISCV_SYSTEM_CSRRCI:
            extoMem.datac = dctoEx.regVal1 & ((CORE_UINT(32))~dctoEx.regVal2);
            extoMem.result = dctoEx.regVal1;
            break;
        }
        break;
    }

    //If the instruction was dropped, we ensure that isBranch is at zero
    if (!dctoEx.we)
    	extoMem.isBranch = 0;

    return extoMem;
}

struct MemtoWB memory(struct ExtoMem extoMem)
{
	struct MemtoWB memtoWB;
#pragma HLS INLINE

    CORE_UINT(2) datasize = extoMem.funct3.SLC(2, 0);
    CORE_UINT(1) signenable = !extoMem.funct3.SLC(1, 2);
    memtoWB.we = extoMem.we;
    memtoWB.useRd = extoMem.useRd;
    memtoWB.rd = extoMem.rd;

	memtoWB.address = extoMem.result;
	memtoWB.valueToWrite = extoMem.datac;
	memtoWB.byteEnable = 0xf; //TODO Handle that
    memtoWB.result = extoMem.result;

    switch(extoMem.opCode)
    {
    case RISCV_LD:
        memtoWB.isLoad = 1;
        break;
    case RISCV_ST:
        	memtoWB.isStore = 1;
        break;
    default:
        break;
    }

    return memtoWB;
}


struct WBOut writeback(struct MemtoWB memtoWB,
				CORE_INT(32) valueRead)
{
	struct WBOut wbOut;
#pragma HLS INLINE
	wbOut.we = memtoWB.we;
	wbOut.rd = 0;
	wbOut.value = memtoWB.isLoad ? valueRead : memtoWB.result;
	wbOut.useRd = 0;

    if((memtoWB.rd != 0) && (memtoWB.we) && memtoWB.useRd){
    	wbOut.rd = memtoWB.rd;
    	wbOut.useRd = 1;

    }
    return wbOut;
}

void branchUnit(CORE_UINT(32) nextPC_fetch,
		CORE_UINT(32) nextPC_decode,
		CORE_UINT(1) isBranch_decode,
		CORE_UINT(32) nextPC_execute,
		CORE_UINT(1) isBranch_execute,
		CORE_UINT(32) &pc,
		CORE_UINT(1) &we_fetch,
		CORE_UINT(1) &we_decode){
#pragma HLS INLINE

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

void forwardUnit(
		CORE_UINT(5) decodeRs1,
		CORE_UINT(1) decodeUseRs1,
		CORE_UINT(5) decodeRs2,
		CORE_UINT(1) decodeUseRs2,
		CORE_UINT(5) decodeRs3,
		CORE_UINT(1) decodeUseRs3,

		CORE_UINT(5) executeRd,
		CORE_UINT(1) executeUseRd,
		CORE_UINT(1) executeIsLongComputation,

		CORE_UINT(5) memoryRd,
		CORE_UINT(1) memoryUseRd,

		CORE_UINT(5) writebackRd,
		CORE_UINT(1) writebackUseRd,

		CORE_UINT(1) stall[5],
		struct ForwardReg &forwardRegisters){
#pragma HLS INLINE

	if (decodeUseRs1){
		if (writebackUseRd && decodeRs1 == writebackRd)
			forwardRegisters.forwardWBtoVal1 = 1;
		else if (memoryUseRd && decodeRs1 == memoryRd)
			forwardRegisters.forwardMemtoVal1 = 1;
		else if (executeUseRd && decodeRs1 == executeRd){
			if (executeIsLongComputation){
				stall[0] = 1;
				stall[1] = 1;
			}
			else {
				forwardRegisters.forwardExtoVal1 = 1;
			}
		}
	}

	if (decodeUseRs2){
		if (writebackUseRd && decodeRs2 == writebackRd)
			forwardRegisters.forwardWBtoVal2 = 1;
		else if (memoryUseRd && decodeRs2 == memoryRd)
			forwardRegisters.forwardMemtoVal2 = 1;
		else if (executeUseRd && decodeRs2 == executeRd){
			if (executeIsLongComputation){
				stall[0] = 1;
				stall[1] = 1;
			}
			else {
				forwardRegisters.forwardExtoVal2 = 1;
			}
		}
	}

	if (decodeUseRs3){
		if (writebackUseRd && decodeRs3 == writebackRd)
			forwardRegisters.forwardWBtoVal3 = 1;
		else if (memoryUseRd && decodeRs3 == memoryRd)
			forwardRegisters.forwardMemtoVal3 = 1;
		else if (executeUseRd && decodeRs3 == executeRd){
			if (executeIsLongComputation){
				stall[0] = 1;
				stall[1] = 1;
			}
			else {
				forwardRegisters.forwardExtoVal3 = 1;
			}
		}
	}
}

void copyFtoDC(struct FtoDC &dest, struct FtoDC src){
#pragma HLS INLINE
    dest.pc = src.pc;           	
    dest.instruction = src.instruction;  
    dest.nextPCFetch = src.nextPCFetch;      
    dest.we = src.we;
    dest.stall = src.stall;
}

void copyDCtoEx(struct DCtoEx &dest, struct DCtoEx src){
#pragma HLS INLINE
    dest.pc = src.pc;       // used for branch
    dest.instruction = src.instruction;

    dest.opCode = src.opCode;    // opCode = instruction[6:0]
    dest.funct7 = src.funct7;    // funct7 = instruction[31:25]
    dest.funct3 = src.funct3;    // funct3 = instruction[14:12]

    dest.regVal1 = src.regVal1;   //  left hand side : operand 1
    dest.regVal2 = src.regVal2;   // right hand side : operand 2
    dest.imm = src.imm; // ST, BR, JAL/R,

    // syscall only
    dest.datad = src.datad;
    dest.datae = src.datae;

    //For branch unit
    dest.nextPCDC = src.nextPCDC;
    dest.isBranch = src.isBranch;

    //Information for forward/stall unit
    dest.useRs1 = src.useRs1;
    dest.useRs2 = src.useRs2;
    dest.useRs3 = src.useRs3;
    dest.useRd = src.useRd;
    dest.rs1 = src.rs1;       // rs1    = instruction[19:15]
    dest.rs2 = src.rs2;       // rs2    = instruction[24:20]
    dest.rs3 = src.rs3;
    dest.rd = src.rd;        // rd     = instruction[11:7]

    //Register for all stages
    dest.we = src.we;
    dest.stall = src.stall; //TODO add that
}

void copyExtoMem(struct ExtoMem &dest, struct ExtoMem src){
#pragma HLS INLINE
    dest.pc = src.pc;
    dest.instruction = src.instruction;

    dest.result = src.result;    // result of the EX stage
    dest.rd = src.rd;        // destination register
    dest.useRd = src.useRd;
    dest.isLongInstruction = src.isLongInstruction;
    dest.opCode = src.opCode;    // LD or ST (can be reduced to 2 bits)
    dest.funct3 = src.funct3;    // datasize and sign extension bit

    dest.datac = src.datac;     // data to be stored in memory or csr result

    //For branch unit
    dest.nextPC = src.nextPC;
    dest.isBranch = src.isBranch;

    //Register for all stages
    dest.we = src.we;
    dest.stall = src.stall; //TODO add that
}

void copyMemtoWB(struct MemtoWB &dest, struct MemtoWB src){
#pragma HLS INLINE
    dest.result = src.result;    // Result to be written back
    dest.rd = src.rd;        // destination register
    dest.useRd = src.useRd;

    dest.address = src.address;
    dest.valueToWrite = src.valueToWrite;
    dest.byteEnable = src.byteEnable;
    dest.isStore = src.isStore;
    dest.isLoad = src.isLoad;

    //Register for all stages
    dest.we = src.we;
    dest.stall = src.stall;
}

void copyWBOut(struct WBOut &dest, struct WBOut src){
#pragma HLS INLINE
    dest.value = src.value;    // Result to be written back
    dest.rd = src.rd;        // destination register
    dest.useRd = src.useRd;
    dest.we = src.we;
}



void doCycle(struct Core &core, 		 //Core containing all values
		CORE_UINT(32) im[DRAM_SIZE], //Instruction memory
		CORE_INT(32) dm[DRAM_SIZE],  //Data memory
		CORE_UINT(1) globalStall)
{

}



void doCore(CORE_UINT(32) im[DRAM_SIZE], CORE_INT(32) dm[DRAM_SIZE])
{
#pragma HLS INLINE recursive
#pragma HLS INTERFACE bram port=dm
#pragma HLS INTERFACE bram port=im
    //declare a core
    CORE_UINT(1) globalStall = 0;

    struct ForwardReg forwardRegisters;



    FtoDC ftoDC;
	DCtoEx dctoEx;
	ExtoMem extoMem;
	MemtoWB memtoWB;
	struct WBOut wbOut;
	CORE_UINT(32) valueRead;
	int cnt = 0;
	CORE_UINT(32) regVal1;
	CORE_UINT(32) regVal2;

	//CoreCtrl ctrl;

	CORE_INT(32) REG1[32];
#pragma HLS ARRAY_PARTITION variable=REG1 complete dim=1

	CORE_UINT(32) pc;


    while(1) {
#pragma HLS latency max=2

#pragma HLS DEPENDENCE array inter RAW false
#pragma HLS DEPENDENCE array inter WAR false

		#pragma HLS PIPELINE II=1
#pragma HLS INLINE

    CORE_UINT(1) stallSignals[5] = {0, 0, 0, 0, 0};
#pragma HLS ARRAY_PARTITION variable=stallSignals complete dim=1



    //declare temporary register file

    struct FtoDC ftoDC_temp = fetch(pc, im);
    struct DCtoEx dctoEx_temp = decode(ftoDC);

    dctoEx.regVal1 = regVal1;
    dctoEx.regVal2 = regVal2;
    struct ExtoMem extoMem_temp = execute(dctoEx);
    struct MemtoWB memtoWB_temp = memory(extoMem);
    struct WBOut wbOut_temp = writeback(memtoWB, valueRead);


    //resolve stalls, forwards
    forwardUnit(dctoEx_temp.rs1, dctoEx_temp.useRs1,
    		dctoEx_temp.rs2, dctoEx_temp.useRs2,
			dctoEx_temp.rs3, dctoEx_temp.useRs3,
			extoMem_temp.rd, extoMem_temp.useRd, extoMem_temp.isLongInstruction,
			memtoWB_temp.rd, memtoWB_temp.useRd,
    		wbOut_temp.rd, wbOut_temp.useRd,
			stallSignals, forwardRegisters);

    //commit the changes to the pipeline register





	regVal1 = REG1[dctoEx_temp.rs1];
	regVal2 = REG1[dctoEx_temp.rs2];
	if (memtoWB_temp.we && memtoWB_temp.isStore)
		dm[memtoWB_temp.address >> 2] = memtoWB_temp.valueToWrite;
	else
	    valueRead = dm[memtoWB_temp.address >> 2];

    if (wbOut_temp.we && wbOut_temp.useRd){

    	REG1[wbOut_temp.rd] = wbOut_temp.value;

    }


    if (!stallSignals[0] && !globalStall)
    	copyFtoDC(ftoDC, ftoDC_temp);

    if (!stallSignals[1] && !globalStall)
    	copyDCtoEx(dctoEx, dctoEx_temp);


    if (!stallSignals[2] && !globalStall)
    	copyExtoMem(extoMem, extoMem_temp);

	copyMemtoWB(memtoWB, memtoWB_temp);

    copyWBOut(wbOut, wbOut_temp);


    if (!stallSignals[1] && !globalStall){



      	if (forwardRegisters.forwardExtoVal1 && extoMem.we)
      		regVal1 = extoMem.result;
      	else if (forwardRegisters.forwardMemtoVal1 && memtoWB.we)
			regVal1 = memtoWB.result;
      	else if (forwardRegisters.forwardWBtoVal1 && wbOut.we)
      		regVal1 = wbOut.value;

      	if (forwardRegisters.forwardExtoVal2 && extoMem.we)
      		regVal2 = extoMem.result;
      	else if (forwardRegisters.forwardMemtoVal2 && memtoWB.we)
			regVal2 = memtoWB.result;
      	else if (forwardRegisters.forwardWBtoVal2 && wbOut.we)
      		regVal2 = wbOut.value;

      	if (forwardRegisters.forwardExtoVal3 && extoMem.we)
      		regVal2 = extoMem.result;
      	else if (forwardRegisters.forwardMemtoVal3 && memtoWB.we)
			regVal2 = memtoWB.result;
      	else if (forwardRegisters.forwardWBtoVal3 && wbOut.we)
      		regVal2 = wbOut.value;

      }

    forwardRegisters.forwardExtoVal1 = 0; forwardRegisters.forwardExtoVal2 = 0; forwardRegisters.forwardExtoVal3 = 0; forwardRegisters.forwardMemtoVal1 = 0; forwardRegisters.forwardMemtoVal2 = 0; forwardRegisters.forwardMemtoVal3 = 0; forwardRegisters.forwardWBtoVal1 = 0; forwardRegisters.forwardWBtoVal2 = 0; forwardRegisters.forwardWBtoVal3 = 0;
    branchUnit(ftoDC.nextPCFetch, dctoEx.nextPCDC, dctoEx.isBranch, extoMem.nextPC, extoMem.isBranch, pc, ftoDC.we, dctoEx.we);



    cnt++;





    }
}
