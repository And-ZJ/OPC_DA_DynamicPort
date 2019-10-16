/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */
#ifndef _UAPI_NF_CONNTRACK_OPC_H
#define _UAPI_NF_CONNTRACK_OPC_H
/* OPC tracking. */

/* This enum is exposed to userspace */
enum nf_ct_opc_type {
	/* PORT command from client */
	NF_CT_OPC_PORT,
	/* PASV response from server */
	NF_CT_OPC_PASV,
	/* EPRT command from client */
	NF_CT_OPC_EPRT,
	/* EPSV response from server */
	NF_CT_OPC_EPSV,
};


#endif /* _UAPI_NF_CONNTRACK_OPC_H */
