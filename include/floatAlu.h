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
	ac_int<48, false> quotient = 0;
	ac_int<48,false> remainder = 0;
	ac_int<48,false> f1Val = 0;
	ac_int<48,false> f2Val = 0;
	ac_int<24,false> tmp = 0;
	
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

      float f1;
	  int g;
	  


      stall =false;       

					f1Mantissa = dctoEx.rhs.slc<23>(0);
					f2Mantissa = dctoEx.lhs.slc<23>(0);
					f1Mantissa[23] = 1;
					f2Mantissa[23] = 1;

					f1Exp = dctoEx.rhs.slc<8>(23) - 127;
					f2Exp = dctoEx.lhs.slc<8>(23) - 127;
					
					f1Sign = dctoEx.rhs.slc<1>(31);
					f2Sign = dctoEx.lhs.slc<1>(31);
   if(!state)                                              
     {switch(dctoEx.opCode)                                                   
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
                  if(f1Exp > f2Exp)
					{
						while(f1Exp != f2Exp)
							{
								f2Exp++;
								f2Mantissa = f2Mantissa >> 1;
							}
							
					}
					else
					{
						while(f1Exp != f2Exp)
							{
								f1Exp++;
								f1Mantissa = f1Mantissa >> 1;
							}
					}

					if(f1Sign == f2Sign)
						extoMem.result.set_slc(31, f1Sign);
					else
					{
						if(f1Mantissa > f2Mantissa)
							extoMem.result.set_slc(31,f1Sign);
						else
							extoMem.result.set_slc(31,f2Sign);
					}

							
					resultMantissa = f1Mantissa + f2Mantissa;
					outputExp = f1Exp +128;

					extoMem.result.set_slc(0, resultMantissa.slc<23>(1));
					extoMem.result.set_slc(23, outputExp);                                      
									
                          break;                                                  
                                                                                  
                  case RISCV_FLOAT_OP_SUB  : 
                    f2Sign = f2Sign ^ f2Sign;	
                    if(f1Exp > f2Exp)
					{
						while(f1Exp != f2Exp)
							{
								f2Exp++;
								f2Mantissa = f2Mantissa >> 1;
							}
							
					}
					else
					{
						while(f1Exp != f2Exp)
							{
								f1Exp++;
								f1Mantissa = f1Mantissa >> 1;
							}
					}

					if(f1Sign == f2Sign)
						extoMem.result.set_slc(31, f1Sign);
					else
					{
						if(f1Mantissa > f2Mantissa)
							extoMem.result.set_slc(31,f1Sign);
						else
							extoMem.result.set_slc(31,f2Sign);
					}

							
					resultMantissa = f1Mantissa + f2Mantissa;
					outputExp = f1Exp +128;

					extoMem.result.set_slc(0, resultMantissa.slc<23>(1));
					extoMem.result.set_slc(23, outputExp);                                      
						                                    
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

						extoMem.result.set_slc(31, outputSign); 
 						extoMem.result.set_slc(0, resultMantissa);
 						extoMem.result.set_slc(23, outputExp.slc<8>(0));

                          break;                                                  
                                                                                  
                  case RISCV_FLOAT_OP_SQRT :                                      
                          break;                                                  
                                                                                  
                  case RISCV_FLOAT_OP_SGN  :
					switch(dctoEx.funct3)
					{

						case 0 :
							extoMem.result[31] = (bool) f2Sign; 
						break;

						case 1: 
							extoMem.result[31] = !( (bool) f2Sign);
						break;
				
						case 2:
							extoMem.result[31] = ((bool) f1Sign) ^ ((bool) f2Sign);
						break;
					}                                      
                          break;                                                  
                                                                                  
                  case RISCV_FLOAT_OP_MINMAX :  
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
					state = ((int) f1Exp) - 23;
					stall = true; 
					if(!dctoEx.rs2)
						{
							
							extoMem.result =  (int) f1Mantissa;
						}
					else
						{
							extoMem.result =(unsigned int) f1Mantissa;
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
								if(dctoEx.rs2) // FCVT.WU.S
								{
									for(int i = 31; i >= 0; i--)
										if (dctoEx.rhs[i])
											{
												extoMem.result.set_slc(23, (ac_int<8,false>) (127 + i));
												extoMem.result.set_slc(0, (dctoEx.rhs << (31 - i)).slc<23>(8) );
											}
								}
								else // FCVT.W.S
								{
									if(dctoEx.rhs[31]) // the number is negative -> 2's complement
									{
										for(int i = 30; i >= 0; i--)
											if (!dctoEx.rhs[i])
												{
													extoMem.result.set_slc(23, (ac_int<8,false>) (127 + i-1));
													extoMem.result.set_slc(0, (dctoEx.rhs << (31 - i+1)).slc<23>(8) );
												}
									
									}
									else 
									{
										for(int i = 31; i >= 0; i--)
											if (dctoEx.rhs[i])
												{
													extoMem.result.set_slc(23, (ac_int<8,false>) (127 + i));
													extoMem.result.set_slc(0, (dctoEx.rhs << (31 - i)).slc<23>(8) );
												}
									}
								}
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
  			extoMem.result.set_slc(0, quotient.slc<23>(1)); 
  			extoMem.result.set_slc(23, outputExp.slc<8>(0));
  			extoMem.result.set_slc(31, outputSign);
	
		}
		break;
		
		case RISCV_FLOAT_OP_CVTWS : 
			if (state > 0)
			{
				if (state >= 32)
					{extoMem.result = extoMem.result << 32; state -= 32;}
				else
					{extoMem.result = extoMem.result << (state % 32); state = 0;}
			}
			else
			{
				if (state <= -32)
					{extoMem.result = extoMem.result >> 32; state += 32;}
				else
					{extoMem.result = extoMem.result >> (32 - (state % 32)); state = 0;}
			
			}
			
			if (!state)
			{
				stall = false;
				if(!dctoEx.rs2 && f1Exp)
					extoMem.result = - extoMem.result; 
			}
			break;
			
	
	
	}
   }                                                        
  }  

		
};



#endif /* FLOAT_ALU_H */ 
