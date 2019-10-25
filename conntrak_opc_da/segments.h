#ifndef _SEGMENTS_H
#define _SEGMENTS_H

#include "my_pr_debug_control.h"

struct TcpSegments
{
    struct TcpSegments *prev;
    struct TcpSegments *next;
    unsigned int seq_h; // host bytes
    unsigned int ack; // network bytes
    unsigned short dataOffset;
    unsigned short dataLen;
    unsigned short fragLen;
    unsigned char deleteMark:1,
             isHead:1,
             count:6;
};


// attempt to store the new data
// step 0: return result if check it has stored
// step 1: find a new place to store if have enough place
// TODO: step 2: if step 1 failed, find the max count ptr and delete it, then try step 1 again
// step 3: return the result
int tryStoreNewTcpData(unsigned int seq_h,unsigned int ack,unsigned int fragLen, const char *appData,unsigned short appLen)
;

// step 0: return result if check it has stored
// step 1: find the existed segments head. if failed, don't store.
// step 2: find a appropriate place to store if it belongs to a existed segments.
// step 3: return the result
int tryStoreAndAssembleNewTcpData(unsigned int seq_h,unsigned ack, const char *appData, unsigned int appLen )
;

unsigned char getAssembleTcpData(const char **tcpDataPtr,unsigned short *dataLen)
;


void updateAndDeleteSegmentsStore()
;

int segments_init()
;

int segments_fini()
;
#endif // _SEGMENTS_H
