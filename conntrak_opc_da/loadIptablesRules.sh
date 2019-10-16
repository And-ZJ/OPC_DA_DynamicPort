#! /bin/bash

# should run this file with permissions

iptables -F
iptables -P INPUT ACCEPT
iptables -P OUTPUT ACCEPT
iptables -P FORWARD DROP
#iptables -P FORWARD ACCEPT


#iptables -A FORWARD -p tcp -m state --state NEW --dport 21 -j ACCEPT
iptables -A FORWARD -p tcp -m state --state NEW --dport 135 -j ACCEPT
iptables -A FORWARD -p tcp -m conntrack --ctstate ESTABLISHED,RELATED -j ACCEPT
#iptables -A FORWARD -m conntrack --ctstate ESTABLISHED,RELATED -m helper --helper opc -p tcp -j ACCEPT

