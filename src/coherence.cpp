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

int cycle = 0;

const char* cdtostr(CoherenceCacheToDirectory::Type t)
{
    switch(t)
    {
    case CoherenceCacheToDirectory::None:
        return "None";
    case CoherenceCacheToDirectory::Ack:
        return "Ack";
    case CoherenceCacheToDirectory::Reply:
        return "Reply";
    case CoherenceCacheToDirectory::ReadMiss:
        return "ReadMiss";
    case CoherenceCacheToDirectory::WriteMiss:
        return "WriteMiss";
    case CoherenceCacheToDirectory::WriteInvalidate:
        return "WriteInvalidate";
    case CoherenceCacheToDirectory::RemoveData:
        return "RemoveData";
    case CoherenceCacheToDirectory::DataWriteBack:
        return "DataWriteBack";
    default:
        return "Unknown";
    }
}

const char* dctostr(CoherenceDirectoryToCache::Type t)
{
    switch(t)
    {
    case CoherenceDirectoryToCache::None:
        return "None";
    case CoherenceDirectoryToCache::Ack:
        return "Ack";
    case CoherenceDirectoryToCache::Reply:
        return "Reply";
    case CoherenceDirectoryToCache::Invalidate:
        return "Invalidate";
    case CoherenceDirectoryToCache::Fetch:
        return "Fetch";
    case CoherenceDirectoryToCache::FetchInvalidate:
        return "FetchInvalidate";
    case CoherenceDirectoryToCache::Abort:
        return "Abort";
    default:
        return "Unknown";
    }
}

const char* linestatetostr(LineCoherence::DCoherenceState s)
{
    switch(s)
    {
    case LineCoherence::Invalid:
        return "Invalid";
    case LineCoherence::Shared:
        return "Shared";
    case LineCoherence::Modified:
        return "Modified";
    default:
        return "Unknown";
    }
}

