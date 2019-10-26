#ifndef _SEGMENTS_H
#define _SEGMENTS_H

#include "my_pr_debug_control.h"

// the 4 bytes times.
struct TcpSegments
{
    struct TcpSegments *prev;
    struct TcpSegments *next;
    unsigned int seq_h; // host bytes
    unsigned int ack; // network bytes
    unsigned int dataOffset;
    unsigned short dataLen;
    unsigned short fragLen;
    unsigned int deleteMark:1,
             isOccpied:1,
             isHead:1,
             count:8;
};

// unsigned short not stored 65536
#define SEGMENTS_BUFFER_LEN 65536
#define MAX_SAVED_COUNT 10

// attempt to store the new data
// step 0: return result if check it has stored
// step 1: find a new place to store if have enough place
// TODO: step 2: if step 1 failed, find the max count ptr and delete it, then try step 1 again
// step 3: return the result
int tryStoreTcpData(unsigned int seq_h,unsigned int ack,unsigned int fragLen, const char *appData,unsigned short appLen);

unsigned char tryAssembleTcpData(unsigned int *seq_h,const char **tcpDataPtr,unsigned short *dataLen);

void updateAndDeleteStore(void);

void markDeleteByUpdate(void);

void deleteAllMarkedStore(void);

int segments_init(void);

int segments_fini(void);
#endif // _SEGMENTS_H
