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
private :
	ac_int<23,false> tmp = 0;
public :
	void process(struct DCtoEx dctoEx, struct ExtoMem &extoMem, bool &stall)
{
          stall =false;                                                          
          extoMem.opCode = dctoEx.opCode;                                         
          extoMem.rd = dctoEx.rd;                                                 
          extoMem.funct3 = dctoEx.funct3;                                         
          extoMem.we = dctoEx.we;                                                 
          extoMem.isBranch = 0;                                                   
          extoMem.useRd = dctoEx.useRd;                                           
          extoMem.isLongInstruction = 0;                                          
                                                                                    
          switch(dctoEx.opCode)                                                   
          {                                                                       
           case RISCV_FLOAT_LW:
		break; 

	   case RISCV_FLOAT_SW:
		break;
 
	   case RISCV_FLOAT_MADD :
		break;
	
	   case RISCV_FLOAT_MSUB :
		break; 

	   case RISCV_FLOAT_NMADD : 
		break;

	   case RISCV_FLOAT_NMSUB : 
		break;

           case RISCV_FLOAT_OP : 
		  switch(dctoEx.funct7)
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
                          break;                                                  
                                                                                  
                  case RISCV_FLOAT_OP_DIV  :                                      
                          extoMem.result.set_slc(31, dctoEx.lhs.slc<1>(31) ^ dctoEx.rhs.slc<1>(31));
                          extoMem.result.set_slc(23, dctoEx.lhs.slc<8>(23) - dctoEx.rhs.slc<8>(23));
				// write aa better algorithm
			  extoMem.result.set_slc(0,(ac_int<23, false>) 0); 
			  tmp = dctoEx.lhs.slc<23>(0);
			  while(tmp > 0) 
			  {
				tmp -= dctoEx.rhs.slc<23>(0);
				extoMem.result.set_slc(0, 1 + extoMem.result.slc<23>(0));	
			  }
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
		  break; 

	  default :  
		break;
          }                                                                       
  }   
		
};



#endif /* FLOAT_ALU_H */ 
