/*
 * This file exists to test the floating point instructions
 *
 * Date : 19/06/2019 
 *
 * Author : Lauric 
 *
 */



#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <ctime>
#include <random>
#include <cmath>
 
 
#include <core.h>


#define MAX(a,b) (a<b)?b:a
#define MIN(a,b) (a<b)?a:b

struct processorState{
	unsigned int regs[64];
	unsigned int pc;
	unsigned int address;
	unsigned int value;
};


std::default_random_engine generator;

// Float random init 
std::uniform_real_distribution<float> distribution(-5000,5000);

// Int random init
std::uniform_int_distribution<int> distributionFunct7(0,11);
std::uniform_int_distribution<int> distributionFunct3(0,2);
std::uniform_int_distribution<int> idistribution(-5000,5000);


// Not use yet
std::uniform_int_distribution<int> mantisse(0,8388607);
std::uniform_int_distribution<int> exposant(0,255);
std::uniform_int_distribution<int> signe(0,1);
 
 
int sgn(float x)
{
	if (x > 0)
		return 1;
	else 
		return -1;
}

void setTest(struct processorState &initialState, struct processorState &finalState,
			 unsigned int &instruction, float &p)
{
	int sa,ma,ea,sb,mb,eb,d;
	float fa,fb,fc;
	float *a,*b,*c;
	int opCode, funct7, funct3;

	opCode = 2; 
	funct7 = distributionFunct3(generator);
	funct3 = distributionFunct3(generator);
	
	
	d = idistribution(generator);
	
	fa = distribution(generator);
	fb = distribution(generator);
	
	fa = fa * sgn(fa);


	

	a = &fa;
	b = &fb;

	initialState.regs[1] = d;
	initialState.regs[32] = *((int*) a);
	initialState.regs[33] = *( (int*) b);
	

	
	finalState.regs[1] = d;
	finalState.regs[32] = *((int*) a);
	finalState.regs[33] = *((int*) b); 

	switch(opCode)
	{
		case 0 : // load 
			instruction = 0x2007;
			break;

		case 1 : // store
			instruction = 0x2017;
			break;	

		case 2 : // op
			switch(funct7)
			{
				case 0 : //Add
					fc = fa + fb;
					c = &fc;
					finalState.regs[34] = *( (int*) c); // correct value is stored
					instruction = 0x100153 ;
					break;
					
				case 1 : //Sub
					fc = fa - fb;
					c = &fc;
					finalState.regs[34] = *( (int*) c); 
					instruction = 0x8100153;
					break;

				case 2 : //Mul
					fc = fa * fb;
					c = &fc;
					finalState.regs[34] = *( (int*) c); 
					instruction = 0x10100153;
					break;

				case 3 : //Div
					fc = fa / fb;
					c = &fc;
					finalState.regs[34] = *( (int*) c); 
					instruction = 0x18100153;
					break;

				case 4 : //SGN
					instruction = 0x20100253;
					switch(funct3)
					{
						case 0 :  // FSGNJ
							fc = fa  * sgn(fa) * sgn(fb);
							break;

						case 1 : //FSGNJN
							fc = fa  * sgn(fa) * (- sgn(fb));
							instruction += 0x1000;
							break;
						
						case 2 : //FSGNJX
							fc = fa * sgn(fb);
							instruction += 2 * 0x1000;
							break;

						c = &fc;
						finalState.regs[34] = *( (int*) c); 

					}
					break;

				case 5 : //MinMax
					instruction = 0x28100153;
					if (funct3)
						{
						fc = MAX(fa,fb);
						instruction += 0x1000;
						}
					else 
						{
						fc = MIN(fa,fb);
						}
					c = &fc;
					finalState.regs[34] = *( (int*) c) ; 
					break;

				case 6 : //Cvt.w.s
					finalState.regs[1] = (int) fa ; 
					instruction = 0xc00000d3;
					break;

				case 7 : //MvClass
					c = &fa;
					finalState.regs[1] = *( (int*) c) ; 
					instruction = 0xe0000153;
					break;

				case 8 : //Cmp 
					instruction = 0xA0100153;
					funct3 = funct3 %2;
					switch(funct3)
					{
						case 0 : // FLE
							finalState.regs[2] = (fa <= fb);
							break;

						case 1 : // FLT
							finalState.regs[2] = (fa < fb);
							instruction +=  0x1000; 
							break;

						case 2 : //FEQ 
							finalState.regs[2] = (fa == fb);
							instruction += 2 * 0x1000;
							break;
					}
					break;

				case 9 : //Cvt.s.w
					fc = (float) d;
					c= &fc;
					finalState.regs[34] = *((int*) c);
					instruction = 0xd0008153;
					break;

				case 10 : //mv.w.x
					finalState.regs[34] = *( (int*) a);
					instruction = 0xf0008153;
					break;
					
				case 11 : //Sqrt
					fc = sqrt(fa);
					c = &fc;
					finalState.regs[34] = *((int*) c);
					instruction = 0x58000153;

			}
			break;
	}
}


