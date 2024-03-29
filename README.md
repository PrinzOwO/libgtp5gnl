# libgtp5gnl - netlink library for Linux kernel module 5G GTP-U

In order to control the kernel-side GTP-U plane, a netlink based control
interface between GTP-C in userspace and GTP-U in kernelspace was invented.

The encoding and decoding of these control messages is implemented in
the libgtp5gnl (library for GTP netlink).

libgtp5gnl is a project based on [libgtpnl](https://github.com/osmocom/libgtpnl)
which is part of the [Osmocom](https://osmocom.org/) Open Source Mobile
Communications project.

## Note:1
The latest implementation of gtp5g and libgtp5gnl is in

https://github.com/free5gc/gtp5g

https://github.com/free5gc/libgtp5gnl


## Usage
### Compile
```
autoreconf -iv
./configure --prefix=`pwd`
make
```

### Command
Sample command is written under `script/` and `tools/gtp5g-tunnel.c`.

### Show Current Rules
```
sudo ./tools/gtp5g-tunnel list pdr
sudo ./tools/gtp5g-tunnel list far
```

### Test
Simple Test (RAN + UPF)
```
./run.sh SimpleUPTest
```

ULCL Test (RAN + I-UPF + A-UPF)
```
./run.sh ULCLTest1
```

### Change Log
#### v0.2.0
+ Changed to host type between gtp5g and libgtp5gnl
+ Supported forwarding policy feature, but need to set `ip rule` and `ip route` with Linux mark by self
+ Fixed bugs about "any" and "assigned" in SDF filter
+ Fixed bugs about netmask translation between gtp5g and libgtp5gnl in SDF filter

#### v0.1.0
+ First version
+ Supported packet matching about UE IP, TEID and SDF filter
