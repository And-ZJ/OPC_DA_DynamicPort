/* OPC extension for connection tracking. */

/* (C) 1999-2001 Paul `Rusty' Russell
 * (C) 2002-2004 Netfilter Core Team <coreteam@netfilter.org>
 * (C) 2003,2004 USAGI/WIDE Project <http://www.linux-ipv6.org>
 * (C) 2006-2012 Patrick McHardy <kaber@trash.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

 /**
 * Modified on nf_conntrack_ftp files.
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/netfilter.h>
#include <linux/ip.h>
#include <linux/slab.h>
#include <linux/ipv6.h>
#include <linux/ctype.h>
#include <linux/inet.h>
#include <net/checksum.h>
#include <net/tcp.h>

#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_expect.h>
#include <net/netfilter/nf_conntrack_ecache.h>
#include <net/netfilter/nf_conntrack_helper.h>
#include "nf_conntrack_opc.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("And_ZJ <ZJ.Cosmos@gmail.com>");
MODULE_DESCRIPTION("opc connection tracking helper");
MODULE_ALIAS("ip_conntrack_opc");
MODULE_ALIAS_NFCT_HELPER("opc");

#include "my_pr_debug_control.h"
#include "dce_rpc_protocol.h"

/* This is slow, but it's simple. --RR */
static char *opc_buffer;

static DEFINE_SPINLOCK(nf_opc_lock);

#define MAX_PORTS 8
static u_int16_t ports[MAX_PORTS];
static unsigned int ports_c;
module_param_array(ports, ushort, &ports_c, 0400);

static bool loose;
module_param(loose, bool, 0600);

unsigned int (*nf_nat_opc_hook)(struct sk_buff *skb,
                                enum ip_conntrack_info ctinfo,
                                enum nf_ct_opc_type type,
                                unsigned int protoff,
                                unsigned int matchoff,
                                unsigned int matchlen,
                                struct nf_conntrack_expect *exp);
EXPORT_SYMBOL_GPL(nf_nat_opc_hook);

/* Look up to see if we're just after a \n. */
static int find_nl_seq(u32 seq, const struct nf_ct_opc_master *info, int dir)
{
    unsigned int i;

    for (i = 0; i < info->seq_aft_nl_num[dir]; i++)
        if (info->seq_aft_nl[dir][i] == seq)
            return 1;
    return 0;
}

/* We don't update if it's older than what we have. */
static void update_nl_seq(struct nf_conn *ct, u32 nl_seq,
                          struct nf_ct_opc_master *info, int dir,
                          struct sk_buff *skb)
{
    unsigned int i, oldest;

    /* Look for oldest: if we find exact match, we're done. */
    for (i = 0; i < info->seq_aft_nl_num[dir]; i++)
    {
        if (info->seq_aft_nl[dir][i] == nl_seq)
            return;
    }

    if (info->seq_aft_nl_num[dir] < NUM_SEQ_TO_REMEMBER)
    {
        info->seq_aft_nl[dir][info->seq_aft_nl_num[dir]++] = nl_seq;
    }
    else
    {
        if (before(info->seq_aft_nl[dir][0], info->seq_aft_nl[dir][1]))
            oldest = 0;
        else
            oldest = 1;

        if (after(nl_seq, info->seq_aft_nl[dir][oldest]))
            info->seq_aft_nl[dir][oldest] = nl_seq;
    }
}


void printConnInfo(
    const char *hint,
    unsigned short l3num, // IP or IPV6
    const union nf_inet_addr *src_u3, // src ip addr
    unsigned short src_port,
    const union nf_inet_addr *dst_u3, // dst ip addr
    unsigned short dst_port
)
{
    src_port = ntohs(src_port);
    dst_port = ntohs(dst_port);
    if (l3num == AF_INET)
    {
        pr_debug("%s(IPv4),Src(%pI4,%u),Dst(%pI4,%u)\n",
                 hint,
                 &(src_u3->ip),
                 src_port,
                 &(dst_u3->ip),
                 dst_port
                );
    }
    else if (l3num == AF_INET6)
    {
        pr_debug("%s(IPv6),Src(%pI6,%u),Dst(%pI6,%u)\n",
                 hint,
                 src_u3->all,
                 src_port,
                 dst_u3->all,
                 dst_port
                );
    }
    else
    {
        pr_debug("%s(Unknown)!\n",hint);
    }
}

static int help(struct sk_buff *skb,
                unsigned int protoff,
                struct nf_conn *ct,
                enum ip_conntrack_info ctinfo)
{
    unsigned int dataoff, datalen;
    const struct tcphdr *th;
    struct tcphdr _tcph;
    const char *fb_ptr;
    int ret;
    u32 seq;
    int dir = CTINFO2DIR(ctinfo);
    unsigned int uninitialized_var(matchlen), uninitialized_var(matchoff);
    struct nf_ct_opc_master *ct_opc_info = nfct_help_data(ct);
    struct nf_conntrack_expect *exp;
    union nf_inet_addr *daddr;
    struct nf_conntrack_man cmd = {};

