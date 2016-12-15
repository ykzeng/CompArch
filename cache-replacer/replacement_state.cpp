#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <sys/mman.h>
#include <map>
#include <iostream>

using namespace std;

#include "replacement_state.h"

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// This file is distributed as part of the Cache Replacement Championship     //
// workshop held in conjunction with ISCA'2010.                               //
//                                                                            //
//                                                                            //
// Everyone is granted permission to copy, modify, and/or re-distribute       //
// this software.                                                             //
//                                                                            //
// Please contact Aamer Jaleel <ajaleel@gmail.com> should you have any        //
// questions                                                                  //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

/*
** This file implements the cache replacement state. Users can enhance the code
** below to develop their cache replacement ideas.
**
*/
UINT32 CACHE_REPLACEMENT_STATE::M = 4;
UINT32 CACHE_REPLACEMENT_STATE::MAX_RRPV = pow(2, M) - 1;

////////////////////////////////////////////////////////////////////////////////
// The replacement state constructor:                                         //
// Inputs: number of sets, associativity, and replacement policy to use       //
// Outputs: None                                                              //
//                                                                            //
// DO NOT CHANGE THE CONSTRUCTOR PROTOTYPE                                    //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
CACHE_REPLACEMENT_STATE::CACHE_REPLACEMENT_STATE( UINT32 _sets, UINT32 _assoc, UINT32 _pol )
{

    numsets    = _sets;
    assoc      = _assoc;
    replPolicy = _pol;

    mytimer    = 0;

    InitReplacementState();
}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// The function prints the statistics for the cache                           //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
ostream & CACHE_REPLACEMENT_STATE::PrintStats(ostream &out)
{

    out<<"=========================================================="<<endl;
    out<<"=========== Replacement Policy Statistics ================"<<endl;
    out<<"=========================================================="<<endl;

    // CONTESTANTS:  Insert your statistics printing here

    return out;

}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// This function initializes the replacement policy hardware by creating      //
// storage for the replacement state on a per-line/per-cache basis.           //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

