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


#define INVOP statusRegister[4] 
#define DIV0 statusRegister[3]
#define OVERF statusRegister[2]
#define UNDERF statusRegister[1]
#define INEX statusRegister[0]
#define RNDM statusRegister.slc<3>(5)
#define CNAN 0x7fc00000
#define INFP 0x7f800000
#define INFN 0xff800000

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
	ac_int<32,false> statusRegister = 0;
	

	
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
	

	//  Nan handeling

	if((dctoEx.opCode == RISCV_FLOAT_OP & dctoEx.funct7 != RISCV_FLOAT_OP_CMP 
	& ((f1Exp == 0xff & f1Mantissa) | (f2Exp == 0xff & f2Mantissa)) )) // we have a float instructions with at leat a Nan
	{
	   if(f1Exp == 0Xff & f2Exp = 0xff & f1Mantissa & f2Mantissa) // both are Nan
	   {
	   localResult = CNAN;
	   }
	   else if (f1Exp == 0xff & f1Mantissa) // f1 is a Nan
	   {
	   		if (f1Mantissa[22]) // f1 is a signaling Nan
	   			localResult = CNAN;
	   		
	   		else // f1 is a quiet Nan and f2 is not a Nan
	   			localResult = dctoEx.lhs; 
	   }
	   else // f1 is not a Nan and f2 is a Nan 
	   	{
	   		if (f2Mantissa[22]) // f2 is a signaling Nan
	   			localResult = CNAN;
	   		
	   		else // f2 is a quiet Nan and f1 is not a Nan
	   			localResult = dctoEx.rhs; 
	   }
	}
	else if(dctoEx.opCode == RISCV_FLOAT_OP & dctoEx.funct7 == RISCV_FLOAT_OP_CMP 
			& ((f1Exp == 0xff & f1Mantissa) | (f2Exp == 0xff & f2Mantissa)) )// CMP case we return false all the time if one of the operand is a Nan 
		{/* default value of localResult is already 0*/	}
	
	else// Normal case
	{


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
										  if(f1Exp != 0xff & f2Exp != 0xff) // operand are not exceptions
										  {
										  
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
												
												if(outputExp > 126) // over and underflow handeling with return of infty
													if(f1Sign)
														{
														UNDERF = 1;		
														localResult = INFN;
														}
													else
														{
														OVERF = 1;		
														localResult = INFP;
														}                                   
											}
										  }
										  else // one of the operand is an exception
										  {
										  	if (f1Exp == 0xff)
										  	{
										  		if (f2Exp == 0xff)
										  		{
										  			if(f1Sign == f2Sign)
										  				localResult = dctoEx.rhs; // infty + infty = infty if they have the same sign
										  			else
										  				localResult = NAN; // + infty - infty = Nan 
										  		}
										  		else
										  			localResult = dctoEx.rhs; // infty + float = infty 
										  	}
										  	else // Nan are not handle yet
										  		localResult = dctoEx.lhs; // infty + float = infty 
										  }  
															
										      break;                                                  
										                                                              
									  case RISCV_FLOAT_OP_SUB  : 
											if(f1Exp != 0xff & f2Exp != 0xff) // float are not exceptions
										  	{
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
													
													if(outputExp > 126) // over and underflow handeling with return of infty
														if(f1Sign)
															{
															UNDERF = 1;		
															localResult = INFN;
															}
														else
															{
															OVERF = 1;		
															localResult = INFP;
															}                                   
												}                        
										  }
										  else // one of the operand is an exception
										  {
										  	if (f1Exp == 0xff)
										  	{
										  		if (f2Exp == 0xff)
										  		{
										  			if(f1Sign == f2Sign)
										  				localResult = dctoEx.rhs; // infty + infty = infty if they have the same sign
										  			else
										  				localResult = CNAN; // + infty - infty = Nan 
										  		}
										  		else
										  			localResult = dctoEx.rhs; // infty + float = infty 
										  	}
										  	else
										  		localResult = dctoEx.lhs; // infty + float = infty 
										  }  
											
													  break;                                                  
										                                                              
									  case RISCV_FLOAT_OP_DIV  : 
									  	if(f1Exp != 0xff & f2Exp != 0xff) // float are not exceptions
										{
											  	if(dctoEx.lhs)
											  	{
											  	
													 state = 48;
													 quotient = 0;
													 remainder = 0;
													 stall = true;
											  	}
												else
												{
													DIV0 = 1;
												}
										}
										  else // one of the operand is an exception
										  {
										  	if (f1Exp == 0xff)
										  	{
										  		if (f2Exp == 0xff) // infty / infty -> NaN
										  		{
										  			localResult = CNAN;
										  		}
										  		else // infty / float -> infty
										  		localResult = dctoEx.rhs;
										  			
										  	} // else float / infty -> 0 
										  		
										  }  
													  break;                                                  
										                                                              
									  case RISCV_FLOAT_OP_MUL  :  
									  	if(f1Exp != 0xff & f2Exp != 0xff) // float are not exceptions
										{                                    
												outputSign = f1Sign ^ f2Sign;                 
												outputMantissa = f1Mantissa * f2Mantissa;
												resultMantissa = (outputMantissa[47] ?outputMantissa.slc<23>(24) : outputMantissa.slc<23>(23));
						 						outputExp = f1Exp + f2Exp - 127; 
						 
						 						
														
						 						if (outputMantissa[47])                                                 
						 							outputExp++;                                                    

												localResult.set_slc(31, outputSign); 
						 						localResult.set_slc(0, resultMantissa);
						 						localResult.set_slc(23, outputExp.slc<8>(0));
						 						
						 						if(outputExp > 126) // over and underflow handeling with return of infty
												if(f1Sign)
													{
													UNDERF = 1;		
													localResult = INFN;
													}
												else
													{
													OVERF = 1;		
													localResult = INFP;
													}
						 				}
										  else // one of the operand is an exception
										  {
										  	if (f1Exp == 0xff)
										  	{
										  		if (f2Exp == 0xff)
										  		{
										  			if(f1Sign == f2Sign)
										  				localResult = INFP;
										  			else
										  				localResult = INFN;
										  		}
										  		else
										  		{	
										  			if(dctoEx.lhs) // infty * float 
										  			{
											  			localResult = dctoEx.rhs;
											  			localResult = f1Sign ^ f2Sign; 
										  			}
										  			else // infty * 0 
										  				localResult = CNAN; 
										  		}
										  	}
										  	else 
										  	{				          	
										  		if(dctoEx.rhs) // infty * float 
										  			{
											  			localResult = dctoEx.lhs;
											  			localResult = f1Sign ^ f2Sign; 
										  			}
										  			else // infty * 0 
										  				localResult = CNAN; 
										  	}	
										  }  
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
													case 0:  // FLE
													
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
													
													case 1:  // FLT
															
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

													case 2:  //FEQ
														localResult[0] = dctoEx.rhs == dctoEx.lhs;
													break;
												}                                    
										      break;                                                  
										                                                              
									  case RISCV_FLOAT_OP_CVTSW :  
									  				if( (f1Exp != 0 & f1Mantissa != 0 ) & f1Exp != 0xff) // if f1 is not an exception (exception are Nan infty and 0)
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
													else //f1 is an exception
													{
														if(f1Exp) // infty or Nan 
														{
															if(f1Mantissa) // f1Mantissa != 0 -> Nan
																{
																	localResult = CNAN;	
																}
															else // infty --> max int
																localResult = 0xffffffff;
															
														}
														else // 0
														{
															localResult.set_slc(0, (ac_int<31, false>) 0);
															localResult.set_slc(31, f1Sign);
														}
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
								
								for(i = 47; i >= 0; i--)
									if (quotient[i])
										break;
										
								if (i > 22) // the result is normal
								{
									localResult.set_slc(0, quotient.slc<23>(i - 23) );
									outputExp += i - 24 ;
								}
								else // the result is a subnormal 
								{
								outputExp = 0;
								localResult.set_slc(0,quotient.slc<23>(0));
								}

								

					  			localResult.set_slc(23, outputExp.slc<8>(0));
					  			localResult.set_slc(31, outputSign);
					  			
					  			if(outputExp > 126)  // over and underflow handeling with return of infty
									if(f1Sign)
									{
										UNDERF = 1;		
										localResult = INFN;
									}
									else
									{
										OVERF = 1;		
										localResult = INFP;
									}
						
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
	}
   
   if(( (dctoEx.opCode == RISCV_FLOAT_OP)|(dctoEx.opCode == RISCV_FLOAT_MADD)|(dctoEx.opCode == RISCV_FLOAT_MSUB)|(dctoEx.opCode == RISCV_FLOAT_NMADD)|(dctoEx.opCode == RISCV_FLOAT_NMSUB) ))
   	   extoMem.result = localResult;                       

} 

		
};



#endif /* FLOAT_ALU_H */ 
