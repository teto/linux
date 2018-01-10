#!/nix/store/sxwh6a4v8xdlvg4jf3vl2jh9l5760hsc-bash-4.4-p12/bin/bash

set -ex

export LKL_TEST_DHCP=1

make mrproper
cd tools/lkl
make -j4
export PATH=$PATH:/sbin
sudo killall netserver || true
make test
make clean
CROSS_COMPILE=i686-w64-mingw32- make -j4
CROSS_COMPILE=i686-w64-mingw32- make test
cd ../..
./tools/lkl/scripts/checkpatch.sh
