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


unsigned char tryDceRpcProtocolAndMatchDynamicPort(unsigned int seq_h, unsigned int ack, const char *dataAddr,unsigned int dataLen, unsigned int start,unsigned int *matchOffPtr,unsigned int *matchLenPtr, unsigned short *dynamicPortPtr)
;


#endif // _DCE_RPC_PROTOCOL_H
