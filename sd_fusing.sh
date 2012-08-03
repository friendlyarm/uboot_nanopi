#!/bin/sh

DEV_NAME=sdb
BLOCK_CNT=`cat /sys/block/${DEV_NAME}/size`

let BL1_POSITION=${BLOCK_CNT}-16-2
let BL2_POSITION=${BLOCK_CNT}-16-2-32-512
#echo ${BL1_POSITION}
#echo ${BL2_POSITION}

set -x

./mkmovi

# BL2
#dd if=u-boot-movi.bin of=/dev/sdb bs=512 seek=20
dd if=u-boot-movi.bin of=/dev/${DEV_NAME} bs=512 seek=${BL2_POSITION} count=512

# BL1
dd if=u-boot.bin      of=/dev/${DEV_NAME} bs=512 seek=${BL1_POSITION} count=16

sync