const char* dirstatetostr(DirectoryControl::State s)
{
    switch(s)
    {
    case DirectoryControl::Idle:
        return "Idle";
    case DirectoryControl::AppropriateReply:
        return "AppropriateReply";
    case DirectoryControl::FirstFetchMem:
        return "FirstFetchMem";
    case DirectoryControl::FetchMem        :
        return "FetchMem";
    case DirectoryControl::FetchCache      :
        return "FetchCache";
    case DirectoryControl::WriteMem        :
        return "WriteMem";
    case DirectoryControl::StoreControl    :
        return "StoreControl";
    case DirectoryControl::WaitForAck      :
        return "WaitForAck";
    default:
        return "Unknown";
    }
}

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
            printf("checking core %d...", i.to_int());
            if(cd[i].type != CoherenceCacheToDirectory::None)
            {
                ctrl.reqcore = i;
                printf("request accepted");
                //break;
            }
            printf("\n");
        } while(i != ctrl.reqcore);

        if(cd[i].type != CoherenceCacheToDirectory::None)
        {
            ac_int<32, false> address = cd[i].address;
            printf("%d: found request %s from core %d @%06x", cycle, cdtostr(cd[i].type),
                    i.to_int(), address.to_int());
            ctrl.line = ctrl.lines[getDSet(address)];
            ctrl.cd = cd[i];

            if(ctrl.line.tag == getDTag(address) && ctrl.line.state != LineCoherence::Invalid)
            {
                ctrl.state = DirectoryControl::AppropriateReply;
                printf(" found in at least one cache\n");
            }
            else
            {
                ctrl.state = DirectoryControl::FirstFetchMem;
                printf(" not found in cache\n");
            }
            printf("%d directory reading control %d : @%06x    %d      %s\n", cycle, getDSet(address).to_int(), ctrl.line.tag.to_int(),
                   ctrl.line.sharers.to_int(), linestatetostr(ctrl.line.state));

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
            {
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
                    if(!foundfetcher)
                    {   // only one fetcher, other cache only invalidate
                        dc[i].type = CoherenceDirectoryToCache::FetchInvalidate;
                        tmpi = i;
                        foundfetcher = true;
                    }
                    else
                    {
                        dc[i].type = CoherenceDirectoryToCache::Invalidate;
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
                    dbgassert(ctrl.cd.type == CoherenceCacheToDirectory::ReadMiss ||
                              ctrl.cd.type == CoherenceCacheToDirectory::WriteMiss||
                              ctrl.cd.type == CoherenceCacheToDirectory::WriteInvalidate,
                              "Wrong type of request @%06x for appropriate reply in directory control\n", ctrl.cd.address.to_int());
                    break;
                }
            }
        }

        if(foundfetcher)
            ctrl.fetchcore = tmpi;

        ctrl.ackers = ctrl.line.sharers;
        ctrl.ackers[ctrl.reqcore] = false;

        switch(ctrl.line.state)
        {
        case LineCoherence::Invalid:
            // i think we should never reach this point because if line is invalid, we jump to firstfetchmem, not dispatch
            dbgassert(false, "Impossible?\n");
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
            if(ctrl.cd.type == CoherenceCacheToDirectory::ReadMiss)
                ctrl.line.state = LineCoherence::Shared;
            else
                ctrl.line.state = LineCoherence::Modified;
            ctrl.line.sharers[ctrl.reqcore] = true;
            ctrl.line.tag = getDTag(ctrl.cd.address);
            break;
        case LineCoherence::Shared:
            /// At least one cache has the data
            /// valid request are ReadMiss, forward data from one cache to another
            ///                   WriteMiss, forward data from one cache to another and invalidate
            ///                   WriteInvalidate, invalidate other cache data
            ///                   RemoveData, remove one sharer
            /// update sharers accordingly

            // Appropriate requests have already been sent to sharers
            switch(ctrl.cd.type)
            {
            case CoherenceCacheToDirectory::ReadMiss:
                ctrl.line.sharers[ctrl.reqcore] = true;
                ctrl.line.state = LineCoherence::Shared;
                ctrl.mem = false;
                ctrl.state = DirectoryControl::FetchCache;
                break;
            case CoherenceCacheToDirectory::WriteMiss:
                ctrl.line.sharers = 0;
                ctrl.line.sharers[ctrl.reqcore] = true;
                ctrl.line.state = LineCoherence::Modified;
                ctrl.mem = false;
                ctrl.state = DirectoryControl::FetchCache;
                break;
            case CoherenceCacheToDirectory::WriteInvalidate:
                ctrl.line.sharers = 0;
                ctrl.line.sharers[ctrl.reqcore] = true;
                ctrl.line.state = LineCoherence::Modified;
                ctrl.state = DirectoryControl::WaitForAck;
                break;
            case CoherenceCacheToDirectory::RemoveData:
                ctrl.line.sharers[ctrl.reqcore] = false;
                if(ctrl.line.sharers == 0)
                    ctrl.line.state = LineCoherence::Invalid;
                // no need to write back, line is shared so non dirty
                ctrl.state = DirectoryControl::StoreControl;
                break;
            default:
                dbgassert(ctrl.cd.type == CoherenceCacheToDirectory::ReadMiss        ||
                          ctrl.cd.type == CoherenceCacheToDirectory::WriteMiss       ||
                          ctrl.cd.type == CoherenceCacheToDirectory::WriteInvalidate ||
                          ctrl.cd.type == CoherenceCacheToDirectory::RemoveData,
                          "Invalid transition for shared line @%06x in appropriate reply\n", ctrl.cd.address.to_int());
                break;
            }

            break;
        case LineCoherence::Modified:
        /// valid request are ReadMiss, forward data from one cache to another, writeback to mem?
        ///                   WriteMiss, forward data from one cache to another, no writeback
        ///                   DataWriteBack, forward data from one cache to main memory, remove line from directory

            // Appropriate request has already been sent to owner
            switch(ctrl.cd.type)
            {
            case CoherenceCacheToDirectory::ReadMiss:
                ctrl.line.state = LineCoherence::Shared;
                ctrl.line.sharers[ctrl.reqcore] = true;
                ctrl.mem = true;
                ctrl.state = DirectoryControl::FetchCache;
                printf("some debug %d   %s\n", ctrl.line.sharers.to_int(), linestatetostr(ctrl.line.state));
                break;
            case CoherenceCacheToDirectory::WriteMiss:
                ctrl.line.state = LineCoherence::Modified;
                ctrl.line.sharers = 0;
                ctrl.line.sharers[ctrl.reqcore] = true;
                ctrl.mem = false;
                ctrl.state = DirectoryControl::FetchCache;
                break;
            case CoherenceCacheToDirectory::DataWriteBack:
                ctrl.line.state = LineCoherence::Invalid;
                ctrl.line.sharers = 0;
                ctrl.state = DirectoryControl::WriteMem;
                break;
            default:
                dbgassert(ctrl.cd.type == CoherenceCacheToDirectory::ReadMiss  ||
                          ctrl.cd.type == CoherenceCacheToDirectory::WriteMiss ||
                          ctrl.cd.type == CoherenceCacheToDirectory::DataWriteBack,
                          "Invalid transition for modified line @%06x in appropriate reply\n", ctrl.cd.address.to_int());
                break;
            }

            break;
        default:
            dbgassert(ctrl.line.state == LineCoherence::Invalid ||
                      ctrl.line.state == LineCoherence::Shared  ||
                      ctrl.line.state == LineCoherence::Modified,
                      "Unknown state for line @%06x in appropriate reply\n", ctrl.cd.address.to_int());
            break;
        }

    }
        break;
    case DirectoryControl::FirstFetchMem:
        dbgassert(ctrl.cd.type == CoherenceCacheToDirectory::ReadMiss ||
                  ctrl.cd.type == CoherenceCacheToDirectory::WriteMiss,
                  "Invalid transition for invalid line @%06x in first fetch\n", ctrl.cd.address.to_int());
        dbgassert(ctrl.line.sharers == 0, "Sharers not null on invalid line @%06x\n", ctrl.cd.address.to_int());

        ctrl.valuetowrite = mem[ctrl.cd.address >> 2];
        ctrl.i = getOffset(ctrl.cd.address);
        if(ctrl.cd.type == CoherenceCacheToDirectory::ReadMiss)
            ctrl.line.state = LineCoherence::Shared;
        else
            ctrl.line.state = LineCoherence::Modified;
        ctrl.line.sharers[ctrl.reqcore] = true;
        ctrl.line.tag = getDTag(ctrl.cd.address);
        ctrl.state = DirectoryControl::FetchMem;
        break;
    case DirectoryControl::FetchMem:
    {
        ac_int<32, false> bytead = ctrl.cd.address;
        setOffset(bytead, ctrl.i);
        dc[ctrl.reqcore].address = bytead;
        dc[ctrl.reqcore].data = ctrl.valuetowrite;
        if(++ctrl.i != getOffset(ctrl.cd.address))
        {
            dc[ctrl.reqcore].type = CoherenceDirectoryToCache::Reply;
            setOffset(bytead, ctrl.i);
            ctrl.valuetowrite = mem[bytead >> 2];
        }
        else
        {
            dc[ctrl.reqcore].type = CoherenceDirectoryToCache::Ack;
            ctrl.state = DirectoryControl::StoreControl;
        }
    }
        break;
    case DirectoryControl::FetchCache:
        #pragma hls_unroll yes
        fetchcache:for(int i(0); i < COMET_CORE; ++i)
        {
            if(cd[i].type == CoherenceCacheToDirectory::Ack)
                ctrl.ackers[i] = false;
        }

        switch(cd[ctrl.fetchcore].type)
        {
        case CoherenceCacheToDirectory::Reply:
            // count to make sure we got full block?
            dc[ctrl.reqcore].type = CoherenceDirectoryToCache::Reply;
            dc[ctrl.reqcore].address = cd[ctrl.fetchcore].address;
            dc[ctrl.reqcore].data = cd[ctrl.fetchcore].data;
            printf("%d dir received @%06x in FetchCache from %d and sending to %d\n", cycle,
                    cd[ctrl.fetchcore].address.to_int(), ctrl.fetchcore.to_int(), ctrl.reqcore.to_int());

            if(ctrl.mem)
                mem[cd[ctrl.fetchcore].address >> 2] = cd[ctrl.fetchcore].data;
            break;
        case CoherenceCacheToDirectory::Ack:
            dc[ctrl.reqcore].type = CoherenceDirectoryToCache::Ack;
            dc[ctrl.reqcore].address = cd[ctrl.fetchcore].address;
            dc[ctrl.reqcore].data = cd[ctrl.fetchcore].data;
            printf("%d dir received last @%06x in FetchCache from %d and sending to %d\n", cycle,
                    cd[ctrl.fetchcore].address.to_int(), ctrl.fetchcore.to_int(), ctrl.reqcore.to_int());

            if(ctrl.mem)
                mem[cd[ctrl.fetchcore].address >> 2] = cd[ctrl.fetchcore].data;
            ctrl.state = DirectoryControl::StoreControl;

            dbgassert(ctrl.ackers == 0, "Not all cache acked @%06x\n", ctrl.cd.address.to_int());
            break;
        case CoherenceCacheToDirectory::None:
            break;
        default:
            dbgassert(cd[ctrl.fetchcore].type == CoherenceCacheToDirectory::Reply ||
                      cd[ctrl.fetchcore].type == CoherenceCacheToDirectory::Ack   ||
                      cd[ctrl.fetchcore].type == CoherenceCacheToDirectory::None,
                      "%d Unexpected type in FetchCache\n", cycle);
            break;
        }

        break;
    case DirectoryControl::WriteMem:
        if(cd[ctrl.fetchcore].type == CoherenceCacheToDirectory::Reply)
        {   // count to make sure we got full block?
            mem[cd[ctrl.fetchcore].address >> 2] = cd[ctrl.fetchcore].data;
        }
        else if(cd[ctrl.fetchcore].type == CoherenceCacheToDirectory::Ack)
        {
            dc[ctrl.reqcore].type = CoherenceDirectoryToCache::Ack;//Ack?
            dc[ctrl.reqcore].address = 0;
            dc[ctrl.reqcore].data = 0;
            ctrl.state = DirectoryControl::StoreControl;

            dbgassert(ctrl.fetchcore == ctrl.reqcore, "WriteBack fetched from non requesting core @%06x\n", ctrl.cd.address.to_int());
        }
        break;
    case DirectoryControl::WaitForAck:
    {
        ac_int<COMET_CORE, false> numsharers = 0;
        #pragma hls_unroll yes
        waitforack:for(int i(0); i < COMET_CORE; ++i)
        {
            if(cd[i].type == CoherenceCacheToDirectory::Ack)
                ctrl.ackers[i] = false;

            numsharers += ctrl.ackers[i];
        }
        if(ctrl.ackers == 0)
            ctrl.state = DirectoryControl::StoreControl;

        printf("Waiting for ack @%06x (%d missing)\n", ctrl.cd.address.to_int(), numsharers.to_int());
     }
        break;
    case DirectoryControl::StoreControl:
        printf("%d directory writing control %d : @%06x    %d      %s\n", cycle, getDSet(ctrl.cd.address).to_int(),
               ctrl.cd.address.to_int(), ctrl.line.sharers.to_int(), linestatetostr(ctrl.line.state));
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

    printf("%d dir state %s\n", cycle, dirstatetostr(ctrl.state));
}


