# Installation

## OS Requirements
Install packages in Ubuntu 16.04
```bash
sudo apt-get install -qq libncurses5-dev texinfo autogen autoconf2.64 g++ libexpat1-dev \
		     flex bison gperf cmake libxml2-dev libtool zlib1g-dev libglib2.0-dev \
		     make pkg-config gawk subversion expect git libxml2-utils syslinux \
		     xsltproc yasm iasl lynx unzip qemu tftpd-hpa isc-dhcp-server
```

## Genode Toolchain
Install genode toolchain in `/usr/local/genode-gcc`
```bash
sudo su
cd /
wget -qO- https://nextcloud.os.in.tum.de/s/9idiw8BLbuwp35z/download | tar xj -C .
```

## Genode Repositories

### genode
```bash
git clone https://github.com/argos-research/genode.git
cd genode
git checkout focnados-1604
cd ..
```

### rtcr
```bash
git clone https://gitlab.lrz.de/rtcr_workspace/rtcr.git
```

## Prepare Ports
```bash
./genode/tool/ports/prepare_port focnados
./genode/tool/ports/prepare_port libc
./genode/tool/ports/prepare_port dde_linux
```


# Build

## Create Build Directory
Create build directory for target pbxa9 (Qemu)

```bash
genode/tool/create_builddir focnados_pbxa9 BUILD_DIR=./build_pbxa9
```

Create build directory for target Zybo board

```bash
genode/tool/create_builddir focnados_zybo BUILD_DIR=./build_zybo
```

## Configuration

Register repositories in Genode by adding following lines to the
`/etc/build.conf` in the build directory (`build_zybo` or `build_pbxa9`):

```bash
REPOSITORIES += $(GENODE_DIR)/repos/libports
REPOSITORIES += $(GENODE_DIR)/repos/world
REPOSITORIES += $(GENODE_DIR)/repos/genode-world
REPOSITORIES += $(GENODE_DIR)/repos/rtcr
```

## Compile
Build target pbxa9
```bash
make -C build_zybo/ run/rtcr
```

Build target Zybo board
```bash
make -C build_zybo/ run/rtcr
```
