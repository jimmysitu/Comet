#include "mc_scverify.h"

#include "coherence.h"
#include "cache.h"


#include "simulator.h"


/********************************************************************
 * switch of switch of switch is kinda ugly                         *
 * Can we represent it with a table of transition? But no function  *
 * pointer                                                          *
 * Maybe a 3d table that outputs a number and switch on this number?*
 * So in the end we have only one big switch (around 30/40 cases)   *
 ********************************************************************/



void directory(unsigned int mem[DRAM_SIZE],
               CoherenceCacheToDirectory in_cd[COMET_CORE],
               CoherenceDirectoryToCache out_dc[COMET_CORE]
            #ifndef __HLS__
               , Simulator* sim
            #endif
               )
{
    static DirectoryControl ctrl;

    CoherenceCacheToDirectory cd[COMET_CORE];
    CoherenceDirectoryToCache dc[COMET_CORE];
    /// Unconditionnal read
    #pragma hls_unroll yes
    input:for(int i(0); i < COMET_CORE;  ++i)
        cd[i] = in_cd[i];

    switch(ctrl.state)
    {
    case DirectoryControl::Idle:
    {
        ac_int<ac::log2_ceil<COMET_CORE>::val, false> i = ctrl.reqcore;
        // this structure prevents low id cores to be prioritized over high id cores
        #pragma hls_unroll yes
        do {
            i++;
            if(cd[i].type != CoherenceCacheToDirectory::None)
            {
                ctrl.reqcore = i;
                //break;
            }
        } while(i != ctrl.reqcore);

        if(cd[i].type != CoherenceCacheToDirectory::None)
        {
            ac_int<32, false> address = cd[i].address;
            ctrl.line = ctrl.lines[getDSet(address)];
            ctrl.cd = cd[i];

            if(ctrl.line.tag == getDTag(address))
                ctrl.state = DirectoryControl::AppropriateReply;
            else
                ctrl.state = DirectoryControl::FirstFetchMem;

            // Acknowledge so the cache knows we are servicing its request
            dc[i].type = CoherenceDirectoryToCache::Ack;
            dc[i].address = address;
            dc[i].data = 0;
        }
        else
            ctrl.state = DirectoryControl::Idle;

    }
        break;
    case DirectoryControl::AppropriateReply:
    {
        //ac_int<ac::log2_ceil<COMET_CORE>::val, false> i = ctrl.fetchcore;
        ac_int<ac::log2_ceil<COMET_CORE>::val, false> tmpi = 0;
        bool foundfetcher = false;
        #pragma hls_unroll yes
        dispatch:for(int i(0); i < COMET_CORE; ++i)    // send appropriate request to sharers of line
        {
            if(i != ctrl.reqcore && ctrl.line.sharers[i])
                switch(ctrl.cd.type)
                {
                case CoherenceCacheToDirectory::ReadMiss:
                    if(!foundfetcher)
                    {   // only one fetcher
                        dc[i].address = ctrl.cd.address;
                        dc[i].type = CoherenceDirectoryToCache::Fetch;
                        dc[i].data = 0;
                        tmpi = i;
                        foundfetcher = true;
                    }
                    break;
                case CoherenceCacheToDirectory::WriteMiss:
                    if(foundfetcher)
                    {
                        dc[i].type = CoherenceDirectoryToCache::Invalidate;
                    }
                    else
                    {   // only one fetcher, other cache only invalidate
                        dc[i].type = CoherenceDirectoryToCache::FetchInvalidate;
                        tmpi = i;
                        foundfetcher = true;
                    }
                    dc[i].address = ctrl.cd.address;
                    dc[i].data = 0;
                    break;
                case CoherenceCacheToDirectory::WriteInvalidate:
                    // all core invalidate
                    dc[i].address = ctrl.cd.address;
                    dc[i].type = CoherenceDirectoryToCache::Invalidate;
                    dc[i].data = 0;
                    break;
                default:
                    dbgassert(ctrl.cd.type != CoherenceCacheToDirectory::Ack &&
                              ctrl.cd.type != CoherenceCacheToDirectory::Reply,
                              "Wrong type of request @%06x for appropriate reply in directory control\n", ctrl.cd.address.to_int());
                    break;
                }
        }

        if(foundfetcher)
        {
            ctrl.fetchcore = tmpi;
        }

        switch(ctrl.line.state)
        {
        case LineCoherence::Invalid:
            /// No cache has the requested data
            /// valid request are ReadMiss, forward data from main memory to cache
            ///                   WriteMiss, forward data from main memory to cache
            /// sharers = P

            dbgassert(ctrl.cd.type == CoherenceCacheToDirectory::ReadMiss ||
                      ctrl.cd.type == CoherenceCacheToDirectory::WriteMiss,
                      "Invalid transition for invalid line @%06x in appropriate reply\n", ctrl.cd.address.to_int());
            dbgassert(ctrl.line.sharers == 0, "Sharers not null on invalid line @%06x\n", ctrl.cd.address.to_int());

            ctrl.valuetowrite = mem[ctrl.cd.address.slc<30>(2)];
            ctrl.i = getOffset(ctrl.cd.address);
            ctrl.state = DirectoryControl::FetchMem;
            break;
        case LineCoherence::Shared:
            /// At least one cache has the data
            /// valid request are ReadMiss, forward data from one cache to another
            ///                   WriteMiss, forward data from one cache to another and invalidate
            ///                   WriteInvalidate, invalidate other cache data
            ///                   WriteBack, remove one sharer
            /// update sharers accordingly

            dbgassert(ctrl.cd.type == CoherenceCacheToDirectory::ReadMiss ||
                      ctrl.cd.type == CoherenceCacheToDirectory::WriteMiss ||
                      ctrl.cd.type == CoherenceCacheToDirectory::WriteInvalidate ||
                      ctrl.cd.type == CoherenceCacheToDirectory::WriteBack,
                      "Invalid transition for shared line @%06x in appropriate reply\n", ctrl.cd.address.to_int());



            // Appropriate requests have already been sent to sharers
            if(ctrl.cd.type == CoherenceCacheToDirectory::WriteInvalidate)
            {
                ctrl.line.sharers = 0;
                ctrl.line.sharers[ctrl.reqcore] = true;
                ctrl.state = DirectoryControl::StoreControl;
            }
            else if(ctrl.cd.type == CoherenceCacheToDirectory::WriteBack)
            {
                ctrl.line.sharers[ctrl.reqcore] = false;
                if(ctrl.line.sharers == 0)
                    ctrl.line.state = LineCoherence::Invalid;
                ctrl.state = DirectoryControl::StoreControl;
            }
            else
                ctrl.state = DirectoryControl::FetchCache;
            //ctrl.i = 0;

            break;
        case LineCoherence::Modified:
        /// valid request are ReadMiss, forward data from one cache to another, writeback to mem?
        ///                   WriteMiss, forward data from one cache to another, no writeback
        ///                   WriteBack, forward data from one cache to main memory, remove line from directory

            dbgassert(ctrl.cd.type == CoherenceCacheToDirectory::ReadMiss ||
                      ctrl.cd.type == CoherenceCacheToDirectory::WriteMiss ||
                      ctrl.cd.type == CoherenceCacheToDirectory::WriteBack,
                      "Invalid transition for modified line @%06x in appropriate reply\n", ctrl.cd.address.to_int());

            // Appropriate request has already been sent to owner
            if(ctrl.cd.type == CoherenceCacheToDirectory::WriteBack)
            {
                ctrl.line.state = LineCoherence::Invalid;
                ctrl.line.sharers = 0;
                ctrl.state = DirectoryControl::StoreControl;
            }
            else
                ctrl.state = DirectoryControl::FetchCache;
            //ctrl.i = 0;

            break;
        default:
            dbgassert(ctrl.line.state == LineCoherence::Invalid ||
                      ctrl.line.state == LineCoherence::Shared ||
                      ctrl.line.state == LineCoherence::Modified,
                      "Unknown state for line @%06x in appropriate reply\n", ctrl.cd.address.to_int());
            break;
        }

    }
        break;
    case DirectoryControl::FirstFetchMem:
        ctrl.valuetowrite = mem[ctrl.cd.address >> 2];
        ctrl.i = getOffset(ctrl.cd.address);
        break;
    case DirectoryControl::FetchMem:
    {
        ac_int<32, false> bytead = ctrl.cd.address;
        setOffset(bytead, ctrl.i);
        dc[ctrl.reqcore].address = bytead;
        dc[ctrl.reqcore].type = CoherenceDirectoryToCache::Reply;
        dc[ctrl.reqcore].data = ctrl.valuetowrite;
        if(++ctrl.i != getOffset(ctrl.cd.address))
        {
            setOffset(bytead, ctrl.i);
            ctrl.valuetowrite = mem[bytead >> 2];
        }
        else
        {
            ctrl.state = DirectoryControl::StoreControl;
            ctrl.line.tag = getDTag(ctrl.cd.address);
            ctrl.line.sharers[ctrl.reqcore] = true;
            if(ctrl.cd.type == CoherenceCacheToDirectory::ReadMiss)
                ctrl.line.state = LineCoherence::Shared;
            else
                ctrl.line.state = LineCoherence::Modified;
        }
    }
        break;
    case DirectoryControl::FetchCache:
        // forward to memory as well if ReadMiss from Modified state
        if(cd[ctrl.fetchcore].type == CoherenceCacheToDirectory::Reply)
        {   // count to make sure we got full block?
            dc[ctrl.reqcore].type = CoherenceDirectoryToCache::Reply;
            dc[ctrl.reqcore].address = cd[ctrl.fetchcore].address;
            dc[ctrl.reqcore].data = cd[ctrl.fetchcore].data;
        }
        else if(cd[ctrl.fetchcore].type == CoherenceCacheToDirectory::Ack)//None?
        {
            dc[ctrl.reqcore].type = CoherenceDirectoryToCache::None;//Ack?
            dc[ctrl.reqcore].address = 0;
            dc[ctrl.reqcore].data = 0;
            ctrl.state = DirectoryControl::StoreControl;

            ctrl.line.tag = getDTag(ctrl.cd.address);
            if(ctrl.cd.type == CoherenceCacheToDirectory::WriteMiss)
            {
                ctrl.line.sharers = 0;
                ctrl.line.state = LineCoherence::Modified;
            }
            else
                ctrl.line.state = LineCoherence::Shared;

            ctrl.line.sharers[ctrl.reqcore] = true;
        }

        break;
    case DirectoryControl::WriteMem:
        if(cd[ctrl.fetchcore].type == CoherenceCacheToDirectory::Reply)
        {   // count to make sure we got full block?
            mem[cd[ctrl.fetchcore].address >> 2] = cd[ctrl.fetchcore].data;
        }
        else if(cd[ctrl.fetchcore].type == CoherenceCacheToDirectory::Ack)//None?
        {
            dc[ctrl.reqcore].type = CoherenceDirectoryToCache::None;//Ack?
            dc[ctrl.reqcore].address = 0;
            dc[ctrl.reqcore].data = 0;
            ctrl.state = DirectoryControl::StoreControl;

            ctrl.line.tag = getDTag(ctrl.cd.address);
            if(ctrl.cd.type == CoherenceCacheToDirectory::WriteMiss)
            {
                ctrl.line.sharers = 0;
                ctrl.line.state = LineCoherence::Modified;
            }
            else
                ctrl.line.state = LineCoherence::Shared;

            ctrl.line.sharers[ctrl.reqcore] = true;
        }
        break;
    case DirectoryControl::StoreControl:
        ctrl.lines[getDSet(ctrl.cd.address)] = ctrl.line;
        ctrl.state = DirectoryControl::Idle;
        break;
    default:
        dbgassert(false, "Unknown state in directory control\n");
        break;
    }

    /// Unconditionnal write
    #pragma hls_unroll yes
    output:for(int i(0); i < COMET_CORE;  ++i)
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

void createCacheMemories(ac_int<DWidth, false>** memdctrl, unsigned int** cdm)
{
    *cdm = new unsigned int[Sets*Blocksize*Associativity];

    *memdctrl = new ac_int<DWidth, false>[Sets];
}

void deleteCacheMemories(ac_int<DWidth, false>** memdctrl, unsigned int** cdm)
{
    delete[] *cdm;
    delete[] *memdctrl;

    *memdctrl = 0;
    *cdm = 0;
}

#define ptrtocache(mem) (*reinterpret_cast<unsigned int (*)[Sets][Blocksize][Associativity]>(mem))

CCS_MAIN(int argc, char**argv)
{
#ifndef nocache
    printf("Parameters : %5s   %8s   %13s   %4s   %6s   %13s    %13s\n", "Size", "Blocksize", "Associativity", "Sets", "Policy", "icontrolwidth", "dcontrolwidth");
    printf("Parameters : %5d   %8d   %13d   %4d   %6d   %13d    %13d\n", Size, 4*Blocksize, Associativity, Sets, Policy, ICacheControlWidth, DCacheControlWidth);
#endif

    (void)argc;
    (void)argv;

    Simulator* sim = new Simulator();
    unsigned int* mem = new unsigned int[DRAM_SIZE];
    CoherenceCacheToDirectory cd[COMET_CORE];
    CoherenceDirectoryToCache dc[COMET_CORE];
    DCacheRequest dreq[COMET_CORE];
    DCacheReply drep[COMET_CORE];
    unsigned int** cdm = new unsigned int*[COMET_CORE];
    ac_int<DWidth, false>** memdctrl = new ac_int<DWidth, false>*[COMET_CORE];

    for(int i(0); i < COMET_CORE; ++i)
    {
        cd[i] = CoherenceCacheToDirectory();
        dc[i] = CoherenceDirectoryToCache();
        createCacheMemories(&memdctrl[i], &cdm[i]);
        // zero the control (although only the valid bit should be zeroed, rest is don't care)
        for(int j(0); j < Sets; ++j)
        {
            memdctrl[i][j] = 0;
        }
    }

    int64_t i = 0;

    while(i < 1e7)
    {
        dcache<0>(memdctrl[0], mem, ptrtocache(cdm[0]), dreq[0], drep[0], sim);
        dcache<1>(memdctrl[1], mem, ptrtocache(cdm[1]), dreq[1], drep[1], sim);
        CCS_DESIGN(directory(mem, cd, dc
      #ifndef __HLS__
        , sim
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

    for(int i(0); i < COMET_CORE; ++i)
        deleteCacheMemories(&memdctrl[i], &cdm[i]);
    delete[] cdm;
    delete[] memdctrl;
    delete[] mem;
    delete sim;

    CCS_RETURN(0);
}

