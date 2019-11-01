#ifndef _DCE_RPC_PROTOCOL_H
#define _DCE_RPC_PROTOCOL_H

/**
* Some simple implement of DCE RPC.
* Some simple skills to get the dynamic port in opc_da protocol. May be not work in everytime.
* Not consider more stable implement.
*/

#define UUID_LEN 16
#define DCERPC_LETTLE_ENDIAN 16
#define DCERPC_BIG_ENDIAN 0

#define DCE_RPC_PORT 135

#define UUID_UNKNOWN_TYPE  0
#define UUID_TYPE_IActivationPropertiesIn   1
#define UUID_TYPE_IActivationPropertiesOut  2


struct DceRpcHeader
{
    unsigned char version;
    unsigned char minor_version;
    unsigned char type;
    unsigned char flags;
    unsigned char drep[4];
    unsigned short frag_length;
    unsigned short auth_length;
    unsigned int id;
};

#ifdef __linux__
#include <linux/ip.h>
//#include <linux/ctype.h>
#else
unsigned short ntohs(unsigned short num);
unsigned int ntohl(unsigned int num);
#endif // __linux__


int identityDceRpcHead(const char *dataAddr,unsigned int dataLen,unsigned int start, unsigned int *offset,  struct DceRpcHeader **dceHdr)
;

// find uuid in dce rpc packet
unsigned char searchDceRpcUuid(const char *dataAddr,unsigned int dataLen,unsigned int start,unsigned int *offset, unsigned int *uuidOff)
;
unsigned int identifyUuidType(const char *dataAddr,unsigned int dataLen,unsigned int uuidOffset,const struct DceRpcHeader *dceHeader)
;


// 返回0，表示匹配失败，未找到可匹配的动态端口。matchOff等参数存0即可。
// 返回1，表示找到匹配位置，matchOff为偏移，相对dataAddr的偏移，matchLen为长度。
unsigned char searchOpcDaDynamicPort(const char *dataAddr,unsigned int dataLen,unsigned int start, unsigned int *matchOff,unsigned int *matchLen)
;

// 返回0表示出错，否则返回解析到的端口值。
// matchOff为偏移，相对dataAddr的偏移，matchLen为长度。
unsigned short parseOpcDaDynamicPort(const char *dataAddr,unsigned int dataLen,unsigned int matchOff, unsigned long matchLen)
;

void printUuidType(int uuidType)
;

// return 0 for match failed.
// return 1 for match success, and got dynamic port.
unsigned char tryMatchDynamicPort(const char *dataAddr,unsigned int dataLen, unsigned int start,unsigned int *matchOffPtr,unsigned int *matchLenPtr, unsigned short *dynamicPortPtr)
;

unsigned char tryDceRpcProtocolAndMatchDynamicPort(unsigned int seq_h, unsigned int ack, const char *dataAddr,unsigned int dataLen, unsigned int start,unsigned int *matchOffPtr,unsigned int *matchLenPtr, unsigned short *dynamicPortPtr)
;



#endif // _DCE_RPC_PROTOCOL_H
