#ifndef __CSR_UNIT
#define __CSR_UNIT

#include <riscvISA.h>
#include <pipelineRegisters.h>
#include <ac_int.h>

//Definition of all CSR address

//Machine information registers
#define CSR_MVENDORID 0xf11
#define CSR_MARCHID 0xf12
#define CSR_MIMPID 0xf13
#define CSR_MHARTID 0xf14

//Machine trap setup
#define CSR_MSTATUS 0x300
#define CSR_MISA 0x301
#define CSR_MEDELEG 0x302
#define CSR_MIDELEG 0x303
#define CSR_MIE 0x304
#define CSR_MTVEC 0x305
#define CSR_MCOUNTEREN 0x306

//Machine trap handling
#define CSR_MSCRATCH 0x340
#define CSR_MEPC 0x341
#define CSR_MCAUSE 0x342
#define CSR_MTVAL 0x343
#define CSR_MIP 0x344

//Machine memory protection
#define CSR_PMPCFG0 0x3a0
#define CSR_PMPCFG1 0x3a1
#define CSR_PMPCFG2 0x3a2
#define CSR_PMPCFG3 0x3a3
#define CSR_PMPADDR0 0x3b0
#define CSR_PMPADDR1 0x3b1
#define CSR_PMPADDR2 0x3b2
#define CSR_PMPADDR3 0x3b3
#define CSR_PMPADDR4 0x3b4
#define CSR_PMPADDR5 0x3b5
#define CSR_PMPADDR6 0x3b6
#define CSR_PMPADDR7 0x3b7
#define CSR_PMPADDR8 0x3b8
#define CSR_PMPADDR9 0x3b9
#define CSR_PMPADDR10 0x3ba
#define CSR_PMPADDR11 0x3bb
#define CSR_PMPADDR12 0x3bc
#define CSR_PMPADDR13 0x3bd
#define CSR_PMPADDR14 0x3be
#define CSR_PMPADDR15 0x3bf

//Machine counter/timers
#define CSR_MCYCLE 0xb00
#define CSR_MINSTRRET 0xb02
//...
#define CSR_MCYCLEH 0xb80
#define CSR_MINSTRRETH 0xb82

//Machine counter setup
#define CSR_MCOUNTINHIBIT 0x320
//...

class CsrUnit {
public:
	ac_int<32, false> mhartid; //TODO : reduce size
	ac_int<32, false> mstatus;
	ac_int<32, false> mtvec;

	ac_int<32, false> mip;
	ac_int<32, false> mie;

	ac_int<64, false> mtimecmp;

	ac_int<64, false> mcycle;
	ac_int<64, false> minstrret;

	ac_int<32, false> mcountinhibit;

	ac_int<32, false> mscratch;
	ac_int<32, false> mepc;
	ac_int<32, false> mcause;
	ac_int<32, false> mtval;

