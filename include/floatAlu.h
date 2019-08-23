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
#define RNDM 1 //statusRegister.slc<3>(5)
#define CNAN 0x7fc00000
#define INFP 0x7f800000
#define INFN 0xff800000

#define BIAS 127
#define S_MTS 23
#define S_EXP 8
#define BIT_W 32
#define SUB_BIAS 126
#define MAX_EXP 0xff

// S_MTS + 1 is for having a variable wich contain a mantissa with the hidden bit
//  S_EXP + 1 is for adding two exp and keeping the precision

// S_MTS + 2 is for the same meaning with full mantissa (and to not use a 48 bits adder)

// 2*(S_MTS + 1) is for keeping precision when computing with two full mantissa
// 2*(S_MTS + 1) - 1 is the number of the MSB in a 2*(S_MTS + 1) bits variable

//MAX_EXP is the maximal value of the exposant : tipically 2^S_EXP -1 

// BIAS and SUB_BIAS are the value of the bias in the IEE-754 norm



class FloatAlu : public ALU
{
private :
	int state = 0;
	int subState = 0;
	ac_int<2*(S_MTS + 1), false> quotient = 0;
	ac_int<2*(S_MTS + 1),false> remainder = 0;
	ac_int<2*(S_MTS + 1),false> f1Val = 0;
	ac_int<2*(S_MTS + 1),false> f2Val = 0;
	ac_int<(S_MTS + 1),false> tmp = 0;
	ac_int<BIT_W, false> localResult = 0;
	ac_int<BIT_W,false> statusRegister = 0;
	
	enum roundingMode {
	 RNE = 0,
	 RTZ = 1,
	 RDN = 2, 
	 RUP = 3, 
	 RMM = 4};
	
	// var for pretreatment
	
	bool performSqrt = false;
	bool performFused = 0;
	int iter = 0;
	int step = 0;
	ac_int<BIT_W,false> tempValue = 0;
	bool doneSqrt = false;
	ac_int<BIT_W,false> tempSqrt; // we should choose a value near the sqrt of lhs, the use of look up table may help.
	

	

	
public :
	void iterMantissa(ac_int<BIT_W,false> &f)
	{
		ac_int<(S_MTS + 1),false> mantissa = f.slc<S_MTS>(0);
		ac_int<(S_EXP + 1),false> exp = f.slc<S_EXP>(S_MTS);
		

			
			mantissa++;
			
			if(mantissa[S_MTS] != 0)
			{
				exp++;
				if(exp[S_EXP] != 0)
				{
					OVERF = 1;
				}
				else
				{
					f.set_slc(S_MTS,exp.slc<S_EXP>(0));
				}
				f.set_slc(0,mantissa.slc<S_MTS>(1));
			}
			else
			{
				f.set_slc(0,mantissa.slc<S_MTS>(0));
			}
		
		
	}



