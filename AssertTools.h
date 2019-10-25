#ifndef _ASSERT_TOOLS_H
#define _ASSERT_TOOLS_H

#include "BytesTools.h"

void assertBytesEqual(const void *bytes1,unsigned int bytesLen1,const void *bytes2,unsigned int bytesLen2)
;

void assertIntegerEqual(long long i1,long long i2)
;

void assertUIntergerEqual(unsigned long long i1,unsigned long long i2)
;

#endif // _ASSERT_TOOLS_H
