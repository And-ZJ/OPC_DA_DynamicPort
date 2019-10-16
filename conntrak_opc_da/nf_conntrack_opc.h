/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _NF_CONNTRACK_OPC_H
#define _NF_CONNTRACK_OPC_H

#include "uapi/nf_conntrack_opc.h"
#include "dce_rpc_protocol.h"

#define OPC_PORT DCE_RPC_PORT

#define NF_CT_OPC_SEQ_PICKUP	(1 << 0)

#define NUM_SEQ_TO_REMEMBER 2
/* This structure exists only once per master */
struct nf_ct_opc_master {
	/* Valid seq positions for cmd matching after newline */
	u_int32_t seq_aft_nl[IP_CT_DIR_MAX][NUM_SEQ_TO_REMEMBER];
	/* 0 means seq_match_aft_nl not set */
	u_int16_t seq_aft_nl_num[IP_CT_DIR_MAX];
	/* pickup sequence tracking, useful for conntrackd */
	u_int16_t flags[IP_CT_DIR_MAX];
};

struct nf_conntrack_expect;

/* For NAT to hook in when we find a packet which describes what other
 * connection we should expect. */
extern unsigned int (*nf_nat_opc_hook)(struct sk_buff *skb,
				       enum ip_conntrack_info ctinfo,
				       enum nf_ct_opc_type type,
				       unsigned int protoff,
				       unsigned int matchoff,
				       unsigned int matchlen,
				       struct nf_conntrack_expect *exp);
#endif /* _NF_CONNTRACK_OPC_H */
