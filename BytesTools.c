#include "BytesTools.h"

void displayBytesInHexChars(const void *bytes,unsigned int length)
{
    const unsigned char *b = (const unsigned char *)bytes;
    for (unsigned int i=0; i<length; ++i)
    {
        printf("%02x ",b[i]&0xFF);
    }
    printf("\n");
}

int isEqualBytes(const void *bytes1,unsigned int bytesLen1,const void *bytes2,unsigned int bytesLen2)
{
    if (bytesLen1 != bytesLen2)
    {
        return 0;
    }
    const unsigned char *b1 = (const unsigned char*)bytes1;
    const unsigned char *b2 = (const unsigned char*)bytes2;
    if ( b1 == b2 ){
        return 1;
    }
    if (b1 == NULL || b2 == NULL){
        return 0;
    }
    for (unsigned int i=0; i<bytesLen1; ++i)
    {
        if ((b1[i]&0xFF) != (b2[i]&0xFF))
        {
            return 0;
        }
    }
    return 1;
}

// 可读十六进制单个字符转10进制数字
// "a" --> 10
unsigned char readableOneHexCharToUChar(const char hexChar)
{
    if (hexChar >= '0' && hexChar <= '9')
    {
        return hexChar - '0';
    }
    if (hexChar >= 'a' && hexChar <= 'f')
    {
        return hexChar - 'a' + 10;
    }
    if (hexChar >= 'A' && hexChar <= 'F')
    {
        return hexChar - 'A' + 10;
    }
    return 0;
}
unsigned char readableHexCharToUChar(const char *hexCharPtr)
{
    return readableOneHexCharToUChar(*hexCharPtr);
}

// 可读十六进制两个字符转10进制数字
// "a1" --> 161
unsigned char readableTwoHexCharsToUChar(const char highByteChar,const char lowByteChar)
{
    return (readableOneHexCharToUChar(highByteChar) << 4) | readableOneHexCharToUChar(lowByteChar);
}
unsigned char readableHexCharsToUChar(const char *hexCharsHeadPtr)
{
    return (readableHexCharToUChar(hexCharsHeadPtr) << 4) | readableHexCharToUChar(hexCharsHeadPtr+1);
}


// 可读十六进制 4个字符 转 unsigned short
// hexCharsHeadPtr 采用 大端模式，下同
unsigned short readableFourHexCharsToUShort(const char bits13_16Char,const char bits9_12Char, const char bits5_8Char, const char bits1_4Char)
{
    return ((unsigned short)readableTwoHexCharsToUChar(bits13_16Char, bits9_12Char) << 8) | readableTwoHexCharsToUChar(bits5_8Char, bits1_4Char);
}
unsigned short readableHexCharsToUShort(const char *hexCharsHeadPtr)
{
    return ((unsigned short)readableHexCharsToUChar(hexCharsHeadPtr) << 8) | readableHexCharsToUChar(hexCharsHeadPtr+2);
}



// 可读十六进制 8个字符 转 unsigned int
unsigned int readableHexCharsToUInt(const char *hexCharsHeadPtr)
{
    return ((unsigned int)readableHexCharsToUShort(hexCharsHeadPtr) << 16) | readableHexCharsToUShort(hexCharsHeadPtr+4);
}


// 可读十六进制字符流 转 10进制 字节数组
// "05000203" --> "\x05\x00\x02\x03"
unsigned int readableHexStreamToBytes(const char *stream, unsigned int streamLen, char **hexBytes)
{
    assert(streamLen % 2 == 0);
    unsigned int bytesLen = streamLen / 2;
    char *bytes = (char *) malloc(streamLen);
    for (unsigned int i=0; i<bytesLen; ++i)
    {
        bytes[i] = readableTwoHexCharsToUChar(stream[2*i],stream[2*i+1]);
    }
    *hexBytes = bytes;
    return bytesLen;
}

// 两个十进制 字节 组成 unsigned short 数字
unsigned short twoBytesToUShort(const char highByte,const char lowByte)
{
    return ((unsigned short)(highByte&0xff) << 8) | (lowByte&0xff);
}
unsigned short bytesToUShort(const char *bytesHeadPtr)
{
    return twoBytesToUShort(*bytesHeadPtr,*(bytesHeadPtr+1));
}

// 四个十进制 字节 组成 unsigned int 数字
unsigned int fourBytesToUInt(const char bits25_32Char,const char bits17_24Char, const char bits9_16Char, const char bits1_8Char)
{
    return ((unsigned int)twoBytesToUShort(bits25_32Char,bits17_24Char) << 16) | twoBytesToUShort(bits9_16Char,bits1_8Char);
}
unsigned int bytesToUInt(const char *bytesHeadPtr)
{
    return ((unsigned int)bytesToUShort(bytesHeadPtr) << 16) | bytesToUShort(bytesHeadPtr+2);
}


// 取出 unsigned short 的两个字节，高位在前
char *uShortToTwoBytes(unsigned short num)
{
    char *bytes = (char *)malloc(sizeof(char)*3);
    bytes[0] = ((num >> 8) & 0xFF);
    bytes[1] = ((num     ) & 0xFF);
    bytes[2] = '\0';
    return bytes;
}

// 取出 unsigned int 的四个字节，高位在前
char *uIntToFourBytes(unsigned int num)
{
    char *bytes = (char *)malloc(sizeof(char)*5);
    bytes[0] = ((num >> 24) & 0xFF);
    bytes[1] = ((num >> 16) & 0xFF);
    bytes[2] = ((num >> 8 ) & 0xFF);
    bytes[3] = ( num        & 0xFF);
    bytes[4] = '\0';
    return bytes;
}

// 拷贝字节
void copyBytes(void *dst, const void *src, int len)
{
    assert(len >= 0);
    memcpy(dst,src,len);
}

unsigned char uCharHigh4BitsToUChar(const unsigned char c)
{
    return (c >> 4) & 0x0F;
}

unsigned char bytesHigh4BitsToUChar(const void *bytes)
{
    return uCharHigh4BitsToUChar(* (const unsigned char *)bytes);
}

unsigned char uCharLow4BitsToUChar(const unsigned char c)
{
    return c & 0x0F;
}

unsigned char bytesLow4BitsToUChar(const void *bytes)
{
    return uCharLow4BitsToUChar(* (const unsigned char *)bytes);
}

unsigned char uCharNthBitToUChar(const unsigned char c,unsigned char n)
{
    return (c >> (n-1)) & 0x01;
}

unsigned char bytesNthBitToUChar(const void *bytes,unsigned char n)
{
    return uCharNthBitToUChar(* (const unsigned char *)bytes, n);
}


unsigned char uShortHigh8BitsToUChar(const unsigned short num)
{
    return (num >> 8) & 0xFF;
}

unsigned char uShortLow8BitsToUChar(const unsigned short num)
{
    return (num) & 0xFF;
}

unsigned short uIntHigh16BitsToUShort(const unsigned int num)
{
    return (num >> 16) & 0xFFFF;
}

unsigned short uIntLow16BitsToUShort(const unsigned int num)
{
    return (num) & 0xFFFF;
}

unsigned char uInt25_32BitsToUChar(const unsigned int num)
{
    return (num >> 24) & 0xFF;
}

unsigned char uInt17_24BitsToUChar(const unsigned int num)
{
    return (num >> 16) & 0xFF;
}

unsigned char uInt9_16BitsToUChar(const unsigned int num)
{
    return (num >> 8) & 0xFF;
}

unsigned char uInt1_8BitsToUChar(const unsigned int num)
{
    return (num) & 0xFF;
}



