#include <stdio.h>


void main(){

	unsigned int val1 = 0x30405210;
	unsigned int val2 = 0x98217832;
	unsigned int result = 0;

	asm volatile(".insn r 0x0b, 1, 0, %[o], %[a], %[b]" : [o] "=r" (result) : [a] "r" (val1), [b] "r" (val2));
	asm volatile(".insn r 0x0b, 0, 0, %[o], %[a], %[b]" : [o] "=r" (result) : [a] "r" (val1), [b] "r" (val2));
	asm volatile(".insn r 0x0b, 2, 0, %[o], %[a], %[b]" : [o] "=r" (result) : [a] "r" (val1), [b] "r" (val2));


	printf("result is %x\n", result);
}
