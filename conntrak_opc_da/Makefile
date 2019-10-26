
obj-m += my_nf_conntrack_opc.o
my_nf_conntrack_opc-objs = nf_conntrack_opc.o dce_rpc_protocol.o segments.o

# obj-m += my_nf_nat_ftp.o
# my_nf_nat_ftp-objs = nf_nat_ftp.o

KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all:
#	$(MAKE) -C $(KDIR) M=$(PWD) modules
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	rm -rf *.ko *.o *.mod.c .*.cmd *.markers *.order *.symvers .tmp_versions
