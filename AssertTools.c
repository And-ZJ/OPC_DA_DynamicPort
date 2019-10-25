#include "AssertTools.h"



void assertBytesEqual(const void *bytes1,unsigned int bytesLen1,const void *bytes2,unsigned int bytesLen2)
{
    if (!isEqualBytes(bytes1,bytesLen1,bytes2,bytesLen2))
    {
        printf("Bytes 1: %d\n",bytesLen1);
        displayBytesInHexChars(bytes1,bytesLen1);
        printf("Bytes 2: %d\n",bytesLen2);
        displayBytesInHexChars(bytes2,bytesLen2);
        assert(0);
    }
}

void assertIntegerEqual(long long i1,long long i2)
{
    if (i1 != i2)
    {
        printf("Integer 1:%I64d\n",i1);
        printf("Integer 2:%I64d\n",i2);
        assert(0);
    }
}

void assertUIntergerEqual(unsigned long long i1,unsigned long long i2)
{
    if (i1 != i2)
    {
        printf("Integer 1:0x%x\n",i1);
        printf("Integer 2:0x%x\n",i2);
        assert(0);
    }
}
