#
# (C) Copyright 2002
# Gary Jennejohn, DENX Software Engineering, <gj@denx.de>
# David Mueller, ELSOFT AG, <d.mueller@elsoft.ch>
#
# SAMSUNG SMDK6400 board with mDirac3 (ARM1176) cpu
#
# see http://www.samsung.com/ for more information on SAMSUNG
#

#
# SMDK6400 has a 64 MB SDR SDRAM
#
# 3000'0000 to 3400'0000
#
#
# Linux-Kernel is expected to be at 3000'8000, entry 3000'8000
# optionally with a ramdisk at 3080'0000
#
# we load ourself to 33e0'0000 without MMU
# with MMU, load address is changed to 0xc3e0_0000
#
# download area is 3000'0000
#


ifndef TEXT_BASE
TEXT_BASE = 0x33e00000
endif