	bool process(struct DCtoEx dctoEx, ac_int<32, false> &result, bool &setPC){
		this->mcycle++;
		this->minstrret++; //FIXME

		if (dctoEx.we && dctoEx.opCode == RISCV_SYSTEM && (dctoEx.funct3 == RISCV_SYSTEM_CSRRW || dctoEx.funct3 == RISCV_SYSTEM_CSRRS || dctoEx.funct3 == RISCV_SYSTEM_CSRRC ||
				dctoEx.funct3 == RISCV_SYSTEM_CSRRWI || dctoEx.funct3 == RISCV_SYSTEM_CSRRSI || dctoEx.funct3 == RISCV_SYSTEM_CSRRCI)){


			//We are in a CSR function
			ac_int<12, false> csr = dctoEx.instruction.slc<12>(20);

			ac_int<32, false> andMask = 0xffffffff;
			ac_int<32, false> orMask = 0;

			if (dctoEx.funct3 == RISCV_SYSTEM_CSRRW){
				//Its a read/write so the value to write is lhs
				andMask = 0;
				orMask = dctoEx.lhs;
			}
			else if (dctoEx.funct3 == RISCV_SYSTEM_CSRRWI){
				//Its a read/write so the value to write is lhs
				andMask = 0;
				orMask = dctoEx.rs1;
			}
			else if (dctoEx.funct3 == RISCV_SYSTEM_CSRRS && dctoEx.rs1 != 0){
				//Its a read/write so the value to write is lhs
				andMask = 0xffffffff;
				orMask = dctoEx.lhs;
			}
			else if (dctoEx.funct3 == RISCV_SYSTEM_CSRRSI && dctoEx.rs1 != 0){
				//Its a read/write so the value to write is lhs
				andMask = 0xffffffff;
				orMask = dctoEx.rs1;
			}
			else if (dctoEx.funct3 == RISCV_SYSTEM_CSRRC && dctoEx.rs1 != 0){
				//Its a read/write so the value to write is lhs
				andMask = !dctoEx.lhs;
				orMask = 0;
			}
			else if (dctoEx.funct3 == RISCV_SYSTEM_CSRRCI && dctoEx.rs1 != 0){
				//Its a read/write so the value to write is lhs
				andMask = !dctoEx.rs1;
				orMask = 0;
			}




			switch (csr){
				case CSR_MISA:
					result = 0x40001100;
				break;
				case CSR_MVENDORID:
					result = 0;
				break;
				case CSR_MARCHID:
					result = 0;
				break;
				case CSR_MIMPID:
					result = 0;
				break;
				case CSR_MHARTID:
					result = this->mhartid;
				break;
				case CSR_MSTATUS:
					result = this->mstatus;
					this->mstatus = (result & andMask) | orMask;
				break;
				case CSR_MTVEC:
					result = this->mtvec;
					this->mtvec = (result & andMask) | orMask;
				break;
				case CSR_MIP:
					result = this->mip;
					this->mip = (result & andMask) | orMask;
				break;
				case CSR_MIE:
					result = this->mie;
					this->mie = (result & andMask) | orMask;
				break;
				case CSR_MCYCLE:
					result = this->mcycle.slc<32>(0);
					this->mcycle.set_slc(0, (result & andMask) | orMask);
				break;
				case CSR_MCYCLEH:
					result = this->mcycle.slc<32>(32);
					this->mcycle.set_slc(32, (result & andMask) | orMask);
				break;
				case CSR_MINSTRRET:
					result = this->minstrret.slc<32>(0);
					this->minstrret.set_slc(0, (result & andMask) | orMask);
				break;
				case CSR_MINSTRRETH:
					result = this->minstrret.slc<32>(32);
					this->minstrret.set_slc(32, (result & andMask) | orMask);
				break;
				case CSR_MCOUNTINHIBIT:
					result = this->mcountinhibit;
					this->mcountinhibit = (result & andMask) | orMask;
				break;
				case CSR_MSCRATCH:
					result = this->mscratch;
					this->mscratch = (result & andMask) | orMask;
				break;
				case CSR_MEPC:
					result = this->mepc;
					this->mepc = (result & andMask) | orMask;
				break;
				case CSR_MCAUSE:
					result = this->mcause;
					this->mcause = (result & andMask) | orMask;
				break;
				case CSR_MTVAL:
					result = this->mtval;
					this->mtval = (result & andMask) | orMask;
				break;
				default:
				break;
			}

			return true;

		}

		if (dctoEx.we && dctoEx.opCode == RISCV_SYSTEM && dctoEx.funct3 == 0){

			//We handle a ecall or a ebreak
			if (dctoEx.rs2 == 0){
				//ECALL jumps to the IRQ handler code, whose adress is stored in mtvec lower bits
				this->mepc = dctoEx.pc;
				this->mcause = 11;
				result = mtvec;
				setPC = true;
				//FIXME: should check that lower bits of mtval are equals to 1 to check the type of jump...
			}
			else if (dctoEx.rs2 == 2){
				//MRET jumps back to the address stored in mepc
				result = this->mepc;
				setPC = true;

				//We change interupt enable bits (MIE = MPIE and MPIE = 1)
				mstatus[3] = mstatus[7];
				mstatus[7] = 1;
			}


		}


		return false;

	}

};



#endif