#include <iostream>
#include <bitset>
#include <string>
#include <cstdio>

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

class SimpleCacheSimulator
{
public:
    void step(DCacheRequest corereq, DCacheReply& crep, CoherenceDirectoryToCache dirreq, CoherenceCacheToDirectory& drep)
    {
        bool datavalid = false;
        DCacheReply corerep;
        CoherenceCacheToDirectory dirrep;
        switch(state)
        {
        case Idle:
            // Coherence has a higher priority than core and so is treated first
            if(dirreq.type != CoherenceDirectoryToCache::None)
            {
                ac_int<32, false> address = dirreq.address;

                if(find(address))
                    printf("%d cache %d : Directory request %s @%06x\n", cycle, id, dctostr(dirreq.type), address.to_int());
                else
                    dbgassert(find(address), "Cache %d : Received a request, but don't have the data @%06x\n", id, address.to_int());

                workaddress = address;
                switch(dirreq.type)
                {
                case CoherenceDirectoryToCache::Fetch:
                    state = DirWriteBack;
                    ctrli = getOffset(address)+1;
                    dirrep.address = address;
                    dirrep.type = CoherenceCacheToDirectory::Reply;
                    dirrep.data = 0;
                    printf("%d cache %d : sending @%06x to directory\n", cycle, id, dirrep.address.to_int());
                    break;
                case CoherenceDirectoryToCache::FetchInvalidate:
                    state = DirWriteBack;
                    valid[getSet(address)][currentway] = false;
                    ctrli = getOffset(address)+1;
                    dirrep.address = address;
                    dirrep.type = CoherenceCacheToDirectory::Reply;
                    dirrep.data = 0;
                    printf("%d cache %d : sending @%06x to directory\n", cycle, id, dirrep.address.to_int());
                    break;
                case CoherenceDirectoryToCache::Invalidate:
                    state = Idle;
                    valid[getSet(address)][currentway] = false;
                    dirrep.address = address;
                    dirrep.type = CoherenceCacheToDirectory::Ack;
                    dirrep.data = 0;
                    break;
                default:
                    dbgassert(dirreq.type == CoherenceDirectoryToCache::Fetch ||
                              dirreq.type == CoherenceDirectoryToCache::FetchInvalidate ||
                              dirreq.type == CoherenceDirectoryToCache::Invalidate,
                              "Cache %d : Wrong type of request %s from directory @%06x\n", id,
                              dctostr(dirreq.type), address.to_int());
                    break;
                }
            }
            else if(corereq.dcacheenable)
            {
                ac_int<32, false> address = corereq.address;
                printf("%d cache %d : Cache request @%06x for %s\n", cycle, id, address.to_int(),
                       corereq.writeenable?"W":"R");

                if(find(address))
                {
                    printf("%d cache %d : found @%06x\n", cycle, id, address.to_int());

                    if(corereq.writeenable && !dirty[getSet(address)][currentway])
                    {
                        dirrep.address = address;
                        workaddress = address;
                        dirrep.type = CoherenceCacheToDirectory::WriteInvalidate;
                        state = WaitReply;
                        datavalid = false;
                    }
                    else
                        datavalid = true;
                    // updatepolicy
                }
                else
                {
                    printf("%d cache %d : miss @%06x\n", cycle, id, address.to_int());
                    datavalid = false;
                    // select policy
                    if(valid[getSet(address)][currentway] && dirty[getSet(address)][currentway])
                    {
                        state = FirstWriteBack;
                        dirrep.address = 0;
                        setTag(dirrep.address, tag[getSet(address)][currentway]);

                        dirrep.type = CoherenceCacheToDirectory::DataWriteBack;
                        ctrli = 0;

                        valid[getSet(address)][currentway] = false;
                        dirty[getSet(address)][currentway] = false;
                    }
                    else
                    {
                        state = FirstFetch;
                        dirrep.address = address;
                        if(corereq.writeenable)
                            dirrep.type = CoherenceCacheToDirectory::WriteMiss;
                        else
                            dirrep.type = CoherenceCacheToDirectory::ReadMiss;
                    }
                    workaddress = address;
                }
                lastrequest = dirrep;
            }
            break;
        case FirstFetch:
            switch(dirreq.type)
            {
            case CoherenceDirectoryToCache::Ack:
                // we are being serviced
                state = DirFetch;
                printf("%d cache %d : serviced for @%06x\n", cycle, id, workaddress.to_int());
                break;
            case CoherenceDirectoryToCache::None:
                // directory is probably busy handling other request
                dirrep = lastrequest;
                printf("%d cache %d : waiting for @%06x\n", cycle, id, workaddress.to_int());
                break;
            case CoherenceDirectoryToCache::Fetch:
                workaddress = dirreq.address;
                state = DirWriteBack;
                ctrli = getOffset(dirreq.address)+1;
                dirrep.address = dirreq.address;
                dirrep.type = CoherenceCacheToDirectory::Reply;
                dirrep.data = 0;
                printf("%d cache %d : sending @%06x to directory\n", cycle, id, dirrep.address.to_int());
                break;
            case CoherenceDirectoryToCache::FetchInvalidate:
                // we have the data, so invalidate it here
                if(find(dirreq.address))
                    valid[getSet(dirreq.address)][currentway] = false;
                else
                    dbgassert(find(dirreq.address), "Cache %d : Received a request, but don't have the data @%06x in %s\n", id, dirreq.address.to_int(), cachestatetostr(state));

                workaddress = dirreq.address;
                state = DirWriteBack;
                ctrli = getOffset(dirreq.address)+1;
                dirrep.address = dirreq.address;
                dirrep.type = CoherenceCacheToDirectory::Reply;
                dirrep.data = 0;
                printf("%d cache %d : sending @%06x to directory\n", cycle, id, dirrep.address.to_int());

                break;
            case CoherenceDirectoryToCache::Invalidate:
                // we have the data, so invalidate it here
                if(find(dirreq.address))
                    valid[getSet(dirreq.address)][currentway] = false;
                else
                    dbgassert(find(dirreq.address), "Cache %d : Received a request, but don't have the data @%06x in %s\n", id, dirreq.address.to_int(), cachestatetostr(state));

                state = Idle;
                valid[getSet(dirreq.address)][currentway] = false;
                dirrep.address = dirreq.address;
                dirrep.type = CoherenceCacheToDirectory::Ack;
                dirrep.data = 0;

                break;
            default:
                dbgassert(dirreq.type == CoherenceDirectoryToCache::Ack ||
                          dirreq.type == CoherenceDirectoryToCache::None ||
                          dirreq.type == CoherenceDirectoryToCache::Fetch ||
                          dirreq.type == CoherenceDirectoryToCache::FetchInvalidate ||
                          dirreq.type == CoherenceDirectoryToCache::Invalidate,
                          "%d cache %d : Invalid request from directory control @%06x\n",
                          cycle, id, dirreq.address.to_int());
                break;
            }
            break;
        case DirFetch:
            if(dirreq.type == CoherenceDirectoryToCache::Reply)
            {
                if(dirreq.address == workaddress)
                    datavalid = true;
                // we receive data
                printf("%d cache %d : received @%06x\n", cycle, id, dirreq.address.to_int());
            }
            else if(dirreq.type == CoherenceDirectoryToCache::None)
            {
                // we should wait
                dirrep = lastrequest;
                printf("%d cache %d : waiting for data\n", cycle, id);
            }
            else if(dirreq.type == CoherenceDirectoryToCache::Ack)
            {
                // transaction done
                printf("%d cache %d : received @%06x & fetching done\n", cycle, id, dirreq.address.to_int());
                valid[getSet(workaddress)][currentway] = true;
                dirty[getSet(workaddress)][currentway] = lastrequest.type==CoherenceCacheToDirectory::WriteMiss;
                tag[getSet(workaddress)][currentway] = getTag(workaddress);
                state = Idle;
            }
            break;
        case FirstWriteBack:
            switch(dirreq.type)
            {
            case CoherenceDirectoryToCache::Ack:
                // we are being serviced
                state = DirWriteBack;
                printf("%d cache %d : serviced for @%06x\n", cycle, id, workaddress.to_int());
                break;
            case CoherenceDirectoryToCache::None:
                // directory is probably busy handling other request
                dirrep = lastrequest;
                state = FirstWriteBack;
                printf("%d cache %d : waiting for @%06x\n", cycle, id, workaddress.to_int());
                break;
            case CoherenceDirectoryToCache::Fetch:
                workaddress = dirreq.address;
                state = DirWriteBack;
                ctrli = getOffset(dirreq.address)+1;
                dirrep.address = dirreq.address;
                dirrep.type = CoherenceCacheToDirectory::Reply;
                dirrep.data = 0;
                printf("%d cache %d : sending @%06x to directory\n", cycle, id, dirrep.address.to_int());
                break;
            case CoherenceDirectoryToCache::FetchInvalidate:
                // we have the data, so invalidate it here
                if(find(dirreq.address))
                    valid[getSet(dirreq.address)][currentway] = false;
                else
                    dbgassert(find(dirreq.address), "Cache %d : Received a request, but don't have the data @%06x in %s\n", id, dirreq.address.to_int(), cachestatetostr(state));

                workaddress = dirreq.address;
                state = DirWriteBack;
                ctrli = getOffset(dirreq.address)+1;
                dirrep.address = dirreq.address;
                dirrep.type = CoherenceCacheToDirectory::Reply;
                dirrep.data = 0;
                printf("%d cache %d : sending @%06x to directory\n", cycle, id, dirrep.address.to_int());

                break;
            case CoherenceDirectoryToCache::Invalidate:
                // we have the data, so invalidate it here
                if(find(dirreq.address))
                    valid[getSet(dirreq.address)][currentway] = false;
                else
                    dbgassert(find(dirreq.address), "Cache %d : Received a request, but don't have the data @%06x in %s\n", id, dirreq.address.to_int(), cachestatetostr(state));

                state = Idle;
                valid[getSet(dirreq.address)][currentway] = false;
                dirrep.address = dirreq.address;
                dirrep.type = CoherenceCacheToDirectory::Ack;
                dirrep.data = 0;

                break;
            default:
                dbgassert(dirreq.type == CoherenceDirectoryToCache::Ack ||
                          dirreq.type == CoherenceDirectoryToCache::None ||
                          dirreq.type == CoherenceDirectoryToCache::Fetch ||
                          dirreq.type == CoherenceDirectoryToCache::FetchInvalidate ||
                          dirreq.type == CoherenceDirectoryToCache::Invalidate,
                          "%d cache %d : Invalid request from directory control @%06x\n",
                          cycle, id, dirreq.address.to_int());
                break;
            }

            break;
        case DirWriteBack:
        {
            ac_int<ac::log2_ceil<Blocksize>::val, false> last = getOffset(workaddress)-1;
            dirrep.address = workaddress;
            setOffset(dirrep.address, ctrli);
            if(ctrli != last)
            {
                dirrep.type = CoherenceCacheToDirectory::Reply;
            }
            else
            {
                dirrep.type = CoherenceCacheToDirectory::Ack;
                state = Idle;
                //dirrep.data = ...
            }
            printf("%d cache %d : sending @%06x to directory\n", cycle, id, dirrep.address.to_int());
            ++ctrli;
        }
            break;
        case WaitReply:
            switch(dirreq.type)
            {
            case CoherenceDirectoryToCache::Ack:
                // here we can write the data
                state = Idle;
                dirty[getSet(workaddress)][currentway] = true;
                datavalid = true;
                // + update policy
                break;
            case CoherenceDirectoryToCache::None:
                dirrep = lastrequest;
                break;
            case CoherenceDirectoryToCache::Fetch:
                workaddress = dirreq.address;
                state = DirWriteBack;
                ctrli = getOffset(dirreq.address)+1;
                dirrep.address = dirreq.address;
                dirrep.type = CoherenceCacheToDirectory::Reply;
                dirrep.data = 0;
                printf("%d cache %d : sending @%06x to directory\n", cycle, id, dirrep.address.to_int());
                break;
            case CoherenceDirectoryToCache::FetchInvalidate:
                // we have the data, so invalidate it here
                if(find(dirreq.address))
                    valid[getSet(dirreq.address)][currentway] = false;
                else
                    dbgassert(find(dirreq.address), "Cache %d : Received a request, but don't have the data @%06x in %s\n", id, dirreq.address.to_int(), cachestatetostr(state));

                workaddress = dirreq.address;
                state = DirWriteBack;
                ctrli = getOffset(dirreq.address)+1;
                dirrep.address = dirreq.address;
                dirrep.type = CoherenceCacheToDirectory::Reply;
                dirrep.data = 0;
                printf("%d cache %d : sending @%06x to directory\n", cycle, id, dirrep.address.to_int());

                break;
            case CoherenceDirectoryToCache::Invalidate:
                // we have the data, so invalidate it here
                if(find(dirreq.address))
                    valid[getSet(dirreq.address)][currentway] = false;
                else
                    dbgassert(find(dirreq.address), "Cache %d : Received a request, but don't have the data @%06x in %s\n", id, dirreq.address.to_int(), cachestatetostr(state));

                state = Idle;   // could stay in the same state if invalidate is not on same block
                valid[getSet(dirreq.address)][currentway] = false;
                dirrep.address = dirreq.address;
                dirrep.type = CoherenceCacheToDirectory::Ack;
                dirrep.data = 0;

                break;
            default:
                dbgassert(dirreq.type == CoherenceDirectoryToCache::Ack ||
                          dirreq.type == CoherenceDirectoryToCache::None ||
                          dirreq.type == CoherenceDirectoryToCache::Fetch ||
                          dirreq.type == CoherenceDirectoryToCache::FetchInvalidate ||
                          dirreq.type == CoherenceDirectoryToCache::Invalidate,
                          "%d cache %d : Invalid request from directory control @%06x\n",
                          cycle, id, dirreq.address.to_int());
                break;
            }
            break;
        default:
            dbgassert(false, "Unknown state in cache %d cycle %d\n", id, cycle);
            break;
        }

        corerep.datavalid = datavalid;
        crep = corerep;
        drep = dirrep;

        printf("%d cache %d state %s\n", cycle, id, cachestatetostr(state));
    }



private:
    int id;
    enum State
    {
        Idle,
        FirstFetch,
        DirFetch,
        FirstWriteBack,
        DirWriteBack,
        WaitReply
    } state;

