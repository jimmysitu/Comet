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
	ac_int<23,true> tmp = 0;
	int state = 0;
public :
	void process(struct DCtoEx dctoEx, struct ExtoMem &extoMem, bool &stall)
{
          stall =false;                                                          
          switch(dctoEx.opCode)                                                   
          {                                                                       
           case RISCV_FLOAT_LW:
		 extoMem.isLongInstruction = 1;                                  
                 extoMem.result = dctoEx.lhs + dctoEx.rhs;   
		break; 

	   case RISCV_FLOAT_SW:
                extoMem.datac = dctoEx.datac;                                   
                extoMem.result = dctoEx.lhs + dctoEx.rhs;  
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
			if ((dctoEx.lhs.slc<8>(23) + dctoEx.rhs.slc<8>(23))[8]) 
			{
                          extoMem.result.set_slc(23,  (dctoEx.lhs.slc<8>(23) + dctoEx.rhs.slc<8>(23)).slc<8>(1) );
                          extoMem.result.set_slc(0,   (dctoEx.lhs.slc<23>(0)       
                                              * dctoEx.rhs.slc<23>(0)).slc<23>(23));
			}
			else
			{
                          extoMem.result.set_slc(23,  (dctoEx.lhs.slc<8>(23) + dctoEx.rhs.slc<8>(23)).slc<8>(0) );
                          extoMem.result.set_slc(0,   (dctoEx.lhs.slc<23>(0)       
                                              * dctoEx.rhs.slc<23>(0)).slc<23>(0));
			}
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
			  extoMem.result = dctoEx.lhs;
                          break;                                                  
                                                                                  
                  case RISCV_FLOAT_OP_CLASSMVXW :                                 
			  if (dctoEx.funct3) // funct3 = 0 -> FMV.X.W
			  {
				extoMem.result = dctoEx.lhs;
			  }
			  else  // FCLASS.S
			  {
				
			  }
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
