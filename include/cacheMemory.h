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
#include "tools.h"

/************************************************************************
 * 	Following values are templates:
 * 		- OFFSET_SIZE
 * 		- TAG_SIZE
 * 		- SET_SIZE
 * 		- ASSOCIATIVITY
 ************************************************************************/
template <unsigned int INTERFACE_SIZE, int LINE_SIZE, int SET_SIZE>
class CacheMemory : public MemoryInterface<INTERFACE_SIZE> {

  static const int LOG_SET_SIZE           = log2const<SET_SIZE>::value;
  static const int LOG_LINE_SIZE          = log2const<LINE_SIZE>::value;
  static const int TAG_SIZE               = (32 - LOG_LINE_SIZE - LOG_SET_SIZE);
  static const int ASSOCIATIVITY          = 4;
  static const int LOG_ASSOCIATIVITY      = 2;
  static const int STATE_CACHE_MISS       = ((LINE_SIZE / INTERFACE_SIZE) * 2 + 2);
  static const int STATE_CACHE_LAST_STORE = ((LINE_SIZE / INTERFACE_SIZE) + 3);
  static const int STATE_CACHE_FIRST_LOAD = ((LINE_SIZE / INTERFACE_SIZE) + 2);
  static const int STATE_CACHE_LAST_LOAD  = 2;
  static const int LOG_INTERFACE_SIZE     = log2const<INTERFACE_SIZE>::value;

public:
  MemoryInterface<INTERFACE_SIZE>* nextLevel;

  HLS_UINT(TAG_SIZE + LINE_SIZE * 8) cacheMemory[SET_SIZE][ASSOCIATIVITY];
  HLS_UINT(40) age[SET_SIZE][ASSOCIATIVITY];
  HLS_UINT(1) dataValid[SET_SIZE][ASSOCIATIVITY];

  HLS_UINT(6) cacheState;                // Used for the internal state machine
  HLS_UINT(LOG_ASSOCIATIVITY) older = 0; // Set where the miss occurs

  // Variables for next level access
  HLS_UINT(LINE_SIZE * 8 + TAG_SIZE) newVal, oldVal;
  HLS_UINT(32) nextLevelAddr;
  memOpType nextLevelOpType;
  HLS_UINT(INTERFACE_SIZE * 8) nextLevelDataIn;
  HLS_UINT(INTERFACE_SIZE * 8) nextLevelDataOut;
  HLS_UINT(40) cycle;
  HLS_UINT(LOG_ASSOCIATIVITY) setMiss;
  bool isValid;

  bool wasStore = false;
  HLS_UINT(LOG_ASSOCIATIVITY) setStore;
  HLS_UINT(LOG_SET_SIZE) placeStore;
  HLS_UINT(LINE_SIZE * 8 + TAG_SIZE) valStore;
  HLS_UINT(INTERFACE_SIZE * 8) dataOutStore;

  bool nextLevelWaitOut;

  bool VERBOSE = false;

  // Stats
  unsigned long numberAccess, numberMiss;

  CacheMemory(MemoryInterface<INTERFACE_SIZE>* nextLevel, bool v)
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