    ac_int<32, false> workaddress;
    ac_int<ac::log2_ceil<Blocksize>::val, false> ctrli;
    ac_int<ac::log2_ceil<Associativity>::val, false> currentway;
    bool invalidate;
    bool storecontrol;      // prevent wasting one cycle when serving directory instead of serving core
    //unsigned int data[Sets][Blocksize][Associativity]; // i don't even need this
    ac_int<32-tagshift, false> tag[Sets][Associativity];
    bool dirty[Sets][Associativity];
    bool valid[Sets][Associativity];

    CoherenceCacheToDirectory lastrequest;


    bool find(ac_int<32, false> address)
    {
        for(int i(0); i < Associativity; ++i)
            if(tag[getSet(address)][i] == getTag(address) && valid[getSet(address)][i])
            {
                currentway = i;
                return true;
            }
        return false;
    }

public:
    SimpleCacheSimulator(int id)
    : id(id), state(Idle), workaddress(0), ctrli(0), currentway(0), invalidate(false),
      storecontrol(false), lastrequest()
    {
        for(int i(0); i < Sets; ++i)
            for(int j(0); j < Associativity; ++j)
            {
                tag[i][j] = 0;
                dirty[i][j] = false;
                valid[i][j] = false;
            }
    }

    const char* cachestatetostr(State s) const
    {
        switch(s)
        {
        case Idle:
            return "Idle";
        case FirstFetch:
            return "FirstFetch";
        case DirFetch:
            return "DirFetch";
        case FirstWriteBack:
            return "FirstWriteBack";
        case DirWriteBack:
            return "DirWriteBack";
        case WaitReply:
            return "WaitReply";
        default:
            return "Unknown";
        }
    }
};

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

    SimpleCacheSimulator cache[COMET_CORE] = {0, 1};

    const int numreq = 10000;
    srand(0);
    DCacheRequest req[COMET_CORE][numreq];
    for(int i(0); i < COMET_CORE; ++i)
        for(int j(0); j < numreq; ++j)
        {
            req[i][j].address = (rand()%0x100) & 0xFFFFFC;
            req[i][j].writeenable = rand()/(float)RAND_MAX < 0.2;
            req[i][j].dcacheenable = true;
        }
    int index[COMET_CORE] = {0};

    dreq[0] = req[0][0];
    dreq[1] = req[1][0];

    while(cycle < 1e6)
    {
        //dcache<0>(memdctrl[0], mem, ptrtocache(cdm[0]), dreq[0], drep[0], sim);
        //dcache<1>(memdctrl[1], mem, ptrtocache(cdm[1]), dreq[1], drep[1], sim);
        CCS_DESIGN(directory(mem, cd, dc
      #ifndef __HLS__
        , sim
      #endif
        ));

        for(int i(0); i < COMET_CORE; ++i)
        {
            cache[i].step(dreq[i], drep[i], dc[i], cd[i]);
            if(drep[i].datavalid)
            {
                index[i] = (index[i] + 1)%numreq;
                dreq[i] = req[i][index[i]];
            }
        }

        ++cycle;
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

