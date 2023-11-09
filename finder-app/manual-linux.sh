#!/bin/bash
# Script outline to install and build kernel.
# Author: Siddhant Jajoo.

set -e
set -u

OUTDIR=/tmp/aeld
KERNEL_REPO=git://git.kernel.org/pub/scm/linux/kernel/git/stable/linux-stable.git
KERNEL_VERSION=v5.1.10
BUSYBOX_VERSION=1_33_1
FINDER_APP_DIR=$(realpath $(dirname $0))
ARCH=arm64
CROSS_COMPILE=aarch64-linux-gnu-

if [ $# -lt 1 ]; then
  echo "Using default directory ${OUTDIR} for output"
else
  OUTDIR=$1
  echo "Using passed directory ${OUTDIR} for output"
fi

mkdir -p ${OUTDIR}

cd "$OUTDIR"
if [ ! -d "${OUTDIR}/linux-stable" ]; then
  # Clone only if the repository does not exist.
  echo "CLONING GIT LINUX STABLE VERSION ${KERNEL_VERSION} IN ${OUTDIR}"
  git clone ${KERNEL_REPO} --depth 1 --single-branch --branch ${KERNEL_VERSION}
fi

if [ ! -e ${OUTDIR}/linux-stable/arch/${ARCH}/boot/Image ]; then
  cd linux-stable
  echo "Checking out version ${KERNEL_VERSION}"
  git checkout ${KERNEL_VERSION}

  # Configure the kernel
  make ARCH=arm64 CROSS_COMPILE=${CROSS_COMPILE} defconfig

  # Build the kernel
  make -j4 ARCH=arm64 CROSS_COMPILE=${CROSS_COMPILE}
  
  # Copy the generated kernel Image
  cp arch/arm64/boot/Image ${OUTDIR}

fi

echo "Adding the Image in outdir"

echo "Creating the staging directory for the root filesystem"
cd "$OUTDIR"
if [ -d "${OUTDIR}/rootfs" ]
then
	echo "Deleting rootfs directory at ${OUTDIR}/rootfs and starting over"
    sudo rm  -rf ${OUTDIR}/rootfs
fi

# TODO: Create necessary base directories
# Create necessary base directories
mkdir -p ${OUTDIR}/rootfs/{bin,dev,etc,home,lib,lib64,proc,sys,tmp,usr,var}
mkdir -p ${OUTDIR}/rootfs/home/conf
cd "$OUTDIR"

if [ ! -d "${OUTDIR}/busybox" ]
then
git clone git://busybox.net/busybox.git
    cd busybox
    git checkout ${BUSYBOX_VERSION}
    # TODO:  Configure busybox
    # Configure BusyBox
    make ARCH=arm64 CROSS_COMPILE=${CROSS_COMPILE} defconfig
else
    cd busybox
fi
# TODO: Make and install busybox

# Build BusyBox
make -j4 ARCH=arm64 CROSS_COMPILE=${CROSS_COMPILE}

# Install BusyBox
make ARCH=arm64 CROSS_COMPILE=${CROSS_COMPILE} CONFIG_PREFIX=${OUTDIR}/rootfs install
cd ${OUTDIR}/rootfs

echo "Library dependencies"
${CROSS_COMPILE}readelf -a bin/busybox | grep "program interpreter"
${CROSS_COMPILE}readelf -a bin/busybox | grep "Shared library"

SYSROOT=/home/roberto/Downloads/toolchain/arm-gnu-toolchain-13.2.Rel1-x86_64-aarch64-none-linux-gnu/aarch64-none-linux-gnu/libc
# TODO: Add library dependencies to rootfs
cp -L $SYSROOT/lib64/libm.so.* lib64
cp -L $SYSROOT/lib64/libresolv.so.* lib64
cp -L $SYSROOT/lib64/libc.so.* lib64
cp -L $SYSROOT/lib/ld-linux-aarch64.* lib
# TODO: Make device nodes
# Create device nodes
sudo mknod -m 666 ${OUTDIR}/rootfs/dev/null c 1 3
sudo mknod -m 600 dev/console c 5 1 

# TODO: Clean and build the writer utility
# Compile the writer utility and copy it to "${OUTDIR}/rootfs/home"
cd $FINDER_APP_DIR 
make clean
make CROSS_COMPILE=${CROSS_COMPILE}


# TODO: Copy the finder related scripts and executables to the /home directory
# on the target rootfs
cp $FINDER_APP_DIR/finder-test.sh ${OUTDIR}/rootfs/home
cp $FINDER_APP_DIR/conf/ -r ${OUTDIR}/rootfs/home
cp $FINDER_APP_DIR/Makefile ${OUTDIR}/rootfs/home
cp $FINDER_APP_DIR/writer ${OUTDIR}/rootfs/home
cp $FINDER_APP_DIR/writer ${OUTDIR}/rootfs/usr/bin
cp $FINDER_APP_DIR/autorun-qemu.sh ${OUTDIR}/rootfs/home

# Copy required files to "${OUTDIR}/rootfs/home"
cp $FINDER_APP_DIR/finder.sh ${OUTDIR}/rootfs/home
cp $FINDER_APP_DIR/conf/username.txt ${OUTDIR}/rootfs/home/conf
cp $FINDER_APP_DIR/conf/assignment.txt ${OUTDIR}/rootfs/home/conf

# TODO: Chown the root directory
# Change ownership of "${OUTDIR}/rootfs" to root
cd ${OUTDIR}/rootfs

sudo chown -R root:root ${OUTDIR}/rootfs

# TODO: Create initramfs.cpio.gz
# Create initramfs.cpio.gz
find . | cpio -H newc -ov --owner root:root > ${OUTDIR}/initramfs.cpio
cd ${OUTDIR}

rm -f initramfs.cpio.gz
gzip initramfs.cpio 