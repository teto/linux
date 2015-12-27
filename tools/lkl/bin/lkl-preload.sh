#!/bin/bash

##
## This wrapper script replaces system calls symbols such as
## socket(2), recvmsg(2) for the redirection to LKL. Ideally it works
## with any applications, but in practice it depends on the maturity
## of preload library (liblkl-preload.so).
##

script_dir=$(cd $(dirname ${BASH_SOURCE:-$0}); pwd)

LD_PRELOAD=${script_dir}/../lib/liblkl_preload.so $*