    int found = 0, ends_in_nl;

    unsigned short opcDaDynamicPort = 0;

    typeof(nf_nat_opc_hook) nf_nat_opc;


//    pr_debug("## start ##\n");

    /* Until there's been traffic both ways, don't look in packets. */
    if (ctinfo != IP_CT_ESTABLISHED &&
            ctinfo != IP_CT_ESTABLISHED_REPLY)
    {
        pr_debug("opc: Conntrackinfo = %u\n", ctinfo);
        return NF_ACCEPT;
    }

    th = skb_header_pointer(skb, protoff, sizeof(_tcph), &_tcph);
    if (th == NULL)
        return NF_ACCEPT;

    // offset to tcp data addr
    dataoff = protoff + th->doff * 4;
    /* No data? */
    if (dataoff >= skb->len)
    {
        pr_debug("opc: dataoff(%u) >= skblen(%u)\n", dataoff,
                 skb->len);
        return NF_ACCEPT;
    }
    // tcp data length
    datalen = skb->len - dataoff;

    spin_lock_bh(&nf_opc_lock);

    // tcp data pointer
    fb_ptr = skb_header_pointer(skb, dataoff, datalen, opc_buffer);
    BUG_ON(fb_ptr == NULL);

    ends_in_nl = (fb_ptr[datalen - 1] == '\n'); // ftp
    seq = ntohl(th->seq) + datalen;

    /* Look up to see if we're just after a \n. */
    if (!find_nl_seq(ntohl(th->seq), ct_opc_info, dir))
    {
        /* We're picking up this, clear flags and let it continue */
        if (unlikely(ct_opc_info->flags[dir] & NF_CT_OPC_SEQ_PICKUP))
        {
            ct_opc_info->flags[dir] ^= NF_CT_OPC_SEQ_PICKUP;
            goto skip_nl_seq;
        }

        /* Now if this ends in \n, update opc info. */
        pr_debug("nf_conntrack_opc: wrong seq pos %s(%u) or %s(%u)\n",
                 ct_opc_info->seq_aft_nl_num[dir] > 0 ? "" : "(UNSET)",
                 ct_opc_info->seq_aft_nl[dir][0],
                 ct_opc_info->seq_aft_nl_num[dir] > 1 ? "" : "(UNSET)",
                 ct_opc_info->seq_aft_nl[dir][1]);
        ret = NF_ACCEPT;
        goto out_update_nl;
    }

skip_nl_seq:
    /* Initialize IP/IPv6 addr to expected address (it's not mentioned
       in EPSV responses) */
    cmd.l3num = nf_ct_l3num(ct);
    memcpy(cmd.u3.all, &ct->tuplehash[dir].tuple.src.u3.all,
           sizeof(cmd.u3.all));

    printConnInfo(
        "Conn",
        cmd.l3num, // IP or IPV6
        &ct->tuplehash[dir].tuple.src.u3, // src ip addr
        th->source,
        &ct->tuplehash[dir].tuple.dst.u3, // dst ip addr
        th->dest
    );

    // add
    found = tryDceRpcProtocolAndMatchDynamicPort(fb_ptr, datalen, 0, &matchoff, &matchlen, &opcDaDynamicPort);

    if (found == 1)
    {
        cmd.u.tcp.port = htons(opcDaDynamicPort);
        memcpy(cmd.u3.all, &ct->tuplehash[dir].tuple.src.u3.all,sizeof(cmd.u3.all));

        ret = NF_ACCEPT;

    }
    else if (found == 0)     /* No match */
    {
        ret = NF_ACCEPT;
        goto out_update_nl;
    }


//    pr_debug("conntrack_opc: match `%.*s' (%u bytes at %u)\n",
//             matchlen, fb_ptr + matchoff,
//             matchlen, ntohl(th->seq) + matchoff);

    exp = nf_ct_expect_alloc(ct);
    if (exp == NULL)
    {
        nf_ct_helper_log(skb, ct, "cannot alloc expectation");
        ret = NF_DROP;
        goto out;
    }

    /* We refer to the reverse direction ("!dir") tuples here,
     * because we're expecting something in the other direction.
     * Doesn't matter unless NAT is happening.  */
    daddr = &ct->tuplehash[!dir].tuple.dst.u3;

