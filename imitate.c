#include "imitate.h"

#include "conntrak_opc_da/dce_rpc_protocol.h"
#include "conntrak_opc_da/segments.h"

#define NF_ACCEPT 1
#define NF_DROP 0

int imitate_help(const char *ethernetAddr,unsigned int ethernetLen,
                 unsigned int ipHeadStart,
                 unsigned int tcpHeadStart,
                 unsigned int tcpDataStart,
                 unsigned short *opcDaDynamicPortAddr)
{

    int found = 0;
    unsigned short opcDaDynamicPort =0;
    unsigned int matchlen =0;
    unsigned int matchoff =0;

    unsigned int seq_h = ntohl( *(const unsigned int *)(ethernetAddr+tcpHeadStart+4));
    unsigned int ack = *(const unsigned int *)(ethernetAddr+tcpHeadStart+8);

    const char *fb_ptr = ethernetAddr + tcpDataStart;
    unsigned int datalen = ethernetLen - tcpDataStart;

    found = tryDceRpcProtocolAndMatchDynamicPort(seq_h, ack, fb_ptr, datalen, 0, &matchoff, &matchlen, &opcDaDynamicPort);

//    printf("match(%d,%d)\n",matchoff,matchlen);

    if (found == 1)
    {
        *opcDaDynamicPortAddr = opcDaDynamicPort;
        return NF_ACCEPT;
    }
    else
    {
        *opcDaDynamicPortAddr = 0;
        return NF_ACCEPT;
    }

}

int imitate_init()
{
    return segments_init();
}

int imitate_fini()
{
    return segments_fini();
}
