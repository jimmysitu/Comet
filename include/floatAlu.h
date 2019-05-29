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
	int state = 0;
public :
	void process(struct DCtoEx dctoEx, struct ExtoMem &extoMem, bool &stall)
{			
   	  ac_int<1, false> f1Sign;
	  ac_int<1, false> f2Sign;
 
	  ac_int<24, false> f1Mantissa;
	  ac_int<24, false> f2Mantissa;
 
	  ac_int<9, false> f1Exp; 
	  ac_int<9, false> f2Exp;

	  ac_int<1, false> outputSign;                 
	  ac_int<48, false> outputMantissa;
	  ac_int<23, false> resultMantissa;
 	  ac_int<9, false> outputExp; 

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
                                                                                  
                  case RISCV_FLOAT_OP_DIV  : 
                         f1Sign = dctoEx.rhs.slc<1>(31);
 						 f2Sign = dctoEx.lhs.slc<1>(31);	

						 f1Mantissa = dctoEx.rhs.slc<23>(0);
  						 f2Mantissa = dctoEx.lhs.slc<23>(0);
							f1Mantissa[23] = 1;
							f2Mantissa[23] = 1;

  			 			 f1Exp = dctoEx.rhs.slc<8>(23) - 127;
			  			 f2Exp = dctoEx.lhs.slc<8>(23) - 127;


  			 			 outputSign = f1Sign ^ f2Sign;
    			  		 outputMantissa = ((ac_int<48,false>)f1Mantissa << 24)/ ((ac_int<48,false>) f2Mantissa);
  			  			 outputExp = f1Exp - f2Exp + 127;

			  			if (outputMantissa[23])
							outputExp--;

  			  			resultMantissa = (ac_int<24,false>) outputMantissa;

  			 			extoMem.result.set_slc(0, resultMantissa.slc<23>(0)); 
  			  			extoMem.result.set_slc(23, outputExp.slc<8>(0));
  			  			extoMem.result.set_slc(31, outputSign);

                          break;                                                  
                                                                                  
                  case RISCV_FLOAT_OP_MUL  :                                      


  						f1Sign = dctoEx.rhs.slc<1>(31);
						f2Sign = dctoEx.lhs.slc<1>(31);
 
  						f1Mantissa = dctoEx.rhs.slc<23>(0);
						f2Mantissa = dctoEx.lhs.slc<23>(0);
						f1Mantissa[23] = 1;                                                     
						f2Mantissa[23] = 1;                                                     
 
						f1Exp = dctoEx.rhs.slc<8>(23); 
						f2Exp = dctoEx.lhs.slc<8>(23);

						outputSign = f1Sign ^ f2Sign;                 
						outputMantissa = f1Mantissa * f2Mantissa;
						resultMantissa = (outputMantissa[47] ?outputMantissa.slc<23>(24) : outputMantissa.slc<23>(23));
 						outputExp = f1Exp + f2Exp - 127; 
 
 						if (outputMantissa[47])                                                 
 							outputExp++;                                                    

						extoMem.result.set_slc(31, outputSign); 
 						extoMem.result.set_slc(0, resultMantissa);
 						extoMem.result.set_slc(23, outputExp.slc<8>(0));
                          break;                                                  
                                                                                  
                  case RISCV_FLOAT_OP_SQRT :                                      
                          break;                                                  
                                                                                  
                  case RISCV_FLOAT_OP_SGN  :                                      
                          break;                                                  
                                                                                  
                  case RISCV_FLOAT_OP_MINMAX :  
					f1Mantissa = dctoEx.rhs.slc<23>(0);
					f2Mantissa = dctoEx.lhs.slc<23>(0);
					f1Mantissa[23] = 1;
					f2Mantissa[23] = 1;

					f1Exp = dctoEx.rhs.slc<8>(23) - 127;
					f2Exp = dctoEx.lhs.slc<8>(23) - 127;
					
					f1Sign = dctoEx.rhs.slc<1>(31);
					f2Sign = dctoEx.lhs.slc<1>(31);
					if(dctoEx.funct3) //FMAX
					{
						if(f1Exp == f2Sign)
						{if(f1Sign){  // both are positive
							if (f1Exp > f2Exp)
								extoMem.result = dctoEx.rhs;
							else
							{
								if(f2Exp > f1Exp)
									extoMem.result = dctoEx.lhs;
								else
								{	
									if(f1Mantissa > f2Mantissa)
										extoMem.result = dctoEx.rhs;
									else
										extoMem.result = dctoEx.lhs;
								}
							}}
						else{ // both are negative
							if (f1Exp < f2Exp)
								extoMem.result = dctoEx.rhs;
							else
							{
								if(f2Exp < f1Exp)
									extoMem.result = dctoEx.lhs;
								else
								{	
									if(f1Mantissa < f2Mantissa)
										extoMem.result = dctoEx.rhs;
									else
										extoMem.result = dctoEx.lhs;
								}
							}}
						} //End if(f1Exp == f2Sign)
						else // rhs and lhs have different sign                            	
						{

						if(f1Sign < f2Sign) // rhs positive and lhs negative 
							extoMem.result = dctoEx.lhs;
						else // rhs negative and lhs positive
							extoMem.result = dctoEx.rhs;
						}  
					}
   
					else //FMIN
					{
						if(f1Exp == f2Sign)
						{if(f1Sign){  // both are positive
							if (f1Exp < f2Exp)
								extoMem.result = dctoEx.rhs;
							else
							{
								if(f2Exp < f1Exp)
									extoMem.result = dctoEx.lhs;
								else
								{	
									if(f1Mantissa < f2Mantissa)
										extoMem.result = dctoEx.rhs;
									else
										extoMem.result = dctoEx.lhs;
								}
							}}
						else{ // both are negative
							if (f1Exp > f2Exp)
								extoMem.result = dctoEx.rhs;
							else
							{
								if(f2Exp > f1Exp)
									extoMem.result = dctoEx.lhs;
								else
								{	
									if(f1Mantissa > f2Mantissa)
										extoMem.result = dctoEx.rhs;
									else
										extoMem.result = dctoEx.lhs;
								}
							}}
						} //End if(f1Exp == f2Sign)
						else // rhs and lhs have different sign                            	
						{

						if(f1Sign < f2Sign) // rhs positive and lhs negative 
							extoMem.result = dctoEx.lhs;
						else // rhs negative and lhs positive
							extoMem.result = dctoEx.rhs;
						}  
					}

					
                          break;                                                  
                                                                                  
                  case RISCV_FLOAT_OP_CVTWS :                                     
                          break;                                                  
                                                                                  
                  case RISCV_FLOAT_OP_CMP  :
					f1Mantissa = dctoEx.rhs.slc<23>(0);
					f2Mantissa = dctoEx.lhs.slc<23>(0);
					f1Mantissa[23] = 1;
					f2Mantissa[23] = 1;

					f1Exp = dctoEx.rhs.slc<8>(23) - 127;
					f2Exp = dctoEx.lhs.slc<8>(23) - 127;
					
					f1Sign = dctoEx.rhs.slc<1>(31);
					f2Sign = dctoEx.lhs.slc<1>(31);
						  
					switch(dctoEx.funct3)
							{
								case 0:
		
									if(f1Sign == f2Sign)
									{
										if(f1Sign)
										{
											if (f1Exp >= f2Exp)
												extoMem.result[0] = true;
											else
											{
												if(f2Exp == f1Exp)
												{	
												if(f1Mantissa >= f2Mantissa)
													extoMem.result[0] = true;
												}
											}	
										}
										else
										{
											if (f1Exp <= f2Exp)
												extoMem.result[0] = true;
											else
											{
											if(f2Exp == f1Exp)
												{	
												if(f1Mantissa <= f2Mantissa)
													extoMem.result[0] = true;
												}
											}
										}
									}
									else
									{
										if(f1Sign > f2Sign)
											extoMem.result[0] = true ;
									}

								break;
								
								case 1:
										
									if(f1Sign == f2Sign)
									{
										if(f1Sign)
										{
											if (f1Exp > f2Exp)
												extoMem.result[0] = true;
											else
											{
												if(f2Exp == f1Exp)
												{	
												if(f1Mantissa > f2Mantissa)
													extoMem.result[0] = true;
												}
											}	
										}
										else
										{
											if (f1Exp < f2Exp)
												extoMem.result[0] = true;
											else
											{
											if(f2Exp == f1Exp)
												{	
												if(f1Mantissa < f2Mantissa)
													extoMem.result[0] = true;
												}
											}
										}
									}
									else
									{
										if(f1Sign > f2Sign)
											extoMem.result[0] = true ;
									}

								break;

								case 2:
									extoMem.result[0] = dctoEx.rhs = dctoEx.lhs;
								break;
							}                                    
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