int main(int argc, char** argv)
{
		srand(time(NULL));
		float p;
		int a = 0,c = 0;

		unsigned int instruction, numberOfCycles;
		struct processorState initialState, finalState;
		numberOfCycles = 900;
		ac_int<32, false> im[8192], dm[8192];
		Core core;


		core.im = new SimpleMemory(im);
		core.dm = new SimpleMemory(dm);
	while(a<100000){
		a++;
		for(int i =0; i <64; i++)
		{
			initialState.regs[i] = 0;
			finalState.regs[i] = 0;
			core.regFile[i] = 0;	
		}
		setTest(initialState, finalState, instruction,p);


		//We initialize a simulator with the state



		core.pc = 0;
		for (int oneReg = 0; oneReg<64; oneReg++)
			core.regFile[oneReg] = initialState.regs[oneReg];


	

		im[0] = instruction;
		dm[initialState.address>>2] = initialState.value;
		
		core.ftoDC.we = false;

		core.dctoEx.pc = 0;
		core.dctoEx.instruction = 0;

		core.dctoEx.opCode = 0;
		core.dctoEx.funct7 = 0;
		core.dctoEx.funct3 = 0;

		core.dctoEx.lhs = 0;
		core.dctoEx.rhs = 0;
		core.dctoEx.datac = 0;

		//For branch unit
		core.dctoEx.nextPCDC = 0;
		core.dctoEx.isBranch = false;

		//Information for forward/stall unit
		core.dctoEx.useRs1 = false;
		core.dctoEx.useRs2 = false;
		core.dctoEx.useRs3 = false;
		core.dctoEx.useRd = false;
		core.dctoEx.rs1 = 0;
		core.dctoEx.rs2 = 0;
		core.dctoEx.rs3 = 0;
		core.dctoEx.rd = 0;

		//Register for all stages
		core.dctoEx.we = false;

		core.extoMem.pc = 0;
		core.extoMem.instruction = 0;

		core.extoMem.result = 0;
		core.extoMem.rd = 0;
		core.extoMem.useRd = false;
		core.extoMem.isLongInstruction = false;
		core.extoMem.opCode = 0;
		core.extoMem.funct3 = 0;

		core.extoMem.datac = 0;

		//For branch unit
		core.extoMem.nextPC = 0;
		core.extoMem.isBranch = false;

		//Register for all stages
		core.extoMem.we = false;

		core.memtoWB.result = 0;
		core.memtoWB.rd = 0;
		core.memtoWB.useRd = false;

		core.memtoWB.address = 0;
		core.memtoWB.valueToWrite = 0;
		core.memtoWB.byteEnable = 0;
		core.memtoWB.isStore = false;
		core.memtoWB.isLoad = false;

		//Register for all stages
		core.memtoWB.we = false;

		// Execute the instruction
	//	printf("Doing instruction %x\n", instruction);
		for (int oneCycle = 0; oneCycle < numberOfCycles; oneCycle++){
			doCycle(core, 0);
		}


		bool worked = true;

		if ((int)core.regFile[2] != (int)finalState.regs[2] )
			{c++; printf("Issue with instruction : %x at register 2, awnser is %x and should be %x\n", instruction,core.regFile[2], finalState.regs[2]);}


		float diff; 
		
		int val1,val2;
		int *val1_p, *val2_p;
			
		val1 = core.regFile[34];
		val2 = finalState.regs[34];
		
		val1_p = &val1;
		val2_p = &val2;
		
		diff =(float) *( (float*) val1_p) - *( (float*) val2_p);
		
		p = 0.0005*finalState.regs[34];
		
		
		if (diff*sgn(diff) > p )
			{c++;printf("Issue with instruction : %x at register 34, awnser is %x and should be %x\n", instruction,core.regFile[34], finalState.regs[34]);}

		

	}
	printf("error rate = %d / %d\n",c,a);
	
	return c;


}
