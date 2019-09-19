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
wget -qO- https://nextcloud.os.in.tum.de/s/oHeGQp3rVk5MPpQ/download | tar x -J .
```

## Downloads

### Genode
```bash
git clone -b origin/focnados-18.02_r78 https://github.com/malsami/genode.git
```

### Genode Repositories
```bash
git clone -b 18.02-v1.0 git@gitlab.lrz.de:rtcr_workspace/rtcr.git genode/repos/rtcr
```

## Prepare Ports
```bash
./genode/tool/ports/prepare_port focnados
```

## Create Build Directory
Choose one of the platforms. Following build instructions target `focnados_pbxa9`.

```bash
genode/tool/create_builddir focnados_pbxa9 BUILD_DIR=./build_pbxa9
```

## Add Repositories to build.conf

```bash
cat <<'EOF' >> ./build_pbxa9/etc/build.conf
REPOSITORIES += $(GENODE_DIR)/repos/rtcr
EOF
```

## Compile
```bash
make -C build_pbxa9/ run/rtcr_singlecore
```

