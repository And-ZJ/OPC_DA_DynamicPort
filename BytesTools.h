#ifndef _BYTES_TOOLS_H
#define _BYTES_TOOLS_H

#include <assert.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>

void displayBytesInHexChars(const void *bytes,unsigned int length)
;

int isEqualBytes(const void *bytes1,unsigned int bytesLen1,const void *bytes2,unsigned int bytesLen2)
;

// 可读十六进制单个字符转10进制数字
// "a" --> 10
unsigned char readableOneHexCharToUChar(const char hexChar)
;
unsigned char readableHexCharToUChar(const char *hexCharPtr)
;

// 可读十六进制两个字符转10进制数字
// "a1" --> 161
unsigned char readableTwoHexCharsToUChar(const char highByteChar,const char lowByteChar)
;
unsigned char readableHexCharsToUChar(const char *hexCharsHeadPtr)
;


// 可读十六进制 4个字符 转 unsigned short
unsigned short readableFourHexCharsToUShort(const char bits13_16Char,const char bits9_12Char, const char bits5_8Char, const char bits1_4Char)
;
unsigned short readableHexCharsToUShort(const char *hexCharsHeadPtr)
;



// 可读十六进制 8个字符 转 unsigned int
unsigned int readableHexCharsToUInt(const char *hexCharsHeadPtr)
;


// 可读十六进制字符流 转 10进制 字节数组
// "05000203" --> "\x05\x00\x02\x03"
unsigned int readableHexStreamToBytes(const char *stream, unsigned int streamLen, char **hexBytes)
;


// 两个十进制 字节 组成 unsigned short 数字
unsigned short twoBytesToUShort(const char highByte,const char lowByte)
;
unsigned short bytesToUShort(const char *bytesHeadPtr)
;

// 四个十进制 字节 组成 unsigned int 数字
unsigned int fourBytesToUInt(const char bits25_32Char,const char bits17_24Char, const char bits9_16Char, const char bits1_8Char);
unsigned int bytesToUInt(const char *bytesHeadPtr)
;



// 取出 unsigned short 的两个字节，高位在前
char *uShortToTwoBytes(unsigned short num)
;

// 取出 unsigned int 的四个字节，高位在前
char *uIntToFourBytes(unsigned int num)
;

// 拷贝字节
void copyBytes(void *dst, const void *src, int len)
;

unsigned char uCharHigh4BitsToUChar(const unsigned char c)
;

unsigned char bytesHigh4BitsToUChar(const void *bytes)
;

unsigned char uCharLow4BitsToUChar(const unsigned char c)
;

unsigned char bytesLow4BitsToUChar(const void *bytes)
;

unsigned char uCharNthBitToUChar(const unsigned char c,unsigned char n)
;

unsigned char bytesNthBitToUChar(const void *bytes,unsigned char n)
;


unsigned char uShortHigh8BitsToUChar(const unsigned short num)
;

unsigned char uShortLow8BitsToUChar(const unsigned short num)
;

unsigned short uIntHigh16BitsToUShort(const unsigned int num)
;
unsigned short uIntLow16BitsToUShort(const unsigned int num)
;

unsigned char uInt25_32BitsToUChar(const unsigned int num)
;

unsigned char uInt17_24BitsToUChar(const unsigned int num)
;

unsigned char uInt9_16BitsToUChar(const unsigned int num)
;

unsigned char uInt1_8BitsToUChar(const unsigned int num)
;

#endif // _BYTES_TOOLS_H
