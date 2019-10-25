#include <string.h>

#include "segments.h"
#include "malloc.h"

#define SEGMENTS_BUFFER_LEN 65536
#define MAX_SAVED_COUNT 10

const unsigned char segStructSize = sizeof(struct TcpSegments);
char *segmentsBuffer=0; // store the tcp segments data
unsigned int MAX_STORED_LEN = SEGMENTS_BUFFER_LEN; // unsigned short not stored 65536

struct TcpSegments *virtualSegmentsPtr;

static void displayTcpSegments(struct TcpSegments *ptr)
{
    if (ptr == 0)
    {
        pr_debug("tcp segments ptr == 0\n");
    }
    else
    {
        pr_debug("TCP Segments:\n");
        pr_debug("\tseq_h = 0x%x\n",ptr->seq_h);
        pr_debug("\tack = 0x%x\n",ptr->ack);
        pr_debug("\tdataOffset = %d\n",ptr->dataOffset);
        pr_debug("\tdataLen = %d\n",ptr->dataLen);
        pr_debug("\tfragLen = %d\n",ptr->fragLen);
        pr_debug("\tdeleteMark = %d\n",ptr->deleteMark);
        pr_debug("\tisHead = %d\n",ptr->isHead);
        pr_debug("\tcount = %d\n",ptr->count);
    }
}

// get a stored segments ptr with same seq_h and ack
// check whether a tcp data has stored
// return 0 if not stored
struct TcpSegments *getStoredSegments(struct TcpSegments *ptr,unsigned int seq_h,unsigned int ack)
{
    while (ptr)
    {
        if (ptr->ack == ack && ptr->seq_h == seq_h)
        {
            return ptr;
        }
        ptr = ptr->next;
    }
    return 0;
}

// find a empty place to store
// length != 0
// failed, return 0
// success, return the data offset position
unsigned short findEnoughPlaceToStore(unsigned short length)
{
    struct TcpSegments *ptr = virtualSegmentsPtr->next;
    unsigned short offset = virtualSegmentsPtr->dataOffset + virtualSegmentsPtr->dataLen; // offset == 1
    while (ptr)
    {
        if (ptr->isHead)
        {
            if (ptr->dataOffset - offset >= length)
            {
                return offset;
            }
            else
            {
                offset = ptr->dataOffset + ptr->fragLen;
            }
        }

        ptr = ptr->next;
    }
    if (MAX_STORED_LEN-offset >= length)
    {
        return offset;
    }
    return 0;
}

// insert the new segments in linked list, the element's dataOffset is ascended
unsigned char insertSegments(struct TcpSegments *newSegments)
{
    struct TcpSegments *prev = virtualSegmentsPtr;
    struct TcpSegments *curr = virtualSegmentsPtr->next;
    while (curr)
    {
        if (prev->dataOffset < newSegments->dataOffset && newSegments->dataOffset < curr->dataOffset)
        {
            prev->next = newSegments;
            newSegments->next = curr;
            curr->prev = newSegments;
            newSegments->prev = prev;
            return 1;
        }
        prev = curr;
        curr = prev->next;
    }
    if (curr == 0 && prev->dataOffset < newSegments->dataOffset)
    {
        prev->next = newSegments;
        newSegments->prev = prev;
        newSegments->next = 0;
        return 1;
    }
    return 0;
}

// generate segment to store and insert it into linked list, copy bytes to buffer
unsigned char storeTcpDataAsSegments(unsigned short storeOffset,unsigned int seq_h, unsigned int ack, unsigned short fragLen, const char *appData, unsigned short appLen)
{
    struct TcpSegments *newSegments = (struct TcpSegments *) malloc(sizeof(struct TcpSegments));
    newSegments->prev = 0;
    newSegments->next = 0;
    newSegments->seq_h = seq_h;
    newSegments->ack = ack;
    newSegments->dataOffset = storeOffset;
    newSegments->dataLen = appLen;
    newSegments->fragLen = fragLen;
    newSegments->deleteMark = 0;
    if (fragLen != 0)
    {
        newSegments->isHead = 1;
    }
    else
    {
        newSegments->isHead = 0;
    }

    newSegments->count = 0;
    if (insertSegments(newSegments)==1)
    {
        memcpy(segmentsBuffer+storeOffset,appData,appLen);
        return 1;
    }
    pr_debug("insertSegments Failed.\n");
    return 0;
}

