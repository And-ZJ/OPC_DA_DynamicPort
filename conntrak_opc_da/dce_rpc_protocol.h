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

#define DCE_RPC_PORT	135

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

unsigned char isDceRpcProtocol(const char *dataAddr,unsigned int dataLen,unsigned int start, unsigned int *offset, struct DceRpcHeader **dceHdr)
;

unsigned char searchDceRpcUuid(const char *dataAddr,unsigned int dataLen,unsigned int start,unsigned int *offset, unsigned int *uuidOff)
;

unsigned int identifyUuidType(const char *dataAddr,unsigned int dataLen,unsigned int uuidOffset,const struct DceRpcHeader *dceHeader)
;

unsigned char searchOpcDaDynamicPort(const char *dataAddr,unsigned int dataLen,unsigned int start, unsigned int *matchOff,unsigned int *matchLen)
;

unsigned short parseOpcDaDynamicPort(const char *dataAddr,unsigned int dataLen,unsigned int matchOff, unsigned long matchLen)
;

unsigned char tryDceRpcProtocolAndType(const char *dataAddr,unsigned int dataLen, unsigned int start,unsigned int *offset,int *dceRpcType)
;

void printUuidType(int uuidType)
;

unsigned char tryMatchDynamicPort(const char *dataAddr,unsigned int dataLen, unsigned int start,unsigned int *matchOffPtr,unsigned int *matchLenPtr, unsigned short *dynamicPortPtr)
;

unsigned char tryDceRpcProtocolAndMatchDynamicPort(const char *dataAddr,unsigned int dataLen, unsigned int start,unsigned int *matchOffPtr,unsigned int *matchLenPtr, unsigned short *dynamicPortPtr)
;

#endif // _DCE_RPC_PROTOCOL_H
