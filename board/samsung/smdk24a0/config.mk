#
# (C) Copyright 2002
# Gary Jennejohn, DENX Software Engineering, <gj@denx.de>
# David Mueller, ELSOFT AG, <d.mueller@elsoft.ch>
#
# SAMSUNG SMDK24A0 board with S3C24A0A (ARM926EJ-S) cpu
#
# see http://www.samsung.com/ for more information on SAMSUNG
#

#
# SMDK24A0 has 1 bank of 64 MB DRAM
#
# 1000'0000 to 1400'0000
#
# Linux-Kernel is expected to be at 1000'8000, entry 1000'8000
# optionally with a ramdisk at 1080'0000
#
# we load ourself to 13e0'0000
#
# download area is 1300'0000
#


TEXT_BASE = 0x13e00000
S3C24X0_NAND = s3c24a0a_nand
S3C24XX_TYPE = s3c24a0
