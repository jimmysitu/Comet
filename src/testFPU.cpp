/*
 * This file exists to test the floating point instructions
 *
 * Date : 15/07/2019
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
#include <cfenv>

#include <core.h>



#define MAX(a,b) (a<b)?b:a
#define MIN(a,b) (a<b)?a:b
#define CNAN 0x7fc00000
#define INFP 0x7f800000
#define INFN 0xff800000

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
std::uniform_int_distribution<int> distributionOpCode(0,6);


std::uniform_int_distribution<int> mantisse(0,16777215);
std::uniform_int_distribution<int> exposant(0,255);
std::uniform_int_distribution<int> signe(0,1);

std::uniform_int_distribution<int> floatType(0,3);

int sgn(float x)
{
	if (x > 0)
		return 1;
	else
		return -1;
}

int setFloat()
{
	int fType = floatType(generator), result = 0, fSign = 0, fExp = 0,fMantissa = 0;
	switch(fType)
	{
		case 0 : // normal float
			 fExp = exposant(generator);
		case 1 : // subnormal  float
			fSign = signe(generator);
			fMantissa = mantisse(generator);

			return (fSign << 31) + (fExp << 23) + fMantissa;
			break;

		case 2 : // infinite
			fSign = signe(generator);

			if(fSign)
				return INFN;
			else
				return INFP;

			break;

		case 3 : // Nan
			fSign = signe(generator);

			if(fSign)
				return CNAN;
			else
				return 0x7fa0000;
		 	break;

	}
	return CNAN;
}

void setTest(struct processorState &initialState, struct processorState &finalState,
			 unsigned int &instruction, int opCode, int funct7, int funct3, int *a, int *b, int *z)
{
	float fa,fb,fc,fz;
	float *c;
	int d;

	d = idistribution(generator);

	initialState.regs[1] = d;
	initialState.regs[32] = *a;
	initialState.regs[33] = *b;
	initialState.regs[35] = *z;


	finalState.regs[1] = d;
	finalState.regs[32] = *a;
	finalState.regs[33] = *b;
	finalState.regs[35] = *z;

	fa = *( (float*) a);
	fb = *( (float*) b);
	fz = *( (float*) z);

	//printf("fa = %x , fb = %x , fz = %x\n", *a, *b, *z);

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
					instruction = 0x100153;
					break;

				case 1 : //Sub
					fc = fa - fb;
					c = &fc;
					finalState.regs[34] = *( (int*) c);
					instruction = 0x8100153;
					break;

				case 2 : //Mul  // Rounding problems
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
					finalState.regs[2] = *a;

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
					finalState.regs[34] = d;
					instruction = 0xf0008153;
					break;

				case 11 : //Sqrt
					fc = sqrt(fa);
					c = &fc;
					finalState.regs[34] = *((int*) c);
					instruction = 0x58000153;

			}
			break;

		case 3 : // FMADD
			fc = fa * fb + fz;
			c = &fc;
			finalState.regs[34] = *((int*) c);
			instruction = 0x18100143;
			break ;

		case 4 : //FMSUB
			fc = fa * fb - fz;
			c = &fc;
			finalState.regs[34] = *((int*) c);
			instruction = 0x18100147;
			break;

		case 5 : //FNMADD
			fc =  - fa * fb + fz;
			c = &fc;
			finalState.regs[34] = *((int*) c);
			instruction = 0x1810014F ;
			break;

		case 6 : //FNMSUB
			fc = - fa * fb - fz;
			c = &fc;
			finalState.regs[34] = *((int*) c);
			instruction = 0x1810014B;
			break;
	}



}

int Test(int opCode, int funct7, int funct3, int *a, int *b, int *z)
{
		srand(time(NULL));
		float p;


		unsigned int instruction, numberOfCycles;
		struct processorState initialState, finalState;
		numberOfCycles = 1000;
		ac_int<32, false> im[8192], dm[8192];
		Core core;

		core.im = new SimpleMemory(im);
		core.dm = new SimpleMemory(dm);


		for(int i =0; i <64; i++)
		{
			initialState.regs[i] = 0;
			finalState.regs[i] = 0;
			core.regFile[i] = 0;
		}


		setTest(initialState, finalState, instruction, opCode, funct7, funct3, a, b , z);
		int c = 0;

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
			//printf("oneCycle = %d\n", oneCycle);
		}


		bool worked = true;

		if ((int)core.regFile[2] != (int)finalState.regs[2] )
			{c++; printf("Issue with instruction : %x at register 2, awnser is %x and should be %x\n", instruction, (unsigned int) core.regFile[2], finalState.regs[2]);}


		float diff;

		int val1,val2;
		int *val1_p, *val2_p;

		val1 = core.regFile[34];
		val2 = finalState.regs[34];

		val1_p = &val1;
		val2_p = &val2;

		diff =(float) *( (float*) val1_p) - *( (float*) val2_p);
		diff *= sgn(diff);


		if (!( (diff == 0) | (val1 ^ val2 < 2) ))
			{c++;printf("Issue with instruction : %x at register 34, awnser is %x and should be %x\n", instruction, (unsigned int) core.regFile[34], finalState.regs[34]);}
	return c;


}



int main(int argc, char** argv)
{
	int a_c = 0,c = 0, d= 0, opCode, funct7, funct3;
	int *a, *b, *z;
	a = new int;
	b = new int;
	z = new int;
	while(d<10000)
	{
		d++;

		*a = setFloat();
		*b = setFloat();
		*z = setFloat();


		opCode = 2;
		funct7 = 7;
		funct3 = 0;

		for(opCode = 0; opCode < 7; opCode++)
			for(funct7 = 0; funct7 < 12; funct7++)
				for(funct3 = 0; funct3 < 3 ; funct3++)
					{
						a_c++;
						c += Test(opCode, funct7, funct3, a, b, z);
					}
	}

	printf("error rate = %d / %d\n",c,a_c);

	delete a;
	delete b;
	delete z;

	return c;
}
