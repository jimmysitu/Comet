#ifndef COHERENCE_H
#define COHERENCE_H

#include "portability.h"
#include "cache.h"

#define COMET_CORE              2
#define DirectoryTag            (32-ac::log2_ceil<COMET_CORE*Size>::val)
#define getDSet(address)        (address.slc<32-DirectoryTag-setshift>(setshift))
#define getDTag(address)        (address.slc<DirectoryTag>(32-DirectoryTag))

#define setDSet(address, set)   (address.set_slc(setshift, (ac_int<32-DirectoryTag-setshift>)set))
#define setDTag(address, tag)   (address.set_slc(32-DirectoryTag, (ac_int<DirectoryTag>)tag))

namespace CoherenceState {  // define this in cache.h?
enum CoherenceState
{
    Invalid ,
    Shared  ,
    Modified
};
}

struct LineCoherence
{
    LineCoherence()
    : state(LineCoherence::Invalid), tag(0), sharers(0)
    {}

    enum DCoherenceState
    {
        Invalid     ,   // Uncached
        Shared      ,   // Read only
        Modified    ,   // Read/Write, assumed dirty
        NUMSTATES = 3
    } state : ac::log2_ceil<NUMSTATES>::val;
    ac_int<32-tagshift, false> tag;
    ac_int<COMET_CORE, false> sharers;
};

// Used from cache to directory
/// request type from cache to directory :
/// read miss --> sharers += P, forward data from memory or other cache, update state(M to S, I to S), keep track of dirtyness
/// write miss --> sharers = P, forward data from memory
/// write invalidate --> sharers = P, no forward, both (miss + invalidate) are same from cache
/// write back --> sharers = {}, forward data to main memory
struct CoherenceCacheToDirectory
{
    CoherenceCacheToDirectory()
    : address(0), type(CoherenceCacheToDirectory::None), data(0)
    {}

    ac_int<32, false> address;
    // type of request
    enum Type
    {
        None            ,   // No request
        Ack             ,   // Acknowledgment (useless?)
        Reply           ,   // Used to reply to directory request with data
        ReadMiss        ,   // Cache doesnt have data and wants to read it
        WriteMiss       ,   // Cache doesnt have data and wants to write it
        WriteInvalidate ,   // Cache has data in shared state and wants to write it
        WriteBack       ,   // Cache has data in Modified state and wants to evict it
        NUMSTATES
    } type : ac::log2_ceil<NUMSTATES>::val;
    ac_int<32, false> data; // data to be written back
};

// Used from directory to cache
/// request type from directory to cache:
/// invalidate --> sharers = P, invalidate line
/// fetch --> sharers += P, forward data from cache to directory to requesting cache
/// fetch invalidate --> sharers = P, cache forward data to directory to requesting writing cache and invalidate its own
struct CoherenceDirectoryToCache
{
    CoherenceDirectoryToCache()
    : address(0), type(CoherenceDirectoryToCache::None), data(0)
    {}

    ac_int<32, false> address;
    // type of reply
    enum Type
    {
        None            ,   // No request
        Ack             ,   // Acknowledgment (useless?)
        Reply           ,   // Used to reply to cache request with data
        Invalidate      ,   // Cache must invalidate its data
        Fetch           ,   // Cache must reply with requested address
        FetchInvalidate ,   // same as previous, but must also invalidate its data after
        Abort           ,   // Used when several cache replies, tell caches to abort ?
        NUMSTATES
    } type : ac::log2_ceil<NUMSTATES>::val;
    ac_int<32, false> data;
};

struct DirectoryControl
{
    DirectoryControl()
    : state(DirectoryControl::Idle), line(), cd(), reqcore(0), fetchcore(0), valuetowrite(0), i(0)
    {
        #pragma hls_pipeline_init_interval 1
        for(int i(0); i < COMET_CORE*Sets*Associativity; ++i)
            lines[i] = LineCoherence();
    }

    // read Ack for every state
    enum
    {
        Idle            ,
        AppropriateReply,   // rename as dispatch?
        FirstFetchMem   ,
        FetchMem        ,
        FetchCache      ,
        WriteMem        ,
        StoreControl    ,
        Waitingforreply ,
        NUMSTATES = Waitingforreply
    } state : ac::log2_ceil<NUMSTATES>::val;

    LineCoherence lines[COMET_CORE*Sets*Associativity];
    LineCoherence line;                                         // line we work on
    CoherenceCacheToDirectory cd;                               // request we must service
    ac_int<ac::log2_ceil<COMET_CORE>::val, false> reqcore;      // requesting core
    ac_int<ac::log2_ceil<COMET_CORE>::val, false> fetchcore;    // core we fetch data from

    ac_int<32, false> valuetowrite;
    ac_int<ac::log2_ceil<Blocksize>::val, false> i;


};


/// workload:
/// producteur producteur (vers une FIFO pour les 2)
/// producteur consommateur
/// consommateur consommateur (d'une FIFO vers autre FIFO?)

class Simulator;

void directory(unsigned int mem[DRAM_SIZE],
               CoherenceCacheToDirectory cd[COMET_CORE],
               CoherenceDirectoryToCache dc[COMET_CORE]
            #ifndef __HLS__
               , Simulator* sim
            #endif
               );

#endif  // COHERENCE_H
