#!/usr/bin/env bash
# this script aims at parsing the Linux Kernel Library (LKL) header to autmatically 
# generate a structure with pointers towards lkl functions.
# todo for fun do it in haskell ?
# might need to export TMPDIR before to work around a ctags bug
# ctags -x --c-kinds=fp tools/lkl/include/lkl.h
# int lkl_add_gateway(int af, void *gwaddr);
# int lkl_add_neighbor(int ifindex, int af, void* addr, void* mac);
export TMPDIR=/tmp
# LKLDIR="$HOME/lkl"
HEADER="$HOME/lkl/tools/lkl/include/lkl.h"

# lkl_sys_lseek
# lkl_netdev_dpdk_create
# be careful ctags can cut the 
# see http://docs.ctags.io/en/latest/news.html#customizing-xref-output
# for the format
# ctags -x --c-kinds=fp "$HEADER"|tr -s '[:blank:]'|cut -d ' ' -f 5-| tee $TMPDIR/ctags.log

# la on veut la valeur de retour
# %{C.properties}
# if I usee xargs -d then I don't need to escape arguments
ctags -x --c-kinds=fp  --_xformat="%N:%{signature}" "$HEADER" > name_ret_params.csv

# xargs -d':' -a name_ret_params.csv
# todo name them 
rm imports.c imports.h
while IFS=':' read name signature
do
    ./gen_struct.sh "$name" "$signature"

    echo "dce_${name}=$name;" >> imports.c
    echo "decltype(${name}) (*dce_${name})$signature;"  >> imports.h
done < <(head name_ret_params.csv)


#|xargs -d':' -n 2 ./gen_struct.sh

# head name_ret_params.csv|cut -d ':'
# -p for print
# cat out.temp | perl -pe "s/(?'ret'.*)lkl_(?'name'\w*)\((?'args'.*)\)/${ret} dce_$+{name}($+{args})/"
# error on lkl_mount_dev
# cat out.temp | perl -pe "s/(?'ret'.*)lkl_(?'name'\w*)\((?'args'.*)\)/$+{ret}:$+{name}:$+{args}/g" > lkl.csv

# https://regex101.com/r/AXEXtm/1


# |xargs toto.sh

