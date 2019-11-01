#include "imitate.h"

#include "conntrak_opc_da/dce_rpc_protocol.h"
#include "conntrak_opc_da/segments.h"

#define NF_ACCEPT 1
#define NF_DROP 0

#define MAX_PORTS 8
static unsigned short ports[MAX_PORTS];
static unsigned int ports_c;

unsigned char isMatchDefaultPort(unsigned short port)
{
    int i=0;
    for (;i<ports_c;++i)
    {
        if (ports[i] == port)
        {
            return 1;
        }
    }
    return 0;
}

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

    unsigned short srcPort_h = ntohs( *(const unsigned short *)(ethernetAddr+tcpHeadStart+0));
    unsigned short dstPort_h = ntohs( *(const unsigned short *)(ethernetAddr+tcpHeadStart+2));

    const char *fb_ptr = ethernetAddr + tcpDataStart;
    unsigned int datalen = ethernetLen - tcpDataStart;

//    printf("(%d)->(%d)\n",srcPort_h,dstPort_h);

    updateAndDeleteStore();

    found = 0;
    if (isMatchDefaultPort(srcPort_h))
    {
        found = tryDceRpcProtocolAndMatchDynamicPort(seq_h, ack, fb_ptr, datalen, 0, &matchoff, &matchlen, &opcDaDynamicPort);
    }

    deleteAllMarkedStore();

//    displayBytesInHexChars(fb_ptr+matchoff, matchlen);
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
     if (ports_c == 0)
        ports[ports_c++] = DCE_RPC_PORT;
    return segments_init();
}

int imitate_fini()
{
    return segments_fini();
}
