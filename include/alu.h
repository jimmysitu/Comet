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

#define abs(a) ( ((a) < 0) ? -(a) : (a) )

// Lookup for butterfly calculations
extern ac_int<16, false> sin_factor[126];

//#include <cstdio>

short FIX_MPY(short a, short b);


class ALU {
protected:
  bool wait;

public:
  virtual void process(struct DCtoEx dctoEx, struct ExtoMem &extoMem, bool &stall) =0;
};


class BasicAlu: public ALU {
public:
	
	ac_int<16, true> lastQi, lastQr, lastTi, lastTr, state4 = 1, state3 = 0, state1=0, state2=32, sin_index = 0; bool unchanged = true;

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



	    ac_int<13, false> imm13 = 0;
	    imm13[12] = dctoEx.instruction[31];
	    imm13.set_slc(5, dctoEx.instruction.slc<6>(25));
	    imm13.set_slc(1, dctoEx.instruction.slc<4>(8));
	    imm13[11] = dctoEx.instruction[7];

	    ac_int<13, true> imm13_signed = 0;
	    imm13_signed.set_slc(0, imm13);

	    ac_int<5, false> shamt = dctoEx.instruction.slc<5>(20);


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
	            extoMem.isBranch = ((ac_int<32, false>)dctoEx.lhs < (ac_int<32, false>)dctoEx.rhs);
	            break;
	        case RISCV_BR_BGEU:
	            extoMem.isBranch = ((ac_int<32, false>)dctoEx.lhs >= (ac_int<32, false>)dctoEx.rhs);
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
	            extoMem.result = (ac_int<32, false>)dctoEx.lhs < (ac_int<32, false>)dctoEx.rhs;
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
	            extoMem.result = dctoEx.lhs << (ac_int<5, false>)dctoEx.rhs;
	            break;
	        case RISCV_OPI_SRI:
	            if (dctoEx.funct7.slc<1>(5)) //SRAI
	                extoMem.result = dctoEx.lhs >> (ac_int<5, false>)shamt;
	            else //SRLI
	                extoMem.result = (ac_int<32, false>)dctoEx.lhs >> (ac_int<5, false>)shamt;
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
	                extoMem.result = dctoEx.lhs << (ac_int<5, false>)dctoEx.rhs;
	                break;
	            case RISCV_OP_SLT:
	                extoMem.result = dctoEx.lhs < dctoEx.rhs;
	                break;
	            case RISCV_OP_SLTU:
	                extoMem.result = (ac_int<32, false>)dctoEx.lhs < (ac_int<32, false>)dctoEx.rhs;
	                break;
	            case RISCV_OP_XOR:
	                extoMem.result = dctoEx.lhs ^ dctoEx.rhs;
	                break;
	            case RISCV_OP_SR:
	                if(dctoEx.funct7.slc<1>(5))   // SRA
	                    extoMem.result = dctoEx.lhs >> (ac_int<5, false>)dctoEx.rhs;
	                else  // SRL
	                    extoMem.result = (ac_int<32, false>)dctoEx.lhs >> (ac_int<5, false>)dctoEx.rhs;
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
	            extoMem.datac = dctoEx.lhs & ((ac_int<32, false>)~dctoEx.rhs);
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
	            extoMem.datac = dctoEx.lhs & ((ac_int<32, false>)~dctoEx.rhs);
	            extoMem.result = dctoEx.lhs;
	            break;
	        }
	        break;

		case RISCV_EXTRAOP:
			if (dctoEx.funct3 == 0){


				printf("Doing btf with %d %d %d %d sin factor of %d   --- inputs %x %x\n", state1, state2, state3, state4, sin_index,dctoEx.lhs, dctoEx.rhs);
				bool reverse = dctoEx.funct7[6];

				ac_int<16, false> W_Imag = sin_factor[sin_index];
				ac_int<16, false> W_Real = sin_factor[sin_index+1];

				if (reverse){
					W_Imag = W_Imag >> 1;				
					W_Real = W_Real >> 1;				
					W_Real = -W_Real;
				}

				ac_int<16, true> ti = FIX_MPY(W_Imag,dctoEx.lhs.slc<16>(16)) - FIX_MPY(W_Real,dctoEx.lhs.slc<16>(00));
        ac_int<16, true> tr = FIX_MPY(W_Imag,dctoEx.lhs.slc<16>(00)) + FIX_MPY(W_Real,dctoEx.lhs.slc<16>(16));
        ac_int<16, true> qi = dctoEx.rhs.slc<16>(16);
        ac_int<16, true> qr = dctoEx.rhs.slc<16>(0);

        if (reverse) {
          qi >>= 1;
          qr >>= 1;
        }

				extoMem.result = 0;
				ac_int<16, true> imag = qi - ti;
				ac_int<16, true> real = qr - tr;
				extoMem.result.set_slc(16, imag);
				extoMem.result.set_slc(0, real);
        
				lastQi = qi;
				lastQr = qr;
				lastTi = ti;
				lastTr = tr;
			
				unchanged = false;


			}
			else {
				ac_int<16, true> imag = lastQi + lastTi;
				ac_int<16, true> real = lastQr + lastTr;
				extoMem.result.set_slc(16, imag);
				extoMem.result.set_slc(0, real);
				


				if (!unchanged){
					state1++;
					if (state1>=state2){
						state3++;
						state1 = 0;
						sin_index += 2;
						if (state3 >= state4){
							if (state2 == 1){
								state1 = 0;
								state2 = 32;	
								state3 = 0;
								state4 = 1;
								sin_index = 0;
							}
							else {
								state3 = 0;
								state2 = state2 >> 1;
								state4 = state4 << 1;
							}
						}
					}	
					unchanged = true;				
				}
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
    ac_int<32, false> quotient, remainder;
    //ac_int<33, false> 
    ac_int<6, false> state = 0; 
    bool resIsNeg;
    ac_int<32, false> dataAUnsigned, dataBUnsigned;

	void process(struct DCtoEx dctoEx, struct ExtoMem &extoMem, bool &stall){
        //no need to fill in the output register fields, the first ALU has that taken care of
        if (dctoEx.opCode == RISCV_OP && dctoEx.funct7 == RISCV_OP_M) {
	        
			    
	        if (state == 0) {
			    dataAUnsigned.set_slc(0, dctoEx.lhs);
    			dataBUnsigned.set_slc(0, dctoEx.rhs);
			    //mult results
			    ac_int<32, false> resultU = dataAUnsigned * dataBUnsigned;
			    ac_int<32, false> resultS = dctoEx.lhs * dctoEx.rhs;
			    ac_int<32, false> resultSU = dctoEx.lhs * dataBUnsigned;
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
