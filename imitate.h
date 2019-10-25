#ifndef _IMITATE_H
#define _IMITATE_H

int imitate_help(const char *ethernetAddr,unsigned int ethernetLen,
                 unsigned int ipHeadStart,
                 unsigned int tcpHeadStart,
                 unsigned int tcpDataStart,
                 unsigned short *opcDaDynamicPortAddr);


#endif // _IMITATE_H
