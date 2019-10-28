#ifdef __linux__
#include <linux/slab.h>
#include <linux/string.h>
#else
#include <malloc.h>
#include <string.h>
#define GFP_KERNEL 0
static void *kmalloc(unsigned int size, int flags)
{
    return malloc(size);
}
static void kfree(void *objp)
{
    free(objp);
}
#endif // __linux__

#include "segments.h"

/**
    segmentsBuffer
    store data at begin
    store the pointer at end.
*/
char *segmentsBuffer=0; // store the tcp segments data
struct TcpSegments *virtualSegmentsPtr; // manage the tcp segments objects, the linked list head

const unsigned char segStructSize = sizeof(struct TcpSegments);


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

// data occpied at buffer[1,offset)
unsigned int getDataOccpiedOffset(void)
{
    struct TcpSegments *ptr = virtualSegmentsPtr->next;
    unsigned int offset = virtualSegmentsPtr->dataOffset + virtualSegmentsPtr->dataLen;
    while (ptr)
    {
        if (ptr->isHead)
        {
            offset = ptr->dataOffset + ptr->fragLen;
        }
        ptr = ptr->next;
    }
    return offset;
}

// pointer occpied at buffer[offset,SEGMENTS_BUFFER_LEN]
unsigned int getPtrOccpiedOffset(void)
{
    struct TcpSegments *ptr = virtualSegmentsPtr;
    unsigned int offset = SEGMENTS_BUFFER_LEN;
    unsigned int distance = 0;
    while (ptr)
    {
        distance = (unsigned int)ptr - (unsigned int)segmentsBuffer;
        if (distance < offset)
        {
            offset = distance;
        }
        ptr = ptr->next;
    }
    return offset;
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

// check the segments store to the segments head conflict
// seq should be ascend
// length should appropriate
int checkStoreSegmentsNoConflict(struct TcpSegments *head,unsigned int seq_h, unsigned int dataStoreOffset, unsigned short length)
{
    struct TcpSegments *prev = head;
    struct TcpSegments *curr = prev->next;
    if (head == 0 || !head->isHead)
    {
        return 0;
    }
    if (seq_h <= head->seq_h)
    {
        return -3; // Seq error
    }
    if (dataStoreOffset <= 0 || !(dataStoreOffset + length <= head->dataOffset + head->fragLen))
    {
        return -4; // length error
    }
    if (!(prev->dataOffset + prev->dataLen <= dataStoreOffset))
    {
        return -5;// pos conflict
    }
    while (curr)
    {
        if (prev->dataOffset + prev->dataLen <= dataStoreOffset && dataStoreOffset + length <= curr->dataOffset)
        {
            return 1; // no conflict
        }
        if (curr->isHead)
        {
            return -5;// pos conflict
        }
        prev = curr;
        curr = prev->next;
    }
    if (curr == 0)
    {
        return 1; // no conflict
    }
    return -5;
}

// find a empty place to store
// length != 0
// failed, return 0
// success, return the data offset position
unsigned int findEmptyDataBufferOffset(unsigned int maxOffset, unsigned short length)
{
    struct TcpSegments *ptr = virtualSegmentsPtr->next;
    unsigned int offset = virtualSegmentsPtr->dataOffset + virtualSegmentsPtr->dataLen; // offset == 1
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
    if (maxOffset - offset >= length)
    {
        return offset;
    }
    return 0;
}

// return SEGMENTS_BUFFER_LEN, not find
// return ptr, find a empty buffer to store a new segments ptr
unsigned int findEmptyPtrBufferOffset(unsigned int maxOffset)
{
    unsigned int offset = SEGMENTS_BUFFER_LEN;
    struct TcpSegments *lastPtr = (struct TcpSegments *)( segmentsBuffer + SEGMENTS_BUFFER_LEN - segStructSize);
    while (lastPtr)
    {
        offset = (unsigned int)lastPtr - (unsigned int)segmentsBuffer;
        if (!lastPtr->isOccpied && offset >= maxOffset)
        {
            break;
        }
        --lastPtr;
    }
    if (offset > maxOffset)
    {
        return offset;
    }
    return SEGMENTS_BUFFER_LEN;
};

// insert the new segments in linked list, the element's dataOffset is ascended
unsigned char insertSegmentsToLinkedList(struct TcpSegments *newSegments)
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
unsigned char store(unsigned int ptrStoreOffset,unsigned int dataStoreOffset,unsigned int seq_h, unsigned int ack, unsigned short fragLen, const char *appData, unsigned short appLen)
{
    struct TcpSegments *newSegments = (struct TcpSegments *) (segmentsBuffer + ptrStoreOffset);
    newSegments->prev = 0;
    newSegments->next = 0;
    newSegments->seq_h = seq_h;
    newSegments->ack = ack;
    newSegments->dataOffset = dataStoreOffset;
    newSegments->dataLen = appLen;
    newSegments->fragLen = fragLen;
    newSegments->deleteMark = 0;
    newSegments->isOccpied = 1;
    newSegments->isHead = (fragLen != 0);
    newSegments->count = 0;
    if (insertSegmentsToLinkedList(newSegments)==1)
    {
        memcpy(segmentsBuffer+dataStoreOffset,appData,appLen);
        return 1;
    }
    pr_debug("insertSegments Failed.\n");
    return 0;
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


// attempt to store the new data as tcp segments head
// step 0: return result if check it has stored
// step 1: find a new place to store if have enough place
// TODO: step 2: if step 1 failed, find the max count ptr and delete it, then try step 1 again
// step 3: return the result
int tryStoreAsSegmentsHead(unsigned int seq_h,unsigned int ack,unsigned int fragLen, const char *appData,unsigned short appLen)
{
    struct TcpSegments *ptr = virtualSegmentsPtr->next;
    unsigned int emptyDataOffset =0;
    unsigned int emptyPtrOffset = 0;
    unsigned int ptrOccpiedOffset = 0;
    unsigned int dataOccpiedOffset = 0;
    int rst = 0; // store failed
    if (!getStoredSegments(ptr,seq_h,ack))
    {
        dataOccpiedOffset = getDataOccpiedOffset();
        emptyPtrOffset = findEmptyPtrBufferOffset(dataOccpiedOffset);
        if (emptyPtrOffset != SEGMENTS_BUFFER_LEN)
        {
            ptrOccpiedOffset = getPtrOccpiedOffset();
            ptrOccpiedOffset = ptrOccpiedOffset < emptyPtrOffset? ptrOccpiedOffset: emptyPtrOffset;
            emptyDataOffset = findEmptyDataBufferOffset(ptrOccpiedOffset,fragLen);
            if (emptyDataOffset != 0)
            {
                rst = store(emptyPtrOffset,emptyDataOffset,seq_h,ack,fragLen,appData,appLen); // rst == 1: success
            }
        }
    }
    else
    {
        rst = -1; // repeat stored
    }
    return rst;
}

// attempt to store the new data as a segments
// step 0: return -2 if cannot find the segments head.
// step 1: return -1 if check it has stored
// step 2: check the parameter of segments with head
// step 2: find a appropriate place to store if have enough place
// step 3: return the result
int tryStoreAsSegments(unsigned int seq_h,unsigned int ack, const char *appData, unsigned int appLen)
{
    struct TcpSegments *ptr = virtualSegmentsPtr->next;
    struct TcpSegments *head = 0;
    unsigned int storeOffset =0;
    unsigned int emptyPtrOffset = 0;
    unsigned int dataOccpiedOffset = 0;
    int rst = 0; // store failed.
    head = findStoredSegmentsHead(ptr,ack);
    if (head)
    {
        if (!getStoredSegments(head,seq_h,ack))
        {
            storeOffset = seq_h - head->seq_h + head->dataOffset;
            rst = checkStoreSegmentsNoConflict(head,seq_h,storeOffset,appLen);
            if (rst == 1)
            {
                dataOccpiedOffset = getDataOccpiedOffset();
                emptyPtrOffset = findEmptyPtrBufferOffset(dataOccpiedOffset);
                if (emptyPtrOffset != SEGMENTS_BUFFER_LEN)
                {
                    rst = store(emptyPtrOffset,storeOffset,seq_h,ack,0,appData,appLen);
                }
                else
                {
                    rst = 0;
                }
            }
        }
        else
        {
            rst = -1; // repeat stored.
        }

    }
    else
    {
        rst =  -2; // not stored because no segments head.
    }
    return rst; // rst == 1: success
}

// step 0: return result if check it has stored
// step 1: find the existed segments head. if failed, don't store.
// step 2: find a appropriate place to store if it belongs to a existed segments.
// step 3: return the result
//int tryStoreNewTcpDataAndAssemble(unsigned int seq_h,unsigned int ack, const char *appData, unsigned int appLen )
//{
//
//}


int tryStoreTcpData(unsigned int seq_h,unsigned int ack,unsigned int fragLen, const char *appData,unsigned short appLen)
{
    int rst =0;
    if (fragLen != 0)
    {
        rst = tryStoreAsSegmentsHead(seq_h,ack,fragLen,appData,appLen);
    }
    else
    {
        rst = tryStoreAsSegments(seq_h,ack,appData,appLen);
    }
    return rst;
}

// return the next element if delete success, or return 0
struct TcpSegments *deleteThisSegments(struct TcpSegments *ptr)
{
    struct TcpSegments *prev =0;
    struct TcpSegments *next =0;
    if (ptr == 0)
    {
        pr_debug("Attempt to delete 0 ptr\n");
        return 0;
    }
    prev = ptr->prev;
    next = ptr->next;
    if (prev)
    {
        prev->next = next;
    }
    if (next)
    {
        next->prev = prev;
    }
    memset(segmentsBuffer + ptr->dataOffset,0,ptr->dataLen);
    memset(ptr,0,segStructSize);
    return next;
}

void markDeleteBySegmentsHead(struct TcpSegments *ptr)
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

void markDeleteByUpdate()
{
    struct TcpSegments *ptr = virtualSegmentsPtr->next;
    while (ptr)
    {
        if (ptr->isHead)
        {
            ++ptr->count;
            if (ptr->count >= MAX_SAVED_COUNT)
            {
                markDeleteBySegmentsHead(ptr);
            }
        }
        ptr = ptr->next;
    }
}

unsigned char isCouldAssembled(struct TcpSegments *ptr)
{
    unsigned int offset = 0;
    unsigned int end = 0;
    struct TcpSegments * assembleHead = ptr;
    unsigned int ack =0;
    unsigned char rst = 0; // current ptr could not be assembled
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
    }

    return rst;
}

struct TcpSegments *findCompleteAssembledSegments(void)
{
    struct TcpSegments *ptr = virtualSegmentsPtr->next;
    while(ptr)
    {
        if (ptr->isHead)
        {
            if (isCouldAssembled(ptr))
            {
                return ptr;
            }
        }
        ptr = ptr->next;
    }
    return 0;
}

// only attempt assemble tcp data once
unsigned char tryAssembleTcpData(unsigned int *seq_h, const char **tcpDataPtr,unsigned short *dataLen)
{
    struct TcpSegments * assembleHead = findCompleteAssembledSegments();
    if (assembleHead==0)
    {
        *seq_h = 0;
        *tcpDataPtr = 0;
        *dataLen = 0;
        return 0;
    }
    markDeleteBySegmentsHead(assembleHead);
    *seq_h = assembleHead->seq_h;
    *tcpDataPtr = segmentsBuffer + assembleHead->dataOffset;
    *dataLen = assembleHead->fragLen;
    return 1;
}

void deleteAllMarkedStore(void)
{
    struct TcpSegments *ptr = virtualSegmentsPtr->next;
    while (ptr)
    {
        if (ptr->deleteMark)
        {
            ptr = deleteThisSegments(ptr);
        }
        else
        {
            ptr = ptr->next;
        }
    }
}

void updateAndDeleteStore(void)
{
    markDeleteByUpdate();
    deleteAllMarkedStore();
}

int segments_init(void)
{
    unsigned int ptrStoreOffset = 0;
    segmentsBuffer = kmalloc(SEGMENTS_BUFFER_LEN, GFP_KERNEL);
    if (!segmentsBuffer)
    {
        return 0;
    }
    memset(segmentsBuffer,0,SEGMENTS_BUFFER_LEN);
    ptrStoreOffset = findEmptyPtrBufferOffset(1);
    if (ptrStoreOffset == SEGMENTS_BUFFER_LEN)
    {
        return 0;
    }
    virtualSegmentsPtr = (struct TcpSegments *)(segmentsBuffer + ptrStoreOffset);
    virtualSegmentsPtr->prev = 0;
    virtualSegmentsPtr->next = 0;
    virtualSegmentsPtr->seq_h = 0;
    virtualSegmentsPtr->ack = 0;
    virtualSegmentsPtr->dataOffset = 0;
    virtualSegmentsPtr->dataLen = 1;
    virtualSegmentsPtr->fragLen = 0;
    virtualSegmentsPtr->count = 0;
    virtualSegmentsPtr->deleteMark = 0;
    virtualSegmentsPtr->isOccpied = 1;
    virtualSegmentsPtr->isHead = 0;
    virtualSegmentsPtr->count = 0;
    return 1;
}

int segments_fini(void)
{
    kfree(segmentsBuffer);
    return 1;
}
