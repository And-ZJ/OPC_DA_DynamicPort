#! /bin/bash -x

# should run this file with permissions
iptables -D PREROUTING -t raw -p tcp --dport 135 -j CT --helper opc
modprobe nf_conntrack
#rmmod my_nf_nat_opc
rmmod my_nf_conntrack_opc # remove it if existed
insmod my_nf_conntrack_opc.ko # insmod a new module
#insmod my_nf_nat_opc.ko
iptables -A PREROUTING -t raw -p tcp --dport 135 -j CT --helper opc
