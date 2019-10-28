#include "test_segments.h"

#include "BytesTools.h"
#include "AssertTools.h"

#include "conntrak_opc_da/segments.h"

extern char *segmentsBuffer;
extern struct TcpSegments *virtualSegmentsPtr;
extern const unsigned char segStructSize;
extern unsigned int getDataOccpiedOffset();
extern unsigned int getPtrOccpiedOffset();

static void displayTcpSegments(struct TcpSegments *ptr)
{
    if (ptr == 0)
    {
        pr_debug("TCP Segments: ptr == 0\n");
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
        pr_debug("\tData = ");
        displayBytesInHexChars(segmentsBuffer+ptr->dataOffset,ptr->dataLen);
    }
}

struct TcpTest
{
    unsigned int seq_h; // host bytes
    unsigned int ack; // network bytes
    const char *appData;
    unsigned short appLen;
    unsigned short fragLen;
};

static int testNum = 1;

void checkLinkedListConnectivity()
{
    assert(virtualSegmentsPtr->prev == 0); // the prev should be NULL

    struct TcpSegments *prev = virtualSegmentsPtr;
    struct TcpSegments *curr = prev->next;

    while (curr)
    {
        assert(curr->prev == prev);
        prev = curr;
        curr = prev->next;
    }
}

void test_segments_1()
{

    pr_debug("TEST SEGMENTS %d\n",testNum++);
    assert(segments_init());

    int rst = 0;
    unsigned seq_h = 0;
    const char *tcpData = 0;
    unsigned short dataLen = 0;
    struct TcpTest t1 = {0x1537d73a,0x6b3b8555,"12345",5,10};
    struct TcpTest t2 = {0x1537d73f,0x6b3b8555,"67890",5,0};
    rst = tryStoreTcpData(t1.seq_h, t1.ack, t1.fragLen, t1.appData, t1.appLen);
    assert(rst == 1);  // t1 store success
    assertPtrEqual(virtualSegmentsPtr->next, virtualSegmentsPtr - 1); // check the t1 store position, notice the vir - 1 is the pointer sub.
    assertUIntegerEqual(virtualSegmentsPtr->next->seq_h,0x1537d73a); // check it is t1
    assertUIntegerEqual(virtualSegmentsPtr->next->dataOffset, 1); // t1 data store offset
    rst = tryStoreTcpData(t1.seq_h, t1.ack, t1.fragLen, t1.appData, t1.appLen);
    /** Test not store repeat **/
    assert(rst == -1); // t1 has stored.
    assertBytesEqual(segmentsBuffer+1, 10, "12345\x00\x00\x00\x00\x00", 10);  // only t1

    rst = tryStoreTcpData( t2.seq_h, t2.ack, 0, t2.appData, t2.appLen);
    /** Test assemble the segments **/
    assert(rst == 1); // t2 store success
    assertUIntegerEqual(virtualSegmentsPtr->next->next->seq_h,0x1537d73f); // check it is t2
    assertPtrEqual(virtualSegmentsPtr->next->next, virtualSegmentsPtr - 2); // // check the t2 store position
    assertUIntegerEqual(virtualSegmentsPtr->next->next->dataOffset, 6); // t2 data store offset
    assert(virtualSegmentsPtr->next->next->next == 0); // check next is null
    rst = tryStoreTcpData( t2.seq_h, t2.ack, 0, t2.appData, t2.appLen);
    assert(rst == -1); // t2 has stored.
    assertBytesEqual(segmentsBuffer+1, 10, "1234567890", 10);  // t1, t2

    /** Test the linked list ptr **/
    checkLinkedListConnectivity();

    struct TcpTest t3 = {12,55,"12345678",8,15};
    struct TcpTest t4 = {20,55,"1122334",7,0};

    rst = tryStoreTcpData( t4.seq_h, t4.ack, 0, t4.appData, t4.appLen);
    /** Test not be assemble when don't have the head segments **/
    assert(rst == -2); // t4 not store
    assertBytesEqual(segmentsBuffer+1, 25, "1234567890\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 25);  // t1, t2

    rst = tryStoreTcpData(t3.seq_h, t3.ack, t3.fragLen, t3.appData, t3.appLen);
    /** Test it will be stored after **/
    assert(rst == 1); // t3 store
    assertBytesEqual(segmentsBuffer+1, 25, "123456789012345678\x00\x00\x00\x00\x00\x00\x00", 25);  //t1, t2, t3
    assertUIntegerEqual(virtualSegmentsPtr->next->next->next->seq_h,12); // check it is t3
    assertPtrEqual(virtualSegmentsPtr->next->next->next, virtualSegmentsPtr - 3); // // check the t3 store position

    rst = tryStoreTcpData( t4.seq_h, t4.ack, 0, t4.appData, t4.appLen);
    assert(rst == 1); // t4 store
    assertUIntegerEqual(virtualSegmentsPtr->next->next->next->next->seq_h,20); // check it is t4
    assertPtrEqual(virtualSegmentsPtr->next->next->next->next, virtualSegmentsPtr - 4); // // check the t4 store position
    assertBytesEqual(segmentsBuffer+1, 25, "1234567890123456781122334", 25);  // t1, t2, t3, t4

    rst = tryAssembleTcpData(&seq_h,&tcpData,&dataLen);
    /** Test it will got a assembled tcp data by t1 and t2. **/
    assert(rst == 1); // find t1 t2 has been assembled
    assertUIntegerEqual(seq_h,t1.seq_h);
    assertBytesEqual(tcpData, dataLen, "1234567890", 10);  // assembled t1, t2

    /** Test the delete mark added by assembled function **/
    assert(virtualSegmentsPtr->next->deleteMark == 1); // t1 should mark
    assert(virtualSegmentsPtr->next->next->deleteMark == 1); // t2 should mark
    assert(virtualSegmentsPtr->next->next->next->deleteMark == 0); // t3 should not mark
    assert(virtualSegmentsPtr->next->next->next->next->deleteMark == 0); // t4 should not mark

    checkLinkedListConnectivity();

    updateAndDeleteStore(); // it should delete t1 and t2, and update the t3->count
    checkLinkedListConnectivity(); // vir->t3->t4
    /** Test the update and delete function **/
    assertBytesEqual(segmentsBuffer+1, 25, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" "123456781122334", 25);
    assertUIntegerEqual(getDataOccpiedOffset(),26);
    assertPtrEqual(segmentsBuffer+getPtrOccpiedOffset(), virtualSegmentsPtr - 4);
    assertUIntegerEqual(getDataOccpiedOffset(),26);
    assertPtrEqual(segmentsBuffer+getPtrOccpiedOffset(), virtualSegmentsPtr - 4);
    assertUIntegerEqual(virtualSegmentsPtr->next->seq_h,12); // check it is t3
    assertUIntegerEqual(virtualSegmentsPtr->next->next->seq_h,20); // check it is t4
    assert(virtualSegmentsPtr->next->next->next == 0); // check it is null
    assert(virtualSegmentsPtr->next->deleteMark == 0); // t3 should not mark
    assert(virtualSegmentsPtr->next->next->deleteMark == 0); // t4 should not mark
    assert(virtualSegmentsPtr->next->count == 1); // t3->count should be 1
    assert(virtualSegmentsPtr->next->next->count == 0); // t4->count should be 0

    rst = tryStoreTcpData(t1.seq_h, t1.ack, t1.fragLen, t1.appData, t1.appLen);
    /** Test the store into the empty space between virtualSegmentsPtr and t3 **/
    assert(rst == 1);  // t1 store success
    assertUIntegerEqual(virtualSegmentsPtr->next->seq_h,0x1537d73a); // t1 will insert into the linked list between vir and t3
    assertPtrEqual(virtualSegmentsPtr->next, virtualSegmentsPtr - 1); // // check the t1 store position
    assertUIntegerEqual(virtualSegmentsPtr->next->dataOffset, 1); // t1 data store offset 1
    assertUIntegerEqual(virtualSegmentsPtr->next->next->seq_h,12); // t1->next should be t3
    assertBytesEqual(segmentsBuffer+1, 25, "12345" "\x00\x00\x00\x00\x00" "123456781122334", 25); // check store buffer, t1 stored
    checkLinkedListConnectivity();
    rst = tryStoreTcpData( t2.seq_h, t2.ack, 0, t2.appData, t2.appLen);
    assert(rst == 1); // t2 stored success
    assertUIntegerEqual(virtualSegmentsPtr->next->next->seq_h,0x1537d73f); // t2 will insert into the linked list between t1 and t3
    assertPtrEqual(virtualSegmentsPtr->next->next, virtualSegmentsPtr - 2); // // check the t2 store position
    assertUIntegerEqual(virtualSegmentsPtr->next->next->next->seq_h,12); // check t2->next is t3
    assertBytesEqual(segmentsBuffer+1, 25, "1234567890" "123456781122334", 25); // check store buffer, t2 stored
    checkLinkedListConnectivity();

    rst = tryAssembleTcpData(&seq_h,&tcpData,&dataLen); // delete the t1 and t2, prepare a empty space which not store enough t5 and t6
    assert(rst == 1);
    updateAndDeleteStore();
    checkLinkedListConnectivity();
    assertBytesEqual(segmentsBuffer+1, 25, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" "123456781122334", 25);
    assertUIntegerEqual(virtualSegmentsPtr->next->seq_h,12); // check it is t3
    assertUIntegerEqual(virtualSegmentsPtr->next->next->seq_h,20); // check it is t4
    assert(virtualSegmentsPtr->next->next->next == 0); // check it is null
    assert(virtualSegmentsPtr->next->deleteMark == 0); // t3 should not mark
    assert(virtualSegmentsPtr->next->next->deleteMark == 0); // t4 should not mark
    assert(virtualSegmentsPtr->next->count == 2); // t1->count should be 2
    assert(virtualSegmentsPtr->next->next->count == 0); // t2->count should be 0

    struct TcpTest t5 = {0xaabbcc01,0xababbaba,"ABCD",4,11}; // t5->fragLen larger 1 than the t1->fragLen
    struct TcpTest t6 = {0xaabbcc05,0xababbaba,"EFGHIJK",7,0};

    rst = tryStoreTcpData(t5.seq_h, t5.ack, t5.fragLen, t5.appData, t5.appLen);
    /** Test t5 will be stored after t4 **/
    assert(rst == 1); // t5 store
    assertUIntegerEqual(virtualSegmentsPtr->next->next->next->seq_h,0xaabbcc01); // check it is t5
    assertPtrEqual(virtualSegmentsPtr->next->next->next, virtualSegmentsPtr - 1); // // check the t5 store position
    assert(virtualSegmentsPtr->next->next->next->next == 0); // check t5->next is NULL
    assertUIntegerEqual(virtualSegmentsPtr->next->next->next->dataOffset,26); // check t5 data offset
    assertBytesEqual(segmentsBuffer+1, 36, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" "123456781122334" "ABCD" "\x00\x00\x00\x00\x00\x00\x00", 36); // stored t3 t4 t5

    rst = tryAssembleTcpData(&seq_h,&tcpData,&dataLen); // delete the t3 and t4
    assert(rst == 1);
    assertBytesEqual(tcpData, dataLen, "123456781122334", 15);
    updateAndDeleteStore();
    checkLinkedListConnectivity(); // vir->t5
    assertBytesEqual(segmentsBuffer+1, 36, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" "ABCD" "\x00\x00\x00\x00\x00\x00\x00", 36);
    assertUIntegerEqual(virtualSegmentsPtr->next->seq_h,0xaabbcc01); // check it is t5
    assert(virtualSegmentsPtr->next->next == 0); // check t5-> is null
    assert(virtualSegmentsPtr->next->deleteMark == 0); // t5 should not mark
    assert(virtualSegmentsPtr->next->count == 1); // t5->count == 1
    assertUIntegerEqual(getDataOccpiedOffset(),37);
    assertPtrEqual(segmentsBuffer+getPtrOccpiedOffset(), virtualSegmentsPtr - 1);

    rst = tryAssembleTcpData(&seq_h,&tcpData,&dataLen); // don't have a assembled data
    /** Test not assembled **/
    assert(rst == 0);
    assert(tcpData == 0);
    assert(dataLen == 0);
    updateAndDeleteStore();
    checkLinkedListConnectivity();
    assertBytesEqual(segmentsBuffer+1, 36, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" "ABCD" "\x00\x00\x00\x00\x00\x00\x00", 36);
    assertUIntegerEqual(virtualSegmentsPtr->next->seq_h,0xaabbcc01); // check it is t5
    assert(virtualSegmentsPtr->next->next == 0); // check t5-> is null
    assert(virtualSegmentsPtr->next->deleteMark == 0); // t5 should not mark
    assert(virtualSegmentsPtr->next->count == 2); // t5->count == 2

    rst = tryStoreTcpData( t6.seq_h, t6.ack, 0, t6.appData, t6.appLen);
    /** Test t6 will be stored after t5 **/
    assert(rst == 1); // t6 store
    assertUIntegerEqual(virtualSegmentsPtr->next->next->seq_h,0xaabbcc05); // check it is t6
    assertPtrEqual(virtualSegmentsPtr->next->next, virtualSegmentsPtr - 2); // // check the t6 store position
    assert(virtualSegmentsPtr->next->next->next == 0); // check t6->next is NULL
    assertBytesEqual(segmentsBuffer+1, 36, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" "ABCD" "EFGHIJK", 36);

    rst = tryAssembleTcpData(&seq_h,&tcpData,&dataLen); // assemble t5, t6
    assert(rst == 1);
    assertBytesEqual(tcpData, dataLen, "ABCD" "EFGHIJK", 11);
    updateAndDeleteStore();
    checkLinkedListConnectivity();
    assertBytesEqual(segmentsBuffer+1, 36, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00\x00\x00\x00", 36);
    assert(virtualSegmentsPtr->next == 0); // check no segments be stored
    assertUIntegerEqual(getDataOccpiedOffset(),1);
    assertPtrEqual(segmentsBuffer+getPtrOccpiedOffset(), virtualSegmentsPtr);

    rst = tryStoreTcpData(t5.seq_h, t5.ack, t5.fragLen, t5.appData, t5.appLen);
    /** Test t5 will be stored at begin **/
    assert(rst == 1); // t5 store
    assertUIntegerEqual(virtualSegmentsPtr->next->seq_h,0xaabbcc01); // check it is t5
    assertUIntegerEqual(virtualSegmentsPtr->next->dataOffset,1); // check t5 data offset
    assert(virtualSegmentsPtr->next->next == 0); // check t5->next is NULL
    assertBytesEqual(segmentsBuffer+1, 15, "ABCD" "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 15); // stored t5

    assert(segments_fini());
}

void test_segments_2()
{
    pr_debug("TEST SEGMENTS %d\n",testNum++);
    assert(segments_init());

    int rst = 0;
    unsigned int seq_h =0;
    const char *tcpData = 0;
    unsigned short dataLen = 0;

    /** Test three tcp segments to assembled **/
    struct TcpTest t1 = {10000,0xa1a2a3a4,"111",3,7};
    struct TcpTest t2 = {10003,0xa1a2a3a4,"22",2,0};
    struct TcpTest t3 = {10005,0xa1a2a3a4,"33",2,0};

    struct TcpTest t4 = {40000,0xb1b2b3b4,"4",1,3};
    struct TcpTest t5 = {40001,0xb1b2b3b4,"55",2,0};

    struct TcpTest t6 = {600000,0xc1c2c3c4,"66",2,8};
    struct TcpTest t7 = {600002,0xc1c2c3c4,"77",2,0};
    struct TcpTest t8 = {600004,0xc1c2c3c4,"8888",4,0};

    rst = tryStoreTcpData(t1.seq_h, t1.ack, t1.fragLen, t1.appData, t1.appLen);
    assert(rst == 1);  // t1 store success
    assertUIntegerEqual(virtualSegmentsPtr->next->seq_h,10000); // check it is t1
    assertUIntegerEqual(virtualSegmentsPtr->next->dataOffset, 1); // t1 data store offset 1

    rst = tryStoreTcpData(t4.seq_h, t4.ack, t4.fragLen, t4.appData, t4.appLen);
    assert(rst == 1);  // t4 store success
    assertUIntegerEqual(virtualSegmentsPtr->next->next->seq_h,40000); // check it is t4
    assertUIntegerEqual(virtualSegmentsPtr->next->next->dataOffset, 8); // t4 data store offset 8

    rst = tryStoreTcpData(t6.seq_h, t6.ack, t6.fragLen, t6.appData, t6.appLen);
    assert(rst == 1);  // t6 store success
    assertUIntegerEqual(virtualSegmentsPtr->next->next->next->seq_h,600000); // check it is t6
    assertUIntegerEqual(virtualSegmentsPtr->next->next->next->dataOffset, 11); // t6 data store offset 11

    checkLinkedListConnectivity(); // vir->t1->t4->t6
    assertBytesEqual(segmentsBuffer+1, 20, "111" "\x00\x00\x00\x00" "4" "\x00\x00" "66" "\x00\x00\x00\x00\x00\x00\x00\x00", 20); // stored t1, t4, t6

    rst = tryStoreTcpData(t2.seq_h, t2.ack, 0, t2.appData, t2.appLen);
    assert(rst == 1);  // t2 store success
    assertUIntegerEqual(virtualSegmentsPtr->next->next->seq_h,10003); // check it is t2
    assertUIntegerEqual(virtualSegmentsPtr->next->next->dataOffset, 4); // t2 data store offset 4

    rst = tryStoreTcpData(t8.seq_h, t8.ack, 0, t8.appData, t8.appLen);
    assert(rst == 1);  // t8 store success
    assertUIntegerEqual(virtualSegmentsPtr->next->next->next->next->next->seq_h,600004); // check it is t8
    assertUIntegerEqual(virtualSegmentsPtr->next->next->next->next->next->dataOffset, 15); // t8 data store offset 15

    rst = tryStoreTcpData(t5.seq_h, t5.ack, 0, t5.appData, t5.appLen);
    assert(rst == 1);  // t5 store success
    assertUIntegerEqual(virtualSegmentsPtr->next->next->next->next->seq_h,40001); // check it is t5
    assertUIntegerEqual(virtualSegmentsPtr->next->next->next->next->dataOffset, 9); // t5 data store offset 9

    checkLinkedListConnectivity(); // vir->t1->t2->t4->t5->t6->t8
    assertBytesEqual(segmentsBuffer+1, 20, "11122" "\x00\x00" "455" "66" "\x00\x00" "8888" "\x00\x00", 20); // stored t1->t2->t4->t5->t6->t8

    rst = tryStoreTcpData(t7.seq_h, t7.ack, 0, t7.appData, t7.appLen);
    assert(rst == 1);  // t7 store success
    assertUIntegerEqual(virtualSegmentsPtr->next->next->next->next->next->next->seq_h,600002); // check it is t7
    assertUIntegerEqual(virtualSegmentsPtr->next->next->next->next->next->next->dataOffset, 13); // t7 data store offset 13

    rst = tryStoreTcpData(t3.seq_h, t3.ack, 0, t3.appData, t3.appLen);
    assert(rst == 1);  // t3 store success
    assertUIntegerEqual(virtualSegmentsPtr->next->next->next->seq_h,10005); // check it is t3
    assertUIntegerEqual(virtualSegmentsPtr->next->next->next->dataOffset, 6); // t3 data store offset 6
    assertPtrEqual(virtualSegmentsPtr->next->next->next, virtualSegmentsPtr - 8); // // check the t3 store position
    assertUIntegerEqual(getDataOccpiedOffset(),19);
    assertPtrEqual(segmentsBuffer+getPtrOccpiedOffset(), virtualSegmentsPtr-8);

    checkLinkedListConnectivity(); // vir->t1->t2->t3->t4->t5->t6->t7->t8
    assertBytesEqual(segmentsBuffer+1, 20, "1112233" "455" "66778888" "\x00\x00", 20); // stored t1->t2->t3->t4->t5->t6->t7->t8

    rst = tryAssembleTcpData(&seq_h,&tcpData,&dataLen); // assemble t1, t2, t3
    assert(rst == 1);
    assertBytesEqual(tcpData, dataLen, "1112233", 7);
    updateAndDeleteStore();
    checkLinkedListConnectivity();

    rst = tryAssembleTcpData(&seq_h,&tcpData,&dataLen); // assemble t4, t5
    assert(rst == 1);
    assertBytesEqual(tcpData, dataLen, "455", 3);
    updateAndDeleteStore();
    checkLinkedListConnectivity();
    assertBytesEqual(segmentsBuffer+1, 20, "\x00\x00\x00\x00\x00\x00\x00" "\x00\x00\x00" "66778888" "\x00\x00", 20); // stored t1->t2->t3->t4->t5->t6->t7->t8
    assertUIntegerEqual(virtualSegmentsPtr->next->seq_h,600000); // check it is t6
    assertUIntegerEqual(virtualSegmentsPtr->next->dataOffset, 11); // t4 data store offset 11
    assert(virtualSegmentsPtr->next->count == 2); // t6->count == 2
    assert(virtualSegmentsPtr->next->deleteMark == 0); // t6 should not mark
    assert(virtualSegmentsPtr->next->next->next->next == 0); // check t6->t7->t8->next is null

    rst = tryStoreTcpData(t4.seq_h, t4.ack, t4.fragLen, t4.appData, t4.appLen);
    assert(rst == 1);  // t4 store success
    assertUIntegerEqual(virtualSegmentsPtr->next->seq_h,40000); // check it is t4
    assertUIntegerEqual(virtualSegmentsPtr->next->dataOffset, 1); // t4 data store offset 8

    rst = tryStoreTcpData(t1.seq_h, t1.ack, t1.fragLen, t1.appData, t1.appLen);
    assert(rst == 1);  // t1 store success
    assertUIntegerEqual(virtualSegmentsPtr->next->next->seq_h,10000); // check it is t1
    assertUIntegerEqual(virtualSegmentsPtr->next->next->dataOffset, 4); // t1 data store offset 1
    checkLinkedListConnectivity(); // vir->t4->t1->t6->t7->t8
    assertBytesEqual(segmentsBuffer+1, 20, "4\x00\x00" "111\x00\x00\x00\x00" "66778888" "\x00\x00", 20); // stored t1->t2->t3->t4->t5->t6->t7->t8
    assertPtrEqual(virtualSegmentsPtr->next->next, virtualSegmentsPtr - 2); // // check the t6 store position

    assert(virtualSegmentsPtr->next->count == 0); // t4->count
    assert(virtualSegmentsPtr->next->next->count == 0); // t1->count
    assert(virtualSegmentsPtr->next->next->next->count == 2); // t6->count
    assert(virtualSegmentsPtr->next->next->next->next->count == 0); // t7->count
    assert(virtualSegmentsPtr->next->next->next->next->next->count ==0); // t8->count

    /** Test count to max **/
    updateAndDeleteStore();
    updateAndDeleteStore();
    updateAndDeleteStore();
    updateAndDeleteStore();
    updateAndDeleteStore();
    updateAndDeleteStore();
    updateAndDeleteStore();

    assert(virtualSegmentsPtr->next->count == 7); // t4->count
    assert(virtualSegmentsPtr->next->next->count == 7); // t1->count
    assert(virtualSegmentsPtr->next->next->next->count == 9); // t6->count
    assert(virtualSegmentsPtr->next->next->next->next->count == 0); // t7->count
    assert(virtualSegmentsPtr->next->next->next->next->next->count ==0); // t8->count

    updateAndDeleteStore(); // t6,t7,t8 will be deleted

    assert(virtualSegmentsPtr->next->count == 8); // t4->count
    assert(virtualSegmentsPtr->next->next->count == 8); // t1->count
    assertUIntegerEqual(virtualSegmentsPtr->next->seq_h,40000); // check it is t4
    assertUIntegerEqual(virtualSegmentsPtr->next->next->seq_h,10000); // check it is t1
    assert(virtualSegmentsPtr->next->next->next == 0); // deleted, NULL
    checkLinkedListConnectivity(); // vir->t4->t1
    assertBytesEqual(segmentsBuffer+1, 20, "4" "\x00\x00" "111" "\x00\x00\x00\x00"  "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 20); // stored t4, t1

    assert(segments_fini());
}


void test_segments_3()
{
    pr_debug("TEST SEGMENTS %d\n",testNum++);
    assert(segments_init());

    int rst = 0;

    /** Test the buffer space not enough **/
    struct TcpTest t1 = {0xff112233,0xa1a2a3a4,"11",2,10};

    struct TcpTest t2 = {0xff445566,0xb1b2b3b4,"222",3, SEGMENTS_BUFFER_LEN -1 - segStructSize * 4 - 10 *2};

    struct TcpTest t3 = {0xff778899,0xc1c2c3c4,"3333",4,11};

    struct TcpTest t4 = {0xffaabbcc,0xd1d2d3d4,"4",1,10};

    struct TcpTest t5 = {0xffddeeff,0xe1e2e3e4,"4",1,1};

    rst = tryStoreTcpData(t1.seq_h, t1.ack, t1.fragLen, t1.appData, t1.appLen);
    assert(rst == 1);  // t1 store success
    assertUIntegerEqual(virtualSegmentsPtr->next->seq_h,0xff112233); // check it is t1
    assertUIntegerEqual(virtualSegmentsPtr->next->dataOffset, 1); // t1 data store offset 1

    rst = tryStoreTcpData(t2.seq_h, t2.ack, t2.fragLen, t2.appData, t2.appLen);
    assert(rst == 1);  // t2 store success
    assertUIntegerEqual(virtualSegmentsPtr->next->next->seq_h,0xff445566); // check it is t2
    assertUIntegerEqual(virtualSegmentsPtr->next->next->dataOffset, 11); // t2 data store offset 11
    assertPtrEqual(virtualSegmentsPtr->next->next, virtualSegmentsPtr - 2); // check the t2 store position
    assertUIntegerEqual(getDataOccpiedOffset(), 11 + SEGMENTS_BUFFER_LEN -1 - segStructSize * 4 - 10 *2);
    assertPtrEqual(segmentsBuffer+getPtrOccpiedOffset(), virtualSegmentsPtr-2);

    rst = tryStoreTcpData(t3.seq_h, t3.ack, t3.fragLen, t3.appData, t3.appLen);
    assert(rst == 0);  // t3 store fail

    rst = tryStoreTcpData(t4.seq_h, t4.ack, t4.fragLen, t4.appData, t4.appLen);
    assert(rst == 1);  // t4 store success
    assertUIntegerEqual(virtualSegmentsPtr->next->next->next->seq_h,0xffaabbcc); // check it is t4
    assertPtrEqual(segmentsBuffer+getDataOccpiedOffset(), segmentsBuffer+getPtrOccpiedOffset()); // store full exactly

    rst = tryStoreTcpData(t5.seq_h, t5.ack, t5.fragLen, t5.appData, t5.appLen);
    assert(rst == 0);  // t5 store fail


    assert(segments_fini());
}

void test_segments_4()
{
    pr_debug("TEST SEGMENTS %d\n",testNum++);
    assert(segments_init());

    int rst = 0;
    /** Test the buffer space not enough **/
    struct TcpTest t1 = {100, 0xaabbaabb,"abcd", 4, SEGMENTS_BUFFER_LEN -1 - segStructSize * 2 + 1};

    struct TcpTest t2 = {100, 0xaabbaabb,"abcd", 4, SEGMENTS_BUFFER_LEN -1 - segStructSize * 2};

    rst = tryStoreTcpData(t1.seq_h, t1.ack, t1.fragLen, t1.appData, t1.appLen);
    assert(rst == 0);  // t1 store fail

    rst = tryStoreTcpData(t2.seq_h, t2.ack, t2.fragLen, t2.appData, t2.appLen);
    assert(rst == 1);  // t2 store success
    assertPtrEqual(segmentsBuffer+getDataOccpiedOffset(), segmentsBuffer+getPtrOccpiedOffset()); // store full exactly

    assert(segments_fini());
}

void test_segments_5()
{
    pr_debug("TEST SEGMENTS %d\n",testNum++);
    assert(segments_init());

    int rst = 0;
    unsigned int seq_h = 0;
    const char *tcpData = 0;
    unsigned short dataLen = 0;

    /** Test the actually packet **/
    const char *tcpDataHexStream1 = "05000203100000009808000004000000800800000100000001000000000000000000020068080000680800004d454f5704000000a301000000000000c0000000000000463903000000000000c0000000000000460000000040080000300800000000000001100800cccccccc6000000000000000300800007000000000000000020000000200000000000000000000000000000000000000000002000400020000000000020000003903000000000000c000000000000046b601000000000000c0000000000000460200000010060000b001000001100800cccccccc000600000000000009000000000002000400020008000200090000004d3ac1391e01d01196750020afd8adb3e2fd1df3b607d211b2d80060083ba1fb84b296b1b4ba1a10b69c00aa00341d07723ac1391e01d01196750020afd8adb34f3ac1391e01d01196750020afd8adb34e3ac1391e01d01196750020afd8adb30b01000000000000c000000000000046047022398fa1574b8b0a5235670f446827b4c0859328bc4cbd78e5fc5146f08f09000000000000000000000000000000000000000000000002400080024000800000000000000000090000000c0002001000020014000200180002001c00020000000000000000002000020024000200b0000000b00000004d454f57010000004d3ac1391e01d01196750020afd8adb30000000005000000149b6b1f597e74bba75862e0ef34bf5a098c0000d8320000d9c1062469d8195b3600200007004400450053004b0054004f0050002d004b00420047004a00410049003000000007003100390032002e00310036002e00310030002e0032003300000000000900ffff00001e00ffff00001000ffff00000a00ffff00001600ffff00001f00ffff00000e00ffff00000000b0000000b00000004d454f5701000000e2fd1df3b607d211b2d80060083ba1fb0000000005000000149b6b1f597e74bba75862e0ef34bf5a10740000d83200001314c347fd1b8f083600200007004400450053004b0054004f0050002d004b00420047004a00410049003000000007003100390032002e00310036002e00310030002e0032003300000000000900ffff00001e00ffff00001000ffff00000a00ffff00001600ffff00001f00ffff00000e00ffff00000000b0000000b00000004d454f570100000084b296b1b4ba1a10b69c00aa00341d070000000005000000149b6b1f597e74bba75862e0ef34bf5a11fc0000d8320000265e254518eba4f83600200007004400450053004b0054004f0050002d004b00420047004a00410049003000000007003100390032002e00310036002e00310030002e0032003300000000000900ffff00001e00ffff00001000ffff00000a00ffff00001600ffff00001f00ffff00000e00ffff00000000b0000000b00000004d454f5701000000723ac1391e01d01196750020afd8adb30000000005000000149b6b1f597e74bba75862e0ef34bf5a128c0000d832000084a74fb9b737754f3600200007004400450053004b0054004f0050002d004b00420047004a00410049003000000007003100390032002e00310036002e00310030002e0032003300000000000900ffff00001e00ffff00001000ffff00000a00ffff00001600ffff00001f00ffff00000e00ffff00000000b0000000b00000004d454f57010000004f3ac1391e01d01196750020afd8adb30000000005000000149b6b1f597e74bba75862e0ef34bf5a13880000d8320000c1c2b127c5eef0383600200007004400450053004b0054004f0050002d004b00420047004a00410049003000000007003100390032002e00310036002e00310030002e0032003300000000000900ffff00001e00ffff00001000ffff00000a00ffff00001600ffff00001f00ffff00000e00ffff00000000b0000000b00000004d454f5701000000047022398fa1574b8b0a5235670f44680000000005000000149b6b1f597e74bba75862e0ef34bf5a14d80000d8320000baf12287";
    unsigned int tcpDataHexLen1 = strlen(tcpDataHexStream1);
    char *tcpDataAddr1 = NULL;
    unsigned int tcpDataLen1 = readableHexStreamToBytes(tcpDataHexStream1, tcpDataHexLen1, &tcpDataAddr1);
    struct TcpTest t1 = {0x1537d73a, 0x55853b6b,tcpDataAddr1, tcpDataLen1, 2200};

    const char *tcpDataHexStream2 = "2bac28d33600200007004400450053004b0054004f0050002d004b00420047004a00410049003000000007003100390032002e00310036002e00310030002e0032003300000000000900ffff00001e00ffff00001000ffff00000a00ffff00001600ffff00001f00ffff00000e00ffff00000000b0000000b00000004d454f570100000027b4c0859328bc4cbd78e5fc5146f08f0000000005000000149b6b1f597e74bba75862e0ef34bf5a15580000d83200008313e25996cf1e3a3600200007004400450053004b0054004f0050002d004b00420047004a00410049003000000007003100390032002e00310036002e00310030002e0032003300000000000900ffff00001e00ffff00001000ffff00000a00ffff00001600ffff00001f00ffff00000e00ffff000000000000000001100800cccccccca0010000000000000000000000000200149b6b1f597e74bb04000200006c0000d8320000c9578baf6574b4510200000005000700b5000000b5002e0007004400450053004b0054004f0050002d004b00420047004a004100490030005b00350033003100340036005d00000007003100390032002e00310036002e00310030002e00320033005b00350033003100340036005d00000000000a00ffff4e005400200041005500540048004f0052004900540059005c00530059005300540045004d0000001e00ffff4e005400200041005500540048004f0052004900540059005c00530059005300540045004d0000001000ffff68006f00730074002f004400450053004b0054004f0050002d004b00420047004a0041004900300000000900ffff68006f00730074002f004400450053004b0054004f0050002d004b00420047004a0041004900300000001600ffff4e005400200041005500540048004f0052004900540059005c00530059005300540045004d0000001f00ffff4e005400200041005500540048004f0052004900540059005c00530059005300540045004d0000000000000000000000";
    unsigned int tcpDataHexLen2 = strlen(tcpDataHexStream2);
    char *tcpDataAddr2 = NULL;
    unsigned int tcpDataLen2 = readableHexStreamToBytes(tcpDataHexStream2, tcpDataHexLen2, &tcpDataAddr2);
    struct TcpTest t2 = {0x1537dcee, 0x55853b6b,tcpDataAddr2, tcpDataLen2, 0};

    const char *tcpDataHexStream3 = "05000203100000009808000004000000800800000100000001000000000000000000020068080000680800004d454f5704000000a301000000000000c0000000000000463903000000000000c0000000000000460000000040080000300800000000000001100800cccccccc6000000000000000300800007000000000000000020000000200000000000000000000000000000000000000000002000400020000000000020000003903000000000000c000000000000046b601000000000000c0000000000000460200000010060000b001000001100800cccccccc000600000000000009000000000002000400020008000200090000004d3ac1391e01d01196750020afd8adb3e2fd1df3b607d211b2d80060083ba1fb84b296b1b4ba1a10b69c00aa00341d07723ac1391e01d01196750020afd8adb34f3ac1391e01d01196750020afd8adb34e3ac1391e01d01196750020afd8adb30b01000000000000c000000000000046047022398fa1574b8b0a5235670f446827b4c0859328bc4cbd78e5fc5146f08f09000000000000000000000000000000000000000000000002400080024000800000000000000000090000000c0002001000020014000200180002001c00020000000000000000002000020024000200b0000000b00000004d454f57010000004d3ac1391e01d01196750020afd8adb30000000005000000149b6b1f597e74bba75862e0ef34bf5a098c0000d8320000d9c1062469d8195b3600200007004400450053004b0054004f0050002d004b00420047004a00410049003000000007003100390032002e00310036002e00310030002e0032003300000000000900ffff00001e00ffff00001000ffff00000a00ffff00001600ffff00001f00ffff00000e00ffff00000000b0000000b00000004d454f5701000000e2fd1df3b607d211b2d80060083ba1fb0000000005000000149b6b1f597e74bba75862e0ef34bf5a10740000d83200001314c347fd1b8f083600200007004400450053004b0054004f0050002d004b00420047004a00410049003000000007003100390032002e00310036002e00310030002e0032003300000000000900ffff00001e00ffff00001000ffff00000a00ffff00001600ffff00001f00ffff00000e00ffff00000000b0000000b00000004d454f570100000084b296b1b4ba1a10b69c00aa00341d070000000005000000149b6b1f597e74bba75862e0ef34bf5a11fc0000d8320000265e254518eba4f83600200007004400450053004b0054004f0050002d004b00420047004a00410049003000000007003100390032002e00310036002e00310030002e0032003300000000000900ffff00001e00ffff00001000ffff00000a00ffff00001600ffff00001f00ffff00000e00ffff00000000b0000000b00000004d454f5701000000723ac1391e01d01196750020afd8adb30000000005000000149b6b1f597e74bba75862e0ef34bf5a128c0000d832000084a74fb9b737754f3600200007004400450053004b0054004f0050002d004b00420047004a00410049003000000007003100390032002e00310036002e00310030002e0032003300000000000900ffff00001e00ffff00001000ffff00000a00ffff00001600ffff00001f00ffff00000e00ffff00000000b0000000b00000004d454f57010000004f3ac1391e01d01196750020afd8adb30000000005000000149b6b1f597e74bba75862e0ef34bf5a13880000d8320000c1c2b127c5eef0383600200007004400450053004b0054004f0050002d004b00420047004a00410049003000000007003100390032002e00310036002e00310030002e0032003300000000000900ffff00001e00ffff00001000ffff00000a00ffff00001600ffff00001f00ffff00000e00ffff00000000b0000000b00000004d454f5701000000047022398fa1574b8b0a5235670f44680000000005000000149b6b1f597e74bba75862e0ef34bf5a14d80000d8320000baf122872bac28d33600200007004400450053004b0054004f0050002d004b00420047004a00410049003000000007003100390032002e00310036002e00310030002e0032003300000000000900ffff00001e00ffff00001000ffff00000a00ffff00001600ffff00001f00ffff00000e00ffff00000000b0000000b00000004d454f570100000027b4c0859328bc4cbd78e5fc5146f08f0000000005000000149b6b1f597e74bba75862e0ef34bf5a15580000d83200008313e25996cf1e3a3600200007004400450053004b0054004f0050002d004b00420047004a00410049003000000007003100390032002e00310036002e00310030002e0032003300000000000900ffff00001e00ffff00001000ffff00000a00ffff00001600ffff00001f00ffff00000e00ffff000000000000000001100800cccccccca0010000000000000000000000000200149b6b1f597e74bb04000200006c0000d8320000c9578baf6574b4510200000005000700b5000000b5002e0007004400450053004b0054004f0050002d004b00420047004a004100490030005b00350033003100340036005d00000007003100390032002e00310036002e00310030002e00320033005b00350033003100340036005d00000000000a00ffff4e005400200041005500540048004f0052004900540059005c00530059005300540045004d0000001e00ffff4e005400200041005500540048004f0052004900540059005c00530059005300540045004d0000001000ffff68006f00730074002f004400450053004b0054004f0050002d004b00420047004a0041004900300000000900ffff68006f00730074002f004400450053004b0054004f0050002d004b00420047004a0041004900300000001600ffff4e005400200041005500540048004f0052004900540059005c00530059005300540045004d0000001f00ffff4e005400200041005500540048004f0052004900540059005c00530059005300540045004d0000000000000000000000";
    unsigned int tcpDataHexLen3 = strlen(tcpDataHexStream3);
    char *tcpDataAddr3 = NULL;
    unsigned int tcpDataLen3 = readableHexStreamToBytes(tcpDataHexStream3, tcpDataHexLen3, &tcpDataAddr3);

    rst = tryStoreTcpData(t1.seq_h, t1.ack, t1.fragLen, t1.appData, t1.appLen);
    assert(rst == 1);  // t1 store success

    rst = tryStoreTcpData(t2.seq_h, t2.ack, 0, t2.appData, t2.appLen);
    assert(rst == 1);  // t2 store success
    assertUIntegerEqual(getDataOccpiedOffset(), 2200 + 1);

    rst = tryAssembleTcpData(&seq_h,&tcpData,&dataLen); // assemble t1, t2
    assert(rst == 1);
    assertBytesEqual(tcpData, dataLen, tcpDataAddr3, tcpDataLen3);
    assertUIntegerEqual(seq_h,0x1537d73a);

    updateAndDeleteStore();
    assert(virtualSegmentsPtr->next == 0);

    free(tcpDataAddr1);
    free(tcpDataAddr2);
    free(tcpDataAddr3);
    assert(segments_fini());
}

void test_segments_6()
{
    pr_debug("TEST SEGMENTS %d\n",testNum++);
    assert(segments_init());

    int rst = 0;
    unsigned int seq_h = 0;
    const char *tcpData = 0;
    unsigned short dataLen = 0;

    /** Test the wrong pos */
    struct TcpTest t1 = {100, 0xaabbaabb,"abcd", 4, 8};
    struct TcpTest t2 = {100, 0xaabbaabb,"efgh", 4, 0};
    struct TcpTest t3 = {99, 0xaabbaabb,"ijkl", 4, 0};
    /** Test the wrong length */
    struct TcpTest t4 = {104, 0xaabbaabb,"mnopq", 5, 0};
    /** Test the wrong pos */
    struct TcpTest t5 = {103, 0xaabbaabb,"mnopq", 5, 0};

    rst = tryStoreTcpData(t1.seq_h, t1.ack, t1.fragLen, t1.appData, t1.appLen);
    assert(rst == 1);  // t1 store success

    rst = tryStoreTcpData(t2.seq_h, t2.ack, t2.fragLen, t2.appData, t2.appLen);
    assert(rst == -1);  // t2 store failed  repeat store

    rst = tryStoreTcpData(t3.seq_h, t3.ack, t3.fragLen, t3.appData, t3.appLen);
    assertIntegerEqual(rst,-3);  // t3 store failed  seq error

    rst = tryStoreTcpData(t4.seq_h, t4.ack, t4.fragLen, t4.appData, t4.appLen);
    assert(rst == -4);  // t4 store failed  length error

    rst = tryStoreTcpData(t5.seq_h, t5.ack, t5.fragLen, t5.appData, t5.appLen);
    assertIntegerEqual(rst,-5);  // t5 store failed  pos conflict

    assert(segments_fini());
}

void test_segments()
{
    assert(segStructSize%4 == 0);
    test_segments_1();
    test_segments_2();
    test_segments_3();
    test_segments_4();
    test_segments_5();
    test_segments_6();
}

