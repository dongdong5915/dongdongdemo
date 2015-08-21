#!/bin/sh
rm kernel
rm ramdisk.img
rm -Rf ramdisk
chmod 777 unpackbootimg
ret=0
./unpackbootimg && ret=1
if [ $ret -ne 1 ]; then
    echo "unpack bootimg fail"
else
    mkdir ramdisk
    cd ramdisk
    gzip -dc ../ramdisk.img | cpio -i
fi

