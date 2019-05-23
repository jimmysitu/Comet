/*
 * floatAlu.h
 *
 *  Created on 23 jun. 2019
 *      Author :  Lauric 
 */

#ifndef FLOAT_ALU_H
#define FLOAT_ALU_H

#include <riscvISA.h>
#include <pipelineRegisters.h>
#include <alu.h>
#include <ac_int.h>



class FloatAlu : public ALU
{
public :
	void process(struct DCtoEx dctoEx, struct ExtoMem &extoMem, bool &stall)
{
          stall = true;                                                          
          extoMem.opCode = dctoEx.opCode;                                         
          extoMem.rd = dctoEx.rd;                                                 
          extoMem.funct3 = dctoEx.funct3;                                         
          extoMem.we = dctoEx.we;                                                 
          extoMem.isBranch = 0;                                                   
          extoMem.useRd = dctoEx.useRd;                                           
          extoMem.isLongInstruction = 0;                                          
                                                                                    
          switch(dctoEx.opCode)                                                   
          {                                                                       
                  case  RISCV_FLOAT_OP_ADD :                                      
                          break;                                                  
                                                                                  
                  case RISCV_FLOAT_OP_SUB  :                                      
                          break;                                                  
                                                                                  
                  case RISCV_FLOAT_OP_MUL  :                                      
                          extoMem.result.set_slc(31, dctoEx.lhs.slc<1>(31) ^ dctoEx.rhs.slc<1>(31));
                          extoMem.result.set_slc(23, dctoEx.lhs.slc<8>(23) + dctoEx.rhs.slc<8>(23));
                          extoMem.result.set_slc(0, (dctoEx.lhs.slc<23>(0)        
                                              * dctoEx.rhs.slc<23>(0)).slc<23>(0) );
			  stall = false;
                          break;                                                  
                                                                                  
                  case RISCV_FLOAT_OP_DIV  :                                      
			  	
                          break;                                                  
                                                                                  
                  case RISCV_FLOAT_OP_SQRT :                                      
                          break;                                                  
                                                                                  
                  case RISCV_FLOAT_OP_SGN  :                                      
                          break;                                                  
                                                                                  
                  case RISCV_FLOAT_OP_MINMAX :                                    
                          break;                                                  
                                                                                  
                  case RISCV_FLOAT_OP_CVTWS :                                     
                          break;                                                  
                                                                                  
                  case RISCV_FLOAT_OP_CMP  :                                      
                          break;                                                  
                                                                                  
                  case RISCV_FLOAT_OP_CVTSW :                                     
                          break;                                                  
                                                                                  
                  case RISCV_FLOAT_OP_MVWX :                                      
                          break;                                                  
                                                                                  
                  case RISCV_FLOAT_OP_CLASSMVXW :                                 
                          break;                                                  
                                                                                  
                  default :                                                       
                          break;                                                  
          }                                                                       
  }   
		
};



#endif /* FLOAT_ALU_H */ 
