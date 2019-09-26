# Installation

## OS Requirements
Install packages in Ubuntu 16.04
```bash
sudo apt-get install -qq libncurses5-dev texinfo autogen autoconf2.64 g++ libexpat1-dev \
		     flex bison gperf cmake libxml2-dev libtool zlib1g-dev libglib2.0-dev \
		     make pkg-config gawk subversion expect git libxml2-utils syslinux \
		     xsltproc yasm iasl lynx unzip qemu tftpd-hpa isc-dhcp-server python-pip

# required by sel4 kernel
pip2 install --user future tempita ply six
```

## Genode Toolchain
Install genode toolchain in `/usr/local/genode-gcc`
```bash
sudo su
cd /
wget -qO- https://nextcloud.os.in.tum.de/s/oHeGQp3rVk5MPpQ/download | tar x -J .
```

## Downloads

### Genode
```bash
git clone -b focnados_sel4_18.02_r78 git@gitlab.lrz.de:rtcr_workspace/genode-malsami.git genode
```

### Genode Repositories
```bash
git clone git@gitlab.lrz.de:rtcr_workspace/rtcr.git genode/repos/rtcr
git clone git@gitlab.lrz.de:rtcr_workspace/genode-world.git genode/repos/world
```

## Prepare Ports
```bash
# required by rtcr
./genode/tool/ports/prepare_port libc stdcxx zlib libprotobuf

# required by focnados kernel
./genode/tool/ports/prepare_port focnados

# required by sel4 kernel
./genode/tool/ports/prepare_port sel4 sel4_elfloader
```


## Create Build Directory

### focnados Kernel
```bash
genode/tool/create_builddir focnados_pbxa9 BUILD_DIR=./build
```

### seL4 Kernel
```bash
genode/tool/create_builddir wand_quad BUILD_DIR=./build
sed -i 's/# KERNEL ?= hw/KERNEL ?= sel4/' ./build/etc/build.conf
```


## Add Repositories to build.conf

```bash
cat <<'EOF' >> ./build/etc/build.conf
REPOSITORIES += $(GENODE_DIR)/repos/libports
REPOSITORIES += $(GENODE_DIR)/repos/world
REPOSITORIES += $(GENODE_DIR)/repos/rtcr
EOF
```

## Compile
```bash
make -C build_pbxa9/ run/rtcr_report
```

