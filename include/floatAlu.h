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
	
	// var for sqrt
	
	bool performSqrt = false;
	int iter = 0;
	int step = 0;
	bool doneSqrt = false;
	ac_int<32,false> tempSqrt; // we should choose a value near the sqrt of lhs, the use of look up table may help.
	ac_int<32,false> tempValue = 0;
	
	

	
public :
	void process(struct DCtoEx dctoEx, struct ExtoMem &extoMem, bool &stall)
{		
   	  ac_int<1, false> f1Sign;
	  ac_int<1, false> f2Sign;
 
	  ac_int<24, false> f1Mantissa;
	  ac_int<24, false> f2Mantissa;
 
	  ac_int<9, true> f1Exp; 
	  ac_int<9, true> f2Exp;
	  
	  ac_int<8,false> f2UExp;
	  ac_int<8,false> f1UExp;

	  ac_int<1, false> outputSign;                 
	  ac_int<48, false> outputMantissa;
	  ac_int<48, false> resultMantissa;
 	  ac_int<9, false> outputExp;


      float f1;
	  int g;
	  int i;
	  
    stall =false;       

	 	// Pretreatment for SQRT
	 	
		if(dctoEx.opCode == RISCV_FLOAT_OP & dctoEx.funct7 == RISCV_FLOAT_OP_SQRT)
		{
			/* set the flags, opCode and funct to proceed the correct compute 
				iteration = 5
				u_n+1 = (u_n + a/u_n) >> 1
			
			while(iter > 0)
				step n°0 : a/u_n
				step n°1 : u_n + a/u_n
				step n°2 : >> 1 & iter--
			*/ 
			
			if(!iter) // iter == 0 case 1 : we haven't compute the value yet; case 2 : we have already compute it.
			{
				if(doneSqrt)
				{
					localResult = tempSqrt;
					tempSqrt = dctoEx.lhs;
					performSqrt = 0;
					doneSqrt = 0;
				}
				else // init the sqrt
				{
							iter = 10;
							performSqrt = 1;
							step = 0;
							tempValue = 0;
							tempSqrt = dctoEx.lhs;
							
							//init the opCode funct and value for step 0
							
							dctoEx.opCode  = RISCV_FLOAT_OP;
							dctoEx.funct7 = RISCV_FLOAT_OP_DIV;
							dctoEx.rhs = tempSqrt;
							stall = true;
				}
			}
			else // iter != 0, we are computing a sqrt, we juste have to set the value 
			{
				switch(step)
				{
					case 0 :
						performSqrt = 1;
						dctoEx.opCode  = RISCV_FLOAT_OP;
						dctoEx.funct7 = RISCV_FLOAT_OP_DIV;
						dctoEx.rhs = tempSqrt;
						stall = true;
						break;
						
					case 1 :
						performSqrt = 1;
						dctoEx.opCode = RISCV_FLOAT_OP;
						dctoEx.funct7 = RISCV_FLOAT_OP_ADD;
						dctoEx.lhs = tempValue;
						dctoEx.rhs = tempSqrt;
						stall = true;
						break; 	
						
					case 2 : 
						performSqrt = 1;
						dctoEx.opCode  = RISCV_FLOAT_OP;
						dctoEx.funct7 = RISCV_FLOAT_OP_DIV;
						dctoEx.lhs = tempValue;
						dctoEx.rhs = 0x40000000;
						stall = true;
						break;
						
					case 3 :
						if(iter == 1)
							doneSqrt = 1;
						
						performSqrt = 1;
						iter--;
						step = 0;
						stall = true;
						tempSqrt = tempValue;
						break;
				}
			}
		}
		
	// Initialisation	


	f1Mantissa = dctoEx.lhs.slc<23>(0);
	f2Mantissa = dctoEx.rhs.slc<23>(0);
	f1Mantissa[23] = 1;
	f2Mantissa[23] = 1;
	
	if(dctoEx.lhs.slc<8>(23) != 0)
	{
		f1Mantissa[23] = 1;
		f1Exp = dctoEx.lhs.slc<8>(23) - 127;  
	 }
	 else 
	 {
		f1Exp = - 126;
	 }
	
	 if(dctoEx.rhs.slc<8>(23) != 0)
	 {
		f2Mantissa[23] = 1;
		 f2Exp = dctoEx.rhs.slc<8>(23) - 127;  
	 }
	 else 
	 {
	  	f2Exp =  - 126;  
	 }
				
	f1Sign = dctoEx.lhs.slc<1>(31);
	f2Sign = dctoEx.rhs.slc<1>(31);
	
	f1UExp = dctoEx.lhs.slc<8>(23);
	f2UExp = dctoEx.rhs.slc<8>(23);
	

	//  Nan handling

	if((dctoEx.opCode == RISCV_FLOAT_OP & dctoEx.funct7 != RISCV_FLOAT_OP_CMP 
	& (( ((ac_int<8,false>) dctoEx.rhs.slc<8>(23)) ==  0xff & f1Mantissa != 0) | ( ((ac_int<8,false>) dctoEx.lhs.slc<8>(23)) == 0xff & f2Mantissa != 0)) )) // we have a float instructions with at least a Nan
	{ // Give seg fault
	   if(((f1Exp == 0xff & f1Mantissa) & (f2Exp == 0xff & f2Mantissa))) // both are Nan
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
	   			if (dctoEx.useRs2)
	   				localResult = dctoEx.rhs;  
	   			else
	   				localResult = CNAN; 
	    }
	}
	else if(dctoEx.opCode == RISCV_FLOAT_OP & dctoEx.funct7 == RISCV_FLOAT_OP_CMP 
			& ((f1Exp == 0xff & f1Mantissa != 0) | (f2Exp == 0xff & f2Mantissa != 0)) )// CMP case we return false all the time if one of the operand is a Nan 
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
									  case RISCV_FLOAT_OP_SUB  : 
											f2Sign = 1 - f2Sign;
									  case  RISCV_FLOAT_OP_ADD :
										  if(f1Exp != 0xff & f2Exp != 0xff) // operand are not exceptions
										  {
										  
											if(!subState)
											{
												state = f1UExp - f2UExp;
												subState = 1;
												stall = true;
												f1Val = (ac_int<48,false>) f1Mantissa;
												f2Val = (ac_int<48,false>) f2Mantissa;
											}
											else
											{
															
												subState = 0;	
												stall = false;					
												
												// Compute result Mantissa, it have to be > 0
												if(f1Sign == f2Sign)
												{
														resultMantissa = f1Val + f2Val;
												}
												else
												{
													if(f1Val > f2Val)
													{
														resultMantissa = f1Val - f2Val;
													} 
													else
													{
														resultMantissa = f2Val - f1Val;
													}
												}
												
												// Compute the sign 
												

													if(f1UExp > f2UExp)
													{
														outputSign = f1Sign;
													}
													else
													{
														if(f1UExp == f2UExp)
														{
															if(f1Mantissa > f2Mantissa)
															{
																outputSign = f1Sign;
															}
															else
															{
																outputSign = f2Sign;
															}
														}
														else
														{
															outputSign = f2Sign;
														}
													}
												
												
												localResult.set_slc(31,outputSign);
													
												if(f1UExp > f2UExp)
													outputExp = f1UExp;
												else
													outputExp = f2UExp;
												

												
												
												for(i = 47; i >= 0; i--)  //Find the MSB
													if (resultMantissa[i])
														break;
														

												if (i > 23) 
												{
													localResult.set_slc(0, resultMantissa.slc<23>(i - 23) );
													outputExp += i - 23;
												}
												else 
												{
													if(outputExp > 23 - i)
													{
															outputExp -=23 - i;
															resultMantissa = resultMantissa << (23 - i);
															localResult.set_slc(0,resultMantissa.slc<23>(0));
													}
													else
													{
															localResult.set_slc(0,resultMantissa.slc<23>(0));
															outputExp = 0;
													}	
												}
																								
												localResult.set_slc(23,outputExp.slc<8>(0));
												
												
												if(outputExp.slc<8>(0) > 254) // over and underflow handeling with return of infty
												{
													if(f1Sign != 0)
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
												
												if(performSqrt)
												{
													tempValue = localResult;
													step++;
													stall = true;
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
										  	else 
										  		localResult = dctoEx.lhs; // infty + float = infty 
										  }  
															
										      break;                                                  
										                                                              
										                
									  case RISCV_FLOAT_OP_DIV  : 

									  	if(f1Exp != 0xff & f2Exp != 0xff) // float are not exceptions
										{
											  	if(dctoEx.lhs != 0)
											  	{
													 state = 48;
													 quotient = 0;
													 remainder = 0;
													 stall = true;
													 f1Val.set_slc(24,f1Mantissa);
													 f2Val.set_slc(0,f2Mantissa);
 											  	}
												else
												{
													DIV0 = 1;
													localResult = INFP;
													localResult[31] = f2Sign;
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
						 						outputExp = f1Exp + f2Exp - 129; 
						 
						 						
														
						 						if (outputMantissa[47])                                                 
						 							outputExp++;                                                    

												localResult.set_slc(31, outputSign); 
						 						localResult.set_slc(0, resultMantissa.slc<23>(0));
						 						localResult.set_slc(23, outputExp.slc<8>(0));
						 						
						 						if(outputExp.slc<8>(0) > 254) // over and underflow handeling with return of infty
												if(f1Sign != 0)
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
										  			if(dctoEx.lhs != 0) // infty * float 
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
										  		if(dctoEx.rhs != 0) // infty * float 
										  			{
											  			localResult = dctoEx.lhs;
											  			localResult = f1Sign ^ f2Sign; 
										  			}
										  			else // infty * 0 
										  				localResult = CNAN; 
										  	}	
										  }  
										      break; 
										      
									  /*case RISCV_FLOAT_OP_SQRT:
										
										 everything is handle in pretreatment
											                                                                                         
								      */ 	                                 
								                                   
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
										localResult = dctoEx.lhs;
											if(f1Sign == f2Sign)
														{
															if(f1Sign.slc<1>(0) != 0) // negative
															{
																if (f1UExp < f2UExp)
																	localResult =  dctoEx.rhs;
																else 
																{
																	if(f2UExp == f1UExp)
																	{
																		if(f1Mantissa < f2Mantissa)
																		localResult =  dctoEx.rhs;
																	}
																																	
																}	
															}
															else
															{
																if (f1UExp > f2UExp)
																	localResult =  dctoEx.rhs;
																else
																{
																	if(f2UExp == f1UExp)
																	{	
																		if(f1Mantissa > f2Mantissa)
																			localResult =  dctoEx.rhs;
																	}
																}
															}
														}
														else
														{
															if(f1Sign < f2Sign)
																localResult =  dctoEx.rhs ;
														}

										}
										else //FMIN
										{
										localResult = dctoEx.rhs;
											if(f1Sign == f2Sign)
														{
															if(f1Sign.slc<1>(0) != 0) // negative
															{
																if (f1UExp < f2UExp)
																	localResult =  dctoEx.lhs;
																else 
																{
																	if(f2UExp == f1UExp)
																	{
																		if(f1Mantissa < f2Mantissa)
																		localResult =  dctoEx.lhs;
																	}
																																	
																}	
															}
															else
															{
																if (f1UExp > f2UExp)
																	localResult =  dctoEx.lhs;
																else
																{
																	if(f2UExp == f1UExp)
																	{	
																		if(f1Mantissa > f2Mantissa)
																			localResult =  dctoEx.lhs;
																	}
																}
															}
														}
														else
														{
															if(f1Sign < f2Sign)
																localResult =  dctoEx.lhs ;
														}

										}

										
										      break;                                                  
										                                                              
									  case RISCV_FLOAT_OP_CVTWS :
									 	if( f1Exp > 0) // the float is normal and superior to 1 so it may be interesting to compute his value
										{	
											if(dctoEx.rs2 == 0)
												localResult =  (int) f1Mantissa;
											else
												localResult =(unsigned int) f1Mantissa;
			
									  
									  		state = 23 - ((int) f1Exp) ;
	
											if (state >= 0)
											{
												if (state >= 32)
													localResult = 0;
												else
													localResult = localResult >> state ; 
											}
											else
											{
												if (state <= -32)
													localResult = 0;
												else
													localResult = localResult << (- state);
											}
										

																	
										}
									  	else // the float is subnormal so his value is near 0 --> do not need to compute his exact value
									  	{
									  		localResult = (ac_int<32,false>) 0;
													  	
									  	}

														
										if(f1Sign != 0)
											localResult = - localResult; 

									  		
									  	
								break;                                                  
										                                                              
									  case RISCV_FLOAT_OP_CMP  :
									  localResult = 1;

										switch(dctoEx.funct3)
												{
													case 0:  // FLE
														if(f1Sign == f2Sign)
														{
															if(f1Sign.slc<1>(0) != 0) // negative
															{
																if (f1UExp < f2UExp)
																	localResult =  0;
																else 
																{
																	if(f2UExp == f1UExp)
																	{
																		if(f1Mantissa <= f2Mantissa)
																		localResult = 0;
																	}
																																	
																}	
															}
															else
															{
																if (f1UExp > f2UExp)
																	localResult = 0;
																else
																{
																	if(f2UExp == f1UExp)
																	{	
																		if(f1Mantissa >= f2Mantissa)
																			localResult = 0;
																	}
																}
															}
														}
														else
														{
															if(f1Sign < f2Sign)
																localResult = 0 ;
														}

													break;
													
													case 1:  // FLT
														if(f1Sign == f2Sign)
														{
															if(f1Sign.slc<1>(0) != 0) // negative
															{
																if (f1UExp < f2UExp)
																	localResult =  0;
																else 
																{
																	if(f2UExp == f1UExp)
																	{
																		if(f1Mantissa < f2Mantissa)
																		localResult = 0;
																	}
																																	
																}	
															}
															else
															{
																if (f1UExp > f2UExp)
																	localResult = 0;
																else
																{
																	if(f2UExp == f1UExp)
																	{	
																		if(f1Mantissa > f2Mantissa)
																			localResult = 0;
																	}
																}
															}
														}
														else
														{
															if(f1Sign < f2Sign)
																localResult = 0 ;
														}

													break;
													

													case 2:  //FEQ
														localResult[0] = dctoEx.rhs == dctoEx.lhs;
													break;
												}                                    
										      break;                                                  
										                                                              
									  case RISCV_FLOAT_OP_CVTSW :  
											if(dctoEx.rs2 != 0 ) // FCVT.S.WU
											{
												for(i = 31; i >= 0; i--)
													if (dctoEx.lhs[i])
															break;
												localResult.set_slc(23, (ac_int<8,false>) (127 + i));
												localResult.set_slc(0, (dctoEx.lhs << (31 - i)).slc<23>(8) );
											}
											else // FCVT.S.W
											{
												if(dctoEx.lhs[31]) // the number is negative -> 2's complement
												{
													for(i = 30; i >= 0; i--)
														if (!dctoEx.lhs[i])
																break;
																	
													localResult.set_slc(23, (ac_int<8,false>) (127 + i-1));
													localResult.set_slc(0, (dctoEx.rhs << (31 - i+1)).slc<23>(8) );
													}
												else 
												{
													for(i = 30; i >= 0; i--)
														if (dctoEx.lhs[i])
															break;
													localResult.set_slc(23, (ac_int<8,false>) (127 + i));
													localResult.set_slc(0, (dctoEx.lhs << (31 - i)).slc<23>(8) );
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
								stall = true;
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
							  	outputExp = f1Exp - f2Exp + 126;
								
								for(i = 47; i >= 0; i--)  //Find the MSB
									if (quotient[i])
										break;
										
								if (i > 22) // the result is normal
								{
									localResult.set_slc(0, quotient.slc<23>(i - 23) );
									outputExp += i - 23;
								}
								else // the result is a subnormal 
								{
								outputExp = 0;
								localResult.set_slc(0,quotient.slc<23>(0));
								}

								

					  			localResult.set_slc(23, outputExp.slc<8>(0));
					  			localResult.set_slc(31, outputSign);
								stall = false;

					  			if(outputExp.slc<8>(0) > 254)  // over and underflow handling with return of infty
									if(f1Sign != 0)
									{
										UNDERF = 1;		
										localResult = INFN;
									}
									else
									{
										OVERF = 1;		
										localResult = INFP;
									}
							 localResult.set_slc(31, outputSign);
						
							
								if(performSqrt)
								{
									step++;
									tempValue = localResult;
									stall = true;
								}
							}
							break;

								
							case RISCV_FLOAT_OP_ADD :
							case RISCV_FLOAT_OP_SUB: 
								stall = true;

								if (state > 0)
								{
									if (state > 32)
										{f2Val = f2Val >> 32; state -=32;}
									else
										{f2Val = f2Val >> state; state =0;}
								}
								else
								{
									if (state < -32)
										{f1Val = f1Val >> 32; state +=32;}
									else
										{f1Val = f1Val >> - state; state =0;}
								
								}
								


								break;  
								
							default: 
								break;		
						}
				   }
	}
   
   if(( (dctoEx.opCode == RISCV_FLOAT_OP)|(dctoEx.opCode == RISCV_FLOAT_MADD)|(dctoEx.opCode == RISCV_FLOAT_MSUB)|(dctoEx.opCode == RISCV_FLOAT_NMADD)|(dctoEx.opCode == RISCV_FLOAT_NMSUB) ))
   {
   	   extoMem.result = localResult;
		   
   }



} 

		
};



#endif /* FLOAT_ALU_H */ 