  void process(HLS_UINT(32) addr, memMask mask, memOpType opType, HLS_UINT(INTERFACE_SIZE * 8) dataIn,
               HLS_UINT(INTERFACE_SIZE * 8)& dataOut, bool& waitOut)
  {

    // bit size is the log(setSize)
    HLS_UINT(LOG_SET_SIZE) place = addr.SLC(LOG_SET_SIZE, LOG_LINE_SIZE);
    // startAddress is log(lineSize) + log(setSize) + 2
    HLS_UINT(TAG_SIZE) tag = addr.SLC(TAG_SIZE, LOG_LINE_SIZE + LOG_SET_SIZE);
    // bitSize is log(lineSize), start address is 2(because of #bytes in a word)
    HLS_UINT(LOG_LINE_SIZE) offset = addr.SLC(LOG_LINE_SIZE - 2, 2);

    if (!nextLevelWaitOut) {
      cycle++;

      if (wasStore || cacheState == 1) {

        cacheMemory[placeStore][setStore] = valStore;
        age[placeStore][setStore]         = cycle;
        dataValid[placeStore][setStore]   = 1;
        dataOut                           = dataOutStore;
        wasStore                          = false;
        cacheState                        = 0;

      } else if (opType != NONE) {

        HLS_UINT(LINE_SIZE * 8 + TAG_SIZE) val1 = cacheMemory[place][0];
        HLS_UINT(LINE_SIZE * 8 + TAG_SIZE) val2 = cacheMemory[place][1];
        HLS_UINT(LINE_SIZE * 8 + TAG_SIZE) val3 = cacheMemory[place][2];
        HLS_UINT(LINE_SIZE * 8 + TAG_SIZE) val4 = cacheMemory[place][3];

        HLS_UINT(1) valid1 = dataValid[place][0];
        HLS_UINT(1) valid2 = dataValid[place][1];
        HLS_UINT(1) valid3 = dataValid[place][2];
        HLS_UINT(1) valid4 = dataValid[place][3];

        HLS_UINT(16) age1 = age[place][0];
        HLS_UINT(16) age2 = age[place][1];
        HLS_UINT(16) age3 = age[place][2];
        HLS_UINT(16) age4 = age[place][3];

        if (cacheState == 0) {
          numberAccess++;

          HLS_UINT(TAG_SIZE) tag1 = val1.template SLC(TAG_SIZE, 0);
          HLS_UINT(TAG_SIZE) tag2 = val2.template SLC(TAG_SIZE, 0);
          HLS_UINT(TAG_SIZE) tag3 = val3.template SLC(TAG_SIZE, 0);
          HLS_UINT(TAG_SIZE) tag4 = val4.template SLC(TAG_SIZE, 0);

          bool hit1 = (tag1 == tag) && valid1;
          bool hit2 = (tag2 == tag) && valid2;
          bool hit3 = (tag3 == tag) && valid3;
          bool hit4 = (tag4 == tag) && valid4;
          bool hit  = hit1 | hit2 | hit3 | hit4;

          HLS_UINT(LOG_ASSOCIATIVITY) set = 0;
          HLS_UINT(LINE_SIZE * 8) selectedValue;
          HLS_UINT(TAG_SIZE) tag;

          if (hit1) {
            selectedValue = val1.template SLC(LINE_SIZE * 8, TAG_SIZE);
            tag           = tag1;
            set           = 0;
          }

          if (hit2) {
            selectedValue = val2.template SLC(LINE_SIZE * 8, TAG_SIZE);
            tag           = tag2;
            set           = 1;
          }

          if (hit3) {
            selectedValue = val3.template SLC(LINE_SIZE * 8, TAG_SIZE);
            tag           = tag3;
            set           = 2;
          }

          if (hit4) {
            selectedValue = val4.template SLC(LINE_SIZE * 8, TAG_SIZE);
            tag           = tag4;
            set           = 3;
          }

          HLS_INT(8) signedByte;
          HLS_INT(16) signedHalf;
          HLS_INT(32) signedWord;

          if (hit) {
            HLS_UINT(LINE_SIZE * 8 + TAG_SIZE) localValStore = 0;
            localValStore.SET_SLC(TAG_SIZE, selectedValue);
            localValStore.SET_SLC(0, tag);

            // First we handle the store
            if (opType == STORE) {
              switch (mask) {
                case BYTE:
                case BYTE_U:
                  localValStore.SET_SLC((((int)addr.SLC(2, 0)) << 3) + TAG_SIZE + 4 * 8 * offset,
                                        dataIn.template SLC(8, 0));
                  break;
                case HALF:
                case HALF_U:
                  localValStore.SET_SLC((addr[1] ? 16 : 0) + TAG_SIZE + 4 * 8 * offset, dataIn.template SLC(16, 0));
                  break;
                case WORD:
                  localValStore.SET_SLC(TAG_SIZE + 4 * 8 * offset, dataIn.template SLC(32, 0));
                  break;
                case LONG:
                  localValStore.SET_SLC(TAG_SIZE + 4 * 8 * offset, dataIn);
                  break;
              }

              placeStore = place;
              setStore   = set;
              valStore   = localValStore;
              wasStore   = true;

            } else {
              switch (mask) {
                case BYTE:
                  signedByte = selectedValue.template SLC(8, (((int)addr.SLC(2, 0)) << 3) + 4 * 8 * offset);
                  signedWord = signedByte;
                  dataOut.SET_SLC(0, signedWord);
                  break;
                case HALF:
                  signedHalf = selectedValue.template SLC(16, (addr[1] ? 16 : 0) + 4 * 8 * offset);
                  signedWord = signedHalf;
                  dataOut.SET_SLC(0, signedWord);
                  break;
                case WORD:
                  dataOut = selectedValue.template SLC(32, 4 * 8 * offset);
                  break;
                case BYTE_U:
                  dataOut = selectedValue.template SLC(8, (((int)addr.SLC(2, 0)) << 3) + 4 * 8 * offset) & 0xff;
                  break;
                case HALF_U:
                  dataOut = selectedValue.template SLC(16, (addr[1] ? 16 : 0) + 4 * 8 * offset) & 0xffff;
                  break;
                case LONG:
                  dataOut = selectedValue.template SLC(INTERFACE_SIZE * 8, 4 * 8 * offset);
                  break;
              }

              // printf("Hit read %x at %x\n", (unsigned int)dataOut.SLC(32, 0), (unsigned int)addr);
            }
            // age[place][set] = cycle;

          } else {
            numberMiss++;
            cacheState = STATE_CACHE_MISS;
          }
        } else {
          // printf("Miss %d\n", (unsigned int)cacheState);

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
            // printf("TAG is %x\n", oldVal.SLC(TAG_SIZE, 0));
          }

          HLS_UINT(32) oldAddress = (((int)oldVal.template SLC(TAG_SIZE, 0)) << (LOG_LINE_SIZE + LOG_SET_SIZE)) |
                                         (((int)place) << LOG_LINE_SIZE);
          // First we write back the four memory values in upper level

          if (cacheState >= STATE_CACHE_LAST_STORE) {
            // We store all values into next memory interface
            nextLevelAddr   = oldAddress + (((int)(cacheState - STATE_CACHE_LAST_STORE)) << LOG_INTERFACE_SIZE);
            nextLevelDataIn = oldVal.template SLC(INTERFACE_SIZE * 8,
                (cacheState - STATE_CACHE_LAST_STORE) * INTERFACE_SIZE * 8 + TAG_SIZE);
            nextLevelOpType = (isValid) ? STORE : NONE;

            // printf("Writing back %x %x at %x\n", (unsigned int)nextLevelDataIn.SLC(32, 0),
            //        (unsigned int)nextLevelDataIn.SLC(32, 32), (unsigned int)nextLevelAddr);

          } else if (cacheState >= STATE_CACHE_LAST_LOAD) {
            // Then we read values from next memory level
            if (cacheState != STATE_CACHE_FIRST_LOAD) {
              newVal.SET_SLC(((unsigned int)(cacheState - STATE_CACHE_LAST_LOAD)) * INTERFACE_SIZE * 8 + TAG_SIZE,
                             nextLevelDataOut); // at addr +1
            }

            if (cacheState != STATE_CACHE_LAST_LOAD) {
              // We initiate the load at the address determined by next cache state
              nextLevelAddr = (((int)addr.SLC(32 - LOG_LINE_SIZE, LOG_LINE_SIZE)) << LOG_LINE_SIZE) +
                              ((cacheState - STATE_CACHE_LAST_LOAD - 1) << LOG_INTERFACE_SIZE);
              nextLevelOpType = LOAD;
            }
          }

          cacheState--;

          if (cacheState == 1) {
            if (opType == STORE) {
              switch (mask) {
                case BYTE:
                case BYTE_U:
                  newVal.SET_SLC((((int)addr.SLC(2, 0)) << 3) + TAG_SIZE + 4 * 8 * offset, dataIn.template SLC(8, 0));
                  break;
                case HALF:
                case HALF_U:
                  newVal.SET_SLC((addr[1] ? 16 : 0) + TAG_SIZE + 4 * 8 * offset, dataIn.template SLC(16, 0));
                  break;
                case WORD:
                  newVal.SET_SLC(TAG_SIZE + 4 * 8 * offset, dataIn.template SLC(32, 0));
                  break;
                case LONG:
                  newVal.SET_SLC(TAG_SIZE + 4 * 8 * offset, dataIn);
                  break;
              }
            }

            placeStore = place;
            setStore   = setMiss;
            valStore   = newVal;

            // cacheMemory[place][setMiss] = newVal;
            // dataValid[place][setMiss] = 1;
            // age[place][setMiss] = cycle;
            nextLevelOpType = NONE;

            HLS_INT(8) signedByte;
            HLS_INT(16) signedHalf;
            HLS_INT(32) signedWord;

            switch (mask) {
              case BYTE:
                signedByte = newVal.template SLC(8, (((int)addr.SLC(2, 0)) << 3) + 4 * 8 * offset + TAG_SIZE);
                signedWord = signedByte;
                dataOut.SET_SLC(0, signedWord);
                break;
              case HALF:
                signedHalf = newVal.template SLC(16, (addr[1] ? 16 : 0) + 4 * 8 * offset + TAG_SIZE);
                signedWord = signedHalf;
                dataOut.SET_SLC(0, signedWord);
                break;
              case WORD:
                dataOut = newVal.template SLC(32, 4 * 8 * offset + TAG_SIZE);
                break;
              case BYTE_U:
                dataOut = newVal.template SLC(8, (((int)addr.SLC(2, 0)) << 3) + 4 * 8 * offset + TAG_SIZE) & 0xff;
                break;
              case HALF_U:
                dataOut = newVal.template SLC(16, (addr[1] ? 16 : 0) + 4 * 8 * offset + TAG_SIZE) & 0xffff;
                break;
              case LONG:
                dataOut = newVal.template SLC(INTERFACE_SIZE * 8, 4 * 8 * offset);
                break;
            }
            // printf("After Miss read %x at %x\n", (unsigned int)dataOut.SLC(32, 0), (unsigned int)addr);

            dataOutStore = dataOut;
          }
        }
      }
    }

    this->nextLevel->process(nextLevelAddr, LONG, nextLevelOpType, nextLevelDataIn, nextLevelDataOut, nextLevelWaitOut);
    waitOut = nextLevelWaitOut || cacheState || (wasStore && opType != NONE);
  }
};

#endif /* INCLUDE_CACHEMEMORY_H_ */

