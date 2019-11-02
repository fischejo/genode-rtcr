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
Download and Install
[genode-toolchain-19.05](https://sourceforge.net/projects/genode/files/genode-toolchain/19.05/).


## Downloads

### Genode
```bash
git clone -b 19.08 git@gitlab.lrz.de:rtcr_workspace/genode-19.08.git genode
```

### Genode Repositories
```bash
git clone -b 19.08 git@gitlab.lrz.de:rtcr_workspace/rtcr.git genode/repos/rtcr
git clone -b 19.08 git@gitlab.lrz.de:rtcr_workspace/genode-world.git genode/repos/world
```

## Prepare Ports
```bash
# required by rtcr
./genode/tool/ports/prepare_port libc stdcxx zlib libprotobuf

# required by focnados kernel
./genode/tool/ports/prepare_port foc

# required by sel4 kernel
./genode/tool/ports/prepare_port sel4 sel4_tools
```

## (Optional) Change UART

If you compile for the Wandboard, the UART will require changes:
```bash
# the Wandboard use another serial console as the sabrelite. This requires changes for foc and sel4:
sed -i 's/CONFIG_PLAT_SABRE/CONFIG_PLAT_WANDQ/g' genode/contrib/sel4-*/src/kernel/sel4/configs/imx6/imx6q_sabrelite/autoconf.h
sed -i 's/PLATFORM_UART_NR.*/PLATFORM_UART_NR = 1/g' genode/contrib/foc-*/src/kernel/foc/l4/mk/platforms/imx6.conf
```


## Create Build Directory

### Choose Platform
Only Wandboard is tested and supported:
```bash
export BUILD_DIR=./build

# for Wandboard, choose The configuration of the sabrelite board
genode/tool/create_builddir imx6q_sabrelite
```

### Choose Kernel
```bash
# for foc kernel:
sed -i '1s/^/KERNEL := foc\n/' $BUILD_DIR/etc/build.conf

# for sel4 kernel:
sed -i '1s/^/KERNEL := sel4\n/' $BUILD_DIR/etc/build.conf
```

## Add Repositories to build.conf

```bash
cat <<'EOF' >> $BUILD_DIR/etc/build.conf
REPOSITORIES += $(GENODE_DIR)/repos/libports
REPOSITORIES += $(GENODE_DIR)/repos/world
REPOSITORIES += $(GENODE_DIR)/repos/rtcr
EOF
```

## Compile
```bash
# some scripts of Genode expect an english environment
export LANG=en_US.UTF-8

make -C $BUILD_DIR run/rtcr_report
```