//struct TcpSegments* findMaxCountPtr(){
//
//};


// attempt to store the new data
// step 0: return result if check it has stored
// step 1: find a new place to store if have enough place
// TODO: step 2: if step 1 failed, find the max count ptr and delete it, then try step 1 again
// step 3: return the result
int tryStoreNewTcpData(unsigned int seq_h,unsigned int ack,unsigned int fragLen, const char *appData,unsigned short appLen)
{
    struct TcpSegments *ptr = virtualSegmentsPtr->next;
    unsigned short emptyOffset =0;
//    struct TcpSegments *maxCountPtr = 0;
    int rst = 0; // store failed.
    if (!getStoredSegments(ptr,seq_h,ack))
    {
        emptyOffset = findEnoughPlaceToStore(fragLen);
//        if (emptyOffset == 0)
//        {
//            maxCountPtr = findMaxCountPtr(ptr);
//            if (maxCountPtr != 0)
//            {
//                deleteOneSegments(maxCountPtr);
//                emptyOffset = findEnoughPlaceToStore(appLen);
//            }
//        }
        if (emptyOffset != 0)
        {
            rst = storeTcpDataAsSegments(emptyOffset,seq_h,ack,fragLen,appData,appLen);
        }
    }
    else
    {
        rst = -1; // stored
    }
    return rst; // rst == 1: success
}

struct TcpSegments *findStoredSegmentsHead(struct TcpSegments *ptr, unsigned int ack)
{
    while (ptr)
    {
        if (ptr->ack == ack && ptr->isHead)
        {
            return ptr;
        }
        ptr = ptr->next;
    }
    return 0;
}

// step 0: return result if check it has stored
// step 1: find the existed segments head. if failed, don't store.
// step 2: find a appropriate place to store if it belongs to a existed segments.
// step 3: return the result
int tryStoreAndAssembleNewTcpData(unsigned int seq_h,unsigned ack, const char *appData, unsigned int appLen )
{
    struct TcpSegments *ptr = virtualSegmentsPtr->next;
    struct TcpSegments *head = 0;
    unsigned short storeOffset =0;
    int rst = 0; // store failed.
    head = findStoredSegmentsHead(ptr,ack);
    if (head)
    {
        if (!getStoredSegments(head,seq_h,ack))
        {
            if (head->seq_h < seq_h)
            {
                storeOffset = seq_h - head->seq_h + head->dataOffset;
                if (storeOffset > 0 && storeOffset + appLen <= head->dataOffset + head->fragLen)
                {
                    rst = storeTcpDataAsSegments(storeOffset,seq_h,ack,0,appData,appLen);
                }
                else
                {
                    pr_debug("Length error");
                }
            }
            else
            {
                pr_debug("Seq error\n");
            }
        }
        else
        {
            rst = -1; // stored.
        }

    }
    else
    {
        rst =  -2; // not stored.
    }
    return rst; // rst == 1: success
}

// return the next element if delete success, or return 0
struct TcpSegments *deleteOneSegments(struct TcpSegments *ptr)
{
    if (ptr == 0)
    {
        pr_debug("Attempt to delete 0 ptr\n");
        return 0;
    }
    struct TcpSegments *prev = ptr->prev;
    struct TcpSegments *next = ptr->next;
    if (prev)
    {
        prev->next = next;
    }
    if (next)
    {
        next->prev = prev;
    }
    memset(segmentsBuffer + ptr->dataOffset,0,ptr->dataLen);
    free(ptr);
    return next;
}

