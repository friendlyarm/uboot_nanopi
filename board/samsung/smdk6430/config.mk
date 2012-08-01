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
# SMDK6410 has a 128 MB SDR SDRAM
#
# 5000'0000 to 5800'0000
#
#
# Linux-Kernel is expected to be at 5000'8000, entry 5000'8000
# optionally with a ramdisk at 5080'0000
#
# we load ourself to 57e0'0000 without MMU
# with MMU, load address is changed to 0xc7e0_0000
#
# download area is 5000'0000
#


ifndef TEXT_BASE
TEXT_BASE = 0xc7e00000
endif

