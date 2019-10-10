/* Fixed point 16-bit input-output in place FFT

This is a modification of fft_general.c, working only for the 64 point FFT.
- Time decimation swaps are stored, not calculated
- Only required sine lookup values are stored, not all values
- Inputs made volatile to read one from memory per clock cycle 
- Inverse FFT is still possible

To avoid floating point numbers, sin values are stored as integers
with respect to 2^15 = 32768 rather than 1, and the length of the sin wave
(2pi) is 1024. E.g. sin(pi/2) = 1, so the 256'th value in the sin array
(256 corresponds to pi/2) is 32768, which corresponds to 1. 

This scaling also applies to the input. For example if input data is a 
sine wave with amplitude 327 (654 peak-to-peak), this is equivalent
to a sine input with amplitude 0.01. The output is also with respect to
0.01. If input values must exceed +/- 1 i.e. exceed 32768, they must be
first scaled down and then the output scaled up by that factor (given
an input vector x, fft(c*x) = c*fft(x), where c is a scaling constant).


Acknowledgements:
1. Fixed point method (multiply/sine-lookup/butterfly calculation):
   Tom Roberts, Malcolm Slaneyand, Dimitrios P. Bouras
2. Time decimation bit-reversal algorithm:
   Steven W. Smith, http://www.dspguide.com/ch12/2.htm               */


#include <stdio.h>
#include <stdlib.h>

#define Input_Size  64
#define inverse     0

#define abs(a) ( ((a) < 0) ? -(a) : (a) )


