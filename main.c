#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <malloc.h>
#include "conntrak_opc_da/dce_rpc_protocol.h"
#include "conntrak_opc_da/my_pr_debug_control.h"

void displayBytesInHexChars(const void *bytes,unsigned int length)
{
    const unsigned char *b = (const unsigned char *)bytes;
    for (unsigned int i=0; i<length; ++i)
    {
        printf("%02x ",b[i]&0xFF);
    }
    printf("\n");
}

int isEqualBytes(const char *b1,unsigned int l1, const char *b2, unsigned int l2)
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

void assertUIntergerEqual(unsigned long long i1,unsigned long long i2)
{
    if (i1 != i2)
    {
        printf("Integer 1:%u\n",i1);
        printf("Integer 2:%u\n",i2);
        assert(0);
    }
}


/***************************************************************/




int testNum = 1;

static unsigned short test_port(const char *ipHexStream,unsigned int start)
{
    pr_debug("TEST %d\n",testNum++);
    unsigned long ipHexLen = strlen(ipHexStream);

    char *ipHeadAddr = NULL;
    unsigned long ipLen = readableHexStreamToBytes(ipHexStream, ipHexLen, &ipHeadAddr);

    char *dataAddr = ipHeadAddr + start;
    unsigned long dataLen = ipLen - start;
    start = 0;


    unsigned int matchOff = 0;
    unsigned int matchLen = 0;
    unsigned short dynamicPort = 0;
    unsigned char found = 0;

    found = tryDceRpcProtocolAndMatchDynamicPort(dataAddr,dataLen,start,&matchOff,&matchLen,&dynamicPort);

    free(ipHeadAddr);
    return dynamicPort;
}

