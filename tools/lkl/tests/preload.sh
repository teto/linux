#!/bin/bash -e

script_dir=$(cd $(dirname ${BASH_SOURCE:-$0}); pwd)
script=lkl-preload.sh
export PATH=${script_dir}/../bin/:${PATH}

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
