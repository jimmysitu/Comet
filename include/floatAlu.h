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
	int state = 0;
	int subState = 0;
	ac_int<48, false> quotient = 0;
	ac_int<48,false> remainder = 0;
	ac_int<48,false> f1Val = 0;
	ac_int<48,false> f2Val = 0;
	ac_int<24,false> tmp = 0;
	ac_int<32, false> localResult = 0; 

	
public :
	void process(struct DCtoEx dctoEx, struct ExtoMem &extoMem, bool &stall)
{		
   	  ac_int<1, false> f1Sign;
	  ac_int<1, false> f2Sign;
 
	  ac_int<24, false> f1Mantissa;
	  ac_int<24, false> f2Mantissa;
 
	  ac_int<9, true> f1Exp; 
	  ac_int<9, true> f2Exp;

	  ac_int<1, false> outputSign;                 
	  ac_int<48, false> outputMantissa;
	  ac_int<24, false> resultMantissa;
 	  ac_int<9, false> outputExp;



      float f1;
	  int g;
	  int i;
	  


      stall =false;       

	f1Mantissa = dctoEx.rhs.slc<23>(0);
	f2Mantissa = dctoEx.lhs.slc<23>(0);
	f1Mantissa[23] = 1;
	f2Mantissa[23] = 1;
	
	if(dctoEx.rhs.slc<8>(23) != 0)
	{
		f1Mantissa[23] = 1;
		f1Exp = dctoEx.rhs.slc<8>(23) - 127;  
	 }
	 else 
	 {
		f1Exp = - 126;
	 }
	
	 if(dctoEx.lhs.slc<8>(23) != 0)
	 {
		f2Mantissa[23] = 1;
		 f2Exp = dctoEx.lhs.slc<8>(23) - 127;  
	 }
	 else 
	 {
	  	f2Exp =  - 126;  
	 }
				
	f1Sign = dctoEx.rhs.slc<1>(31);
	f2Sign = dctoEx.lhs.slc<1>(31);
   
   if(!state)                                              
     {
		 switch(dctoEx.opCode)                                                   
		  {                                                                       
		     case RISCV_FLOAT_LW:
			 	extoMem.isLongInstruction = 1;                                  
		     	localResult = dctoEx.lhs + dctoEx.rhs;   
			 break; 

		   case RISCV_FLOAT_SW:
		            extoMem.datac = dctoEx.datac;                                   
		            localResult = dctoEx.lhs + dctoEx.rhs;  
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
						if(!subState)
						{
							state = 1;
							subState = 1;
							stall = true;
							f1Val  = f1Mantissa;
							f2Val = f2Mantissa;
						}
						else
						{
							subState = 0;	
							stall = false;					
							if(f1Sign == f2Sign)
								localResult.set_slc(31, f1Sign);
							else
							{
								if(f1Val > f2Val)
									localResult.set_slc(31,f1Sign);
								else
									localResult.set_slc(31,f2Sign);
							}

									
							resultMantissa = f1Val + f2Val;
							outputExp = f1Exp +128;

							localResult.set_slc(0, resultMantissa.slc<23>(1));
							localResult.set_slc(23, outputExp);                                      
						}
										
		                      break;                                                  
		                                                                              
		              case RISCV_FLOAT_OP_SUB  : 
		                
			if(!subState)
						{
							state = 1;
							subState = 1;
							stall = true;
							f1Val  = f1Mantissa;
							f2Val = f2Mantissa;
						}
						else
						{
							subState = 0;	
							stall = false;					
							if(f1Sign == f2Sign)
								localResult.set_slc(31, f1Sign);
							else
							{
								if(f1Val > f2Val)
									localResult.set_slc(31,f1Sign);
								else
									localResult.set_slc(31,f2Sign);
							}
							
							f2Sign = f2Sign ^ f2Sign;										
							resultMantissa = f1Val + f2Val;
							outputExp = f1Exp +128;

							localResult.set_slc(0, resultMantissa.slc<23>(1));
							localResult.set_slc(23, outputExp);                                      
						}                        
		                      break;                                                  
		                                                                              
		              case RISCV_FLOAT_OP_DIV  : 
							 state = 48;
							 quotient = 0;
							 remainder = 0;
							 stall = true;
		                      break;                                                  
		                                                                              
		              case RISCV_FLOAT_OP_MUL  :                                      
							outputSign = f1Sign ^ f2Sign;                 
							outputMantissa = f1Mantissa * f2Mantissa;
							resultMantissa = (outputMantissa[47] ?outputMantissa.slc<23>(24) : outputMantissa.slc<23>(23));
	 						outputExp = f1Exp + f2Exp - 127; 
	 
	 						if (outputMantissa[47])                                                 
	 							outputExp++;                                                    

							localResult.set_slc(31, outputSign); 
	 						localResult.set_slc(0, resultMantissa);
	 						localResult.set_slc(23, outputExp.slc<8>(0));

		                      break;                                                  
		                                                                              
		              case RISCV_FLOAT_OP_SQRT :                                      
		                      break;                                                  
		                                                                              
		              case RISCV_FLOAT_OP_SGN  :
						switch(dctoEx.funct3)
						{

							case 0 :
								localResult[31] = (bool) f2Sign; 
							break;

							case 1: 
								localResult[31] = !( (bool) f2Sign);
							break;
					
							case 2:
								localResult[31] = ((bool) f1Sign) ^ ((bool) f2Sign);
							break;
						}                                      
		                      break;                                                  
		                                                                              
		              case RISCV_FLOAT_OP_MINMAX :  
						if(dctoEx.funct3) //FMAX
						{
							if(f1Exp == f2Sign)
							{if(f1Sign){  // both are positive
								if (f1Exp > f2Exp)
									localResult = dctoEx.rhs;
								else
								{
									if(f2Exp > f1Exp)
										localResult = dctoEx.lhs;
									else
									{	
										if(f1Mantissa > f2Mantissa)
											localResult = dctoEx.rhs;
										else
											localResult = dctoEx.lhs;
									}
								}}
							else{ // both are negative
								if (f1Exp < f2Exp)
									localResult = dctoEx.rhs;
								else
								{
									if(f2Exp < f1Exp)
										localResult = dctoEx.lhs;
									else
									{	
										if(f1Mantissa < f2Mantissa)
											localResult = dctoEx.rhs;
										else
											localResult = dctoEx.lhs;
									}
								}}
							} //End if(f1Exp == f2Sign)
							else // rhs and lhs have different sign                            	
							{

							if(f1Sign < f2Sign) // rhs positive and lhs negative 
								localResult = dctoEx.lhs;
							else // rhs negative and lhs positive
								localResult = dctoEx.rhs;
							}  
						}
	   
						else //FMIN
						{
							if(f1Exp == f2Sign)
							{if(f1Sign){  // both are positive
								if (f1Exp < f2Exp)
									localResult = dctoEx.rhs;
								else
								{
									if(f2Exp < f1Exp)
										localResult = dctoEx.lhs;
									else
									{	
										if(f1Mantissa < f2Mantissa)
											localResult = dctoEx.rhs;
										else
											localResult = dctoEx.lhs;
									}
								}}
							else{ // both are negative
								if (f1Exp > f2Exp)
									localResult = dctoEx.rhs;
								else
								{
									if(f2Exp > f1Exp)
										localResult = dctoEx.lhs;
									else
									{	
										if(f1Mantissa > f2Mantissa)
											localResult = dctoEx.rhs;
										else
											localResult = dctoEx.lhs;
									}
								}}
							} //End if(f1Exp == f2Sign)
							else // rhs and lhs have different sign                            	
							{

							if(f1Sign < f2Sign) // rhs positive and lhs negative 
								localResult = dctoEx.lhs;
							else // rhs negative and lhs positive
								localResult = dctoEx.rhs;
							}  
						}

						
		                      break;                                                  
		                                                                              
		              case RISCV_FLOAT_OP_CVTWS :
		              	if(dctoEx.rhs.slc<8>(23)) // the float is normal so it may be interesting to compute his value
		              	{
		              	
							state = ((int) f1Exp) - 23;
							stall = true; 
							if(!dctoEx.rs2)
								{
									
									localResult =  (int) f1Mantissa;
								}
							else
								{
									localResult =(unsigned int) f1Mantissa;
								}                   
		              	}
		              	else // the float is subnormal so his value is near 0 --> do not need to compute his exact value
		              	{
		              		localResult = (ac_int<32,false>) 0;
							localResult.set_slc(31, f1Sign);		              	
		              	}
		              		
		              	
		                      break;                                                  
		                                                                              
		              case RISCV_FLOAT_OP_CMP  :
						switch(dctoEx.funct3)
								{
									case 0:
			
										if(f1Sign == f2Sign)
										{
											if(f1Sign)
											{
												if (f1Exp >= f2Exp)
													localResult[0] = true;
												else
												{
													if(f2Exp == f1Exp)
													{	
													if(f1Mantissa >= f2Mantissa)
														localResult[0] = true;
													}
												}	
											}
											else
											{
												if (f1Exp <= f2Exp)
													localResult[0] = true;
												else
												{
												if(f2Exp == f1Exp)
													{	
													if(f1Mantissa <= f2Mantissa)
														localResult[0] = true;
													}
												}
											}
										}
										else
										{
											if(f1Sign > f2Sign)
												localResult[0] = true ;
										}

									break;
									
									case 1:
											
										if(f1Sign == f2Sign)
										{
											if(f1Sign)
											{
												if (f1Exp > f2Exp)
													localResult[0] = true;
												else
												{
													if(f2Exp == f1Exp)
													{	
													if(f1Mantissa > f2Mantissa)
														localResult[0] = true;
													}
												}	
											}
											else
											{
												if (f1Exp < f2Exp)
													localResult[0] = true;
												else
												{
												if(f2Exp == f1Exp)
													{	
													if(f1Mantissa < f2Mantissa)
														localResult[0] = true;
													}
												}
											}
										}
										else
										{
											if(f1Sign > f2Sign)
												localResult[0] = true ;
										}

									break;

									case 2:
										localResult[0] = dctoEx.rhs = dctoEx.lhs;
									break;
								}                                    
		                      break;                                                  
		                                                                              
		              case RISCV_FLOAT_OP_CVTSW :  
		              				if((int) f1Exp) // if f1 is not null
		              				{
				          				
										if(dctoEx.rs2) // FCVT.S.WU
										{
											for(i = 31; i >= 0; i--)
												if (dctoEx.rhs[i])
														break;

											localResult.set_slc(23, (ac_int<8,false>) (127 + i));
											localResult.set_slc(0, (dctoEx.rhs << (31 - i)).slc<23>(8) );
										}
										else // FCVT.S.W
										{
											if(dctoEx.rhs[31]) // the number is negative -> 2's complement
											{
												for(i = 30; i >= 0; i--)
													if (!dctoEx.rhs[i])
															break;
															
												localResult.set_slc(23, (ac_int<8,false>) (127 + i-1));
												localResult.set_slc(0, (dctoEx.rhs << (31 - i+1)).slc<23>(8) );

											
											}
											else 
											{
												for(i = 31; i >= 0; i--)
													if (dctoEx.rhs[i])
														break;
												localResult.set_slc(23, (ac_int<8,false>) (127 + i));
												localResult.set_slc(0, (dctoEx.rhs << (31 - i)).slc<23>(8) );
											}
										}
		              				}   
									else //f1 is zero
									{
									localResult.set_slc(0, (ac_int<31, false>) 0);
									localResult.set_slc(31, f1Sign);
									}
		                      break;                                                  
		                                                                              
		              case RISCV_FLOAT_OP_MVWX :                                      
				  localResult = dctoEx.lhs;
		                      break;                                                  
		                                                                              
		              case RISCV_FLOAT_OP_CLASSMVXW :                                 
				  if (dctoEx.funct3) // funct3 = 0 -> FMV.X.W
				  {
					localResult = dctoEx.lhs;
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
 else // state != 0 ->  loop
   { 
	   switch(dctoEx.funct7)
	   {
	 	  case RISCV_FLOAT_OP_DIV : 

			for(int i =0; i <4; i++) 
			{
				state--;
				remainder = remainder << 1;
				remainder[0] = f1Val[state];
				if(remainder >= f2Val)
				{
					remainder = remainder - f2Val;
					quotient[state] = 1;
				}
			}
			if(!state)
			{
				outputSign = f1Sign ^ f2Sign;
			  	outputExp = f1Exp - f2Exp + 127;
				stall = false;
				
				if(!quotient[24])
				{
						outputExp--;
						localResult.set_slc(0, quotient.slc<23>(0)); 
				}
				else 
					 	localResult.set_slc(0, quotient.slc<23>(1)); 
				

	  			localResult.set_slc(23, outputExp.slc<8>(0));
	  			localResult.set_slc(31, outputSign);
		
			}
			break;
			
		    case RISCV_FLOAT_OP_CVTWS : 
				if (state > 0)
				{
					if (state >= 32)
						{localResult = localResult << 32; state -= 32;}
					else
						{localResult = localResult << (state % 32); state = 0;}
				}
				else
				{
					if (state <= -32)
						{localResult = localResult >> 32; state += 32;}
					else
						{localResult = localResult >> (32 - (state % 32)); state = 0;}
				
				}
				
				if (!state)
				{
					stall = false;
					if(!dctoEx.rs2 && f1Exp)
						localResult = - localResult; 
				}
				break;
				
			case RISCV_FLOAT_OP_ADD :
			case RISCV_FLOAT_OP_SUB: 
				
				if (f1Exp > f2Exp)
				{
					if (f1Exp - f2Exp >= 32)
						{f1Val = f1Val << 32; f1Exp -= 32;}
					else
						{f1Val = f1Val << (f1Exp - f2Exp % 32); f1Exp = f2Exp; state = 0;}
				}
				else
				{
					if (f2Exp - f1Exp >= 32)
						{f2Val = f2Val << 32; f2Exp -= 32;}
					else
						{f2Val = f2Val << (f2Exp - f1Exp % 32); f2Exp = f1Exp; state = 0;}
				
				}
		
				break;  
				
			default: 
				break;		
		}
   }
   
   if(( (dctoEx.opCode == RISCV_FLOAT_OP)|(dctoEx.opCode == RISCV_FLOAT_MADD)|(dctoEx.opCode == RISCV_FLOAT_MSUB)|(dctoEx.opCode == RISCV_FLOAT_NMADD)|(dctoEx.opCode == RISCV_FLOAT_NMSUB) ))
   	   extoMem.result = localResult;                       

} 

		
};



#endif /* FLOAT_ALU_H */ 
