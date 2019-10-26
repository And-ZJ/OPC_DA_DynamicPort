#ifndef _ASSERT_TOOLS_H
#define _ASSERT_TOOLS_H

#include "BytesTools.h"

// judge (const char * or const unsigned char *) bytes with bytes length
#define assertBytesEqual( bytes1, bytesLen1, bytes2, bytesLen2) \
{ \
    if (!isEqualBytes( (bytes1) , (bytesLen1) , (bytes2) , (bytesLen2) )) \
    { \
        printf("Bytes 1: %d\n",(bytesLen1)); \
        displayBytesInHexChars((bytes1),(bytesLen1)); \
        printf("Bytes 2: %d\n",(bytesLen2)); \
        displayBytesInHexChars((bytes2),(bytesLen2)); \
        assert(0); \
    } \
}

// judge integer(char, short, int, long, long long)
#define assertIntegerEqual( i1 , i2 ) \
{ \
    long long int1 = (long long) (i1); \
    long long int2 = (long long) (i2); \
    if (int1 != int2) \
    { \
        printf("Integer 1:%I64d\n",int1); \
        printf("Integer 2:%I64d\n",int2); \
        assert(0); \
    } \
}

// judge unsigned integer(char, short, int, long, long long)
#define assertUIntegerEqual( i1 , i2 ) \
{ \
    unsigned long long uint1 = (unsigned long long) (i1); \
    unsigned long long uint2 = (unsigned long long) (i2); \
    if (uint1 != uint2) \
    { \
        printf("UInteger 1:0x%x\n",uint1); \
        printf("UInteger 2:0x%x\n",uint2); \
        assert(0); \
    } \
}

// judge pointer of any type
#define assertPtrEqual(p1, p2) \
{ \
    if ( (const char *) (p1) != (const char *) (p2)) \
    { \
        printf("Ptr 1:0x%x\n", (p1) ); \
        printf("Ptr 2:0x%x\n", (p2) ); \
        assert(0); \
    } \
}

#endif // _ASSERT_TOOLS_H