void CACHE_REPLACEMENT_STATE::InitReplacementState()
{
    // Create the state for sets, then create the state for the ways

    repl  = new LINE_REPLACEMENT_STATE* [ numsets ];

    // ensure that we were able to create replacement state

    assert(repl);

    // Create the state for the sets
    for(UINT32 setIndex=0; setIndex<numsets; setIndex++)
    {
        repl[ setIndex ]  = new LINE_REPLACEMENT_STATE[ assoc ];
        // TODO decouple LRU init from here
        for(UINT32 way=0; way<assoc; way++)
        {
            // initialize stack position (for true LRU)
            repl[ setIndex ][ way ].LRUstackposition = way;
        }
    }

    if (replPolicy == CRC_REPL_NRU){
      for(UINT32 setIndex=0; setIndex<numsets; setIndex++)
      {
          for(UINT32 way=0; way<assoc; way++)
          {
              // initialize NRU bits
              repl[ setIndex ][ way ].nru_bit = 1;
          }
      }
    }
    else if (replPolicy == CRC_REPL_HP_RRIP
        || replPolicy == CRC_REPL_FP_RRIP
        || replPolicy == CRC_REPL_CONTESTANT){
      // TODO init Hit Promotion RRIP variables
      for(UINT32 setIndex=0; setIndex<numsets; setIndex++)
      {
          for(UINT32 way=0; way<assoc; way++)
          {
              // initialize HP per block RRPV
              repl[ setIndex ][ way ].rrpv = MAX_RRPV - 1;
          }
      }

    }

      //else if (replPolicy == CRC_REPL_FP_RRIP)
      // TODO init Frequency Promotion RRIP variables
    //else if (replPolicy == CRC_REPL_CONTESTANT)

    else
      return;

    // Contestants:  ADD INITIALIZATION FOR YOUR HARDWARE HERE
    // TODO variables initialization
}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// This function is called by the cache on every cache miss. The input        //
// argument is the set index. The return value is the physical way            //
// index for the line being replaced.                                         //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
INT32 CACHE_REPLACEMENT_STATE::GetVictimInSet( UINT32 tid, UINT32 setIndex, const LINE_STATE *vicSet, UINT32 assoc, Addr_t PC, Addr_t paddr, UINT32 accessType ) {
    // If no invalid lines, then replace based on replacement policy
    if( replPolicy == CRC_REPL_LRU )
    {
        return Get_LRU_Victim( setIndex );
    }
    else if( replPolicy == CRC_REPL_RANDOM )
    {
        return Get_Random_Victim( setIndex );
    }
    else if (replPolicy == CRC_REPL_NRU)
    {
        return Get_NRU_Victim(setIndex);
    }
    else if( replPolicy == CRC_REPL_CONTESTANT )
    {
        // Contestants:  ADD YOUR VICTIM SELECTION FUNCTION HERE
	return Get_My_Victim (setIndex);
    }
    else if ( replPolicy == CRC_REPL_HP_RRIP
        || replPolicy == CRC_REPL_FP_RRIP)
    {
        return Get_RRIP_Victim(setIndex);
    }

    // We should never here here

    assert(0);
    return -1;
}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// This function is called by the cache after every cache hit/miss            //
// The arguments are: the set index, the physical way of the cache,           //
// the pointer to the physical line (should contestants need access           //
// to information of the line filled or hit upon), the thread id              //
// of the request, the PC of the request, the accesstype, and finall          //
// whether the line was a cachehit or not (cacheHit=true implies hit)         //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
void CACHE_REPLACEMENT_STATE::UpdateReplacementState(
    UINT32 setIndex, INT32 updateWayID, const LINE_STATE *currLine,
    UINT32 tid, Addr_t PC, UINT32 accessType, bool cacheHit )
{
	//fprintf (stderr, "ain't I a stinker? %lld\n", get_cycle_count ());
	//fflush (stderr);
    // What replacement policy?
    if( replPolicy == CRC_REPL_LRU )
    {
        UpdateLRU( setIndex, updateWayID );
    }
    else if( replPolicy == CRC_REPL_RANDOM )
    {
        // Random replacement requires no replacement state update
    }
    else if (replPolicy == CRC_REPL_NRU)
    {
        UpdateNRU(setIndex, updateWayID, cacheHit);
    }
    else if ( replPolicy == CRC_REPL_CONTESTANT )
    {
        // Contestants:  ADD YOUR UPDATE REPLACEMENT STATE FUNCTION HERE
        // Feel free to use any of the input parameters to make
        // updates to your replacement policy
        //UpdateMyPolicy(setIndex, updateWayID);
        UpdateHPRRIP(setIndex, updateWayID, cacheHit);
    }
    else if ( replPolicy == CRC_REPL_HP_RRIP )
    {
        UpdateHPRRIP(setIndex, updateWayID, cacheHit);
    }
    else if ( replPolicy == CRC_REPL_FP_RRIP )
    {
        UpdateFPRRIP(setIndex, updateWayID, cacheHit);
    }
}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//////// HELPER FUNCTIONS FOR REPLACEMENT UPDATE AND VICTIM SELECTION //////////
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// This function finds the LRU victim in the cache set by returning the       //
// cache block at the bottom of the LRU stack. Top of LRU stack is '0'        //
// while bottom of LRU stack is 'assoc-1'                                     //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
INT32 CACHE_REPLACEMENT_STATE::Get_LRU_Victim( UINT32 setIndex )
{
	// Get pointer to replacement state of current set

	LINE_REPLACEMENT_STATE *replSet = repl[ setIndex ];
	INT32   lruWay   = 0;

	// Search for victim whose stack position is assoc-1

	for(UINT32 way=0; way<assoc; way++) {
		if (replSet[way].LRUstackposition == (assoc-1)) {
			lruWay = way;
			break;
		}
	}

	// return lru way

	return lruWay;
}

INT32 CACHE_REPLACEMENT_STATE::Get_RRIP_Victim( UINT32 setIndex ) {
  // TODO return the victim block of HP RRIP policy
  UINT32 blockIndex = assoc, lruLineRrpv = 0;
  LINE_REPLACEMENT_STATE *replSet = repl[ setIndex ];
  // Search for victim whose RRPV is MAX_RRPV - 1 and was accessed the earliest
  // i.e., rrpv = MAX_RRPV - 1, index is the smallest
  for(UINT32 way=0; way<assoc; way++) {
  	if (replSet[way].rrpv == MAX_RRPV) {
                //blockIndex = way;
                replSet[way].rrpv = MAX_RRPV - 2;
                return way;
  	}
      if (replSet[way].rrpv > lruLineRrpv){
        lruLineRrpv = replSet[way].rrpv;
        blockIndex = way;
      }
  }
  // TODO here we can simply find the max rrpv and increment all with the diff
  // from the max one to MAX_RRPV
  // if found
  INT32 diff = MAX_RRPV - lruLineRrpv;
  for (UINT32 way = 0; way < assoc; way ++){
    replSet[way].rrpv += diff;
  }
  // here we are sure that all lines are 0 rrpv upon the insertion
  if (blockIndex == assoc)
    return 0;
  else{
    // use the line with max rrpv
    replSet[blockIndex].rrpv = MAX_RRPV - 2;
    return blockIndex;
  }
  //hopefully we don't do this
  //assert(0);
  //return -1;
}