void test()
{
    //分片的IsystemActivator包 长度会出现错误 因此port应为0
    const char *ipHexStream1 = "450005dc08e340008006574cc0100ab5c0100a170087d576f7b1f52d77e6329450100201f3d1000005000203100000003007000003000000180700000100000001000000000000000000020000070000000700004d454f5704000000a301000000000000c0000000000000463903000000000000c00000000000004600000000d8060000c80600000000000001100800cccccccc6000000000000000c80600007000000000000000020000000200000000000000000000000000000000000000000002000400020000000000020000003903000000000000c000000000000046b601000000000000c00000000000004602000000480400001002000001100800cccccccc380400000000000007000000000002000400020008000200070000004d3ac1391e01d01196750020afd8adb3e2fd1df3b607d211b2d80060083ba1fb84b296b1b4ba1a10b69c00aa00341d07723ac1391e01d01196750020afd8adb34f3ac1391e01d01196750020afd8adb34e3ac1391e01d01196750020afd8adb30b01000000000000c0000000000000460700000000000000540104800000000000000000000000000240008002400080070000000c000200000000001000020014000200180002000000000000000000d4000000d40000004d454f57010000004d3ac1391e01d01196750020afd8adb300000000050000007d5ef43cd5caca7095c4d6f4a9aae32809300000e820000059529a80d0ca18274800320007004400450053004b0054004f0050002d004d0039005000320048004d004400000007003100390032002e00310036002e00310030002e00310038003100000007003100360039002e003200350034002e003100310039002e00310031003300000000000900ffff00001e00ffff00001000ffff00000a00ffff00001600ffff00001f00ffff00000e00ffff00000000d4000000d40000004d454f570100000084b296b1b4ba1a10b69c00aa00341d0700000000050000007d5ef43cd5caca7095c4d6f4a9aae3280a140000e8200000a17b58da563b2b2b4800320007004400450053004b0054004f0050002d004d0039005000320048004d004400000007003100390032002e00310036002e00310030002e00310038003100000007003100360039002e003200350034002e003100310039002e00310031003300000000000900ffff00001e00ffff00001000ffff00000a00ffff00001600ffff00001f00ffff00000e00ffff00000000d4000000d40000004d454f5701000000723ac1391e01d01196750020afd8adb300000000050000007d5ef43cd5caca7095c4d6f4a9aae3280b4c0000e82000009c5eed90834420d34800320007004400450053004b0054004f0050002d004d0039005000320048004d004400000007003100390032002e00310036002e00310030002e00310038003100000007003100360039002e003200350034002e003100310039002e00310031003300000000000900ffff00001e00ffff00001000ffff00000a00ffff00001600ffff00001f00ffff00000e00ffff00000000d4000000d40000004d454f57010000004f3ac1391e01d01196750020afd8adb300000000050000007d5ef43cd5caca7095c4d6f4a9aae3280c7c0000e8200000b019f2d77b00656f4800320007004400450053004b0054004f0050002d004d0039005000320048004d004400000007003100390032002e00310036002e00310030002e00310038003100000007003100360039002e003200350034002e003100310039002e00310031003300000000000900ffff00001e00ffff00001000ffff00000a00ffff00001600ffff00001f00ffff00000e00ffff000000000000000001100800cccccccc000200000000000000000000000002007d5ef43cd5caca700400020000900000e8200000f6dcdd1faf640ba20500000005000700e4000000e400470007004400450053004b0054004f0050002d004d0039005000320048004d0044005b00350036003100380030005d00000007003100390032002e00310036002e00310030002e003100380031005b00350036003100";
    unsigned short port1 = test_port(ipHexStream1,40);
    assertUIntergerEqual(port1,0);

    //普通bind包
    const char *ipHexStream2 = "4500009c7580400040062fefc0100a17c0100ab5d57500873ce006187ccec3bf501801008fc9000005000b03100000007400000002000000d016d016000000000200000000000100c4fefc9960521b10bbcb00aa0021347a00000000045d888aeb1cc9119fe808002b1048600200000001000100c4fefc9960521b10bbcb00aa0021347a000000002c1cb76c12984045030000000000000001000000";
    unsigned short port2 = test_port(ipHexStream2,40);
    assertUIntergerEqual(port2,0);

    //RemQueryInterface response包
    const char *ipHexStream3 = "450000a808e9400080065c7ac0100ab5c0100a17db74d5778bebdc554fd64e5a501801fc056f000005000203100000008000100003000000440000000000000000000000000000000000020001000000024000800000000000000000000000000000000000000000000000000000000000000000000000000000000000000000024000800000000000000000000000000a050c000000000001000000fc0b29ab7264b4f101000000";
    unsigned short port3 = test_port(ipHexStream3,40);
    assertUIntergerEqual(port3,0);

    //普通response包
    const char *ipHexStream4 = "4500015808ec400080065bc7c0100ab5c0100a17db74d5778bebdda54fd6506e50180201c441000005000203100000003001100005000000fc00000003000000000000000000000001000000700000000000020000000000d400000000000000d40000004d454f57010000000000000000000000c00000000000004600000000050000007d5ef43cd5caca703cf77c0b20a68b530d800000e8200000ba5ccb425c6f15ad4800320007004400450053004b0054004f0050002d004d0039005000320048004d004400000007003100390032002e00310036002e00310030002e00310038003100000007003100360039002e003200350034002e003100310039002e00310031003300000000000900ffff00001e00ffff00001000ffff00000a00ffff00001600ffff00001f00ffff00000e00ffff000000000d000400000000000a0504000000000001000000a6c802fb40c695f503000000";
    unsigned short port4 = test_port(ipHexStream4,40);
    assertUIntergerEqual(port4,0);

    //普通request包
    const char *ipHexStream5 = "2c4d54eddd0a1008b1d635d70800450001287596400040062f4dc0100a17c0100ab5d577db744fd652028bebe025501800fa5730000005000083100000000001100008000000b8000000050003000e940000e8200000acd3f0039ea7a2d4050007000000000000000000a68ece2d14c69c4590799a3b77cd03e6000000000100000000000000010000000000000000000000000000000000020000000000010000000000000000000000000000000000000000000000020055db000000000500000000000000000000000000000005000000000000006c0075006c007500000000008ae3137102f4367102402800543ac1391e01d01196750020afd8adb30000000033057171babe37498319b5dbef9ccc360100000000000000000000000a0508000000000001000000eedcf898bf81f95b06000000";
    unsigned short port5 = test_port(ipHexStream5,40);
    assertUIntergerEqual(port5,0);

    //IsystemActivator Request包
    const char *ipHexStream6 = "450003c07587400040062cc4c0100a17c0100ab5d576008777e62efcf7b1f52d501800ffd1140000050000031000000098030000030000008003000001000400050007000100000000000000a68ece2d14c69c4590799a3b77cd03e600000000000000000000020050030000500300004d454f5704000000a201000000000000c0000000000000463803000000000000c0000000000000460000000028030000180300000000000001100800ccccccccb00000000000000018030000c00000000000000002000000060000000000000000000000000000000000000000000200040002000000000006000000b901000000000000c000000000000046ab01000000000000c000000000000046a501000000000000c000000000000046a601000000000000c000000000000046a401000000000000c000000000000046aa01000000000000c0000000000000460600000068000000b80000009000000058000000200000003000000001100800cccccccc5800000000000000ffffffff00000000000000000000000002000000000000000000000000000000000000000000000010000000020000000000000000000000000000000000000000000000000000000000000000000000000000000000000001100800cccccccca8000000000000003d71a24e07cad411bef500002120db5c100000000000000000000000070000000000000000000200b800000005000700070000004d3ac1391e01d01196750020afd8adb3e2fd1df3b607d211b2d80060083ba1fb84b296b1b4ba1a10b69c00aa00341d07723ac1391e01d01196750020afd8adb34f3ac1391e01d01196750020afd8adb34e3ac1391e01d01196750020afd8adb30b01000000000000c0000000000000460000000001100800cccccccc800000000000000000000000000000000000000000000000000002000000000060000000600000004d454f5704000000c001000000000000c0000000000000463b03000000000000c0000000000000460000000030000000010001007a571ff845ab2840b45ac5cff64af5940200000000000000000000000000000000000000000000000100000001100800cccccccc4800000000000000000000000000020000000000000000000400020000000000000000000e000000000000000e0000003100390032002e00310036002e00310030002e0031003800310000000000000001100800cccccccc10000000000000000000000000000000000000000000000001100800cccccccc20000000000000000000000000000200020000000100000004000200010000000700000000000000";
    unsigned short port6 = test_port(ipHexStream6,40);
    assertUIntergerEqual(port6,0);

    //IsystemActivator Response包1
    const char *ipHexStream7 = "450003e02a17400040067788c0100aa3c0100ab50087ddcbb5821f0245038cf3501807ff66d500000500020310000000b803000003000000a00300000100000001000000000000000000020088030000880300004d454f5704000000a301000000000000c0000000000000463903000000000000c0000000000000460000000060030000500300000000000001100800cccccccc6000000000000000500300007000000000000000020000000200000000000000000000000000000000000000000002000400020000000000020000003903000000000000c000000000000046b601000000000000c0000000000000460200000000010000e001000001100800ccccccccf0000000000000000100000000000200040002000800020001000000506d48132148d211a4943cb306c100000100000000000000010000000c000200b2000000b20000004d454f5701000000506d48132148d211a4943cb306c100000000000005000000c95cff7079832604aaed528f94eb5b2e02b0000038225c20cd7f3690c5d95f973700210007004400450053004b0054004f0050002d004b00420047004a00410049003000000007003100390032002e00310036002e00310030002e00310036003300000000000900ffff00001e00ffff00001000ffff00000a00ffff00001600ffff00001f00ffff00000e00ffff00000000000001100800ccccccccd0010000000000000000000000000200c95cff70798326040400020000f4000038225c2004966f5454e4dcb80100000005000700cc000000cc002f0007004400450053004b0054004f0050002d004b00420047004a004100490030005b00350030003600300030005d00000007003100390032002e00310036002e00310030002e003100360033005b00350030003600300030005d00000000000a00ffff4400450053004b0054004f0050002d004b00420047004a004100490030005c00570069006e00690067006800740000001e00ffff4400450053004b0054004f0050002d004b00420047004a004100490030005c00570069006e00690067006800740000001000ffff4400450053004b0054004f0050002d004b00420047004a004100490030005c00570069006e00690067006800740000000900ffff4400450053004b0054004f0050002d004b00420047004a004100490030005c00570069006e00690067006800740000001600ffff4400450053004b0054004f0050002d004b00420047004a004100490030005c00570069006e00690067006800740000001f00ffff4400450053004b0054004f0050002d004b00420047004a004100490030005c00570069006e006900670068007400000000000000000000000000";
    unsigned short port7 = test_port(ipHexStream7,40);
    assertUIntergerEqual(port7,50600);

    //IsystemActivator Response包2
    const char *ipHexStream8 = "450003e02a28400040067777c0100aa3c0100ab50087ddd27298fa76c12547e8501807ff14b000000500020310000000b803000004000000a00300000000000001000000000000000000020088030000880300004d454f5704000000a301000000000000c0000000000000463903000000000000c0000000000000460000000060030000500300000000000001100800cccccccc6000000000000000500300007000000000000000020000000200000000000000000000000000000000000000000002000400020000000000020000003903000000000000c000000000000046b601000000000000c0000000000000460200000000010000e001000001100800ccccccccf0000000000000000100000000000200040002000800020001000000506d48132148d211a4943cb306c100000100000000000000010000000c000200b2000000b20000004d454f5701000000506d48132148d211a4943cb306c100000000000005000000141b4382cf864e8169025264af0fc01b02940000d00cd40ca3badb72e5ccdaae3700210007004400450053004b0054004f0050002d004b00420047004a00410049003000000007003100390032002e00310036002e00310030002e00310036003300000000000900ffff00001e00ffff00001000ffff00000a00ffff00001600ffff00001f00ffff00000e00ffff00000000000001100800ccccccccd0010000000000000000000000000200141b4382cf864e810400020000540000d00cd40c89734c344b2a787c0100000005000700cc000000cc002f0007004400450053004b0054004f0050002d004b00420047004a004100490030005b00350030003600300031005d00000007003100390032002e00310036002e00310030002e003100360033005b00350030003600300031005d00000000000a00ffff4400450053004b0054004f0050002d004b00420047004a004100490030005c00570069006e00690067006800740000001e00ffff4400450053004b0054004f0050002d004b00420047004a004100490030005c00570069006e00690067006800740000001000ffff4400450053004b0054004f0050002d004b00420047004a004100490030005c00570069006e00690067006800740000000900ffff4400450053004b0054004f0050002d004b00420047004a004100490030005c00570069006e00690067006800740000001600ffff4400450053004b0054004f0050002d004b00420047004a004100490030005c00570069006e00690067006800740000001f00ffff4400450053004b0054004f0050002d004b00420047004a004100490030005c00570069006e006900670068007400000000000000000000000000";
    unsigned short port8 = test_port(ipHexStream8,40);
    assertUIntergerEqual(port8,50601);

    //IsystemActivator Response包3
    const char *ipHexStream9 = "45000430ecc74000800674a1c0100ab5c0100a8900871b32008c53908490074a501810044a84000005000203100000000804000006000000f003000000000000010000000000000000000200d8030000d80300004d454f5704000000a301000000000000c0000000000000463903000000000000c00000000000004600000000b0030000a00300000000000001100800cccccccc6000000000000000a00300007000000000000000020000000200000000000000000000000000000000000000000002000400020000000000020000003903000000000000c000000000000046b601000000000000c00000000000004602000000200100001002000001100800cccccccc10010000000000000100000000000200040002000800020001000000506d48132148d211a4943cb306c100000100000000000000010000000c000200d4000000d40000004d454f5701000000506d48132148d211a4943cb306c100000000000005000000b48039857c064b475330d7f5b845bb30021c0000ec343045b0ff754d44ad168d4800320007004400450053004b0054004f0050002d004d0039005000320048004d004400000007003100390032002e00310036002e00310030002e00310038003100000007003100360039002e003200350034002e003100300039002e00310038003200000000000900ffff00001e00ffff00001000ffff00000a00ffff00001600ffff00001f00ffff00000e00ffff0000000001100800cccccccc00020000000000000000000000000200b48039857c064b470400020000980000ec343045569ee56ae5e4ca3a0100000005000700e4000000e400470007004400450053004b0054004f0050002d004d0039005000320048004d0044005b00350034003800370030005d00000007003100390032002e00310036002e00310030002e003100380031005b00350034003800370030005d00000007003100360039002e003200350034002e003100300039002e003100380032005b00350034003800370030005d00000000000a00ffff4400450053004b0054004f0050002d004d0039005000320048004d0044005c00570069006e00690067006800740000001e00ffff4400450053004b0054004f0050002d004d0039005000320048004d0044005c00570069006e00690067006800740000001000ffff4400450053004b0054004f0050002d004d0039005000320048004d0044005c00570069006e00690067006800740000000900ffff4400450053004b0054004f0050002d004d0039005000320048004d0044005c00570069006e00690067006800740000001600ffff4400450053004b0054004f0050002d004d0039005000320048004d0044005c00570069006e00690067006800740000001f00ffff4400450053004b0054004f0050002d004d0039005000320048004d0044005c00570069006e006900670068007400000000000000000000000000";
    unsigned short port9 = test_port(ipHexStream9,40);
    assertUIntergerEqual(port9,54870);

    //IsystemActivator Response包4
    const char *ipHexStream10 = "450003e078e24000400628bdc0100aa3c0100ab50087f35a79f638bf12fe6d10501807ff299a00000500020310000000b80300000c000000a00300000000000001000000000000000000020088030000880300004d454f5704000000a301000000000000c0000000000000463903000000000000c0000000000000460000000060030000500300000000000001100800cccccccc6000000000000000500300007000000000000000020000000200000000000000000000000000000000000000000002000400020000000000020000003903000000000000c000000000000046b601000000000000c0000000000000460200000000010000e001000001100800ccccccccf0000000000000000100000000000200040002000800020001000000506d48132148d211a4943cb306c100000100000000000000010000000c000200b2000000b20000004d454f5701000000506d48132148d211a4943cb306c100000000000005000000e54136d7908b6fa6a1a82cb35fe28dfd02540000d037543e8637e81faf8ab6113700210007004400450053004b0054004f0050002d004b00420047004a00410049003000000007003100390032002e00310036002e00310030002e00310036003300000000000900ffff00001e00ffff00001000ffff00000a00ffff00001600ffff00001f00ffff00000e00ffff00000000000001100800ccccccccd0010000000000000000000000000200e54136d7908b6fa60400020000700000d037543ee36aad33390446be0100000005000700cc000000cc002f0007004400450053004b0054004f0050002d004b00420047004a004100490030005b00360032003900380036005d00000007003100390032002e00310036002e00310030002e003100360033005b00360032003900380036005d00000000000a00ffff4400450053004b0054004f0050002d004b00420047004a004100490030005c00570069006e00690067006800740000001e00ffff4400450053004b0054004f0050002d004b00420047004a004100490030005c00570069006e00690067006800740000001000ffff4400450053004b0054004f0050002d004b00420047004a004100490030005c00570069006e00690067006800740000000900ffff4400450053004b0054004f0050002d004b00420047004a004100490030005c00570069006e00690067006800740000001600ffff4400450053004b0054004f0050002d004b00420047004a004100490030005c00570069006e00690067006800740000001f00ffff4400450053004b0054004f0050002d004b00420047004a004100490030005c00570069006e006900670068007400000000000000000000000000";
    unsigned short port10 = test_port(ipHexStream10,40);
    assertUIntergerEqual(port10,62986);

    //IsystemActivator Response包5
    const char *ipHexStream11 = "45000400eccd4000800674cbc0100ab5c0100a8900871b32008c579884900a825018100a89f300000500020310000000d803000007000000c003000000000000010000000000000000000200a8030000a80300004d454f5704000000a301000000000000c0000000000000463903000000000000c0000000000000460000000080030000700300000000000001100800cccccccc6000000000000000700300007000000000000000020000000200000000000000000000000000000000000000000002000400020000000000020000003903000000000000c000000000000046b601000000000000c0000000000000460200000020010000e001000001100800cccccccc100100000000000001000000000002000400020008000200010000004d3ac1391e01d01196750020afd8adb30100000000000000010000000c000200d4000000d40000004d454f57010000004d3ac1391e01d01196750020afd8adb30000000005000000129fceace91a47a56bde9602474415ad19a800001c360000886a5142b14bf1e64800320007004400450053004b0054004f0050002d004d0039005000320048004d004400000007003100390032002e00310036002e00310030002e00310038003100000007003100360039002e003200350034002e003100300039002e00310038003200000000000900ffff00001e00ffff00001000ffff00000a00ffff00001600ffff00001f00ffff00000e00ffff0000000001100800ccccccccd0010000000000000000000000000200129fceace91a47a504000200002400001c36000079889830cb3ff19b0200000005000700ce000000ce00470007004400450053004b0054004f0050002d004d0039005000320048004d0044005b00350032003100390039005d00000007003100390032002e00310036002e00310030002e003100380031005b00350032003100390039005d00000007003100360039002e003200350034002e003100300039002e003100380032005b00350032003100390039005d00000000000a00ffff4e005400200041005500540048004f0052004900540059005c00530059005300540045004d0000001e00ffff4e005400200041005500540048004f0052004900540059005c00530059005300540045004d0000001000ffff68006f00730074002f004400450053004b0054004f0050002d004d0039005000320048004d00440000000900ffff68006f00730074002f004400450053004b0054004f0050002d004d0039005000320048004d00440000001600ffff4e005400200041005500540048004f0052004900540059005c00530059005300540045004d0000001f00ffff4e005400200041005500540048004f0052004900540059005c00530059005300540045004d000000000000000000";
    unsigned short port11 = test_port(ipHexStream11,40);
    assertUIntergerEqual(port11,52199);


}

int main()
{
    test();
    printf("Test Finish!\n");
    return 0;
}