void fft(short *val, short *old_val)
{
  short m, i, j, l, sin_index=0, istep, shift,
  	    W_Imag, W_Real, // W = exp(-sqrt(-1)*2*pi/Input)Size
	      qi, qr, ti, tr; // Temporary coefficients


  // Time decimation, swaps are hard-coded, not calculated

	// Note: Each of these corresponds to a load/store. In Legup1.0, the memory
	// controller has 1 port and this takes a lot of clock cycles
	val[2*0+1] = old_val[1+2*0];
	val[2*0] = 0;
	val[2*1+1] = old_val[1+2*32];
	val[2*1] = 0;
	val[2*2+1] = old_val[1+2*16];
	val[2*2] = 0;
	val[2*3+1] = old_val[1+2*48];
	val[2*3] = 0;
	val[2*4+1] = old_val[1+2*8];
	val[2*4] = 0;
	val[2*5+1] = old_val[1+2*40];
	val[2*5] = 0;
	val[2*6+1] = old_val[1+2*24];
	val[2*6] = 0;
	val[2*7+1] = old_val[1+2*56];
	val[2*7] = 0;
	val[2*8+1] = old_val[1+2*4];
	val[2*8] = 0;
	val[2*9+1] = old_val[1+2*36];
	val[2*9] = 0;
	val[2*10+1] = old_val[1+2*20];
	val[2*10] = 0;
	val[2*11+1] = old_val[1+2*52];
	val[2*11] = 0;
	val[2*12+1] = old_val[1+2*12];
	val[2*12] = 0;
	val[2*13+1] = old_val[1+2*44];
	val[2*13] = 0;
	val[2*14+1] = old_val[1+2*28];
	val[2*14] = 0;
	val[2*15+1] = old_val[1+2*60];
	val[2*15] = 0;
	val[2*16+1] = old_val[1+2*2];
	val[2*16] = 0;
	val[2*17+1] = old_val[1+2*34];
	val[2*17] = 0;
	val[2*18+1] = old_val[1+2*18];
	val[2*18] = 0;
	val[2*19+1] = old_val[1+2*50];
	val[2*19] = 0;
	val[2*20+1] = old_val[1+2*10];
	val[2*20] = 0;
	val[2*21+1] = old_val[1+2*42];
	val[2*21] = 0;
	val[2*22+1] = old_val[1+2*26];
	val[2*22] = 0;
	val[2*23+1] = old_val[1+2*58];
	val[2*23] = 0;
	val[2*24+1] = old_val[1+2*6];
	val[2*24] = 0;
	val[2*25+1] = old_val[1+2*38];
	val[2*25] = 0;
	val[2*26+1] = old_val[1+2*22];
	val[2*26] = 0;
	val[2*27+1] = old_val[1+2*54];
	val[2*27] = 0;
	val[2*28+1] = old_val[1+2*14];
	val[2*28] = 0;
	val[2*29+1] = old_val[1+2*46];
	val[2*29] = 0;
	val[2*30+1] = old_val[1+2*30];
	val[2*30] = 0;
	val[2*31+1] = old_val[1+2*62];
	val[2*31] = 0;
	val[2*32+1] = old_val[1+2*1];
	val[2*32] = 0;
	val[2*33+1] = old_val[1+2*33];
	val[2*33] = 0;
	val[2*34+1] = old_val[1+2*17];
	val[2*34] = 0;
	val[2*35+1] = old_val[1+2*49];
	val[2*35] = 0;
	val[2*36+1] = old_val[1+2*9];
	val[2*36] = 0;
	val[2*37+1] = old_val[1+2*41];
	val[2*37] = 0;
	val[2*38+1] = old_val[1+2*25];
	val[2*38] = 0;
	val[2*39+1] = old_val[1+2*57];
	val[2*39] = 0;
	val[2*40+1] = old_val[1+2*5];
	val[2*40] = 0;
	val[2*41+1] = old_val[1+2*37];
	val[2*41] = 0;
	val[2*42+1] = old_val[1+2*21];
	val[2*42] = 0;
	val[2*43+1] = old_val[1+2*53];
	val[2*43] = 0;
	val[2*44+1] = old_val[1+2*13];
	val[2*44] = 0;
	val[2*45+1] = old_val[1+2*45];
	val[2*45] = 0;
	val[2*46+1] = old_val[1+2*29];
	val[2*46] = 0;
	val[2*47+1] = old_val[1+2*61];
	val[2*47] = 0;
	val[2*48+1] = old_val[1+2*3];
	val[2*48] = 0;
	val[2*49+1] = old_val[1+2*35];
	val[2*49] = 0;
	val[2*50+1] = old_val[1+2*19];
	val[2*50] = 0;
	val[2*51+1] = old_val[1+2*51];
	val[2*51] = 0;
	val[2*52+1] = old_val[1+2*11];
	val[2*52] = 0;
	val[2*53+1] = old_val[1+2*43];
	val[2*53] = 0;
	val[2*54+1] = old_val[1+2*27];
	val[2*54] = 0;
	val[2*55+1] = old_val[1+2*59];
	val[2*55] = 0;
	val[2*56+1] = old_val[1+2*7];
	val[2*56] = 0;
	val[2*57+1] = old_val[1+2*39];
	val[2*57] = 0;
	val[2*58+1] = old_val[1+2*23];
	val[2*58] = 0;
	val[2*59+1] = old_val[1+2*55];
	val[2*59] = 0;
	val[2*60+1] = old_val[1+2*15];
	val[2*60] = 0;
	val[2*61+1] = old_val[1+2*47];
	val[2*61] = 0;
	val[2*62+1] = old_val[1+2*31];
	val[2*62] = 0;
	val[2*63+1] = old_val[1+2*63];
	val[2*63] = 0;

  /* Butterfly computation. See Summaryp.pdf. For N =  64 point 
     FFT, there are log2(N) = 6 stages of butterflies, with N/2 = 32 butterflies
     per stage. In stage 1, every entry is butterflied with its neighbor:
     (the function B() denotes a butterfly computation)

     Stage 1: B(0,1), B(2,3) ... B(62,63).

     The results are  stored in-place (a butterfly is a 2-input 2-output operation).
     In stage 2, the butterfly is performed with every other entry:
     
     Stage 2: B(0,2), B(1,3), B(4,6), B(5,7) ... B(60,62), B(62,63).

     In the third stage, this pattern repeats but skips 4: 
     
     Stage 3: B(0,4), B(1,5), B(2,6), B(3,7),
                  B(8,12), B(9,13), B(10,14), B(11,15) ...

     The pattern repeats skipping 8, 16 and 32. This is accomplished using 2 nested for
     loops. The inner for loop is the actual butterfly computation, while the outer 
     creates the correct indices. The outermost  while loop counts the number of passes 
     (hence executes 6x in the 64 point example). 

NOTE: In hardware, all 32 butterflies should be in parallel during each stage. 
Currently this is not done by LegUp, as it is unclear that these 
loops can be unrolled. But the required computations do not depend on input values hence 
do not change for any 64-point FFT. Therefore, this set of three loops can be manually 
unrolled to either create six sets of nested for loops (eliminating the while loop) or 
to completely remove the for loops as well.

Therefore while in software using for loops is convenient, the loops can be unrolled 
manually in order to guide LegUp. */

	int *valCmplx = (int*) val;

  l = 1;
  while (l < Input_Size) // Executes log2(N) times
  {
    // Everything within the while loop should be in parellel (no dependence)
    if (inverse)
      shift = Input_Size-1; // We must scale down the inverse FFT by 1/N (by its definition)
    else shift = 0;
		
    istep = l << 1;    
    for (m=0; m<l; m++) {
    //  W_Imag =  sin_lookup[sin_index];
    //  W_Real = sin_lookup[sin_index+1];    
			sin_index = sin_index+2;
			

      for (i=m; i<Input_Size; i+=istep) {	// inside this loop is the butterfly
        j = i + l;
				int o1, o2, o3, o4;
				int p1, p2, p3, p4;
				int a1, a2, a3, a4;
				int b1, b2, b3, b4;


				a1 = valCmplx[i+l];
				b1 = valCmplx[i];


				asm volatile(".insn r 0x0b, 0, 0, %[o], %[a], %[b]" : [o] "=r" (o1) : [a] "r" (a1), [b] "r" (b1));
				asm volatile(".insn r 0x0b, 1, 0, %[o], zero, zero" : [o] "=r" (p1) :);

				valCmplx[(i+l)] = o1;
				valCmplx[i] = p1;



			}
    }
    l = istep;
  }
}



int main()
{
  short i, val[2*Input_Size], old_val[2*Input_Size];
	volatile short input_temp;
  int sum = 0;
  
  // Set input, each input value consists of a Real and imaginary part
	for (i=0; i<Input_Size; i++){
		input_temp = 10*i;
    old_val[2*i] = 0;
    old_val[2*i+1] = input_temp;
  }

  fft(val, old_val);

  for (i=0; i<Input_Size; i++) {
    sum += abs(val[2*i]);
    sum += abs(val[2*i+1]);
  }
/*
  printf ("Result: %d\n", sum);
  if (sum == 87100) {
      printf("RESULT: PASS\n");
  } else {
      printf("RESULT: FAIL\n");
  }*/
  
  return sum; 
}

