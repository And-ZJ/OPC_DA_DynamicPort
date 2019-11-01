#include <string.h>
#include <assert.h>
#include <malloc.h>

#include "conntrak_opc_da/dce_rpc_protocol.h"
#include "conntrak_opc_da/my_pr_debug_control.h"

#include "test_port.h"
#include "BytesTools.h"

#include "AssertTools.h"
#include "imitate.h"

static int testNum = 1;

static unsigned short test_port(const char *ethernetHexStream,
                                 unsigned int ipHeadStart,
                                 unsigned int tcpHeadStart,
                                 unsigned int tcpDataStart)
{
    pr_debug("TEST %d\n",testNum++);
    unsigned int ethernetHexLen = strlen(ethernetHexStream);
    char *ethernetAddr = NULL;
    unsigned int ethernetLen = readableHexStreamToBytes(ethernetHexStream, ethernetHexLen, &ethernetAddr);

    unsigned short opcDaDynamicPort = 0;

    imitate_help(ethernetAddr, ethernetLen, ipHeadStart, tcpHeadStart, tcpDataStart,&opcDaDynamicPort);

    free(ethernetAddr);
    return opcDaDynamicPort;
}


void test()
{
    assert(imitate_init());
    //分片的IsystemActivator包
    const char *ethernetHexStream1_tcp1_segments_1 = "00cfe04a9bfd201a062c86980800450005dc1678400080064923c0100ad7c0100a8900871dc52270f91e14055247501000fb16dd00000500020310000000e805000004000000d005000000000000010000000000000000000200b8050000b80500004d454f5704000000a301000000000000c0000000000000463903000000000000c0000000000000460000000090050000800500000000000001100800cccccccc6000000000000000800500007000000000000000020000000200000000000000000000000000000000000000000002000400020000000000020000003903000000000000c000000000000046b601000000000000c0000000000000460200000020020000f002000001100800cccccccc10020000000000000100000000000200040002000800020001000000506d48132148d211a4943cb306c100000100000000000000010000000c000200d4010000d40100004d454f5701000000506d48132148d211a4943cb306c100000000000005000000fdfa4af7d627ce2eecd1681e99b21a51029c0000001fb01790ffe13a05eae8d0c800b20007005a004a002d0050004300000007003100360039002e003200350034002e00320039002e00310039003100000007003100360039002e003200350034002e00350032002e0035003600000007003100360039002e003200350034002e003200310035002e00310030003400000007003100360039002e003200350034002e003200300034002e00310034003900000007003100360039002e003200350034002e00380030002e00320034003100000007003100360039002e003200350034002e00350037002e00310034003400000007003100360039002e003200350034002e003200310034002e00310037003100000007003100360039002e003200350034002e003200330033002e0039003300000007003100390032002e00310036002e00310030002e003200310035000000070032003000300032003a0063003000310030003a006100640037003a003a0063003000310030003a00610064003700000000000900ffff00001e00ffff00001000ffff00000a00ffff00000e00ffff00001600ffff00001f00ffff0000000001100800cccccccce0020000000000000000000000000200fdfa4af7d627ce2e0400020000f40000001fb0173edf783447aa80a10100000005000700550100005501f40007005a004a002d00500043005b0034003600330030005d00000007003100360039002e003200350034002e00320039002e003100390031005b0034003600330030005d00000007003100360039002e003200350034002e00350032002e00350036005b0034003600330030005d00000007003100360039002e003200350034002e003200310035002e003100300034005b0034003600330030005d00000007003100360039002e003200350034002e003200300034002e003100340039005b0034003600330030005d00000007003100360039002e003200350034002e00380030002e003200340031005b0034003600330030005d00000007003100360039002e003200350034002e00350037002e003100340034005b0034003600330030005d00000007003100360039002e003200350034002e003200310034002e003100370031005b0034003600330030005d00000007003100360039002e003200350034002e003200330033002e00390033005b0034003600330030005d00000007003100390032002e00310036002e00310030002e003200310035005b0034003600330030005d000000070032003000300032003a0063003000310030003a006100640037003a003a0063003000310030003a006100640037005b0034003600330030005d00000000000a00ffff5a004a002d00500043005c00570069006e00690067006800740000001e00ffff5a004a002d00500043005c00570069006e00690067006800740000001000ffff5a004a002d00500043005c00570069006e00690067006800740000000900ffff5a004a002d00500043005c00570069006e00690067006800740000001600ffff5a004a002d00500043005c0057006900";
    unsigned short port1_1_1 = test_port(ethernetHexStream1_tcp1_segments_1,14,34,54);
    assertUIntegerEqual(port1_1_1,0);

//    const char *ethernetHexStream1_tcp1_segments_2 = "00cfe04a9bfd201a062c869808004500005c1679400080064ea2c0100ad7c0100a8900871dc52270fed214055247501800fba03a00006e00690067006800740000001f00ffff5a004a002d00500043005c00570069006e00690067006800740000000000000000000000";
//    unsigned short port1_1_2 = test_port(ethernetHexStream1_tcp1_segments_2,14,34,54);
//    assertUIntegerEqual(port1_1_2,4630);

    //普通bind包
    const char *ethernetHexStream2 = "2c4d54eddd0a1008b1d635d708004500009c7580400040062fefc0100a17c0100ab5d57500873ce006187ccec3bf501801008fc9000005000b03100000007400000002000000d016d016000000000200000000000100c4fefc9960521b10bbcb00aa0021347a00000000045d888aeb1cc9119fe808002b1048600200000001000100c4fefc9960521b10bbcb00aa0021347a000000002c1cb76c12984045030000000000000001000000";
    unsigned short port2 = test_port(ethernetHexStream2,14,34,54);
    assertUIntegerEqual(port2,0);

    //RemQueryInterface response包
    const char *ethernetHexStream3 = "2c4d54eddd0a1008b1d635d70800450000a808e9400080065c7ac0100ab5c0100a17db74d5778bebdc554fd64e5a501801fc056f000005000203100000008000100003000000440000000000000000000000000000000000020001000000024000800000000000000000000000000000000000000000000000000000000000000000000000000000000000000000024000800000000000000000000000000a050c000000000001000000fc0b29ab7264b4f101000000";
    unsigned short port3 = test_port(ethernetHexStream3,14,34,54);
    assertUIntegerEqual(port3,0);

    //普通response包
    const char *ethernetHexStream4 = "2c4d54eddd0a1008b1d635d708004500015808ec400080065bc7c0100ab5c0100a17db74d5778bebdda54fd6506e50180201c441000005000203100000003001100005000000fc00000003000000000000000000000001000000700000000000020000000000d400000000000000d40000004d454f57010000000000000000000000c00000000000004600000000050000007d5ef43cd5caca703cf77c0b20a68b530d800000e8200000ba5ccb425c6f15ad4800320007004400450053004b0054004f0050002d004d0039005000320048004d004400000007003100390032002e00310036002e00310030002e00310038003100000007003100360039002e003200350034002e003100310039002e00310031003300000000000900ffff00001e00ffff00001000ffff00000a00ffff00001600ffff00001f00ffff00000e00ffff000000000d000400000000000a0504000000000001000000a6c802fb40c695f503000000";
    unsigned short port4 = test_port(ethernetHexStream4,14,34,54);
    assertUIntegerEqual(port4,0);

    //普通request包
    const char *ethernetHexStream5 = "2c4d54eddd0a1008b1d635d708002c4d54eddd0a1008b1d635d70800450001287596400040062f4dc0100a17c0100ab5d577db744fd652028bebe025501800fa5730000005000083100000000001100008000000b8000000050003000e940000e8200000acd3f0039ea7a2d4050007000000000000000000a68ece2d14c69c4590799a3b77cd03e6000000000100000000000000010000000000000000000000000000000000020000000000010000000000000000000000000000000000000000000000020055db000000000500000000000000000000000000000005000000000000006c0075006c007500000000008ae3137102f4367102402800543ac1391e01d01196750020afd8adb30000000033057171babe37498319b5dbef9ccc360100000000000000000000000a0508000000000001000000eedcf898bf81f95b06000000";
    unsigned short port5 = test_port(ethernetHexStream5,14,34,54);
    assertUIntegerEqual(port5,0);

    //IsystemActivator Request包
    const char *ethernetHexStream6 = "2c4d54eddd0a1008b1d635d70800450003c07587400040062cc4c0100a17c0100ab5d576008777e62efcf7b1f52d501800ffd1140000050000031000000098030000030000008003000001000400050007000100000000000000a68ece2d14c69c4590799a3b77cd03e600000000000000000000020050030000500300004d454f5704000000a201000000000000c0000000000000463803000000000000c0000000000000460000000028030000180300000000000001100800ccccccccb00000000000000018030000c00000000000000002000000060000000000000000000000000000000000000000000200040002000000000006000000b901000000000000c000000000000046ab01000000000000c000000000000046a501000000000000c000000000000046a601000000000000c000000000000046a401000000000000c000000000000046aa01000000000000c0000000000000460600000068000000b80000009000000058000000200000003000000001100800cccccccc5800000000000000ffffffff00000000000000000000000002000000000000000000000000000000000000000000000010000000020000000000000000000000000000000000000000000000000000000000000000000000000000000000000001100800cccccccca8000000000000003d71a24e07cad411bef500002120db5c100000000000000000000000070000000000000000000200b800000005000700070000004d3ac1391e01d01196750020afd8adb3e2fd1df3b607d211b2d80060083ba1fb84b296b1b4ba1a10b69c00aa00341d07723ac1391e01d01196750020afd8adb34f3ac1391e01d01196750020afd8adb34e3ac1391e01d01196750020afd8adb30b01000000000000c0000000000000460000000001100800cccccccc800000000000000000000000000000000000000000000000000002000000000060000000600000004d454f5704000000c001000000000000c0000000000000463b03000000000000c0000000000000460000000030000000010001007a571ff845ab2840b45ac5cff64af5940200000000000000000000000000000000000000000000000100000001100800cccccccc4800000000000000000000000000020000000000000000000400020000000000000000000e000000000000000e0000003100390032002e00310036002e00310030002e0031003800310000000000000001100800cccccccc10000000000000000000000000000000000000000000000001100800cccccccc20000000000000000000000000000200020000000100000004000200010000000700000000000000";
    unsigned short port6 = test_port(ethernetHexStream6,14,34,54);
    assertUIntegerEqual(port6,0);

    //IsystemActivator Response包1
    const char *ethernetHexStream7 = "2c4d54eddd0a1008b1d635d70800450003e02a17400040067788c0100aa3c0100ab50087ddcbb5821f0245038cf3501807ff66d500000500020310000000b803000003000000a00300000100000001000000000000000000020088030000880300004d454f5704000000a301000000000000c0000000000000463903000000000000c0000000000000460000000060030000500300000000000001100800cccccccc6000000000000000500300007000000000000000020000000200000000000000000000000000000000000000000002000400020000000000020000003903000000000000c000000000000046b601000000000000c0000000000000460200000000010000e001000001100800ccccccccf0000000000000000100000000000200040002000800020001000000506d48132148d211a4943cb306c100000100000000000000010000000c000200b2000000b20000004d454f5701000000506d48132148d211a4943cb306c100000000000005000000c95cff7079832604aaed528f94eb5b2e02b0000038225c20cd7f3690c5d95f973700210007004400450053004b0054004f0050002d004b00420047004a00410049003000000007003100390032002e00310036002e00310030002e00310036003300000000000900ffff00001e00ffff00001000ffff00000a00ffff00001600ffff00001f00ffff00000e00ffff00000000000001100800ccccccccd0010000000000000000000000000200c95cff70798326040400020000f4000038225c2004966f5454e4dcb80100000005000700cc000000cc002f0007004400450053004b0054004f0050002d004b00420047004a004100490030005b00350030003600300030005d00000007003100390032002e00310036002e00310030002e003100360033005b00350030003600300030005d00000000000a00ffff4400450053004b0054004f0050002d004b00420047004a004100490030005c00570069006e00690067006800740000001e00ffff4400450053004b0054004f0050002d004b00420047004a004100490030005c00570069006e00690067006800740000001000ffff4400450053004b0054004f0050002d004b00420047004a004100490030005c00570069006e00690067006800740000000900ffff4400450053004b0054004f0050002d004b00420047004a004100490030005c00570069006e00690067006800740000001600ffff4400450053004b0054004f0050002d004b00420047004a004100490030005c00570069006e00690067006800740000001f00ffff4400450053004b0054004f0050002d004b00420047004a004100490030005c00570069006e006900670068007400000000000000000000000000";
    unsigned short port7 = test_port(ethernetHexStream7,14,34,54);
    assertUIntegerEqual(port7,50600);

    //IsystemActivator Response包2
    const char *ethernetHexStream8 = "2c4d54eddd0a1008b1d635d70800450003e02a28400040067777c0100aa3c0100ab50087ddd27298fa76c12547e8501807ff14b000000500020310000000b803000004000000a00300000000000001000000000000000000020088030000880300004d454f5704000000a301000000000000c0000000000000463903000000000000c0000000000000460000000060030000500300000000000001100800cccccccc6000000000000000500300007000000000000000020000000200000000000000000000000000000000000000000002000400020000000000020000003903000000000000c000000000000046b601000000000000c0000000000000460200000000010000e001000001100800ccccccccf0000000000000000100000000000200040002000800020001000000506d48132148d211a4943cb306c100000100000000000000010000000c000200b2000000b20000004d454f5701000000506d48132148d211a4943cb306c100000000000005000000141b4382cf864e8169025264af0fc01b02940000d00cd40ca3badb72e5ccdaae3700210007004400450053004b0054004f0050002d004b00420047004a00410049003000000007003100390032002e00310036002e00310030002e00310036003300000000000900ffff00001e00ffff00001000ffff00000a00ffff00001600ffff00001f00ffff00000e00ffff00000000000001100800ccccccccd0010000000000000000000000000200141b4382cf864e810400020000540000d00cd40c89734c344b2a787c0100000005000700cc000000cc002f0007004400450053004b0054004f0050002d004b00420047004a004100490030005b00350030003600300031005d00000007003100390032002e00310036002e00310030002e003100360033005b00350030003600300031005d00000000000a00ffff4400450053004b0054004f0050002d004b00420047004a004100490030005c00570069006e00690067006800740000001e00ffff4400450053004b0054004f0050002d004b00420047004a004100490030005c00570069006e00690067006800740000001000ffff4400450053004b0054004f0050002d004b00420047004a004100490030005c00570069006e00690067006800740000000900ffff4400450053004b0054004f0050002d004b00420047004a004100490030005c00570069006e00690067006800740000001600ffff4400450053004b0054004f0050002d004b00420047004a004100490030005c00570069006e00690067006800740000001f00ffff4400450053004b0054004f0050002d004b00420047004a004100490030005c00570069006e006900670068007400000000000000000000000000";
    unsigned short port8 = test_port(ethernetHexStream8,14,34,54);
    assertUIntegerEqual(port8,50601);

    //IsystemActivator Response包3
    const char *ethernetHexStream9 = "2c4d54eddd0a1008b1d635d7080045000430ecc74000800674a1c0100ab5c0100a8900871b32008c53908490074a501810044a84000005000203100000000804000006000000f003000000000000010000000000000000000200d8030000d80300004d454f5704000000a301000000000000c0000000000000463903000000000000c00000000000004600000000b0030000a00300000000000001100800cccccccc6000000000000000a00300007000000000000000020000000200000000000000000000000000000000000000000002000400020000000000020000003903000000000000c000000000000046b601000000000000c00000000000004602000000200100001002000001100800cccccccc10010000000000000100000000000200040002000800020001000000506d48132148d211a4943cb306c100000100000000000000010000000c000200d4000000d40000004d454f5701000000506d48132148d211a4943cb306c100000000000005000000b48039857c064b475330d7f5b845bb30021c0000ec343045b0ff754d44ad168d4800320007004400450053004b0054004f0050002d004d0039005000320048004d004400000007003100390032002e00310036002e00310030002e00310038003100000007003100360039002e003200350034002e003100300039002e00310038003200000000000900ffff00001e00ffff00001000ffff00000a00ffff00001600ffff00001f00ffff00000e00ffff0000000001100800cccccccc00020000000000000000000000000200b48039857c064b470400020000980000ec343045569ee56ae5e4ca3a0100000005000700e4000000e400470007004400450053004b0054004f0050002d004d0039005000320048004d0044005b00350034003800370030005d00000007003100390032002e00310036002e00310030002e003100380031005b00350034003800370030005d00000007003100360039002e003200350034002e003100300039002e003100380032005b00350034003800370030005d00000000000a00ffff4400450053004b0054004f0050002d004d0039005000320048004d0044005c00570069006e00690067006800740000001e00ffff4400450053004b0054004f0050002d004d0039005000320048004d0044005c00570069006e00690067006800740000001000ffff4400450053004b0054004f0050002d004d0039005000320048004d0044005c00570069006e00690067006800740000000900ffff4400450053004b0054004f0050002d004d0039005000320048004d0044005c00570069006e00690067006800740000001600ffff4400450053004b0054004f0050002d004d0039005000320048004d0044005c00570069006e00690067006800740000001f00ffff4400450053004b0054004f0050002d004d0039005000320048004d0044005c00570069006e006900670068007400000000000000000000000000";
    unsigned short port9 = test_port(ethernetHexStream9,14,34,54);
    assertUIntegerEqual(port9,54870);



    //IsystemActivator Response包4
    const char *ethernetHexStream10 = "2c4d54eddd0a1008b1d635d70800450003e078e24000400628bdc0100aa3c0100ab50087f35a79f638bf12fe6d10501807ff299a00000500020310000000b80300000c000000a00300000000000001000000000000000000020088030000880300004d454f5704000000a301000000000000c0000000000000463903000000000000c0000000000000460000000060030000500300000000000001100800cccccccc6000000000000000500300007000000000000000020000000200000000000000000000000000000000000000000002000400020000000000020000003903000000000000c000000000000046b601000000000000c0000000000000460200000000010000e001000001100800ccccccccf0000000000000000100000000000200040002000800020001000000506d48132148d211a4943cb306c100000100000000000000010000000c000200b2000000b20000004d454f5701000000506d48132148d211a4943cb306c100000000000005000000e54136d7908b6fa6a1a82cb35fe28dfd02540000d037543e8637e81faf8ab6113700210007004400450053004b0054004f0050002d004b00420047004a00410049003000000007003100390032002e00310036002e00310030002e00310036003300000000000900ffff00001e00ffff00001000ffff00000a00ffff00001600ffff00001f00ffff00000e00ffff00000000000001100800ccccccccd0010000000000000000000000000200e54136d7908b6fa60400020000700000d037543ee36aad33390446be0100000005000700cc000000cc002f0007004400450053004b0054004f0050002d004b00420047004a004100490030005b00360032003900380036005d00000007003100390032002e00310036002e00310030002e003100360033005b00360032003900380036005d00000000000a00ffff4400450053004b0054004f0050002d004b00420047004a004100490030005c00570069006e00690067006800740000001e00ffff4400450053004b0054004f0050002d004b00420047004a004100490030005c00570069006e00690067006800740000001000ffff4400450053004b0054004f0050002d004b00420047004a004100490030005c00570069006e00690067006800740000000900ffff4400450053004b0054004f0050002d004b00420047004a004100490030005c00570069006e00690067006800740000001600ffff4400450053004b0054004f0050002d004b00420047004a004100490030005c00570069006e00690067006800740000001f00ffff4400450053004b0054004f0050002d004b00420047004a004100490030005c00570069006e006900670068007400000000000000000000000000";
    unsigned short port10 = test_port(ethernetHexStream10,14,34,54);
    assertUIntegerEqual(port10,62986);

    // 第一个包的一部分，但是过去了太久远，第一个包已被丢弃
    const char *ethernetHexStream1_tcp1_segments_2 = "00cfe04a9bfd201a062c869808004500005c1679400080064ea2c0100ad7c0100a8900871dc52270fed214055247501800fba03a00006e00690067006800740000001f00ffff5a004a002d00500043005c00570069006e00690067006800740000000000000000000000";
    unsigned short port1_1_2 = test_port(ethernetHexStream1_tcp1_segments_2,14,34,54);
    assertUIntegerEqual(port1_1_2,0);


    //IsystemActivator Response包5
    const char *ethernetHexStream11 = "2c4d54eddd0a1008b1d635d7080045000400eccd4000800674cbc0100ab5c0100a8900871b32008c579884900a825018100a89f300000500020310000000d803000007000000c003000000000000010000000000000000000200a8030000a80300004d454f5704000000a301000000000000c0000000000000463903000000000000c0000000000000460000000080030000700300000000000001100800cccccccc6000000000000000700300007000000000000000020000000200000000000000000000000000000000000000000002000400020000000000020000003903000000000000c000000000000046b601000000000000c0000000000000460200000020010000e001000001100800cccccccc100100000000000001000000000002000400020008000200010000004d3ac1391e01d01196750020afd8adb30100000000000000010000000c000200d4000000d40000004d454f57010000004d3ac1391e01d01196750020afd8adb30000000005000000129fceace91a47a56bde9602474415ad19a800001c360000886a5142b14bf1e64800320007004400450053004b0054004f0050002d004d0039005000320048004d004400000007003100390032002e00310036002e00310030002e00310038003100000007003100360039002e003200350034002e003100300039002e00310038003200000000000900ffff00001e00ffff00001000ffff00000a00ffff00001600ffff00001f00ffff00000e00ffff0000000001100800ccccccccd0010000000000000000000000000200129fceace91a47a504000200002400001c36000079889830cb3ff19b0200000005000700ce000000ce00470007004400450053004b0054004f0050002d004d0039005000320048004d0044005b00350032003100390039005d00000007003100390032002e00310036002e00310030002e003100380031005b00350032003100390039005d00000007003100360039002e003200350034002e003100300039002e003100380032005b00350032003100390039005d00000000000a00ffff4e005400200041005500540048004f0052004900540059005c00530059005300540045004d0000001e00ffff4e005400200041005500540048004f0052004900540059005c00530059005300540045004d0000001000ffff68006f00730074002f004400450053004b0054004f0050002d004d0039005000320048004d00440000000900ffff68006f00730074002f004400450053004b0054004f0050002d004d0039005000320048004d00440000001600ffff4e005400200041005500540048004f0052004900540059005c00530059005300540045004d0000001f00ffff4e005400200041005500540048004f0052004900540059005c00530059005300540045004d000000000000000000";
    unsigned short port11 = test_port(ethernetHexStream11,14,34,54);
    assertUIntegerEqual(port11,52199);

    const char *ethernetHexStream12_tcp2_segments1 = "2c4d54eddd0a1008b1d635d70800450005dc47c540004006586ac0100a17c0100ab50087e48d1537d73a55853b6b501000fc3b18000005000203100000009808000004000000800800000100000001000000000000000000020068080000680800004d454f5704000000a301000000000000c0000000000000463903000000000000c0000000000000460000000040080000300800000000000001100800cccccccc6000000000000000300800007000000000000000020000000200000000000000000000000000000000000000000002000400020000000000020000003903000000000000c000000000000046b601000000000000c0000000000000460200000010060000b001000001100800cccccccc000600000000000009000000000002000400020008000200090000004d3ac1391e01d01196750020afd8adb3e2fd1df3b607d211b2d80060083ba1fb84b296b1b4ba1a10b69c00aa00341d07723ac1391e01d01196750020afd8adb34f3ac1391e01d01196750020afd8adb34e3ac1391e01d01196750020afd8adb30b01000000000000c000000000000046047022398fa1574b8b0a5235670f446827b4c0859328bc4cbd78e5fc5146f08f09000000000000000000000000000000000000000000000002400080024000800000000000000000090000000c0002001000020014000200180002001c00020000000000000000002000020024000200b0000000b00000004d454f57010000004d3ac1391e01d01196750020afd8adb30000000005000000149b6b1f597e74bba75862e0ef34bf5a098c0000d8320000d9c1062469d8195b3600200007004400450053004b0054004f0050002d004b00420047004a00410049003000000007003100390032002e00310036002e00310030002e0032003300000000000900ffff00001e00ffff00001000ffff00000a00ffff00001600ffff00001f00ffff00000e00ffff00000000b0000000b00000004d454f5701000000e2fd1df3b607d211b2d80060083ba1fb0000000005000000149b6b1f597e74bba75862e0ef34bf5a10740000d83200001314c347fd1b8f083600200007004400450053004b0054004f0050002d004b00420047004a00410049003000000007003100390032002e00310036002e00310030002e0032003300000000000900ffff00001e00ffff00001000ffff00000a00ffff00001600ffff00001f00ffff00000e00ffff00000000b0000000b00000004d454f570100000084b296b1b4ba1a10b69c00aa00341d070000000005000000149b6b1f597e74bba75862e0ef34bf5a11fc0000d8320000265e254518eba4f83600200007004400450053004b0054004f0050002d004b00420047004a00410049003000000007003100390032002e00310036002e00310030002e0032003300000000000900ffff00001e00ffff00001000ffff00000a00ffff00001600ffff00001f00ffff00000e00ffff00000000b0000000b00000004d454f5701000000723ac1391e01d01196750020afd8adb30000000005000000149b6b1f597e74bba75862e0ef34bf5a128c0000d832000084a74fb9b737754f3600200007004400450053004b0054004f0050002d004b00420047004a00410049003000000007003100390032002e00310036002e00310030002e0032003300000000000900ffff00001e00ffff00001000ffff00000a00ffff00001600ffff00001f00ffff00000e00ffff00000000b0000000b00000004d454f57010000004f3ac1391e01d01196750020afd8adb30000000005000000149b6b1f597e74bba75862e0ef34bf5a13880000d8320000c1c2b127c5eef0383600200007004400450053004b0054004f0050002d004b00420047004a00410049003000000007003100390032002e00310036002e00310030002e0032003300000000000900ffff00001e00ffff00001000ffff00000a00ffff00001600ffff00001f00ffff00000e00ffff00000000b0000000b00000004d454f5701000000047022398fa1574b8b0a5235670f44680000000005000000149b6b1f597e74bba75862e0ef34bf5a14d80000d8320000baf12287";

    unsigned short port12_2_1 = test_port(ethernetHexStream12_tcp2_segments1,14,34,54);
    assertUIntegerEqual(port12_2_1,0);

    const char *ethernetHexStream13_tcp2_segments2 = "2c4d54eddd0a1008b1d635d708004500030c47c6400040065b39c0100a17c0100ab50087e48d1537dcee55853b6b501800fc3db300002bac28d33600200007004400450053004b0054004f0050002d004b00420047004a00410049003000000007003100390032002e00310036002e00310030002e0032003300000000000900ffff00001e00ffff00001000ffff00000a00ffff00001600ffff00001f00ffff00000e00ffff00000000b0000000b00000004d454f570100000027b4c0859328bc4cbd78e5fc5146f08f0000000005000000149b6b1f597e74bba75862e0ef34bf5a15580000d83200008313e25996cf1e3a3600200007004400450053004b0054004f0050002d004b00420047004a00410049003000000007003100390032002e00310036002e00310030002e0032003300000000000900ffff00001e00ffff00001000ffff00000a00ffff00001600ffff00001f00ffff00000e00ffff000000000000000001100800cccccccca0010000000000000000000000000200149b6b1f597e74bb04000200006c0000d8320000c9578baf6574b4510200000005000700b5000000b5002e0007004400450053004b0054004f0050002d004b00420047004a004100490030005b00350033003100340036005d00000007003100390032002e00310036002e00310030002e00320033005b00350033003100340036005d00000000000a00ffff4e005400200041005500540048004f0052004900540059005c00530059005300540045004d0000001e00ffff4e005400200041005500540048004f0052004900540059005c00530059005300540045004d0000001000ffff68006f00730074002f004400450053004b0054004f0050002d004b00420047004a0041004900300000000900ffff68006f00730074002f004400450053004b0054004f0050002d004b00420047004a0041004900300000001600ffff4e005400200041005500540048004f0052004900540059005c00530059005300540045004d0000001f00ffff4e005400200041005500540048004f0052004900540059005c00530059005300540045004d0000000000000000000000";

    unsigned short port13_2_1 = test_port(ethernetHexStream13_tcp2_segments2,14,34,54);
    assertUIntegerEqual(port13_2_1,53146);

    // IOXIDResolver Response
    const char *ethernetHexStream14 = "1008b1d635d72c4d54eddd0a080045000230773240008006eca8c0100ab5c0100a170087e7c004576867f328243f501801ffa476000005000203100000000802000011000000f00100000000000000000200e4000000e400470007004400450053004b0054004f0050002d004d0039005000320048004d0044005b00350037003200360035005d00000007003100360039002e003200350034002e003100310039002e003100310033005b00350037003200360035005d00000007003100390032002e00310036002e00310030002e003100380031005b00350037003200360035005d00000000000a00ffff4400450053004b0054004f0050002d004d0039005000320048004d0044005c00570069006e00690067006800740000001e00ffff4400450053004b0054004f0050002d004d0039005000320048004d0044005c00570069006e00690067006800740000001000ffff4400450053004b0054004f0050002d004d0039005000320048004d0044005c00570069006e00690067006800740000000900ffff4400450053004b0054004f0050002d004d0039005000320048004d0044005c00570069006e00690067006800740000001600ffff4400450053004b0054004f0050002d004d0039005000320048004d0044005c00570069006e00690067006800740000001f00ffff4400450053004b0054004f0050002d004d0039005000320048004d0044005c00570069006e00690067006800740000000000009c00005c1a000009cb6b6dc7fa86c5010000000500070000000000";
    unsigned short port14 = test_port(ethernetHexStream14,14,34,54);
    assertUIntegerEqual(port14,57265);

    const char *ethernetHexStream15 = "2c4d54eddd0a1008b1d635d786dd600841a801ec0680fe80000000000000a5ec52dfb230d54bfe80000000000000bd3049f950947bbc0087d575332ce5cfc1da653150180101c83e00000500020310000000d801000046010000c00100000000000000000200cb000000cb002e0007004400450053004b0054004f0050002d004b00420047004a004100490030005b00350034003500360039005d00000007003100390032002e00310036002e00310030002e00320033005b00350034003500360039005d00000000000a00ffff4400450053004b0054004f0050002d004b00420047004a004100490030005c00570069006e00690067006800740000001e00ffff4400450053004b0054004f0050002d004b00420047004a004100490030005c00570069006e00690067006800740000001000ffff4400450053004b0054004f0050002d004b00420047004a004100490030005c00570069006e00690067006800740000000900ffff4400450053004b0054004f0050002d004b00420047004a004100490030005c00570069006e00690067006800740000001600ffff4400450053004b0054004f0050002d004b00420047004a004100490030005c00570069006e00690067006800740000001f00ffff4400450053004b0054004f0050002d004b00420047004a004100490030005c00570069006e00690067006800740000000000000000fc000000620000d6d02cae16c31c0b010000000500070000000000";
    unsigned short port15 = test_port(ethernetHexStream15,14,54,74);
    assertUIntegerEqual(port15,54569);

    assert(imitate_fini());
}
