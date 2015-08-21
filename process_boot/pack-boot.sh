#!/bin/sh
if [ -d ramdisk ]; then
    ./mkbootfs ramdisk | gzip > ramdisk_new.img
    ret=0
    ./mkimage -A arm -O linux -T ramdisk -C none -a 0x2800000 -n "Root Filesystem" -d ./ramdisk_new.img ./ramdisk_new_uboot.img &&ret=1
    if [ $ret -ne 1 ]; then
        echo "pack ramdisk_new_uboot.img fail"
    else
        ./mkbootimg --kernel kernel --ramdisk ramdisk_new_uboot.img --output boot_new.img
    fi
else
    echo "no ramdisk folder"
fi
