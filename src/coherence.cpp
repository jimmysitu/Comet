#include "mc_scverify.h"

#include "coherence.h"

#ifndef __HLS__
#include "simulator.h"
#endif

/******************************************************************
 * switch of switch of switch is kinda ugly
 * Can we represent it with a table of transition?
 ******************************************************************/



void directory(CoherenceCacheToDirectory in_cd[COMET_CORE],
               CoherenceDirectoryToCache out_dc[COMET_CORE]
            #ifndef __HLS__
               , Simulator* sim
            #endif
               )
{
    static DirectoryControl ctrl;

    CoherenceCacheToDirectory cd[COMET_CORE];
    CoherenceDirectoryToCache dc[COMET_CORE];
    #pragma hls_unroll yes
    for(int i(0); i < COMET_CORE;  ++i)
        cd[i] = in_cd[i];

    switch(ctrl.state)
    {
    case DirectoryControl::Idle:
    {
        ac_int<ac::log2_ceil<COMET_CORE>::val, false> i = ctrl.core;
        #pragma hls_unroll yes
        do {
            i++;
            if(cd[i].type != CoherenceCacheToDirectory::None)
            {
                ctrl.core = i;
                //break;
            }
        } while(i != ctrl.core);

        if(cd[i].type != CoherenceCacheToDirectory::None)
        {
            ac_int<32, false> address = cd[i].address;
            ctrl.line = ctrl.lines[i][getDSet(address)];
            ctrl.state = DirectoryControl::Reply;

            for(int j(0); j < COMET_CORE; ++j)
            {
                if(ctrl.line.sharers[j] && i != j)
                    switch(cd[i].type)
                    {
                    case CoherenceCacheToDirectory::ReadMiss:
                        dc[j].address = address;
                        dc[j].type = CoherenceDirectoryToCache::Fetch;
                        dc[j].data = 0;
                        break;  // double break? change value of j to comet_core-1?
                    case CoherenceCacheToDirectory::WriteMiss:
                        dc[j].address = address;
                        dc[j].type = CoherenceDirectoryToCache::FetchInvalidate;
                        dc[j].data = 0;
                        break;  // double break? change value of j to comet_core-1?
                        // what if more than 1 cache has the data, we must invalidate all, but fetch
                        // from only one
                    case CoherenceCacheToDirectory::WriteInvalidate:
                        dc[j].address = address;
                        dc[j].type = CoherenceDirectoryToCache::Invalidate;
                        dc[j].data = 0;
                        break;
                    default:
                        dbgassert(false, "Unknown transition in directory control\n");
                        break;
                    }
            }
        }
        else
            ctrl.state = DirectoryControl::Idle;

    }
        break;
    case DirectoryControl::Reply:
    {
        ac_int<ac::log2_ceil<COMET_CORE>::val+1, false> numsharers = 0;

        #pragma hls_unroll yes
        for(int j(0); j < COMET_CORE; ++j)
            numsharers += ctrl.line.sharers[j];

        switch(ctrl.line.state)
        {
        case LineCoherence::Invalid:
            /// No cache has the requested data
            /// valid request are ReadMiss, forward data from main memory to cache
            ///                   WriteMiss, forward data from main memory to cache
            /// sharers = P

            switch(cd[ctrl.core].type)
            {
            case CoherenceCacheToDirectory::ReadMiss:

                break;
            case CoherenceCacheToDirectory::WriteMiss:
                break;
            default:
                dbgassert(false, "Unknown transition in directory control\n");
                break;
            }


            break;
        case LineCoherence::Shared:
            /// At least one cache has the data
            /// valid request are ReadMiss, forward data from one cache to another
            ///                   WriteMiss, forward data from one cache to another and invalidate
            ///                   WriteInvalidate, invalidate other cache data
            ///                   WriteBack, forward data from one cache to main memory, remove line from directory
            /// update sharers accordingly
            break;
        case LineCoherence::Modified:
            /// valid request are ReadMiss, forward data from one cache to another, writeback to mem?
            ///                   WriteMiss, forward data from one cache to another, no writeback needed
            ///                   WriteBack, forward data from one cache to main memory, remove line from directory
            break;
        default:
            dbgassert(false, "Unknown line state in directory control\n");
            break;
        }
    }
        break;
    case DirectoryControl::ReWrite:
        break;
    default:
        dbgassert(false, "Unknown state in directory control\n");
        break;
    }

    #pragma hls_unroll yes
    for(int i(0); i < COMET_CORE;  ++i)
        out_dc[i] = dc[i];
}


#include <iostream>
#include <bitset>
#include <string>
#include <cstdio>

#ifdef __HLS__
typedef long long int64_t;
typedef unsigned long long uint64_t;
#endif

using namespace std;

CCS_MAIN(int argc, char**argv)
{
#ifndef nocache
    printf("Parameters : %5s   %8s   %13s   %4s   %6s   %13s    %13s\n", "Size", "Blocksize", "Associativity", "Sets", "Policy", "icontrolwidth", "dcontrolwidth");
    printf("Parameters : %5d   %8d   %13d   %4d   %6d   %13d    %13d\n", Size, 4*Blocksize, Associativity, Sets, Policy, ICacheControlWidth, DCacheControlWidth);
#endif

    (void)argc;
    (void)argv;

    CoherenceCacheToDirectory cd[COMET_CORE];
    CoherenceDirectoryToCache dc[COMET_CORE];

    for(int i(0); i < COMET_CORE; ++i)
    {
        cd[i] = CoherenceCacheToDirectory();
        dc[i] = CoherenceDirectoryToCache();
    }

    int64_t i = 0;

    while(i < 1e7)
    {
        CCS_DESIGN(directory(cd, dc
      #ifndef __HLS__
        , 0
      #endif
        ));

        ++i;
    }

    /*printf("%d\n", DirectoryTag);
    printf("%8s --> %6s %4s\n", "address", "tag", "set");
    for(i = 0; i < 10; ++i)
    {
        ac_int<32, false> address = rand();
        ac_int<32, false> rebuild = -1;
        setDTag(rebuild, getDTag(address));
        setDSet(rebuild, getDSet(address));

        cout << std::bitset<32>(address.to_int()) << " --> \n"
             << std::bitset<DirectoryTag>(getDTag(address).to_int()) << endl
             << string(DirectoryTag, ' ') << std::bitset<32-DirectoryTag-setshift>(getDSet(address).to_int()) << endl
             << std::bitset<32>(rebuild.to_int()) << endl;

    }*/

    CCS_RETURN(0);
}