INT32 CACHE_REPLACEMENT_STATE::Get_NRU_Victim( UINT32 setIndex ) {
  UINT32 blockIndex = assoc;
  LINE_REPLACEMENT_STATE *replSet = repl[ setIndex ];
  // Search for victim whose nru bit is 1 and was accessed the earliest
  // i.e., nru_bit = 1, nru_position is the smallest
  for(UINT32 way=0; way<assoc; way++) {
  	if (replSet[way].nru_bit) {
                // I currently think all the block nru position will not change
                // just using the block index as nru position seems suffice
                blockIndex = way;
                break;
  	}
  }
  // if found nru_bit = 1
  if (blockIndex < assoc){
    //set nru bit and replace it
    replSet[blockIndex].nru_bit = 0;
    return blockIndex;
  }
  else{
    // consider here as replacement operation
    replSet[0].nru_bit = 0;
    for (UINT32 way=1; way<assoc; way++) {
      replSet[way].nru_bit = 1;
    }
    return 0;
    // guess in this case we have to return the earliest
    // notice we should return the nru_position = 0, not the first line
    //return 0;
  }
}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// This function finds a random victim in the cache set                       //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
INT32 CACHE_REPLACEMENT_STATE::Get_Random_Victim( UINT32 setIndex )
{
    INT32 way = (rand() % assoc);

    return way;
}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// This function implements the LRU update routine for the traditional        //
// LRU replacement policy. The arguments to the function are the physical     //
// way and set index.                                                         //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

void CACHE_REPLACEMENT_STATE::UpdateLRU( UINT32 setIndex, INT32 updateWayID )
{
	// Determine current LRU stack position
	UINT32 currLRUstackposition = repl[ setIndex ][ updateWayID ].LRUstackposition;

	// Update the stack position of all lines before the current line
	// Update implies incremeting their stack positions by one

	for(UINT32 way=0; way<assoc; way++) {
		if( repl[setIndex][way].LRUstackposition < currLRUstackposition ) {
			repl[setIndex][way].LRUstackposition++;
		}
	}

	// Set the LRU stack position of new line to be zero
	repl[ setIndex ][ updateWayID ].LRUstackposition = 0;
}

void CACHE_REPLACEMENT_STATE::UpdateNRU( UINT32 setIndex, INT32 updateWayID,
    bool cacheHit ) {
  // we only do this when cache hit, since the miss case we have already dealed
  // when selecting victim block
  if (cacheHit)
    repl[setIndex][updateWayID].nru_bit = 0;
}

void CACHE_REPLACEMENT_STATE::UpdateHPRRIP( UINT32 setIndex, INT32 updateWayID,
    bool cacheHit ) {
  // we only do this when cache hit, since the miss case we have already dealed
  // when selecting victim block
  if (cacheHit)
    repl[setIndex][updateWayID].rrpv = 0;
}

void CACHE_REPLACEMENT_STATE::UpdateFPRRIP( UINT32 setIndex, INT32 updateWayID,
    bool cacheHit ) {
  // we only do this when cache hit, since the miss case we have already dealed
  // when selecting victim block
  if (cacheHit){
    if (repl[setIndex][updateWayID].rrpv > 0)
      repl[setIndex][updateWayID].rrpv--;
  }
}

INT32 CACHE_REPLACEMENT_STATE::Get_My_Victim( UINT32 setIndex ) {
  // default return value is 0
  return Get_RRIP_Victim(setIndex);
}

void CACHE_REPLACEMENT_STATE::UpdateMyPolicy( UINT32 setIndex, INT32 updateWayID ) {
  // do nothing
}

CACHE_REPLACEMENT_STATE::~CACHE_REPLACEMENT_STATE (void) {
}
