#ifndef __MEMORY_H__
#define __MEMORY_H__

#define STRINGIFY(a) #a

#ifdef __SYNTHESIS__
#define __HLS__
#elif CCS_DUT_RTL || CCS_DUT_SYSC   // modelsim defines
#define __HLS__
#endif

#define DRAM_WIDTH  32
#ifndef __HLS__
#define DRAM_SIZE   ((size_t)1 << 26)
#else
#define DRAM_SIZE   8192
#endif

#define STACK_INIT  (DRAM_SIZE-0x1000)


#endif /* __MEMORY_H__ */