    /* Update the opc info */
    if ((cmd.l3num == nf_ct_l3num(ct)) &&
            memcmp(&cmd.u3.all, &ct->tuplehash[dir].tuple.src.u3.all,
                   sizeof(cmd.u3.all)))
    {
        /* Enrico Scholz's passive OPC to partially RNAT'd opc
           server: it really wants us to connect to a
           different IP address.  Simply don't record it for
           NAT. */
        if (cmd.l3num == PF_INET)
        {
            pr_debug("NOT RECORDING: %pI4 != %pI4\n",
                     &cmd.u3.ip,
                     &ct->tuplehash[dir].tuple.src.u3.ip);
        }
        else
        {
            pr_debug("NOT RECORDING: %pI6 != %pI6\n",
                     cmd.u3.ip6,
                     ct->tuplehash[dir].tuple.src.u3.ip6);
        }

        /* Thanks to Cristiano Lincoln Mattos
           <lincoln@cesar.org.br> for reporting this potential
           problem (DMZ machines opening holes to internal
           networks, or the packet filter itself). */
        if (!loose)
        {
            ret = NF_ACCEPT;
            goto out_put_expect;
        }
        daddr = &cmd.u3;
    }

    nf_ct_expect_init(exp, NF_CT_EXPECT_CLASS_DEFAULT, cmd.l3num,
                      &ct->tuplehash[!dir].tuple.src.u3, daddr,
                      IPPROTO_TCP, NULL, &cmd.u.tcp.port);

    printConnInfo(
        "Match",
        cmd.l3num, // IP or IPV6
        &ct->tuplehash[!dir].tuple.src.u3, // src ip addr
        0,
        daddr, // dst ip addr
        cmd.u.tcp.port
    );



    /* Now, NAT might want to mangle the packet, and register the
     * (possibly changed) expectation itself. */
    nf_nat_opc = rcu_dereference(nf_nat_opc_hook);
    if (nf_nat_opc && ct->status & IPS_NAT_MASK)
    {
//        ret = nf_nat_opc(skb, ctinfo, search[dir][i].opctype,
//                         protoff, matchoff, matchlen, exp);
        ret = NF_ACCEPT;
        printk("Should NAT.\n");
    }

    else
    {
        /* Can't expect this?  Best to drop packet now. */
        if (nf_ct_expect_related(exp) != 0)
        {
            nf_ct_helper_log(skb, ct, "cannot add expectation");
            ret = NF_DROP;
        }
        else
            ret = NF_ACCEPT;
    }

out_put_expect:
    nf_ct_expect_put(exp);

out_update_nl:
    /* Now if this ends in \n, update opc info.  Seq may have been
     * adjusted by NAT code. */
//    if (ends_in_nl)
    update_nl_seq(ct, seq, ct_opc_info, dir, skb);
out:
    spin_unlock_bh(&nf_opc_lock);
    return ret;
}

static int nf_ct_opc_from_nlattr(struct nlattr *attr, struct nf_conn *ct)
{
    struct nf_ct_opc_master *opc = nfct_help_data(ct);

    /* This conntrack has been injected from user-space, always pick up
     * sequence tracking. Otherwise, the first OPC command after the
     * failover breaks.
     */
    opc->flags[IP_CT_DIR_ORIGINAL] |= NF_CT_OPC_SEQ_PICKUP;
    opc->flags[IP_CT_DIR_REPLY] |= NF_CT_OPC_SEQ_PICKUP;
    return 0;
}

static struct nf_conntrack_helper opc[MAX_PORTS * 2] __read_mostly;

static const struct nf_conntrack_expect_policy opc_exp_policy =
{
    .max_expected	= 1,
    .timeout	= 5 * 60,
};

static void __exit nf_conntrack_opc_fini(void)
{
    printk("unregister nf_conntrack_opc");
    nf_conntrack_helpers_unregister(opc, ports_c * 2);
    kfree(opc_buffer);
}

static int __init nf_conntrack_opc_init(void)
{
    int i, ret = 0;

    NF_CT_HELPER_BUILD_BUG_ON(sizeof(struct nf_ct_opc_master));

    opc_buffer = kmalloc(65536, GFP_KERNEL);
    if (!opc_buffer)
        return -ENOMEM;

    if (ports_c == 0)
        ports[ports_c++] = OPC_PORT;

    /* FIXME should be configurable whether IPv4 and IPv6 OPC connections
    	 are tracked or not - YK */
    for (i = 0; i < ports_c; i++)
    {
        nf_ct_helper_init(&opc[2 * i], AF_INET, IPPROTO_TCP, "opc",
                          OPC_PORT, ports[i], ports[i], &opc_exp_policy,
                          0, help, nf_ct_opc_from_nlattr, THIS_MODULE);
        nf_ct_helper_init(&opc[2 * i + 1], AF_INET6, IPPROTO_TCP, "opc",
                          OPC_PORT, ports[i], ports[i], &opc_exp_policy,
                          0, help, nf_ct_opc_from_nlattr, THIS_MODULE);
    }

    ret = nf_conntrack_helpers_register(opc, ports_c * 2);
    if (ret < 0)
    {
        printk("failed to register nf_conntrack_opc\n");
        kfree(opc_buffer);
        return ret;
    }
    printk("register nf_conntrack_opc");
    return 0;
}

module_init(nf_conntrack_opc_init);
module_exit(nf_conntrack_opc_fini);
