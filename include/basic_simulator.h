#ifndef __BASIC_SIMULATOR_H__
#define __BASIC_SIMULATOR_H__

#include "tools.h"
#include "simulator.h"
#include <map>
#include <vector>

class BasicSimulator : public Simulator {
  unsigned heapAddress;

  HLS_UINT(32)*im, *dm;

  FILE* inputFile;
  FILE* outputFile;
  FILE* traceFile;

public:
  BasicSimulator(const char* binaryFile, std::vector<std::string>, const char* inFile, const char* outFile,
                 const char* tFile);
  ~BasicSimulator();

protected:
  void printCycle();
  void printEnd(){};
  void extend(){};

  // Functions for memory accesses
  void stb(HLS_UINT(32) addr, HLS_INT(8) value);
  void sth(HLS_UINT(32) addr, HLS_INT(16) value);
  void stw(HLS_UINT(32) addr, HLS_INT(32) value);
  void std(HLS_UINT(32) addr, HLS_INT(64) value);

  HLS_INT(8) ldb(HLS_UINT(32) addr);
  HLS_INT(16) ldh(HLS_UINT(32) addr);
  HLS_INT(32) ldw(HLS_UINT(32) addr);
  HLS_INT(32) ldd(HLS_UINT(32) addr);

  // Functions for solving syscalls
  void solveSyscall();

  HLS_INT(32) doRead(HLS_UINT(32) file, HLS_UINT(32) bufferAddr, HLS_UINT(32) size);
  HLS_INT(32) doWrite(HLS_UINT(32) file, HLS_UINT(32) bufferAddr, HLS_UINT(32) size);
  HLS_INT(32) doOpen(HLS_UINT(32) name, HLS_UINT(32) flags, HLS_UINT(32) mode);
  HLS_INT(32) doOpenat(HLS_UINT(32) dir, HLS_UINT(32) name, HLS_UINT(32) flags,
                            HLS_UINT(32) mode);
  HLS_INT(32) doLseek(HLS_UINT(32) file, HLS_UINT(32) ptr, HLS_UINT(32) dir);
  HLS_INT(32) doClose(HLS_UINT(32) file);
  HLS_INT(32) doStat(HLS_UINT(32) filename, HLS_UINT(32) ptr);
  HLS_INT(32) doSbrk(HLS_UINT(32) value);
  HLS_INT(32) doGettimeofday(HLS_UINT(32) timeValPtr);
  HLS_INT(32) doUnlink(HLS_UINT(32) path);
  HLS_INT(32) doFstat(HLS_UINT(32) file, HLS_UINT(32) stataddr);
};

#endif // __BASIC_SIMULATOR_H__
