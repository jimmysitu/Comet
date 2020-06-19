/*
 * cacheMemory.h
 *
 *  Created on: 15 févr. 2019
 *      Author: simon
 */

#ifndef INCLUDE_CACHEMEMORY_H_
#define INCLUDE_CACHEMEMORY_H_

#include "logarithm.h"
#include "memoryInterface.h"
#include "pipelineRegisters.h"
#include <ac_int.h>

/************************************************************************
 * 	Following values are templates:
 * 		- OFFSET_SIZE
 * 		- TAG_SIZE
 * 		- SET_SIZE
 * 		- ASSOCIATIVITY
 ************************************************************************/
template <template <unsigned int> class NEXT_LEVEL, unsigned int INTERFACE_SIZE, int LINE_SIZE, int SET_SIZE>
class CacheMemory : public MemoryInterface<INTERFACE_SIZE> {

  static const int LOG_SET_SIZE           = log2const<SET_SIZE>::value;
  static const int LOG_LINE_SIZE          = log2const<LINE_SIZE>::value;
  static const int TAG_SIZE               = (32 - LOG_LINE_SIZE - LOG_SET_SIZE);
  static const int ASSOCIATIVITY          = 4;
  static const int LOG_ASSOCIATIVITY      = 2;
  static const int STATE_CACHE_MISS       = ((LINE_SIZE / INTERFACE_SIZE) * 2 + 3);
  static const int STATE_CACHE_LAST_STORE = ((LINE_SIZE / INTERFACE_SIZE) + 4);
  static const int STATE_CACHE_FIRST_LOAD = ((LINE_SIZE / INTERFACE_SIZE) + 3);
  static const int STATE_CACHE_LAST_LOAD  = 3;
  static const int LOG_INTERFACE_SIZE     = log2const<INTERFACE_SIZE>::value;

public:
  NEXT_LEVEL<INTERFACE_SIZE>* nextLevel;

  ac_int<TAG_SIZE + LINE_SIZE * 8, false> cacheMemory[SET_SIZE][ASSOCIATIVITY];
  ac_int<40, false> age[SET_SIZE][ASSOCIATIVITY];
  ac_int<1, false> dataValid[SET_SIZE][ASSOCIATIVITY];

  ac_int<6, false> cacheState;                // Used for the internal state machine
  ac_int<LOG_ASSOCIATIVITY, false> older = 0; // Set where the miss occurs

  // Variables for next level access
  ac_int<LINE_SIZE * 8 + TAG_SIZE, false> newVal, oldVal;
  ac_int<32, false> nextLevelAddr;
  memOpType nextLevelOpType;
  memMask nextLevelMask;
  ac_int<INTERFACE_SIZE * 8, false> nextLevelDataIn;
  ac_int<INTERFACE_SIZE * 8, false> nextLevelDataOut;
  ac_int<40, false> cycle;
  ac_int<LOG_ASSOCIATIVITY, false> setMiss;
  bool isValid;

  bool wasStore = false;
  ac_int<32, false> addrStore;
  ac_int<LOG_ASSOCIATIVITY, false> setStore;
  ac_int<LOG_SET_SIZE, false> placeStore;
  ac_int<LINE_SIZE * 8 + TAG_SIZE, false> valStore;
  ac_int<INTERFACE_SIZE * 8, false> dataOutStore;

  bool nextLevelWaitOut;

  bool VERBOSE = false;

  // Stats
  unsigned long numberAccess, numberMiss;

  ac_int<32, false> prefetchedAddr, oldPrefetchedAddr;
  ac_int<LINE_SIZE * 8 + TAG_SIZE, false> cacheVal1, cacheVal2, cacheVal3, cacheVal4;
  ac_int<1, false> cacheValid1, cacheValid2, cacheValid3, cacheValid4;
  ac_int<16, false> cacheAge1, cacheAge2, cacheAge3, cacheAge4;
  ac_int<LINE_SIZE * 8 + TAG_SIZE, false> cacheValPre1, cacheValPre2, cacheValPre3, cacheValPre4;
  ac_int<1, false> cacheValidPre1, cacheValidPre2, cacheValidPre3, cacheValidPre4;
  ac_int<16, false> cacheAgePre1, cacheAgePre2, cacheAgePre3, cacheAgePre4;

