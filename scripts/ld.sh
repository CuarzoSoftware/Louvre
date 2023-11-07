#!/bin/bash

# Fix linking problems

set -e
mkdir -p /etc/ld.so.conf.d
echo $1 > /etc/ld.so.conf.d/louvre.conf
ldconfig
exit 0