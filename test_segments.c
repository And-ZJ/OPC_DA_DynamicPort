#include "test_segments.h"

#include "BytesTools.h"
#include "AssertTools.h"

extern char *segmentsBuffer;
extern struct TcpSegments;
extern struct TcpSegments *virtualSegmentsPtr;

unsigned char segptrSize = sizeof(struct TcpSegments);

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
    const char *tcpData = 0;
    unsigned short dataLen = 0;
    struct TcpTest t1 = {0x1537d73a,0x6b3b8555,"12345",5,10};
    struct TcpTest t2 = {0x1537d73f,0x6b3b8555,"67890",5,0};
    rst = tryStoreNewTcpData(t1.seq_h, t1.ack, t1.fragLen, t1.appData, t1.appLen);
    assert(rst == 1);  // t1 store success
    assertUIntergerEqual(virtualSegmentsPtr->next->seq_h,0x1537d73a); // check it is t1
    assertUIntergerEqual(virtualSegmentsPtr->next->dataOffset, 1); // t1 data store offset
    rst = tryStoreNewTcpData(t1.seq_h, t1.ack, t1.fragLen, t1.appData, t1.appLen);
    /** Test not store repeat **/
    assert(rst == -1); // t1 has stored.
    assertBytesEqual(segmentsBuffer+1, 10, "12345\x00\x00\x00\x00\x00", 10);  // only t1

    rst = tryStoreAndAssembleNewTcpData( t2.seq_h, t2.ack, t2.appData, t2.appLen);
    /** Test assemble the segments **/
    assert(rst == 1); // t2 store success
    assertUIntergerEqual(virtualSegmentsPtr->next->next->seq_h,0x1537d73f); // check it is t2
    assert(virtualSegmentsPtr->next->next->next == 0); // check next is null
    rst = tryStoreAndAssembleNewTcpData( t2.seq_h, t2.ack, t2.appData, t2.appLen);
    assert(rst == -1); // t2 has stored.
    assertBytesEqual(segmentsBuffer+1, 10, "1234567890", 10);  // t1, t2

    /** Test the linked list ptr **/
    checkLinkedListConnectivity();

    struct TcpTest t3 = {12,55,"12345678",8,15};
    struct TcpTest t4 = {20,55,"1122334",7,0};

    rst = tryStoreAndAssembleNewTcpData( t4.seq_h, t4.ack, t4.appData, t4.appLen);
    /** Test not be assemble when don't have the head segments **/
    assert(rst == -2); // t4 not store
    assertBytesEqual(segmentsBuffer+1, 25, "1234567890\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 25);  // t1, t2

    rst = tryStoreNewTcpData(t3.seq_h, t3.ack, t3.fragLen, t3.appData, t3.appLen);
    /** Test it will be stored after **/
    assert(rst == 1); // t3 store
    assertBytesEqual(segmentsBuffer+1, 25, "123456789012345678\x00\x00\x00\x00\x00\x00\x00", 25);  //t1, t2, t3
    assertUIntergerEqual(virtualSegmentsPtr->next->next->next->seq_h,12); // check it is t3

    rst = tryStoreAndAssembleNewTcpData( t4.seq_h, t4.ack, t4.appData, t4.appLen);
    assert(rst == 1); // t4 store
    assertUIntergerEqual(virtualSegmentsPtr->next->next->next->next->seq_h,20); // check it is t4
    assertBytesEqual(segmentsBuffer+1, 25, "1234567890123456781122334", 25);  // t1, t2, t3, t4

    rst = getAssembleTcpData(&tcpData,&dataLen);
    /** Test it will got a assembled tcp data by t1 and t2. **/
    assert(rst == 1); // find t1 t2 has been assembled
    assertBytesEqual(tcpData, dataLen, "1234567890", 10);  // assemble t1, t2

    /** Test the delete mark added by assembled function **/
    assert(virtualSegmentsPtr->next->deleteMark == 1); // t1 should mark
    assert(virtualSegmentsPtr->next->next->deleteMark == 1); // t2 should mark
    assert(virtualSegmentsPtr->next->next->next->deleteMark == 0); // t3 should not mark
    assert(virtualSegmentsPtr->next->next->next->next->deleteMark == 0); // t4 should not mark

    checkLinkedListConnectivity();

    updateAndDeleteSegmentsStore(); // it should delete t1 and t2, and update the t3->count
    checkLinkedListConnectivity();
    /** Test the update and delete function **/
    assertBytesEqual(segmentsBuffer+1, 25, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" "123456781122334", 25);
    assertUIntergerEqual(virtualSegmentsPtr->next->seq_h,12); // check it is t3
    assertUIntergerEqual(virtualSegmentsPtr->next->next->seq_h,20); // check it is t4
    assert(virtualSegmentsPtr->next->next->next == 0); // check it is null
    assert(virtualSegmentsPtr->next->deleteMark == 0); // t3 should not mark
    assert(virtualSegmentsPtr->next->next->deleteMark == 0); // t4 should not mark
    assert(virtualSegmentsPtr->next->count == 1); // t1->count should be 1
    assert(virtualSegmentsPtr->next->next->count == 0); // t2->count should be 0

    rst = tryStoreNewTcpData(t1.seq_h, t1.ack, t1.fragLen, t1.appData, t1.appLen);
    /** Test the store into the empty space between virtualSegmentsPtr and t3 **/
    assert(rst == 1);  // t1 store success
    assertUIntergerEqual(virtualSegmentsPtr->next->seq_h,0x1537d73a); // t1 will insert into the linked list between vir and t3
    assertUIntergerEqual(virtualSegmentsPtr->next->dataOffset, 1); // t1 data store offset 1
    assertUIntergerEqual(virtualSegmentsPtr->next->next->seq_h,12); // t1->next should be t3
    assertBytesEqual(segmentsBuffer+1, 25, "12345" "\x00\x00\x00\x00\x00" "123456781122334", 25); // check store buffer, t1 stored
    checkLinkedListConnectivity();
    rst = tryStoreAndAssembleNewTcpData( t2.seq_h, t2.ack, t2.appData, t2.appLen);
    assert(rst == 1); // t2 stored success
    assertUIntergerEqual(virtualSegmentsPtr->next->next->seq_h,0x1537d73f); // t2 will insert into the linked list between t1 and t3
    assertUIntergerEqual(virtualSegmentsPtr->next->next->next->seq_h,12); // check t2->next is t3
    assertBytesEqual(segmentsBuffer+1, 25, "1234567890" "123456781122334", 25); // check store buffer, t2 stored
    checkLinkedListConnectivity();

    rst = getAssembleTcpData(&tcpData,&dataLen); // delete the t1 and t2, prepare a empty space which not store enough t5 and t6
    assert(rst == 1);
    updateAndDeleteSegmentsStore();
    checkLinkedListConnectivity();
    assertBytesEqual(segmentsBuffer+1, 25, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" "123456781122334", 25);
    assertUIntergerEqual(virtualSegmentsPtr->next->seq_h,12); // check it is t3
    assertUIntergerEqual(virtualSegmentsPtr->next->next->seq_h,20); // check it is t4
    assert(virtualSegmentsPtr->next->next->next == 0); // check it is null
    assert(virtualSegmentsPtr->next->deleteMark == 0); // t3 should not mark
    assert(virtualSegmentsPtr->next->next->deleteMark == 0); // t4 should not mark
    assert(virtualSegmentsPtr->next->count == 2); // t1->count should be 2
    assert(virtualSegmentsPtr->next->next->count == 0); // t2->count should be 0

    struct TcpTest t5 = {0xaabbcc01,0xababbaba,"ABCD",4,11}; // t5->fragLen larger 1 than the t1->fragLen
    struct TcpTest t6 = {0xaabbcc05,0xababbaba,"EFGHIJK",7,0};

    rst = tryStoreNewTcpData(t5.seq_h, t5.ack, t5.fragLen, t5.appData, t5.appLen);
    /** Test t5 will be stored after t4 **/
    assert(rst == 1); // t5 store
    assertUIntergerEqual(virtualSegmentsPtr->next->next->next->seq_h,0xaabbcc01); // check it is t5
    assert(virtualSegmentsPtr->next->next->next->next == 0); // check t5->next is NULL
    assertUIntergerEqual(virtualSegmentsPtr->next->next->next->dataOffset,26); // check t5 data offset
    assertBytesEqual(segmentsBuffer+1, 36, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" "123456781122334" "ABCD" "\x00\x00\x00\x00\x00\x00\x00", 36); // stored t3 t4 t5

    rst = getAssembleTcpData(&tcpData,&dataLen); // delete the t3 and t4
    assert(rst == 1);
    assertBytesEqual(tcpData, dataLen, "123456781122334", 15);
    updateAndDeleteSegmentsStore();
    checkLinkedListConnectivity();
    assertBytesEqual(segmentsBuffer+1, 36, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" "ABCD" "\x00\x00\x00\x00\x00\x00\x00", 36);
    assertUIntergerEqual(virtualSegmentsPtr->next->seq_h,0xaabbcc01); // check it is t5
    assert(virtualSegmentsPtr->next->next == 0); // check t5-> is null
    assert(virtualSegmentsPtr->next->deleteMark == 0); // t5 should not mark
    assert(virtualSegmentsPtr->next->count == 1); // t5->count == 1

    rst = getAssembleTcpData(&tcpData,&dataLen); // don't have a assembled data
    /** Test not assembled **/
    assert(rst == 0);
    assert(tcpData == 0);
    assert(dataLen == 0);
    updateAndDeleteSegmentsStore();
    checkLinkedListConnectivity();
    assertBytesEqual(segmentsBuffer+1, 36, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" "ABCD" "\x00\x00\x00\x00\x00\x00\x00", 36);
    assertUIntergerEqual(virtualSegmentsPtr->next->seq_h,0xaabbcc01); // check it is t5
    assert(virtualSegmentsPtr->next->next == 0); // check t5-> is null
    assert(virtualSegmentsPtr->next->deleteMark == 0); // t5 should not mark
    assert(virtualSegmentsPtr->next->count == 2); // t5->count == 2

    rst = tryStoreAndAssembleNewTcpData( t6.seq_h, t6.ack, t6.appData, t6.appLen);
    /** Test t6 will be stored after t5 **/
    assert(rst == 1); // t6 store
    assertUIntergerEqual(virtualSegmentsPtr->next->next->seq_h,0xaabbcc05); // check it is t6
    assert(virtualSegmentsPtr->next->next->next == 0); // check t6->next is NULL
    assertBytesEqual(segmentsBuffer+1, 36, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" "ABCD" "EFGHIJK", 36);

    rst = getAssembleTcpData(&tcpData,&dataLen); // assemble t5, t6
    assert(rst == 1);
    assertBytesEqual(tcpData, dataLen, "ABCD" "EFGHIJK", 11);
    updateAndDeleteSegmentsStore();
    checkLinkedListConnectivity();
    assertBytesEqual(segmentsBuffer+1, 36, "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" "\x00\x00\x00\x00" "\x00\x00\x00\x00\x00\x00\x00", 36);
    assert(virtualSegmentsPtr->next == 0); // check no segments be stored

    rst = tryStoreNewTcpData(t5.seq_h, t5.ack, t5.fragLen, t5.appData, t5.appLen);
    /** Test t5 will be stored at begin **/
    assert(rst == 1); // t5 store
    assertUIntergerEqual(virtualSegmentsPtr->next->seq_h,0xaabbcc01); // check it is t5
    assertUIntergerEqual(virtualSegmentsPtr->next->dataOffset,1); // check t5 data offset
    assert(virtualSegmentsPtr->next->next == 0); // check t5->next is NULL
    assertBytesEqual(segmentsBuffer+1, 15, "ABCD" "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 15); // stored t5

    assert(segments_fini());
}

void test_segments_2()
{
    pr_debug("TEST SEGMENTS %d\n",testNum++);
    assert(segments_init());

    int rst = 0;
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

    rst = tryStoreNewTcpData(t1.seq_h, t1.ack, t1.fragLen, t1.appData, t1.appLen);
    assert(rst == 1);  // t1 store success
    assertUIntergerEqual(virtualSegmentsPtr->next->seq_h,10000); // check it is t1
    assertUIntergerEqual(virtualSegmentsPtr->next->dataOffset, 1); // t1 data store offset 1

    rst = tryStoreNewTcpData(t4.seq_h, t4.ack, t4.fragLen, t4.appData, t4.appLen);
    assert(rst == 1);  // t4 store success
    assertUIntergerEqual(virtualSegmentsPtr->next->next->seq_h,40000); // check it is t4
    assertUIntergerEqual(virtualSegmentsPtr->next->next->dataOffset, 8); // t4 data store offset 8

    rst = tryStoreNewTcpData(t6.seq_h, t6.ack, t6.fragLen, t6.appData, t6.appLen);
    assert(rst == 1);  // t6 store success
    assertUIntergerEqual(virtualSegmentsPtr->next->next->next->seq_h,600000); // check it is t6
    assertUIntergerEqual(virtualSegmentsPtr->next->next->next->dataOffset, 11); // t4 data store offset 11

    checkLinkedListConnectivity(); // vir->t1->t4->t6
    assertBytesEqual(segmentsBuffer+1, 20, "111" "\x00\x00\x00\x00" "4" "\x00\x00" "66" "\x00\x00\x00\x00\x00\x00\x00\x00", 20); // stored t1, t4, t6

    rst = tryStoreAndAssembleNewTcpData(t2.seq_h, t2.ack, t2.appData, t2.appLen);
    assert(rst == 1);  // t2 store success
    assertUIntergerEqual(virtualSegmentsPtr->next->next->seq_h,10003); // check it is t2
    assertUIntergerEqual(virtualSegmentsPtr->next->next->dataOffset, 4); // t2 data store offset 4

    rst = tryStoreAndAssembleNewTcpData(t8.seq_h, t8.ack, t8.appData, t8.appLen);
    assert(rst == 1);  // t8 store success
    assertUIntergerEqual(virtualSegmentsPtr->next->next->next->next->next->seq_h,600004); // check it is t8
    assertUIntergerEqual(virtualSegmentsPtr->next->next->next->next->next->dataOffset, 15); // t8 data store offset 15

    rst = tryStoreAndAssembleNewTcpData(t5.seq_h, t5.ack, t5.appData, t5.appLen);
    assert(rst == 1);  // t5 store success
    assertUIntergerEqual(virtualSegmentsPtr->next->next->next->next->seq_h,40001); // check it is t5
    assertUIntergerEqual(virtualSegmentsPtr->next->next->next->next->dataOffset, 9); // t8 data store offset 9

    checkLinkedListConnectivity(); // vir->t1->t2->t4->t5->t6->t8
    assertBytesEqual(segmentsBuffer+1, 20, "11122" "\x00\x00" "455" "66" "\x00\x00" "8888" "\x00\x00", 20); // stored t1->t2->t4->t5->t6->t8

    rst = tryStoreAndAssembleNewTcpData(t7.seq_h, t7.ack, t7.appData, t7.appLen);
    assert(rst == 1);  // t7 store success
    assertUIntergerEqual(virtualSegmentsPtr->next->next->next->next->next->next->seq_h,600002); // check it is t7
    assertUIntergerEqual(virtualSegmentsPtr->next->next->next->next->next->next->dataOffset, 13); // t7 data store offset 13

    rst = tryStoreAndAssembleNewTcpData(t3.seq_h, t3.ack, t3.appData, t3.appLen);
    assert(rst == 1);  // t3 store success
    assertUIntergerEqual(virtualSegmentsPtr->next->next->next->seq_h,10005); // check it is t3
    assertUIntergerEqual(virtualSegmentsPtr->next->next->next->dataOffset, 6); // t8 data store offset 6

    checkLinkedListConnectivity(); // vir->t1->t2->t3->t4->t5->t6->t7->t8
    assertBytesEqual(segmentsBuffer+1, 20, "1112233" "455" "66778888" "\x00\x00", 20); // stored t1->t2->t3->t4->t5->t6->t7->t8

    rst = getAssembleTcpData(&tcpData,&dataLen); // assemble t1, t2, t3
    assert(rst == 1);
    assertBytesEqual(tcpData, dataLen, "1112233", 7);
    updateAndDeleteSegmentsStore();
    checkLinkedListConnectivity();

    rst = getAssembleTcpData(&tcpData,&dataLen); // assemble t4, t5
    assert(rst == 1);
    assertBytesEqual(tcpData, dataLen, "455", 3);
    updateAndDeleteSegmentsStore();
    checkLinkedListConnectivity();
    assertBytesEqual(segmentsBuffer+1, 20, "\x00\x00\x00\x00\x00\x00\x00" "\x00\x00\x00" "66778888" "\x00\x00", 20); // stored t1->t2->t3->t4->t5->t6->t7->t8
    assertUIntergerEqual(virtualSegmentsPtr->next->seq_h,600000); // check it is t6
    assertUIntergerEqual(virtualSegmentsPtr->next->dataOffset, 11); // t4 data store offset 11
    assert(virtualSegmentsPtr->next->count == 2); // t6->count == 2
    assert(virtualSegmentsPtr->next->deleteMark == 0); // t6 should not mark
    assert(virtualSegmentsPtr->next->next->next->next == 0); // check t6->t7->t8->next is null

    rst = tryStoreNewTcpData(t4.seq_h, t4.ack, t4.fragLen, t4.appData, t4.appLen);
    assert(rst == 1);  // t4 store success
    assertUIntergerEqual(virtualSegmentsPtr->next->seq_h,40000); // check it is t4
    assertUIntergerEqual(virtualSegmentsPtr->next->dataOffset, 1); // t4 data store offset 8

    rst = tryStoreNewTcpData(t1.seq_h, t1.ack, t1.fragLen, t1.appData, t1.appLen);
    assert(rst == 1);  // t1 store success
    assertUIntergerEqual(virtualSegmentsPtr->next->next->seq_h,10000); // check it is t1
    assertUIntergerEqual(virtualSegmentsPtr->next->next->dataOffset, 4); // t1 data store offset 1
    checkLinkedListConnectivity(); // vir->t4->t1->t6->t7->t8
    assertBytesEqual(segmentsBuffer+1, 20, "4\x00\x00" "111\x00\x00\x00\x00" "66778888" "\x00\x00", 20); // stored t1->t2->t3->t4->t5->t6->t7->t8

    assert(virtualSegmentsPtr->next->count == 0); // t4->count
    assert(virtualSegmentsPtr->next->next->count == 0); // t1->count
    assert(virtualSegmentsPtr->next->next->next->count == 2); // t6->count
    assert(virtualSegmentsPtr->next->next->next->next->count == 0); // t7->count
    assert(virtualSegmentsPtr->next->next->next->next->next->count ==0); // t8->count

    /** Test count to max **/
    updateAndDeleteSegmentsStore();
    updateAndDeleteSegmentsStore();
    updateAndDeleteSegmentsStore();
    updateAndDeleteSegmentsStore();
    updateAndDeleteSegmentsStore();
    updateAndDeleteSegmentsStore();
    updateAndDeleteSegmentsStore();

    assert(virtualSegmentsPtr->next->count == 7); // t4->count
    assert(virtualSegmentsPtr->next->next->count == 7); // t1->count
    assert(virtualSegmentsPtr->next->next->next->count == 9); // t6->count
    assert(virtualSegmentsPtr->next->next->next->next->count == 0); // t7->count
    assert(virtualSegmentsPtr->next->next->next->next->next->count ==0); // t8->count

    updateAndDeleteSegmentsStore(); // t6,t7,t8 will be deleted

    assert(virtualSegmentsPtr->next->count == 8); // t4->count
    assert(virtualSegmentsPtr->next->next->count == 8); // t1->count
    assertUIntergerEqual(virtualSegmentsPtr->next->seq_h,40000); // check it is t4
    assertUIntergerEqual(virtualSegmentsPtr->next->next->seq_h,10000); // check it is t1
    assert(virtualSegmentsPtr->next->next->next == 0); // deleted, NULL
    checkLinkedListConnectivity(); // vir->t4->t1
    assertBytesEqual(segmentsBuffer+1, 20, "4" "\x00\x00" "111" "\x00\x00\x00\x00"  "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 20); // stored t4, t1

    assert(segments_fini());
}


void test_segments_3()
{
    pr_debug("TEST SEGMENTS %d\n",testNum++);
    segments_init();

    int rst = 0;
    const char *tcpData = 0;
    unsigned short dataLen = 0;

    /** Test the buffer space not enough **/
    struct TcpTest t1 = {0xff112233,0xa1a2a3a4,"11",2,10};

    struct TcpTest t2 = {0xff445566,0xb1b2b3b4,"222",3,65515};

    struct TcpTest t3 = {0xff778899,0xc1c2c3c4,"3333",4,11};

    struct TcpTest t4 = {0xffaabbcc,0xd1d2d3d4,"4",1,10};

    rst = tryStoreNewTcpData(t1.seq_h, t1.ack, t1.fragLen, t1.appData, t1.appLen);
    assert(rst == 1);  // t1 store success
    assertUIntergerEqual(virtualSegmentsPtr->next->seq_h,0xff112233); // check it is t1
    assertUIntergerEqual(virtualSegmentsPtr->next->dataOffset, 1); // t1 data store offset 1

    rst = tryStoreNewTcpData(t2.seq_h, t2.ack, t2.fragLen, t2.appData, t2.appLen);
    assert(rst == 1);  // t2 store success
    assertUIntergerEqual(virtualSegmentsPtr->next->next->seq_h,0xff445566); // check it is t2
    assertUIntergerEqual(virtualSegmentsPtr->next->next->dataOffset, 11); // t2 data store offset 11

    rst = tryStoreNewTcpData(t2.seq_h, t2.ack, t2.fragLen, t2.appData, t2.appLen);
    assert(rst == 0);  // t3 store fail

    rst = tryStoreNewTcpData(t4.seq_h, t4.ack, t4.fragLen, t4.appData, t4.appLen);
    assert(rst == 1);  // t4 store success
    assertUIntergerEqual(virtualSegmentsPtr->next->next->next->seq_h,0xffaabbcc); // check it is t4
    assertUIntergerEqual(virtualSegmentsPtr->next->next->next->dataOffset, 65526); // t4 data store offset 65526


    assert(segments_fini());
}
void test_segments()
{
    test_segments_1();
    test_segments_2();
    test_segments_3();
}

