#include "dce_rpc_protocol.h"
#include "my_pr_debug_control.h"

const char *IID_IActivationPropertiesIn = "\xa2\x01\x00\x00\x00\x00\x00\x00\xc0\x00\x00\x00\x00\x00\x00\x46";
const char *IID_IActivationPropertiesOut = "\xa3\x01\x00\x00\x00\x00\x00\x00\xc0\x00\x00\x00\x00\x00\x00\x46";

const char *CONST_MEOW = "\x4d\x45\x4f\x57";

#ifdef __linux__
#include <linux/ip.h>
#include <linux/ctype.h>
#else
static unsigned short ntohs(unsigned short num)
{
    union
    {
        unsigned long int i;
        unsigned char s[4];
    } c;
    c.i = 0x12345678;
    //判断本机字节序,本机为大端则直接返回，为小端则将网络的大端转为小端
    if(0x12 == c.s[0])
    {
        return num;
    }
    else
    {
        return ((((num) & 0xff00) >> 8) | (((num) & 0x00ff) << 8));
    }
}
#endif // __linux__


static int isEqualBytes(const char *b1,unsigned int l1, const char *b2, unsigned int l2)
{
    unsigned int i=0;
    if (l1 != l2)
    {
        return 0;
    }
    if (b1 == b2)
    {
        return 1;
    }
    if (b1 == 0 || b2 == 0)
    {
        return 0;
    }
    for (i=0; i < l1; ++i)
    {
        if (b1[i] != b2[i])
        {
            return 0;
        }
    }
    return 1;
}

// 返回0表示不是
// 返回1表示是
// start 是相对 dataAddr 的偏移，可指示本函数 在 此偏移之后做操作，offset 是 搜索结束后的偏移，返回回去可用于指示其他函数在本偏移之后做操作。
unsigned char isDceRpcProtocol(const char *dataAddr,unsigned int dataLen,unsigned int start, unsigned int *offset, struct DceRpcHeader **dceHdr)
{
    unsigned short len;
    struct DceRpcHeader *dceHeader = 0;
    const char* appDataAddr = dataAddr + start;
    const short appDataLen = dataLen - start;

    *dceHdr = 0;
    if (appDataLen < (int)sizeof(struct DceRpcHeader)) return 0;
    dceHeader = (struct DceRpcHeader *)appDataAddr;

    if (dceHeader->version != 5) return 0;
    if (dceHeader->minor_version > 1) return 0;
    if (dceHeader->type > 19) return 0;
    if (dceHeader->drep[0] & DCERPC_LETTLE_ENDIAN)
    {
        len = dceHeader->frag_length;
    }
    else
    {
        len = ntohs(dceHeader->frag_length);
    }
    if (len < (int)sizeof(struct DceRpcHeader)) return 0;
    if (appDataLen < len) return 0;

    *offset = start + (unsigned int)sizeof(struct DceRpcHeader);
    *dceHdr = dceHeader;
    return 1;
}

// find uuid in dce rpc packet
unsigned char searchDceRpcUuid(const char *dataAddr,unsigned int dataLen,unsigned int start,unsigned int *offset, unsigned int *uuidOff)
{
    unsigned int iidStart = 0;
    const unsigned short signatureLength = 4;
    const unsigned short objrefLength = 4;
    unsigned int i = 0;
    unsigned int end = start + 55; // a skill to limited the str "MEOW" search range.

    if (dataLen < end)
    {
        return 0;
    }

    for(i = start; i < end; i++)
    {
        //找到MEOW
        if (isEqualBytes(dataAddr+i,4,CONST_MEOW,4))
        {
            iidStart = i + signatureLength + objrefLength;
            *offset = iidStart + UUID_LEN;
            *uuidOff = iidStart;
            return 1;
        }
    }
    return 0;
}

