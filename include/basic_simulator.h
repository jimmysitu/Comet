#ifndef __BASIC_SIMULATOR_H__
#define __BASIC_SIMULATOR_H__

#include "simulator.h"
#include <map>
#include <vector>

class BasicSimulator : public Simulator {
  unsigned heapAddress;

  ac_int<32, false>*im, *dm;

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
  void stb(int coreId, ac_int<32, false> addr, ac_int<8, true> value);
  void sth(int coreId, ac_int<32, false> addr, ac_int<16, true> value);
  void stw(int coreId, ac_int<32, false> addr, ac_int<32, true> value);
  void std(int coreId, ac_int<32, false> addr, ac_int<64, true> value);

  ac_int<8, true> ldb(int coreId, ac_int<32, false> addr);
  ac_int<16, true> ldh(int coreId, ac_int<32, false> addr);
  ac_int<32, true> ldw(int coreId, ac_int<32, false> addr);
  ac_int<32, true> ldd(int coreId, ac_int<32, false> addr);

  // Functions for solving syscalls
  void solveSyscall();

  ac_int<32, true> doRead(int coreId, ac_int<32, false> file, ac_int<32, false> bufferAddr, ac_int<32, false> size);
  ac_int<32, true> doWrite(int coreId, ac_int<32, false> file, ac_int<32, false> bufferAddr, ac_int<32, false> size);
  ac_int<32, true> doOpen(int coreId, ac_int<32, false> name, ac_int<32, false> flags, ac_int<32, false> mode);
  ac_int<32, true> doOpenat(int coreId, ac_int<32, false> dir, ac_int<32, false> name, ac_int<32, false> flags,
                            ac_int<32, false> mode);
  ac_int<32, true> doLseek(int coreId, ac_int<32, false> file, ac_int<32, false> ptr, ac_int<32, false> dir);
  ac_int<32, true> doClose(int coreId, ac_int<32, false> file);
  ac_int<32, true> doStat(int coreId, ac_int<32, false> filename, ac_int<32, false> ptr);
  ac_int<32, true> doSbrk(int coreId, ac_int<32, false> value);
  ac_int<32, true> doGettimeofday(int coreId, ac_int<32, false> timeValPtr);
  ac_int<32, true> doUnlink(int coreId, ac_int<32, false> path);
  ac_int<32, true> doFstat(int coreId, ac_int<32, false> file, ac_int<32, false> stataddr);
};

#endif // __BASIC_SIMULATOR_H__