  bool process(struct DCtoEx dctoEx, ac_int<BIT_W, false> &result, bool &stall) 
{		
   	  ac_int<1, false> f1Sign;
	  ac_int<1, false> f2Sign;
 
	  ac_int<(S_MTS + 1), false> f1Mantissa;
	  ac_int<(S_MTS + 1), false> f2Mantissa;
 
	  ac_int<(S_EXP + 1), true> f1Exp; 
	  ac_int<(S_EXP + 1), true> f2Exp;
	  
	  ac_int<S_EXP,false> f2UExp;
	  ac_int<S_EXP,false> f1UExp;

	  ac_int<1, false> outputSign;                 
	  ac_int<2*(S_MTS + 1), false> outputMantissa;
	  ac_int<2*(S_MTS + 1), false> resultMantissa;
 	  ac_int<(S_EXP + 1), true> outputExp;
 	  
 	  ac_int<2,false> roundingFlags; // roundingFlags[0] = (cuttedBits >= 0.5) et roundingFlags[1] = (cuttedBits == 0.5) 


	  int i,k;
	  bool isHalf = false, changeResult = false;
	  
    stall = false;       

	 	// Pretreatment for SQRT
	 	
		if(dctoEx.opCode == RISCV_FLOAT_OP & dctoEx.funct7 == RISCV_FLOAT_OP_SQRT)
		{
			/* set the flags, opCode and funct to proceed the correct compute 
				iteration = 10
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
					changeResult = true;
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
		
	if((dctoEx.opCode == RISCV_FLOAT_MADD)|(dctoEx.opCode == RISCV_FLOAT_MSUB)|(dctoEx.opCode == RISCV_FLOAT_NMADD)|(dctoEx.opCode == RISCV_FLOAT_NMSUB))
	{
		/*
		FMADD 
			step n°0 : rs1 * rs2
			step n°1 : rs1 * rs2 + rs3 
			
		FMSUB 
			step n°0 : same as FMADD
			step n°1 : same as FMADD with - rs3
			
		FNMADD 
			step n°0 : same as FMADD with - rs1
			step n°1 : same as FMADD
			
		FNMSUB 
			step n°0 : same as FMADD with - rs1
			step n°1 : same as FMSUB
		
		*/
	if(!performFused)
	{	
				performFused = 1;
				//printf("step 0.0\n");
				switch(dctoEx.opCode)
				{
					case RISCV_FLOAT_NMADD : 
					case RISCV_FLOAT_NMSUB : 
						dctoEx.lhs[(BIT_W -1)] = 1 - dctoEx.lhs.slc<1>((BIT_W -1)); 
					case RISCV_FLOAT_MSUB : 
					case RISCV_FLOAT_MADD : 
						dctoEx.opCode = RISCV_FLOAT_OP;
						dctoEx.funct7 = RISCV_FLOAT_OP_MUL;
						// rhs and lhs are already set		
						break;
				}
	}
	else
	{
			if(step == 0)
			{
				//printf("step 0.1\n");
				switch(dctoEx.opCode)
				{
					case RISCV_FLOAT_NMADD : 
					case RISCV_FLOAT_NMSUB : 
						dctoEx.lhs[(BIT_W -1)] = 1 - dctoEx.lhs.slc<1>((BIT_W -1)); 
					case RISCV_FLOAT_MSUB : 
					case RISCV_FLOAT_MADD : 
						dctoEx.opCode = RISCV_FLOAT_OP;
						dctoEx.funct7 = RISCV_FLOAT_OP_MUL;
						stall = true;
						// rhs and lhs are already set		
						break;
				}
			}
			else if(step == 1)
			{
				//printf("step 1\n");
				switch(dctoEx.opCode)
				{
					case RISCV_FLOAT_NMSUB : 
					case RISCV_FLOAT_MSUB : 
						dctoEx.mhs[(BIT_W -1)] = 1 - dctoEx.mhs.slc<1>((BIT_W -1));
					case RISCV_FLOAT_NMADD : 
					case RISCV_FLOAT_MADD : 
						dctoEx.opCode = RISCV_FLOAT_OP;
						dctoEx.funct7 = RISCV_FLOAT_OP_ADD;
						dctoEx.lhs = tempValue;
						dctoEx.rhs = dctoEx.mhs;
						stall = true;
						break;
				}
			}
			else // step n°2 : return step
			{
				//printf("return result\n");
				step = 0;
				performFused = 0;
				localResult = tempValue;
				tempValue = 0;
				changeResult = true;
				stall = false;
			}
		}
	}
		
	// Initialisation	
	


	f1Mantissa = dctoEx.lhs.slc<S_MTS>(0);
	f2Mantissa = dctoEx.rhs.slc<S_MTS>(0);
	f1Mantissa[S_MTS] = 1;
	f2Mantissa[S_MTS] = 1;
	
	if(dctoEx.lhs.slc<S_EXP>(S_MTS) != 0)
	{
		f1Mantissa[S_MTS] = 1;
		f1Exp = dctoEx.lhs.slc<S_EXP>(S_MTS) - BIAS;  
	 }
	 else 
	 {
		f1Exp = - SUB_BIAS;
	 }
	
	 if(dctoEx.rhs.slc<S_EXP>(S_MTS) != 0)
	 {
		f2Mantissa[S_MTS] = 1;
		 f2Exp = dctoEx.rhs.slc<S_EXP>(S_MTS) - BIAS;  
	 }
	 else 
	 {
	  	f2Exp =  - SUB_BIAS;  
	 }
				
	f1Sign = dctoEx.lhs.slc<1>((BIT_W -1));
	f2Sign = dctoEx.rhs.slc<1>((BIT_W -1));
	
	f1UExp = dctoEx.lhs.slc<S_EXP>(S_MTS);
	f2UExp = dctoEx.rhs.slc<S_EXP>(S_MTS);
	

	//  Nan handling
	
	

	if(dctoEx.useRs2 & (dctoEx.opCode == RISCV_FLOAT_OP & dctoEx.funct7 != RISCV_FLOAT_OP_CMP 
	& (( f1UExp ==  MAX_EXP & f1Mantissa.slc<S_MTS>(0) != 0) | ( f2UExp == MAX_EXP & f2Mantissa.slc<S_MTS>(0) != 0)) )) // we have a float instructions at two operand with at least a Nan
	{ 
		changeResult = true;
	   if(((f1UExp == MAX_EXP & f1Mantissa.slc<S_MTS>(0) != 0) & (f2UExp == MAX_EXP & f2Mantissa.slc<S_MTS>(0) != 0))) // both are Nan
	 	  localResult = CNAN;

	   else if (f1UExp == MAX_EXP & f1Mantissa.slc<S_MTS>(0) != 0) // f1 is a Nan
	   {

	   		localResult = CNAN;
	   }
	 	 else // f1 is not a Nan and f2 is a Nan 
	   	{
	   		localResult = CNAN;
	    }
	    
	if(performFused | performSqrt)
		{
			step++;
			tempValue = localResult;
		}
		
	

	}
	else if(dctoEx.opCode == RISCV_FLOAT_OP & dctoEx.funct7 == RISCV_FLOAT_OP_CMP 
			& ((f1UExp == MAX_EXP & f1Mantissa.slc<S_MTS>(0) != 0) | (f2UExp == MAX_EXP & f2Mantissa.slc<S_MTS>(0) != 0)) )// CMP case we return false all the time if one of the operand is a Nan
		{
			changeResult = true;
			localResult = 0;
		}
	
	else// Normal case
	{
				   if(!state)                                              
					 {
						 switch(dctoEx.opCode)                                                   
						  {                                                                       
							 case RISCV_FLOAT_LW:
							 changeResult = true;
							 	localResult = dctoEx.lhs + dctoEx.rhs;   
								 fprintf(stderr, "Changing the result for a load %x\n", localResult);

							 break; 

						   case RISCV_FLOAT_SW:
						   changeResult = true;
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
										  if(f1UExp != MAX_EXP & f2UExp != MAX_EXP) // operand are not exceptions
										  {										  
											if(!subState)
											{
												state = f1UExp - f2UExp;
												subState = 1;
												stall = true;
												f1Val = (ac_int<2*(S_MTS + 1),false>) f1Mantissa;
												f2Val = (ac_int<2*(S_MTS + 1),false>) f2Mantissa;
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
												
												
												localResult.set_slc((BIT_W -1),outputSign);
													
												if(f1UExp > f2UExp)
													outputExp = f1UExp;
												else
													outputExp = f2UExp;
												

												
												
												for(i = (2*(S_MTS + 1) - 1); i >= 0; i--)  //Find the MSB
													if (resultMantissa[i])
														break;
														

												if (i > S_MTS) 
												{
													localResult.set_slc(0, resultMantissa.slc<S_MTS>(i - S_MTS) );
													outputExp += i - S_MTS;
													roundingFlags[0] = resultMantissa.slc<1>(i - (S_MTS + 1));
													
													if(i == (S_MTS + 1))
														roundingFlags[1] = roundingFlags[0];
													else
													{
													
														for(k = i - (S_MTS + 2); i != 0; i--)
															isHalf = isHalf & resultMantissa[k];
														
														roundingFlags[1] = isHalf;
													}
												}
												else 
												{
													if(outputExp > S_MTS - i)
													{
															outputExp -=S_MTS - i;
															resultMantissa = resultMantissa << (S_MTS - i);
															localResult.set_slc(0,resultMantissa.slc<S_MTS>(0));
													}
													else
													{
															localResult.set_slc(0,resultMantissa.slc<S_MTS>(0));
															outputExp = 0;
													}
													roundingFlags = 0;
												}
																								
												localResult.set_slc(S_MTS,outputExp.slc<S_EXP>(0));
												
												
												if(outputExp.slc<S_EXP>(0) > (MAX_EXP - 1)) // over and underflow handeling with return of infty
												{
													OVERF = 1;		
													if(f1Sign != 0)
														{
														localResult = INFN;
														}
													else
														{
														localResult = INFP;
														}    
												}                     
												
												if(performSqrt | performFused)
												{
													tempValue = localResult;
													step++;
													stall = true;
												}
												
												changeResult = true;
											}
										  }
										  else // one of the operand is an exception
										  {
										  	changeResult = true;
										  	if (f1UExp == MAX_EXP)
										  	{
										  		if (f2UExp == MAX_EXP)
										  		{
										  			if(f1Sign == f2Sign)
										  				localResult = dctoEx.rhs; // infty + infty = infty if they have the same sign
										  			else
										  				localResult = NAN; // + infty - infty = Nan 
										  		}
										  		else
										  			localResult = dctoEx.lhs; // infty + float = infty 
										  	}
										  	else 
										  		localResult = dctoEx.rhs; // infty + float = infty 
																																						
				

												if(performSqrt | performFused)
												{
													tempValue = localResult;
													step++;
													stall = true;
												}
									}  
															
										      break;                                                  
										                                                              
										                
									  case RISCV_FLOAT_OP_DIV  : 

									  	if(f1UExp != MAX_EXP & f2UExp != MAX_EXP) // float are not exceptions
										{
											  	if(dctoEx.lhs != 0)
											  	{
													 state = 2*(S_MTS + 1);
													 quotient = 0;
													 remainder = 0;
													 stall = true;
													 f1Val.set_slc((S_MTS + 1),f1Mantissa);
													 f2Val.set_slc(0,f2Mantissa);
 											  	}
												else
												{
													DIV0 = 1;
													localResult = INFP;
													localResult[(BIT_W -1)] = f2Sign;
												}
										}
										  else // one of the operand is an exception
										  {
										  	if (f1UExp == MAX_EXP)
										  	{
										  		if (f2UExp == MAX_EXP) // infty / infty -> NaN
										  		{
										  			localResult = CNAN;
										  		}
										  		else // infty / float -> infty
										  		localResult = dctoEx.rhs;
										  			
										  	} // else float / infty -> 0 
										  	
											if(performSqrt)
											{
												step++;	
												tempValue = localResult;
												stall = true;
											}
											
										  }  
													  break;                                                  
										                                                              
									  case RISCV_FLOAT_OP_MUL  :  
									  	if(f1UExp != MAX_EXP & f2UExp != MAX_EXP) // float are not exceptions
										{                                    
												outputSign = f1Sign ^ f2Sign;                 
												outputMantissa = f1Mantissa * f2Mantissa;
												outputExp = f1Exp + f2Exp - BIAS - (S_MTS + 2);

						 						if(f1UExp + f2UExp >= MAX_EXP + BIAS) // overflow 
						 						{
						 							if(outputSign != 0)
													{
													localResult = INFN;
													OVERF = 1;		
													}
												else
													{
													OVERF = 1;		
													localResult = INFP;
													}
						 						}
						 						else 
						 						{
							 						if(f1UExp + f2UExp >= BIAS)
							 						{
								 						
								 						for(i = (2*(S_MTS + 1) - 1); i >= 0; i--)  //Find the MSB
															if (outputMantissa[i])
																break;
															
														if (i > S_MTS) 
														{
															localResult.set_slc(0, outputMantissa.slc<S_MTS>(i - S_MTS) );
															outputExp += i - S_MTS;
															roundingFlags[0] = outputMantissa.slc<S_MTS>(i - (S_MTS + 1));
															
															if(i == (S_MTS + 1))
																roundingFlags[1] = roundingFlags[0];
															else
															{
															
																for(k = i - (S_MTS + 2); i != 0; i--)
																	isHalf = isHalf & outputMantissa[k];
																
																roundingFlags[1] = isHalf;
															}
														}
														else 
														{
															if(outputExp > S_MTS - i)
															{
																	outputExp -=S_MTS - i;
																	outputMantissa = outputMantissa << (S_MTS - i);
																	localResult.set_slc(0,outputMantissa.slc<S_MTS>(0));
															}
															else
															{
																	localResult.set_slc(0,outputMantissa.slc<S_MTS>(0));
																	outputExp = 0;
															}
															roundingFlags = 0;
														}
								 					}
								 					else if (f1UExp + f2UExp >= BIAS - S_MTS)
								 					{
								 						outputExp = 0;
								 						
								 						outputMantissa = outputMantissa >> (BIAS + (S_MTS + 1) - (f1UExp + f2UExp) );
								 						localResult.set_slc(0, outputMantissa.slc<S_MTS>(0));
								 					}
								 					else
								 						{
								 						localResult = 0;	
								 						outputExp = 0;
								 						outputSign = 0;
								 						}

								  				localResult.set_slc(S_MTS, outputExp.slc<S_EXP>(0));
								  				localResult.set_slc((BIT_W -1), outputSign);

											}
						 					
											if(performFused)
											{
												tempValue = localResult;
												step++;
												stall = true;
											}
												changeResult = true;
												
						 				}
										  else // one of the operand is an exception
										  {
										  	changeResult = true;
										  	if (f1UExp == MAX_EXP)
										  	{
										  		if (f2UExp == MAX_EXP)
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
											  			localResult = INFP;
											  			localResult.set_slc((BIT_W -1), (ac_int<1,false>) f1Sign ^ f2Sign); 
										  			}
										  			else // infty * 0 
										  				localResult = CNAN; 
										  		}
										  	}
										  	else 
										  	{				          	
										  		if(dctoEx.rhs != 0) // infty * float 
										  			{
											  			localResult = INFP;
											  			localResult.set_slc((BIT_W -1), (ac_int<1,false>) f1Sign ^ f2Sign); 
										  			}
										  			else // infty * 0 
										  				localResult = CNAN; 
										  	}	
										  	
										  	if(performFused)
											{
												tempValue = localResult;
												step++;
												stall = true;
											}
										  }  
										  	
										      break; 
										     											                                                             
								      	                                 
								                                   
									  case RISCV_FLOAT_OP_SGN  :
									  	changeResult = true;
									  	localResult = dctoEx.lhs;
										switch(dctoEx.funct3)
										{

											case 0 :
												localResult[(BIT_W -1)] = (bool) f2Sign; 
											break;

											case 1: 
												localResult[(BIT_W -1)] = !( (bool) f2Sign);
											break;
									
											case 2:
												localResult[(BIT_W -1)] = ((bool) f1Sign) ^ ((bool) f2Sign);
											break;
										}                                      
										      break;                                                  
										                                                              
									  case RISCV_FLOAT_OP_MINMAX :  
									  	changeResult = true;
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
									  changeResult = true;
									 	if( f1Exp > 0) // the float is normal and superior to 1 so it may be interesting to compute his value
										{	
											if(dctoEx.rs2 == 0)
												localResult =  (int) f1Mantissa;
											else
												localResult =(unsigned int) f1Mantissa;
			
									  
									  		state = S_MTS - ((int) f1Exp) ;
	
											if (state >= 0)
											{
												if (state >= BIT_W)
													localResult = 0;
												else
													localResult = localResult >> state ; 
											}
											else
											{
												if (state <= -BIT_W)
													localResult = 0;
												else
													localResult = localResult << (- state);
											}
										

																	
										}
									  	else // the float is subnormal --> we return 0
									  	{
									  		localResult = (ac_int<BIT_W,false>) 0;
													  	
									  	}

														
										if(f1Sign != 0)
											localResult = - localResult; 

									  		
									  	
								break;                                                  
										                                                              
									  case RISCV_FLOAT_OP_CMP  :
									  localResult = 1;
									   changeResult = true;

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
													

													case 2:  //FEQ
														localResult[0] = dctoEx.rhs == dctoEx.lhs;
													break;
												}                                    
										      break;                                                  
										                                                              
									  case RISCV_FLOAT_OP_CVTSW :  
									  changeResult = true;
											if(dctoEx.rs2 != 0 ) // FCVT.S.WU
											{
												for(i = (BIT_W -1); i >= 0; i--)
													if (dctoEx.lhs[i])
															break;
															
												localResult.set_slc(S_MTS, (ac_int<S_EXP,false>) (BIAS + i));
												localResult.set_slc(0, (dctoEx.lhs << ((BIT_W -1) - i)).slc<S_MTS>(S_EXP));
												roundingFlags[0] = (dctoEx.lhs << ((BIT_W -1) - i)).slc<S_MTS>(7);
												roundingFlags[1] = (dctoEx.lhs << ((BIT_W -1) - i)).slc<7>(0) == 0x40;
											}
											else // FCVT.S.W
											{
												if(dctoEx.lhs[(BIT_W -1)]) // the number is negative -> 2's complement
												{
													for(i = 30; i >= 0; i--)
														if (!dctoEx.lhs[i])
																break;
																	
													localResult.set_slc(S_MTS, (ac_int<S_EXP,false>) (BIAS + i-1));
													localResult.set_slc(0, (dctoEx.rhs << ((BIT_W -1) - i+1)).slc<S_MTS>(S_EXP) );
													roundingFlags[0] = (dctoEx.rhs << ((BIT_W -1) - i+1)).slc<S_MTS>(7);
													roundingFlags[1] = (dctoEx.rhs << ((BIT_W -1) - i+1)).slc<7>(0) == 0x40;
												}
												else 
												{
													for(i = 30; i >= 0; i--)
														if (dctoEx.lhs[i])
															break;
															
													localResult.set_slc(S_MTS, (ac_int<S_EXP,false>) (BIAS + i));
													localResult.set_slc(0, (dctoEx.lhs << ((BIT_W -1) - i)).slc<S_MTS>(S_EXP) );
													roundingFlags[0] = (dctoEx.lhs << ((BIT_W -1) - i)).slc<S_MTS>(7);
													roundingFlags[1] = (dctoEx.lhs << ((BIT_W -1) - i)).slc<7>(0) == 0x40;
												}
											}
							  				  
									      break;                                                  
									                                                              
									  case RISCV_FLOAT_OP_MVWX :   
										  changeResult = true;                                   
										  localResult = dctoEx.lhs;
										      break;                                                  
										                                                              
									  case RISCV_FLOAT_OP_CLASSMVXW :    
										  changeResult = true;                             
										  if (!dctoEx.funct3) // funct3 = 0 -> FMV.X.W
										  {
											localResult = dctoEx.lhs;
										  }
										  else  // FCLASS.S
										  {
										  	#ifndef __CATAPULT
											printf("FCLASS not implemented yet\n");
											#endif
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
								quotient = quotient << 1;
								if(remainder >= f2Val)
								{
									remainder = remainder - f2Val;
									quotient[0] = 1;
								}
							}
										 
							if(!state)
							{
								outputSign = f1Sign ^ f2Sign;
							  	outputExp = f1Exp - f2Exp + SUB_BIAS;
								
								for(i = (2*(S_MTS + 1) - 1); i >= 0; i--)  //Find the MSB
									if (quotient[i])
										break;
										
								if (i > S_MTS) // the result is normal
								{
									localResult.set_slc(0, quotient.slc<S_MTS>(i - S_MTS) );
									outputExp += i - S_MTS;
									roundingFlags[0] = quotient[i - (S_MTS + 1)];
									
									if(i == (S_MTS + 1))
										roundingFlags[1] = 0;
									else
									{
									
										for( k = i - (S_MTS + 1); k != 0; k--)
											isHalf = isHalf & quotient[k];
										roundingFlags[1] = isHalf;											
									}
								}
								else // the result is a subnormal 
								{
								outputExp = 0;
								localResult.set_slc(0,quotient.slc<S_MTS>(0));
								roundingFlags = 0;
								}

								

					  			localResult.set_slc(S_MTS, outputExp.slc<S_EXP>(0));
					  			localResult.set_slc((BIT_W -1), outputSign);
								stall = false;

					  			if(outputExp.slc<S_EXP>(0) > (MAX_EXP - 1))  // over and underflow handling with return of infty
									if(f1Sign != 0)
									{
										OVERF = 1;		
										localResult = INFN;
									}
									else
									{
										OVERF = 1;		
										localResult = INFP;
									}
							 localResult.set_slc((BIT_W -1), outputSign);
						
							
								if(performSqrt)
								{
									step++;
									tempValue = localResult;
									stall = true;
								}
								
								changeResult = true;
							}
							break;

								
							case RISCV_FLOAT_OP_ADD :
							case RISCV_FLOAT_OP_SUB: 
								stall = true;

								if (state > 0)
								{
									if (state > BIT_W)
										{f2Val = f2Val >> BIT_W; state -=BIT_W;}
									else
										{f2Val = f2Val >> state; state =0;}
								}
								else
								{
									if (state < -BIT_W)
										{f1Val = f1Val >> BIT_W; state +=BIT_W;}
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
   		// Rounding mode
   		switch(RNDM)
   		{
   			case RNE : 
   				if(roundingFlags[0] != 0)
   					iterMantissa(localResult);
   				
   				if(roundingFlags[1] != 0 & localResult[0]) // tie to even, so if we are already even we change nothing, else we take the successor
   					iterMantissa(localResult);
   				break;
   				
   			case RTZ : 
   				break;
   				
   			case RDN :
   				if(localResult[(BIT_W -1)] & roundingFlags[0] != 0)
   						iterMantissa(localResult);
   				break;
   				
   			case RUP : 
   				if(!localResult[(BIT_W -1)] & roundingFlags[0] != 0)
   					iterMantissa(localResult);
   				break; 
   			
   			case RMM : // tie to maximal amplitude so in the tie case we take the successor
				 if(roundingFlags[0] != 0 | roundingFlags[1] != 0) 
   					iterMantissa(localResult);
				break;
   				
   			default : 
   				break;
   		}
   		
   		
		   
   }

   result = localResult;

   return changeResult;

} 

		
};



#endif /* FLOAT_ALU_H */ 