unsigned int identifyUuidType(const char *dataAddr,unsigned int dataLen,unsigned int uuidOffset,const struct DceRpcHeader *dceHeader)
{
    const char *uuidAddr = dataAddr + uuidOffset;

    if (uuidOffset + UUID_LEN > dataLen)
    {
        return UUID_UNKNOWN_TYPE;
    }

    if (isEqualBytes(uuidAddr,UUID_LEN, IID_IActivationPropertiesOut,UUID_LEN))
    {
        if(dceHeader->type == 2)
        {
            return UUID_TYPE_IActivationPropertiesOut;
        }
    }
    if (isEqualBytes(uuidAddr,UUID_LEN, IID_IActivationPropertiesIn,UUID_LEN))
    {
        if(dceHeader->type == 0)
        {
            return UUID_TYPE_IActivationPropertiesIn;
        }
    }
    return UUID_UNKNOWN_TYPE;
}


// 返回0，表示匹配失败，未找到可匹配的动态端口。matchOff等参数存0即可。
// 返回1，表示找到匹配位置，matchOff为偏移，相对dataAddr的偏移，matchLen为长度。
unsigned char searchOpcDaDynamicPort(const char *dataAddr,unsigned int dataLen,unsigned int start, unsigned int *matchOff,unsigned int *matchLen)
{
    unsigned char inBraceFlag = 0;
    unsigned char isDynamicPort = 0;
    unsigned int braceStart = 0;
    unsigned int braceEnd = 0;
    unsigned int i =0;
    start = 356; // a skill to jump some bytes to search the dynamic port in current packet.

    for(i = start; i < dataLen; i++)
    {
        if(dataAddr[i] != ']' && inBraceFlag ==1)
        {
            if((i - braceStart)%2 == 1 && dataAddr[i] != '\x00')
            {
                isDynamicPort = 0;
                inBraceFlag = 0;
            }
            if((i - braceStart)%2 == 0 && (dataAddr[i] < '0' || dataAddr[i] > '9'))
            {
                isDynamicPort = 0;
                inBraceFlag = 0;
            }
        }
        if(dataAddr[i] == '[' && isDynamicPort == 0 && inBraceFlag == 0)
        {
            inBraceFlag = 1;
            isDynamicPort = 1;
            braceStart = i;
        }
        if(dataAddr[i] == ']' && inBraceFlag == 1)
        {
            inBraceFlag = 0;
            braceEnd = i;
            if(isDynamicPort)
            {
                break;
            }
        }
    }


    if(isDynamicPort == 1 && inBraceFlag == 0) //说明已经找到动态端口
    {
        *matchOff = braceStart;
        *matchLen = braceEnd - braceStart + 1;

        printf("%d\n",*matchLen);
        for (unsigned int i =braceStart; i< braceStart + *matchLen; ++i )
        {
            printf("%x ",dataAddr[i]);
        }
        printf("\n");


    }
    else
    {
        *matchOff = 0;
        *matchLen = 0;
        return 0;
    }
    return 1;
}

// 返回0表示出错，否则返回解析到的端口值。
// matchOff为偏移，相对dataAddr的偏移，matchLen为长度。
unsigned short parseOpcDaDynamicPort(const char *dataAddr,unsigned int dataLen,unsigned int matchOff, unsigned long matchLen)
{
    unsigned short dynamicPort = 0;
    unsigned short temp = 1;
    unsigned int i =0;

    for(i = matchOff + matchLen -2; i > matchOff ; i--)
    {
        if(dataAddr[i] != '\x00')
        {
            dynamicPort = dynamicPort + temp * (dataAddr[i]-48);
            temp = temp * 10;
        }
    }

    return dynamicPort;
}

