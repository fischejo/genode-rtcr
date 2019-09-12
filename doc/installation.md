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

## Downloads

### Genode
```bash
git clone origin/focnados-1608_red-mem
git checkout origin/focnados-1608_red-mem
```

### Genode Repositories
```bash
git clone git@gitlab.lrz.de:rtcr_workspace/rtcr.git genode/repos/rtcr
git clone git@gitlab.lrz.de:rtcr_workspace/profiler.git genode/repos/profiler
git clone git@gitlab.lrz.de:rtcr_workspace/genode-world.git genode/repos/world
```

## Prepare Ports
```bash
./genode/tool/ports/prepare_port focnados libc stdcxx zlib libprotobuf
```

## Create Build Directory
Choose one of the platforms. Following build instructions target `focnados_pbxa9`.

```bash
genode/tool/create_builddir focnados_pbxa9 BUILD_DIR=./build_pbxa9
```

## Add Repositories to build.conf

```bash
cat <<'EOF' >> ./build_pbxa9/etc/build.conf
REPOSITORIES += $(GENODE_DIR)/repos/libports
REPOSITORIES += $(GENODE_DIR)/repos/world
REPOSITORIES += $(GENODE_DIR)/repos/profiler
REPOSITORIES += $(GENODE_DIR)/repos/rtcr
EOF
```

## Compile
```bash
make -C build_pbxa9/ run/rtcr_singlecore
```

