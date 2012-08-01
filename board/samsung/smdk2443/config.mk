#
# (C) Copyright 2006
# Seung-Chull, Suh, Samsung Electronics, <sc.suh@samsung.com>
#
# SAMSUNG SMDK2443 board with S3C2443(ARM920T) cpu
#
# see http://www.samsung.com/ for more information on SAMSUNG
#

#
# SMDK2443 has 1 bank of 64 MB DRAM
# 3000'0000 to 3400'0000 : TEXT_BASE=0x33e00000
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
TEXT_BASE = 0xc3e00000
endif

