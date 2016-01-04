#!/bin/bash -e

script_dir=$(cd $(dirname ${BASH_SOURCE:-$0}); pwd)
script=lkl-preload.sh
export PATH=${script_dir}/../bin/:${PATH}

if ! [ -e ${script_dir}/../lib/liblkl_preload.so ]; then
    exit;
fi

echo "== ip addr test=="
${script} ip addr

echo "== ip route test=="
${script} ip route show table local

echo "== ping test=="
# we don't need root so get rid of the setuid bit
cp `which ping` .
${script} ./ping 127.0.0.1 -c 2 -W 10
rm ping

echo "== ping6 test=="
# we don't need root so get rid of the setuid bit
cp `which ping6` .
${script} ./ping6 ::1 -c 2 -W 10
rm ping6

echo "== TAP tests =="
if [ -c /dev/net/tun ]; then
    sudo ip link set dev lkl_ptt0 down || true
    sudo ip tuntap del dev lkl_ptt0 mode tap || true
    sudo ip tuntap add dev lkl_ptt0 mode tap user $USER
    sudo ip link set dev lkl_ptt0 up
    sudo ip addr add dev lkl_ptt0 192.168.13.1
    LKL_PRELOAD_NET_TAP=lkl_ptt0 LKL_PRELOAD_NET_IP=192.168.13.2 LKL_PRELOAD_NET_NETMASK_LEN=24 ${script} ip link | grep eth0
    LKL_PRELOAD_NET_TAP=lkl_ptt0 LKL_PRELOAD_NET_IP=192.168.13.2 LKL_PRELOAD_NET_NETMASK_LEN=24 ${script} ip addr | grep 192.168.13.2
    cp `which ping` .
    LKL_PRELOAD_NET_TAP=lkl_ptt0 LKL_PRELOAD_NET_IP=192.168.13.2 LKL_PRELOAD_NET_NETMASK_LEN=24 ${script} ./ping 192.168.13.2 -i 0.2 -c 65
    rm ./ping
    sudo ip link set dev lkl_ptt0 down
    sudo ip tuntap del dev lkl_ptt0 mode tap
fi
