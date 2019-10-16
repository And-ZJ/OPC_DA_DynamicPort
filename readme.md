# OPC_DA_DynamicPort



OPC DA 协议的动态端口开放简单实现，在 Linux 的 Netfilter 上。

代码参照了 nf_conntrack_ftp 模块

测试系统 Ubuntu 18.04，内核版本：5.0.0-31-generic，iptables 版本：1.6.1

此处为简单实现，没有完整分析 OPC DA 的报文。可看做是对 conntrack 的学习，此代码供交流学习，请勿用于实际工程，由此引发的安全等问题本人不会负责。



内置两个Codeblocks工程。

## OPC_DA_TEST 工程

主要用于测试提取 动态端口 的代码。

## conntrack_opc_da 工程

在Linux下编译出 my_nf_conntrack_opc 模块的工程，编译步骤在自带Makefile中。

模块的导入示例在`loadModules.sh`中，开启动态端口的示例规则在`loadIptablesRules.sh`中。

