#ifndef TOOLS_H_
#define TOOLS_H_

#define STRINGIFY(a) #a


#ifdef __VIVADO__
#include <ap_int.h>
#define HLS_UINT(param)     ap_uint<param>
#define HLS_INT(param)      ap_int<param>
#define SLC(size, low)      range(((size) + (low) -1), (low))
#define RANGE(msb, lsb)     range((msb), (lsb))
#define SET_SLC(low, value) range(((low) + ((value).length())-1),(low)) = (value)

#define HLS_PIPELINE(param) _Pragma(STRINGIFY(HLS PIPELINE II=param))
#define HLS_UNROLL(param)   _Pragma(STRINGIFY(HLS UNROLL factor=param))
#define HLS_TOP(param)      _Pragma(STRINGIFY(HLS top name=param))
#define HLS_DESIGN
#endif //__VIVADO__

#ifdef __CATAPULT__
#include <ac_int.h>
#define HLS_UINT(param)     ac_int<param, false>
#define HLS_INT(param)      ac_int<param, true>
#define SLC(size, low)      slc<(size)>(low)
#define RANGE(msb, lsb)     range<(msb), (lsb)>()
#define SET_SLC(low, value) set_slc((low), (value))

#define HLS_PIPELINE(param) _Pragma(STRINGIFY(hls_pipeline_init_interval param))
#define HLS_UNROLL(param)   _Pragma(STRINGIFY(hls_unroll param))
#define HLS_TOP(param)      _Pragma(STRINGIFY(hls_design top))
#define HLS_DESIGN          _Pragma(STRINGIFY(hls_design))
#endif //__CATAPULT__

#ifndef __HLS__
#include <cstdio>
#include <stdint.h>

#ifdef __DEBUG__
#define gdebug(...)     printf(__VA_ARGS__)     // generic debug, can be deactivated
#else
#define gdebug(...)
#endif
#define simul(...)      __VA_ARGS__

#ifdef __DEBUG__
#define coredebug(...)  printf(__VA_ARGS__)     // mandatory debug
#else
#define coredebug(...)
#endif

#ifdef __DEBUG__
#define dbglog(...) do { \
    fprintf(stderr, __VA_ARGS__); \
    printf(__VA_ARGS__); \
} while(0)
#else
#define dbglog(...)
#endif

#define dbgassert(cond, ...) do { \
    if(!(cond)) {  \
        fprintf(stderr, __VA_ARGS__); \
        printf(__VA_ARGS__); \
        assert(cond); \
    } \
} while(0)

#else
#define gdebug(...)
#define simul(...)
#undef assert
#define assert(a)
#define coredebug(...)
#define dbgassert(cond, ...)
#define dbglog(...)
#endif

#endif /* For TOOLS_H_ */