void markAssembleSegmentsDelete(struct TcpSegments *ptr)
{
    unsigned int ack = 0;
    if (ptr && ptr->isHead)
    {
        ptr->deleteMark = 1; // head also should mark
        ack = ptr->ack;
        ptr = ptr->next;
        while (ptr)
        {
            if (ptr->ack == ack && !ptr->isHead)
            {
                ptr->deleteMark = 1;
            }
            else
            {
                break;
            }
            ptr = ptr->next;
        }
    }
}

void updateCountAndMarkSegments()
{
    struct TcpSegments *ptr = virtualSegmentsPtr->next;
    while (ptr)
    {
        if (ptr->isHead)
        {
            ++ptr->count;
            if (ptr->count >= MAX_SAVED_COUNT)
            {
                markAssembleSegmentsDelete(ptr);
            }
        }
        ptr = ptr->next;
    }
}

unsigned char checkCompleteAndMark(struct TcpSegments *ptr)
{
    unsigned short offset = 0;
    unsigned short end = 0;
    struct TcpSegments * assembleHead = ptr;
    unsigned int ack =0;
    unsigned char rst = 0; // not found
    if (assembleHead && assembleHead->isHead)
    {
        offset = assembleHead->dataOffset + assembleHead->dataLen;
        end = assembleHead->dataOffset + assembleHead->fragLen;
        ack = assembleHead->ack;
        ptr = assembleHead->next;
        while (ptr)
        {
            if (ptr->isHead || offset != ptr->dataOffset || ptr->ack != ack)
            {
                rst = 0;
                break;
            }
            offset = ptr->dataOffset + ptr->dataLen;
            if (offset == end)
            {
                rst = 1;
                break;
            }
            ptr = ptr->next;
        }
        if (rst)
        {
            markAssembleSegmentsDelete(assembleHead);
        }
    }

    return rst;
}

struct TcpSegments *findCompleteAssembledSegments()
{
    struct TcpSegments *ptr = virtualSegmentsPtr->next;
    while(ptr)
    {
        if (ptr->isHead)
        {
            if (checkCompleteAndMark(ptr))
            {
                return ptr;
            }
        }
        ptr = ptr->next;
    }
    return 0;
}

unsigned char getAssembleTcpData(const char **tcpDataPtr,unsigned short *dataLen)
{
    struct TcpSegments * assembleHead = findCompleteAssembledSegments();
    if (assembleHead==0)
    {
        *tcpDataPtr = 0;
        *dataLen = 0;
        return 0;
    }
    *tcpDataPtr = segmentsBuffer + assembleHead->dataOffset;
    *dataLen = assembleHead->fragLen;
    return 1;
}

void deleteAllMarkedSegments()
{
    struct TcpSegments *ptr = virtualSegmentsPtr->next;
    while (ptr)
    {
        if (ptr->deleteMark)
        {
            ptr = deleteOneSegments(ptr);
        }
        else
        {
            ptr = ptr->next;
        }
    }
}

void updateAndDeleteSegmentsStore()
{
    updateCountAndMarkSegments();
    deleteAllMarkedSegments();
}

int segments_init()
{
    segmentsBuffer = malloc(SEGMENTS_BUFFER_LEN);
    virtualSegmentsPtr = (struct TcpSegments *) malloc(sizeof(struct TcpSegments));
    if (!segmentsBuffer || !virtualSegmentsPtr)
    {
        return 0;
    }
    virtualSegmentsPtr->prev = 0;
    virtualSegmentsPtr->next = 0;
    virtualSegmentsPtr->seq_h = 0;
    virtualSegmentsPtr->ack = 0;
    virtualSegmentsPtr->dataOffset = 0;
    virtualSegmentsPtr->dataLen = 1;
    virtualSegmentsPtr->fragLen = 0;
    virtualSegmentsPtr->count = 0;
    virtualSegmentsPtr->isHead = 0;
    virtualSegmentsPtr->deleteMark = 0;
    memset(segmentsBuffer,0,SEGMENTS_BUFFER_LEN);
    return 1;
}

int segments_fini()
{
    free(segmentsBuffer);
    free(virtualSegmentsPtr);
    return 1;
}
