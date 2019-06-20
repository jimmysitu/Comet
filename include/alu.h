/*
 * alu.h
 *
 *  Created on: 17 avr. 2019
 *      Author: simon
 */

#ifndef INCLUDE_ALU_H_
#define INCLUDE_ALU_H_

#include <riscvISA.h>
#include <pipelineRegisters.h>

//#include <cstdio>

class ALU {
protected:
  bool wait;

public:
  virtual void process(struct DCtoEx dctoEx, struct ExtoMem &extoMem, bool &stall) =0;
};


class BasicAlu: public ALU {
public:
	void process(struct DCtoEx dctoEx, struct ExtoMem &extoMem, bool &stall){
		stall = false;
	    extoMem.pc = dctoEx.pc;
	    extoMem.opCode = dctoEx.opCode;
	    extoMem.rd = dctoEx.rd;
	    extoMem.funct3 = dctoEx.funct3;
	    extoMem.we = dctoEx.we;
	    extoMem.isBranch = 0;
	    extoMem.useRd = dctoEx.useRd;
	    extoMem.isLongInstruction = 0;



	    ca_uint<13> imm13 = 0;
	    imm13[12] = dctoEx.instruction[31];
	    imm13.set_slc(5, dctoEx.instruction.slc<6>(25));
	    imm13.set_slc(1, dctoEx.instruction.slc<4>(8));
	    imm13[11] = dctoEx.instruction[7];

	    ca_int<13> imm13_signed = 0;
	    imm13_signed.set_slc(0, imm13);

	    ca_uint<5> shamt = dctoEx.instruction.slc<5>(20);


	    // switch must be in the else, otherwise external op may trigger default case
	    switch(dctoEx.opCode)
	    {
	    case RISCV_LUI:
	    	extoMem.result = dctoEx.lhs;
	        break;
	    case RISCV_AUIPC:
	    	extoMem.result = dctoEx.lhs + dctoEx.rhs;
	        break;
	    case RISCV_JAL:
	        //Note: in current version, the addition is made in the decode stage
	        //The value to store in rd (pc+4) is stored in lhs
	    	extoMem.result = dctoEx.lhs;
	        break;
	    case RISCV_JALR:
	        //Note: in current version, the addition is made in the decode stage
	        //The value to store in rd (pc+4) is stored in lhs
	    	extoMem.nextPC = dctoEx.rhs + dctoEx.lhs;
	    	extoMem.isBranch = 1;

	        extoMem.result = dctoEx.pc+4;
	        break;
	    case RISCV_BR:
	        extoMem.nextPC = extoMem.pc + imm13_signed;

	        switch(dctoEx.funct3)
	        {
	        case RISCV_BR_BEQ:
	            extoMem.isBranch = (dctoEx.lhs == dctoEx.rhs);
	            break;
	        case RISCV_BR_BNE:
	            extoMem.isBranch = (dctoEx.lhs != dctoEx.rhs);
	            break;
	        case RISCV_BR_BLT:
	            extoMem.isBranch = (dctoEx.lhs < dctoEx.rhs);
	            break;
	        case RISCV_BR_BGE:
	            extoMem.isBranch = (dctoEx.lhs >= dctoEx.rhs);
	            break;
	        case RISCV_BR_BLTU:
	            extoMem.isBranch = ((ca_uint<32>)dctoEx.lhs < (ca_uint<32>)dctoEx.rhs);
	            break;
	        case RISCV_BR_BGEU:
	            extoMem.isBranch = ((ca_uint<32>)dctoEx.lhs >= (ca_uint<32>)dctoEx.rhs);
	            break;
	        }
	        break;
	    case RISCV_LD:
	        extoMem.isLongInstruction = 1;
	        extoMem.result = dctoEx.lhs + dctoEx.rhs;
	        break;
	    case RISCV_ST:
	    	extoMem.datac = dctoEx.datac;
	        extoMem.result = dctoEx.lhs + dctoEx.rhs;
	        break;
	    case RISCV_OPI:
	        switch(dctoEx.funct3)
	        {
	        case RISCV_OPI_ADDI:
	            extoMem.result = dctoEx.lhs + dctoEx.rhs;
	            break;
	        case RISCV_OPI_SLTI:
	            extoMem.result = dctoEx.lhs < dctoEx.rhs;
	            break;
	        case RISCV_OPI_SLTIU:
	            extoMem.result = (ca_uint<32>)dctoEx.lhs < (ca_uint<32>)dctoEx.rhs;
	            break;
	        case RISCV_OPI_XORI:
	            extoMem.result = dctoEx.lhs ^ dctoEx.rhs;
	            break;
	        case RISCV_OPI_ORI:
	            extoMem.result = dctoEx.lhs | dctoEx.rhs;
	            break;
	        case RISCV_OPI_ANDI:
	            extoMem.result = dctoEx.lhs & dctoEx.rhs;
	            break;
	        case RISCV_OPI_SLLI: // cast rhs as 5 bits, otherwise generated hardware is 32 bits
	            // & shift amount held in the lower 5 bits (riscv spec)
	            extoMem.result = dctoEx.lhs << (ca_uint<5>)dctoEx.rhs;
	            break;
	        case RISCV_OPI_SRI:
	            if (dctoEx.funct7.slc<1>(5)) //SRAI
	                extoMem.result = dctoEx.lhs >> (ca_uint<5>)shamt;
	            else //SRLI
	                extoMem.result = (ca_uint<32>)dctoEx.lhs >> (ca_uint<5>)shamt;
	            break;
	        }
	        break;
	    case RISCV_OP:
	        if(dctoEx.funct7.slc<1>(0))     // M Extension
	        {

	        }
	        else{
	            switch(dctoEx.funct3){
	            case RISCV_OP_ADD:
	                if (dctoEx.funct7.slc<1>(5))   // SUB
	                    extoMem.result = dctoEx.lhs - dctoEx.rhs;
	                else   // ADD
	                    extoMem.result = dctoEx.lhs + dctoEx.rhs;
	                break;
	            case RISCV_OP_SLL:
	                extoMem.result = dctoEx.lhs << (ca_uint<5>)dctoEx.rhs;
	                break;
	            case RISCV_OP_SLT:
	                extoMem.result = dctoEx.lhs < dctoEx.rhs;
	                break;
	            case RISCV_OP_SLTU:
	                extoMem.result = (ca_uint<32>)dctoEx.lhs < (ca_uint<32>)dctoEx.rhs;
	                break;
	            case RISCV_OP_XOR:
	                extoMem.result = dctoEx.lhs ^ dctoEx.rhs;
	                break;
	            case RISCV_OP_SR:
	                if(dctoEx.funct7.slc<1>(5))   // SRA
	                    extoMem.result = dctoEx.lhs >> (ca_uint<5>)dctoEx.rhs;
	                else  // SRL
	                    extoMem.result = (ca_uint<32>)dctoEx.lhs >> (ca_uint<5>)dctoEx.rhs;
	                break;
	            case RISCV_OP_OR:
	                extoMem.result = dctoEx.lhs | dctoEx.rhs;
	                break;
	            case RISCV_OP_AND:
	                extoMem.result = dctoEx.lhs & dctoEx.rhs;
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
	            extoMem.datac = dctoEx.rhs;       // written back to csr
	            extoMem.result = dctoEx.lhs;      // written back to rd
	            break;
	        case RISCV_SYSTEM_CSRRS:
	            extoMem.datac = dctoEx.lhs | dctoEx.rhs;
	            extoMem.result = dctoEx.lhs;
	            break;
	        case RISCV_SYSTEM_CSRRC:
	            extoMem.datac = dctoEx.lhs & ((ca_uint<32>)~dctoEx.rhs);
	            extoMem.result = dctoEx.lhs;
	            break;
	        case RISCV_SYSTEM_CSRRWI:
	            extoMem.datac = dctoEx.rhs;
	            extoMem.result = dctoEx.lhs;
	            break;
	        case RISCV_SYSTEM_CSRRSI:
	            extoMem.datac = dctoEx.lhs | dctoEx.rhs;
	            extoMem.result = dctoEx.lhs;
	            break;
	        case RISCV_SYSTEM_CSRRCI:
	            extoMem.datac = dctoEx.lhs & ((ca_uint<32>)~dctoEx.rhs);
	            extoMem.result = dctoEx.lhs;
	            break;
	        }
	        break;
	    }


	    //If the instruction was dropped, we ensure that isBranch is at zero
	    if (!dctoEx.we){
	    	extoMem.isBranch = 0;
	    	extoMem.useRd = 0;
	    }

	}
};

class MultAlu: public ALU {
public:
    ca_uint<32> quotient, remainder;
    //ca_uint<33>
    ca_uint<6> state = 0;
    bool resIsNeg;
    ca_uint<32> dataAUnsigned, dataBUnsigned;

