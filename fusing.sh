#!/bin/bash

# ----------------------------------------------------------
# Checking device for fusing

if [ -z $1 ]; then
	echo "Usage: ./hc_fusing.sh <SD Reader's device file>"
	exit 0
fi

case $1 in
/dev/sd[bcd] | /dev/loop0)
	DEV_NAME=`basename $1`
	BLOCK_CNT=`cat /sys/block/${DEV_NAME}/size`;;
*)
	echo "Error: Unsupported SD reader"
	exit 0
esac

if [ ${BLOCK_CNT} -le 0 ]; then
	echo "Error: $1 is inaccessible. Stop fusing now!"
	exit 1
fi

if [ ${BLOCK_CNT} -gt 16000000 ]; then
	echo "Error: $1 size (${BLOCK_CNT}) is too large"
	exit 1
fi

if [ "sd$2" = "sdsd" ]; then
	echo "Card type: SD"
	RSD_BLKCOUNT=0
else
	echo "Card type: SDHC (default)"
	RSD_BLKCOUNT=1024
fi

let BL1_POSITION=${BLOCK_CNT}-${RSD_BLKCOUNT}-16-2
let BL2_POSITION=${BL1_POSITION}-32-512
#echo ${KERNEL_POSITION}


# ----------------------------------------------------------
# Create a u-boot binary for movinand/mmc boot

UBOOT_BIN=u-boot.bin

# padding to 256k u-boot
cat ${UBOOT_BIN} >> u-boot-2x.bin
cat ${UBOOT_BIN} >> u-boot-2x.bin
split -d -a 1 -b 256k u-boot-2x.bin u-boot-256k.bin

# make BL1 u-boot (8kb)
split -d -a 2 -b 8k ${UBOOT_BIN} u-boot-8k.bin

# concat the BL1 behind of padded 256k binary
cat u-boot-8k.bin00 >> u-boot-256k.bin0

# rename and chmod
mv u-boot-256k.bin0 u-boot-movi.bin
chmod 777 u-boot-movi.bin

# remove temp files
rm -f u-boot-8k*
rm -f u-boot-256k*
rm -f u-boot-2x.bin


# ----------------------------------------------------------
# Fusing uboot, kernel to SDHC card

# umount all at first
umount /dev/${DEV_NAME}* > /dev/null 2>&1

echo "---------------------------------"
echo "BL2 fusing"
dd if=u-boot-movi.bin of=/dev/${DEV_NAME} bs=512 seek=${BL2_POSITION} count=512
rm -f u-boot-movi.bin

echo "---------------------------------"
echo "BL1 fusing"
dd if=${UBOOT_BIN}    of=/dev/${DEV_NAME} bs=512 seek=${BL1_POSITION} count=16

sync

#<Message Display>
echo "---------------------------------"
echo "U-boot image is fused successfully."
echo "Please detach $1"