unsigned char tryDceRpcProtocolAndType(const char *dataAddr,unsigned int dataLen, unsigned int start,unsigned int *offset,int *dceRpcType)
{
    unsigned int dceOffset = 0;
    struct DceRpcHeader *dceRpcHdr = 0;
    unsigned char isDceRpcPacket = 0;
    unsigned char isFindUuid = 0;
    unsigned int uuidOffset = 0;
    unsigned int uuidType = 0;

    isDceRpcPacket = isDceRpcProtocol(dataAddr,dataLen, dceOffset, &dceOffset,&dceRpcHdr);

    if (!isDceRpcPacket)
    {
        // Accept, although it is not dce rpc packet. May be other protocol communication.
        pr_debug("Not DCE RPC\n");
        return 0;
    }

    isFindUuid = searchDceRpcUuid(dataAddr,dataLen,dceOffset,&dceOffset,&uuidOffset);

    if (!isFindUuid)
    {
        pr_debug("Find UUID failed\n");
        return 0;
    }
    uuidType = identifyUuidType(dataAddr,dataLen,uuidOffset,dceRpcHdr);
    if (uuidType == UUID_UNKNOWN_TYPE)
    {
        pr_debug("UUID unknown\n");
        return 0;
    }
    *offset = dceOffset;
    *dceRpcType = uuidType;
    return 1;
}

void printUuidType(int uuidType)
{
    if (uuidType == UUID_UNKNOWN_TYPE)
    {
        pr_debug("Type(Unknown)\n");
    }
    else if (uuidType == UUID_TYPE_IActivationPropertiesOut)
    {
        pr_debug("Type(ISA resp)\n");
    }
    else if (uuidType == UUID_TYPE_IActivationPropertiesIn)
    {
        pr_debug("Type(ISA requ)\n");
    }
    else
    {
        pr_debug("Type(Unknown)\n");
    }
}

// return 0 for match failed.
// return 1 for match success, and got dynamic port.
unsigned char tryMatchDynamicPort(const char *dataAddr,unsigned int dataLen, unsigned int start,unsigned int *matchOffPtr,unsigned int *matchLenPtr, unsigned short *dynamicPortPtr)
{
    unsigned int dceOffset = start;
    unsigned char isMatchDynamicPortSuc = 0;
    unsigned int matchOff = 0;
    unsigned int matchLen = 0;
    unsigned short dynamicPort = 0;


    isMatchDynamicPortSuc = searchOpcDaDynamicPort( dataAddr, dataLen, dceOffset, &matchOff, &matchLen);

    if (!isMatchDynamicPortSuc)
    {
        // Accept, but should judge to why match failed.
        pr_debug("Match dynamic port failed\n");
        return 0;
    }

    // match success
    dynamicPort = parseOpcDaDynamicPort(dataAddr,dataLen, matchOff, matchLen);

    if (dynamicPort == 0)
    {
        // Accept, but should judge to why parse failed.
        pr_debug("Parse dynamic port failed\n");
        return 0;
    }
    *matchOffPtr = matchOff;
    *matchLenPtr = matchLen;
    *dynamicPortPtr = dynamicPort;
    return 1;

}

unsigned char tryDceRpcProtocolAndMatchDynamicPort(const char *dataAddr,unsigned int dataLen, unsigned int start,unsigned int *matchOffPtr,unsigned int *matchLenPtr, unsigned short *dynamicPortPtr)
{
    unsigned int offset = 0;
    unsigned int matchOff = 0;
    unsigned int matchLen = 0;
    unsigned short dynamicPort = 0;
    unsigned char isDcerpcPacket = 0;
    unsigned char isGotDynamicPort = 0;
    int dceRpcType = 0;
    isDcerpcPacket =  tryDceRpcProtocolAndType(dataAddr,dataLen, offset,&offset,&dceRpcType);

    if (isDcerpcPacket)
    {
        printUuidType(dceRpcType);
        if (dceRpcType == UUID_TYPE_IActivationPropertiesOut)
        {
            isGotDynamicPort = tryMatchDynamicPort(dataAddr,dataLen,offset,&matchOff,&matchLen,&dynamicPort);
        }
    }
    if (isGotDynamicPort)
    {
        *matchOffPtr = matchOff;
        *matchLenPtr = matchLen;
        *dynamicPortPtr = dynamicPort;
        return 1;
    }
    return 0;
}