	void process(struct DCtoEx dctoEx, struct ExtoMem &extoMem, bool &stall){
        //no need to fill in the output register fields, the first ALU has that taken care of
        if (dctoEx.opCode == RISCV_OP && dctoEx.funct7 == RISCV_OP_M) {
	        
			    
	        if (state == 0) {
			    dataAUnsigned.set_slc(0, dctoEx.lhs);
    			dataBUnsigned.set_slc(0, dctoEx.rhs);
			    //mult results
			    ca_uint<32> resultU = dataAUnsigned * dataBUnsigned;
			    ca_uint<32> resultS = dctoEx.lhs * dctoEx.rhs;
			    ca_uint<32> resultSU = dctoEx.lhs * dataBUnsigned;
                resIsNeg = dctoEx.lhs[31] ^ dctoEx.rhs[31];

			    switch (dctoEx.funct3){
			    case RISCV_OP_M_MUL:
				    extoMem.result = resultS.slc<32>(0);
			    break;
			    case RISCV_OP_M_MULH:
				    extoMem.result = resultS.slc<32>(32);
			    break;
			    case RISCV_OP_M_MULHSU:
				    extoMem.result = resultSU.slc<32>(32);
			    break;
			    case RISCV_OP_M_MULHU:
				    extoMem.result = resultU.slc<32>(32);
			    break;
			    case RISCV_OP_M_DIV:
			        if(dctoEx.lhs[31]) {
			            dataAUnsigned = -dctoEx.lhs;
			        }
			        if(dctoEx.rhs[31]) {
			            dataBUnsigned = -dctoEx.rhs;
			        }
			        //printf("Dividing %d by %d\n", dataAUnsigned, dataBUnsigned);
			    case RISCV_OP_M_DIVU:
			        if(dataBUnsigned == 0) {
			            extoMem.result = -1;
			        }
			        else {
			            state = 32;
			            quotient = 0;
			            remainder = 0;
			        }
			    break;
			    case RISCV_OP_M_REM:
			        if(dctoEx.lhs[31]) {
			            dataAUnsigned = -dctoEx.lhs;
			        }
			        if(dctoEx.rhs[31]) {
			            dataBUnsigned = -dctoEx.rhs;
			        }
			        //printf("Moduling %d by %d\n", dataAUnsigned, dataBUnsigned);
			    case RISCV_OP_M_REMU:
			        if(dataBUnsigned == 0) {
			            extoMem.result = dataAUnsigned;
			        }
			        else {
			            state = 32;
			            quotient = 0;
			            remainder = 0;
			        }        
			    break;
			    }
			}
			else {
			    //Loop for the division
			    state--;
			    remainder = remainder  << 1;
			    remainder[0] = dataAUnsigned[state];
			    if(remainder >= dataBUnsigned) {
			        remainder = remainder - dataBUnsigned;
			        quotient[state] = 1;
			    }
			    //printf("Quotient : %d, Remainder : %d\n", quotient, remainder);
			    if(state == 0) {
			        switch(dctoEx.funct3) {
			        case RISCV_OP_M_DIV:
			            if(resIsNeg)
			                extoMem.result = -quotient;
			            else
			                extoMem.result = quotient;
			        break;
			        case RISCV_OP_M_DIVU:
			            extoMem.result = quotient;
			        break;
			        case RISCV_OP_M_REM:
			            if(dataAUnsigned[31])
			                extoMem.result = -remainder;
			            else
			                extoMem.result = remainder;
			        break;
			        case RISCV_OP_M_REMU:
			            extoMem.result = remainder;
			        break;
			        }
			        //printf("result : %d\n", extoMem.result);
			    }
			}
			stall |= (state != 0);
		}
	}
};

#endif /* INCLUDE_ALU_H_ */
