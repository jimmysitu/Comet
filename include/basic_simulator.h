#ifndef __BASIC_SIMULATOR_H__
#define __BASIC_SIMULATOR_H__

#include <map>
#include <vector>
#include "simulator.h"

class BasicSimulator : public Simulator
{
    unsigned heapAddress;
    std::map<ac_int<COMET_WORD_LENGTH, false>, ac_int<8, false> > imemMap;
    std::map<ac_int<COMET_WORD_LENGTH, false>, ac_int<8, false> > dmemMap;

    ac_int<COMET_WORD_LENGTH, false> *im, *dm;

    FILE* inputFile;
    FILE* outputFile;
    FILE* traceFile;
public:
    BasicSimulator(const char* binaryFile, std::vector<std::string>, const char* inFile, const char* outFile, const char *tFile );
    ~BasicSimulator();

protected:
    void fillMemory();

    void printCycle();
    void printStat(){};
    void extend(){};
    void solveSyscall();

    void insertInstructionMemoryMap(ac_int<COMET_WORD_LENGTH, false> addr, ac_int<8, false> value);
    void insertDataMemoryMap(ac_int<COMET_WORD_LENGTH, false> addr, ac_int<8, false> value);

    ac_int<COMET_WORD_LENGTH, true> doRead(ac_int<COMET_WORD_LENGTH, false> file, ac_int<COMET_WORD_LENGTH, false> bufferAddr, ac_int<COMET_WORD_LENGTH, false> size);
    ac_int<COMET_WORD_LENGTH, true> doWrite(ac_int<COMET_WORD_LENGTH, false> file, ac_int<COMET_WORD_LENGTH, false> bufferAddr, ac_int<COMET_WORD_LENGTH, false> size);
    ac_int<COMET_WORD_LENGTH, true> doOpen(ac_int<COMET_WORD_LENGTH, false> name, ac_int<COMET_WORD_LENGTH, false> flags, ac_int<COMET_WORD_LENGTH, false> mode);
    ac_int<COMET_WORD_LENGTH, true> doOpenat(ac_int<COMET_WORD_LENGTH, false> dir, ac_int<COMET_WORD_LENGTH, false> name, ac_int<COMET_WORD_LENGTH, false> flags, ac_int<COMET_WORD_LENGTH, false> mode);
    ac_int<COMET_WORD_LENGTH, true> doLseek(ac_int<COMET_WORD_LENGTH, false> file, ac_int<COMET_WORD_LENGTH, false> ptr, ac_int<COMET_WORD_LENGTH, false> dir);
    ac_int<COMET_WORD_LENGTH, true> doClose(ac_int<COMET_WORD_LENGTH, false> file);
    ac_int<COMET_WORD_LENGTH, true> doStat(ac_int<COMET_WORD_LENGTH, false> filename, ac_int<COMET_WORD_LENGTH, false> ptr);
    ac_int<COMET_WORD_LENGTH, true> doSbrk(ac_int<COMET_WORD_LENGTH, false> value);
    ac_int<COMET_WORD_LENGTH, true> doGettimeofday(ac_int<COMET_WORD_LENGTH, false> timeValPtr);
    ac_int<COMET_WORD_LENGTH, true> doUnlink(ac_int<COMET_WORD_LENGTH, false> path);
    ac_int<COMET_WORD_LENGTH, true> doFstat(ac_int<COMET_WORD_LENGTH, false> file, ac_int<COMET_WORD_LENGTH, false> stataddr);

    void stb(ac_int<COMET_WORD_LENGTH, false> addr, ac_int<8, true> value);
    void sth(ac_int<COMET_WORD_LENGTH, false> addr, ac_int<16, true> value);
    void stw(ac_int<COMET_WORD_LENGTH, false> addr, ac_int<32, true> value);
    void std(ac_int<COMET_WORD_LENGTH, false> addr, ac_int<64, true> value);

    ac_int<8, true> ldb(ac_int<COMET_WORD_LENGTH, false> addr);
    ac_int<16, true> ldh(ac_int<COMET_WORD_LENGTH, false> addr);
    ac_int<32, true> ldw(ac_int<COMET_WORD_LENGTH, false> addr);
    ac_int<64, true> ldd(ac_int<COMET_WORD_LENGTH, false> addr);
};

#endif // __BASIC_SIMULATOR_H__
