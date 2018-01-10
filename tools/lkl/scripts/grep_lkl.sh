#!/usr/bin/env bash
# this script aims at parsing the Linux Kernel Library (LKL) header to autmatically
# generate a structure with pointers towards lkl functions.
# todo for fun do it in haskell ?
#
# LKLDIR="$HOME/lkl"
LKL_ROOT=.

# lkl_sys_lseek
# lkl_netdev_dpdk_create
# be careful ctags can cut the
# see http://docs.ctags.io/en/latest/news.html#customizing-xref-output
# for the format
# ctags -x --c-kinds=fp "$HEADER"|tr -s '[:blank:]'|cut -d ' ' -f 5-| tee $TMPDIR/ctags.log

cd "$LKL_ROOT/tools/lkl"

# la on veut la valeur de retour
# %{C.properties}
# if I usee xargs -d then I don't need to escape arguments
# typeref is not fixed yet
ctags -x --c-kinds=fp  --_xformat="%N:%{typeref}:%{signature}" include/lkl.h > name_ret_params.csv

# xargs -d':' -a name_ret_params.csv
# todo use tempdir ?
rm -f include/lkl_exports.generated.h
rm -f lib/exports.generated.c
# rm exports.generated.[hc]
while IFS=':' read name rtype signature
do
    # ./gen_struct.sh "$name" "$signature"
	# I badly need the 
    echo "exported->dce_${name}=$name;" >> lib/exports.generated.c
    echo "${rtype} (*dce_${name})$signature;"  >> include/lkl_exports.generated.h
done < <(head name_ret_params.csv)

echo "head $PWD/lib/exports.generated.c"
echo "head $PWD/include/lkl_exports.generated.h"

#|xargs -d':' -n 2 ./gen_struct.sh

# head name_ret_params.csv|cut -d ':'
# -p for print
# cat out.temp | perl -pe "s/(?'ret'.*)lkl_(?'name'\w*)\((?'args'.*)\)/${ret} dce_$+{name}($+{args})/"
# error on lkl_mount_dev
# cat out.temp | perl -pe "s/(?'ret'.*)lkl_(?'name'\w*)\((?'args'.*)\)/$+{ret}:$+{name}:$+{args}/g" > lkl.csv

# https://regex101.com/r/AXEXtm/1


# |xargs toto.sh