  CacheMemory(NEXT_LEVEL<INTERFACE_SIZE>* nextLevel, bool v)
  {
    this->nextLevel = nextLevel;
    for (int oneSetElement = 0; oneSetElement < SET_SIZE; oneSetElement++) {
      for (int oneSet = 0; oneSet < ASSOCIATIVITY; oneSet++) {
        cacheMemory[oneSetElement][oneSet] = 0;
        age[oneSetElement][oneSet]         = 0;
        dataValid[oneSetElement][oneSet]   = 0;
      }
    }
    VERBOSE          = v;
    numberAccess     = 0;
    numberMiss       = 0;
    nextLevelWaitOut = false;
    wasStore         = false;
    cacheState       = 0;
    nextLevelOpType  = NONE;
  }

  void process(ac_int<32, false> addr, memMask mask, memOpType opType, ac_int<INTERFACE_SIZE * 8, false> dataIn,
               ac_int<INTERFACE_SIZE * 8, false>& dataOut, bool& waitOut, ac_int<32, false> nextAddr)
  {

    // bit size is the log(setSize)
    ac_int<LOG_SET_SIZE, false> place = addr.slc<LOG_SET_SIZE>(LOG_LINE_SIZE);
    // startAddress is log(lineSize) + log(setSize) + 2
    ac_int<TAG_SIZE, false> tag = addr.slc<TAG_SIZE>(LOG_LINE_SIZE + LOG_SET_SIZE);
    // bitSize is log(lineSize), start address is 2(because of #bytes in a word)
    ac_int<LOG_LINE_SIZE, false> offset = addr.slc<LOG_LINE_SIZE - 2>(2);

    ac_int<LINE_SIZE * 8 + TAG_SIZE, false> cacheVal1_local = 0, cacheVal2_local = 0, cacheVal3_local = 0,
                                            cacheVal4_local = 0;
    ac_int<1, false> cacheValid1_local = 0, cacheValid2_local = 0, cacheValid3_local = 0, cacheValid4_local = 0;
    ac_int<16, false> cacheAge1_local = 0, cacheAge2_local = 0, cacheAge3_local = 0, cacheAge4_local = 0;
    bool updateLoadedLine = cacheState == 1 || cacheState == 0;

    if (!nextLevelWaitOut) {
      cycle++;

      if (opType != NONE && cacheState == 0 && addr >= 0x30000) {
        printf("Forwarding to peripherics (%x at %x)\n", addr, dataOut);
        nextLevelAddr    = addr;
        nextLevelDataIn  = dataIn;
        nextLevelDataOut = dataOut;
        nextLevelOpType  = opType;
        nextLevelMask    = mask;
      }

      if (wasStore || cacheState == 2) {
        // if (wasStore)
        //   printf("Storing in cache %d %d -> %x %x %x %x   (addrStore is %x  addrPre is %x)\n", placeStore,
        //   setStore,
        //          valStore.template slc<32>(TAG_SIZE), valStore.template slc<32>(TAG_SIZE + 32),
        //          valStore.template slc<32>(TAG_SIZE + 64), valStore.template slc<32>(TAG_SIZE + 96), addrStore,
        //          prefetchedAddr);

        cacheMemory[placeStore][setStore] = valStore;
        age[placeStore][setStore]         = cycle;
        dataValid[placeStore][setStore]   = 1;
        dataOut                           = dataOutStore;
        wasStore                          = false;
        cacheState                        = 1;

        ac_int<LOG_SET_SIZE, false> placePre = prefetchedAddr.slc<LOG_SET_SIZE>(LOG_LINE_SIZE);
        if (placeStore == placePre) {
          if (setStore == 0)
            cacheValPre1 = valStore;
          if (setStore == 1)
            cacheValPre2 = valStore;
          if (setStore == 2)
            cacheValPre3 = valStore;
          if (setStore == 3)
            cacheValPre4 = valStore;
        }

      } else {
        ac_int<LOG_SET_SIZE, false> nextPlace = nextAddr.slc<LOG_SET_SIZE>(LOG_LINE_SIZE);

        cacheVal1_local = cacheMemory[nextPlace][0];
        cacheVal2_local = cacheMemory[nextPlace][1];
        cacheVal3_local = cacheMemory[nextPlace][2];
        cacheVal4_local = cacheMemory[nextPlace][3];

        cacheValid1_local = dataValid[nextPlace][0];
        cacheValid2_local = dataValid[nextPlace][1];
        cacheValid3_local = dataValid[nextPlace][2];
        cacheValid4_local = dataValid[nextPlace][3];

        cacheAge1_local = age[nextPlace][0];
        cacheAge2_local = age[nextPlace][1];
        cacheAge3_local = age[nextPlace][2];
        cacheAge4_local = age[nextPlace][3];

        if (opType != NONE) {
          ac_int<LINE_SIZE * 8 + TAG_SIZE, false> val1 = addr == prefetchedAddr ? cacheValPre1 : cacheVal1;
          ac_int<LINE_SIZE * 8 + TAG_SIZE, false> val2 = addr == prefetchedAddr ? cacheValPre2 : cacheVal2;
          ac_int<LINE_SIZE * 8 + TAG_SIZE, false> val3 = addr == prefetchedAddr ? cacheValPre3 : cacheVal3;
          ac_int<LINE_SIZE * 8 + TAG_SIZE, false> val4 = addr == prefetchedAddr ? cacheValPre4 : cacheVal4;

          ac_int<1, false> valid1 = addr == prefetchedAddr ? cacheValidPre1 : cacheValid1;
          ac_int<1, false> valid2 = addr == prefetchedAddr ? cacheValidPre2 : cacheValid2;
          ac_int<1, false> valid3 = addr == prefetchedAddr ? cacheValidPre3 : cacheValid3;
          ac_int<1, false> valid4 = addr == prefetchedAddr ? cacheValidPre4 : cacheValid4;

          ac_int<16, false> age1 = addr == prefetchedAddr ? cacheAgePre1 : cacheAge1;
          ac_int<16, false> age2 = addr == prefetchedAddr ? cacheAgePre2 : cacheAge2;
          ac_int<16, false> age3 = addr == prefetchedAddr ? cacheAgePre3 : cacheAge3;
          ac_int<16, false> age4 = addr == prefetchedAddr ? cacheAgePre4 : cacheAge4;

          if (cacheState == 0) {
            numberAccess++;

            ac_int<TAG_SIZE, false> tag1 = val1.template slc<TAG_SIZE>(0);
            ac_int<TAG_SIZE, false> tag2 = val2.template slc<TAG_SIZE>(0);
            ac_int<TAG_SIZE, false> tag3 = val3.template slc<TAG_SIZE>(0);
            ac_int<TAG_SIZE, false> tag4 = val4.template slc<TAG_SIZE>(0);

            bool hit1 = (tag1 == tag) && valid1;
            bool hit2 = (tag2 == tag) && valid2;
            bool hit3 = (tag3 == tag) && valid3;
            bool hit4 = (tag4 == tag) && valid4;
            bool hit  = hit1 | hit2 | hit3 | hit4;

            ac_int<LOG_ASSOCIATIVITY, false> set = 0;
            ac_int<LINE_SIZE * 8, false> selectedValue;
            ac_int<TAG_SIZE, false> tag;

            if (hit1) {
              selectedValue = val1.template slc<LINE_SIZE * 8>(TAG_SIZE);
              tag           = tag1;
              set           = 0;
            }

            if (hit2) {
              selectedValue = val2.template slc<LINE_SIZE * 8>(TAG_SIZE);
              tag           = tag2;
              set           = 1;
            }

            if (hit3) {
              selectedValue = val3.template slc<LINE_SIZE * 8>(TAG_SIZE);
              tag           = tag3;
              set           = 2;
            }

            if (hit4) {
              selectedValue = val4.template slc<LINE_SIZE * 8>(TAG_SIZE);
              tag           = tag4;
              set           = 3;
            }

            ac_int<8, true> signedByte;
            ac_int<16, true> signedHalf;
            ac_int<32, true> signedWord;

            if (hit) {
              ac_int<LINE_SIZE * 8 + TAG_SIZE, false> localValStore = 0;
              localValStore.set_slc(TAG_SIZE, selectedValue);
              localValStore.set_slc(0, tag);

              // First we handle the store
              if (opType == STORE) {
                // printf("Doing store of %x at %x\n", dataIn, addr);
                switch (mask) {
                  case BYTE:
                  case BYTE_U:
                    localValStore.set_slc((((int)addr.slc<2>(0)) << 3) + TAG_SIZE + 4 * 8 * offset,
                                          dataIn.template slc<8>(0));
                    break;
                  case HALF:
                  case HALF_U:
                    localValStore.set_slc((addr[1] ? 16 : 0) + TAG_SIZE + 4 * 8 * offset, dataIn.template slc<16>(0));
                    break;
                  case WORD:
                    localValStore.set_slc(TAG_SIZE + 4 * 8 * offset, dataIn.template slc<32>(0));
                    break;
                  case LONG:
                    localValStore.set_slc(TAG_SIZE + 4 * 8 * offset, dataIn);
                    break;
                }

                placeStore = place;
                setStore   = set;
                valStore   = localValStore;
                wasStore   = true;
                addrStore  = addr;

              } else {
                switch (mask) {
                  case BYTE:
                    signedByte = selectedValue.template slc<8>((((int)addr.slc<2>(0)) << 3) + 4 * 8 * offset);
                    signedWord = signedByte;
                    dataOut.set_slc(0, signedWord);
                    break;
                  case HALF:
                    signedHalf = selectedValue.template slc<16>((addr[1] ? 16 : 0) + 4 * 8 * offset);
                    signedWord = signedHalf;
                    dataOut.set_slc(0, signedWord);
                    break;
                  case WORD:
                    dataOut = selectedValue.template slc<32>(4 * 8 * offset);
                    break;
                  case BYTE_U:
                    dataOut = selectedValue.template slc<8>((((int)addr.slc<2>(0)) << 3) + 4 * 8 * offset) & 0xff;
                    break;
                  case HALF_U:
                    dataOut = selectedValue.template slc<16>((addr[1] ? 16 : 0) + 4 * 8 * offset) & 0xffff;
                    break;
                  case LONG:
                    dataOut = selectedValue.template slc<INTERFACE_SIZE * 8>(4 * 8 * offset);
                    break;
                }
                // if (addr > 0x12bf4)
                //   printf("[H] Reading %x at %x\n", (unsigned int)dataOut, (unsigned int)addr);
              }
              // age[place][set] = cycle;

            } else {
              numberMiss++;
              cacheState = STATE_CACHE_MISS;
            }
          } else {

            if (cacheState == STATE_CACHE_MISS) {
              newVal  = tag;
              setMiss = (age1 < age2 && age1 < age3 && age1 < age4)
                            ? 0
                            : ((age2 < age1 && age2 < age3 && age2 < age4)
                                   ? 1
                                   : ((age3 < age2 && age3 < age1 && age3 < age4) ? 2 : 3));
              oldVal = (age1 < age2 && age1 < age3 && age1 < age4)
                           ? val1
                           : ((age2 < age1 && age2 < age3 && age2 < age4)
                                  ? val2
                                  : ((age3 < age2 && age3 < age1 && age3 < age4) ? val3 : val4));
              isValid = (age1 < age2 && age1 < age3 && age1 < age4)
                            ? valid1
                            : ((age2 < age1 && age2 < age3 && age2 < age4)
                                   ? valid2
                                   : ((age3 < age2 && age3 < age1 && age3 < age4) ? valid3 : valid4));
              // printf("TAG is %x\n", oldVal.slc<TAG_SIZE>(0));
            }

            ac_int<32, false> oldAddress = (((int)oldVal.template slc<TAG_SIZE>(0)) << (LOG_LINE_SIZE + LOG_SET_SIZE)) |
                                           (((int)place) << LOG_LINE_SIZE);
            // First we write back the four memory values in upper level

            if (cacheState >= STATE_CACHE_LAST_STORE) {
              // We store all values into next memory interface
              nextLevelAddr   = oldAddress + (((int)(cacheState - STATE_CACHE_LAST_STORE)) << LOG_INTERFACE_SIZE);
              nextLevelDataIn = oldVal.template slc<INTERFACE_SIZE * 8>(
                  (cacheState - STATE_CACHE_LAST_STORE) * INTERFACE_SIZE * 8 + TAG_SIZE);
              nextLevelOpType = (isValid) ? STORE : NONE;
              nextLevelMask   = LONG;
              // if (isValid && addr > 0x12bf4)
              //   printf("Writing back %x at %x\n", nextLevelDataIn, nextLevelAddr);

            } else if (cacheState >= STATE_CACHE_LAST_LOAD) {
              // Then we read values from next memory level
              if (cacheState != STATE_CACHE_FIRST_LOAD) {
                newVal.set_slc(((unsigned int)(cacheState - STATE_CACHE_LAST_LOAD)) * INTERFACE_SIZE * 8 + TAG_SIZE,
                               nextLevelDataOut); // at addr +1
              }

              if (cacheState != STATE_CACHE_LAST_LOAD) {
                // We initiate the load at the address determined by next cache state
                nextLevelAddr = (((int)addr.slc<32 - LOG_LINE_SIZE>(LOG_LINE_SIZE)) << LOG_LINE_SIZE) +
                                ((cacheState - STATE_CACHE_LAST_LOAD - 1) << LOG_INTERFACE_SIZE);
                nextLevelOpType = LOAD;
                nextLevelMask   = LONG;
              }
            }

            cacheState--;

            if (cacheState == 2) {
              if (opType == STORE) {
                // printf("Doing store of %x at %x\n", dataIn, addr);

                switch (mask) {
                  case BYTE:
                  case BYTE_U:
                    newVal.set_slc((((int)addr.slc<2>(0)) << 3) + TAG_SIZE + 4 * 8 * offset, dataIn.template slc<8>(0));
                    break;
                  case HALF:
                  case HALF_U:
                    newVal.set_slc((addr[1] ? 16 : 0) + TAG_SIZE + 4 * 8 * offset, dataIn.template slc<16>(0));
                    break;
                  case WORD:
                    newVal.set_slc(TAG_SIZE + 4 * 8 * offset, dataIn.template slc<32>(0));
                    break;
                  case LONG:
                    newVal.set_slc(TAG_SIZE + 4 * 8 * offset, dataIn);
                    break;
                }
              }

              placeStore = place;
              setStore   = setMiss;
              valStore   = newVal;
              addrStore  = addr;

              // cacheMemory[place][setMiss] = newVal;
              // dataValid[place][setMiss] = 1;
              // age[place][setMiss] = cycle;
              nextLevelOpType = NONE;

              ac_int<8, true> signedByte;
              ac_int<16, true> signedHalf;
              ac_int<32, true> signedWord;

              switch (mask) {
                case BYTE:
                  signedByte = newVal.template slc<8>((((int)addr.slc<2>(0)) << 3) + 4 * 8 * offset + TAG_SIZE);
                  signedWord = signedByte;
                  dataOut.set_slc(0, signedWord);
                  break;
                case HALF:
                  signedHalf = newVal.template slc<16>((addr[1] ? 16 : 0) + 4 * 8 * offset + TAG_SIZE);
                  signedWord = signedHalf;
                  dataOut.set_slc(0, signedWord);
                  break;
                case WORD:
                  dataOut = newVal.template slc<32>(4 * 8 * offset + TAG_SIZE);
                  break;
                case BYTE_U:
                  dataOut = newVal.template slc<8>((((int)addr.slc<2>(0)) << 3) + 4 * 8 * offset + TAG_SIZE) & 0xff;
                  break;
                case HALF_U:
                  dataOut = newVal.template slc<16>((addr[1] ? 16 : 0) + 4 * 8 * offset + TAG_SIZE) & 0xffff;
                  break;
                case LONG:
                  dataOut = newVal.template slc<INTERFACE_SIZE * 8>(4 * 8 * offset);
                  break;
              }
              // printf("After Miss read %x at %x\n", (unsigned int)dataOut.slc<32>(0), (unsigned int)addr);

              dataOutStore = dataOut;
            } else if (cacheState == 0 && opType == LOAD) {
              // if (addr > 0x12bf4)
              //   printf("[M] Reading %x at %x\n", dataOutStore, addr);
              dataOut = dataOutStore;
            }
          }
        }
      }
    }

    this->nextLevel->process(nextLevelAddr, nextLevelMask, nextLevelOpType, nextLevelDataIn, nextLevelDataOut,
                             nextLevelWaitOut, nextLevelAddr);
    waitOut = nextLevelWaitOut || cacheState || (wasStore && opType != NONE);

    if (updateLoadedLine && cacheState != STATE_CACHE_MISS) {

      if (nextAddr != prefetchedAddr) {
        oldPrefetchedAddr = prefetchedAddr;
        prefetchedAddr    = nextAddr;
        cacheVal1         = cacheValPre1;
        cacheVal2         = cacheValPre2;
        cacheVal3         = cacheValPre3;
        cacheVal4         = cacheValPre4;

        cacheValid1 = cacheValidPre1;
        cacheValid2 = cacheValidPre2;
        cacheValid3 = cacheValidPre3;
        cacheValid4 = cacheValidPre4;

        cacheAge1 = cacheAgePre1;
        cacheAge2 = cacheAgePre2;
        cacheAge3 = cacheAgePre3;
        cacheAge4 = cacheAgePre4;

        cacheValPre1 = cacheVal1_local;
        cacheValPre2 = cacheVal2_local;
        cacheValPre3 = cacheVal3_local;
        cacheValPre4 = cacheVal4_local;

        cacheValidPre1 = cacheValid1_local;
        cacheValidPre2 = cacheValid2_local;
        cacheValidPre3 = cacheValid3_local;
        cacheValidPre4 = cacheValid4_local;

        cacheAgePre1 = cacheAge1_local;
        cacheAgePre2 = cacheAge2_local;
        cacheAgePre3 = cacheAge3_local;
        cacheAgePre4 = cacheAge4_local;
      }
    }
  }
};

#endif /* INCLUDE_CACHEMEMORY_H_ */
